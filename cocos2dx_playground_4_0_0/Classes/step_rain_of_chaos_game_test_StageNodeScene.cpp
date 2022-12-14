#include "step_rain_of_chaos_game_test_StageNodeScene.h"

#include <new>
#include <numeric>

#include "2d/CCLabel.h"
#include "2d/CCLayer.h"
#include "2d/CCSprite.h"
#include "base/CCDirector.h"
#include "base/CCEventListenerKeyboard.h"
#include "base/CCEventDispatcher.h"
#include "base/ccUTF8.h"

#include "cpg_SStream.h"
#include "cpg_StringTable.h"

#include "step_mole_CircleCollisionComponentConfig.h"
#include "step_rain_of_chaos_game_EnemyNode.h"
#include "step_rain_of_chaos_game_PlayerNode.h"
#include "step_rain_of_chaos_game_StageNode.h"

USING_NS_CC;

namespace
{
	const int TAG_MoveSpeedView = 20140416;
	const int BulletCachingAmount = 500;
	const int TAG_FireAmountView = 20160528;
}

namespace step_rain_of_chaos
{
	namespace game_test
	{
		StageNodeScene::StageNodeScene( const helper::FuncSceneMover& back_to_the_previous_scene_callback ) :
			helper::BackToThePreviousScene( back_to_the_previous_scene_callback )
			, mKeyboardListener( nullptr )
			, mStageConfig()
			, mStageNode( nullptr )
			, mCurrentMoveSpeed( 150 )
			, mCurrentFireAmount( 1 )

			, mKeyCodeCollector()
		{}

		Scene* StageNodeScene::create( const helper::FuncSceneMover& back_to_the_previous_scene_callback )
		{
			auto ret = new ( std::nothrow ) StageNodeScene( back_to_the_previous_scene_callback );
			if( !ret || !ret->init() )
			{
				delete ret;
				ret = nullptr;
			}
			else
			{
				ret->autorelease();
			}

			return ret;
		}

		bool StageNodeScene::init()
		{
			if( !Scene::init() )
			{
				return false;
			}

			schedule( CC_SCHEDULE_SELECTOR( StageNodeScene::UpdateForInput ) );

			const auto visibleSize = _director->getVisibleSize();
			const auto visibleOrigin = _director->getVisibleOrigin();

			//
			// Summury
			//
			{
				std::stringstream ss;
				ss << "+ " << getTitle();
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[ESC] Return to Root";
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[ARROW KEY] Actor Move";
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[SPACE] Do Bullet";
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[Q] Bullet Speed - Increase";
				ss << cpg::linefeed;
				ss << "[W] Bullet Speed - Decrease";
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[A] Fire Amount - Increase";
				ss << cpg::linefeed;
				ss << "[S] Fire Amount - Decrease";

				auto label = Label::createWithTTF( ss.str(), cpg::StringTable::GetFontPath(), 9, Size::ZERO, TextHAlignment::LEFT );
				label->setAnchorPoint( Vec2( 0.f, 1.f ) );
				label->setPosition( Vec2(
					visibleOrigin.x
					, visibleOrigin.y + visibleSize.height
				) );
				addChild( label, std::numeric_limits<int>::max() );
			}
			
			//
			// Background
			//
			{
				auto background_layer = LayerColor::create( Color4B::BLACK );
				addChild( background_layer, std::numeric_limits<int>::min() );
			}

			//
			// Current Move Speed
			//
			{
				auto label = Label::createWithTTF( "", cpg::StringTable::GetFontPath(), 12, Size::ZERO, TextHAlignment::LEFT );
				label->setTag( TAG_MoveSpeedView );
				label->setAnchorPoint( Vec2( 1.f, 1.f ) );
				label->setColor( Color3B::GREEN );
				label->setPosition( Vec2(
					visibleOrigin.x + visibleSize.width
					, visibleOrigin.y + visibleSize.height
				) );
				addChild( label, std::numeric_limits<int>::max() );

				updateMoveSpeedView();
			}

			//
			// Spawn Target Count
			//
			{
				auto label = Label::createWithTTF( "Temp", cpg::StringTable::GetFontPath(), 12, Size::ZERO, TextHAlignment::LEFT );
				label->setTag( TAG_FireAmountView );
				label->setAnchorPoint( Vec2( 1.f, 1.f ) );
				label->setColor( Color3B::GREEN );
				label->setPosition( Vec2(
					visibleOrigin.x + visibleSize.width
					, visibleOrigin.y + visibleSize.height - label->getContentSize().height
				) );
				addChild( label, std::numeric_limits<int>::max() );

				updateFireAmountView();
			}

			//
			// Stage Node
			//
			{
				mStageConfig.Build(
					visibleOrigin.x + visibleSize.width * 0.5f, visibleOrigin.y + visibleSize.height * 0.5f
					, 120.f
				);

				mStageNode = game::StageNode::create(
					mStageConfig
					, game::StageNode::DebugConfig{ true, true, true, true }
					, step_mole::CircleCollisionComponentConfig { false, false, false }
					, BulletCachingAmount
				);
				addChild( mStageNode );
			}

			//
			// Player Node
			//
			{
				auto player_node = game::PlayerNode::create( 3.f, game::PlayerNode::DebugConfig{ true }, step_mole::CircleCollisionComponentConfig{ true, true, true } );
				mStageNode->AddPlayer( player_node );
			}

			//
			// Enemy Node
			//
			{
				Vec2 enemy_position = mStageConfig.GetCenter();
				enemy_position.y += ( mStageConfig.GetBulletGenerateRadiusMax() );

				auto enemy_node = game::EnemyNode::create( 2.f, game::EnemyNode::DebugConfig{ true }, step_mole::CircleCollisionComponentConfig{ true, true, true }, []() {}, []( Vec2, Vec2 ) {} );
				enemy_node->setPosition( enemy_position );
				mStageNode->AddEnemy( enemy_node );
			}

			return true;
		}

		void StageNodeScene::onEnter()
		{
			Scene::onEnter();

			assert( !mKeyboardListener );
			mKeyboardListener = EventListenerKeyboard::create();
			mKeyboardListener->onKeyPressed = CC_CALLBACK_2( StageNodeScene::onKeyPressed, this );
			mKeyboardListener->onKeyReleased = CC_CALLBACK_2( StageNodeScene::onKeyReleased, this );
			getEventDispatcher()->addEventListenerWithSceneGraphPriority( mKeyboardListener, this );
		}
		void StageNodeScene::onExit()
		{
			assert( mKeyboardListener );
			getEventDispatcher()->removeEventListener( mKeyboardListener );
			mKeyboardListener = nullptr;

			Scene::onExit();
		}

		void StageNodeScene::UpdateForInput( float delta_time )
		{
			Vec2 move_vector;
			if( mKeyCodeCollector.isActiveKey( EventKeyboard::KeyCode::KEY_UP_ARROW ) )
			{
				move_vector.y += 1.f;
			}
			if( mKeyCodeCollector.isActiveKey( EventKeyboard::KeyCode::KEY_DOWN_ARROW ) )
			{
				move_vector.y -= 1.f;
			}
			if( mKeyCodeCollector.isActiveKey( EventKeyboard::KeyCode::KEY_RIGHT_ARROW ) )
			{
				move_vector.x += 1.f;
			}
			if( mKeyCodeCollector.isActiveKey( EventKeyboard::KeyCode::KEY_LEFT_ARROW ) )
			{
				move_vector.x -= 1.f;
			}

			if( 0.f != move_vector.x || 0.f != move_vector.y )
			{
				move_vector.normalize();
				move_vector.scale( 150.f * delta_time );

				mStageNode->PlayerMoveRequest( move_vector );
			}
		}

		void StageNodeScene::updateMoveSpeedView()
		{
			auto label = static_cast<Label*>( getChildByTag( TAG_MoveSpeedView ) );
			label->setString( StringUtils::format( "Move Speed : %d", mCurrentMoveSpeed ) );
		}
		void StageNodeScene::updateFireAmountView()
		{
			auto label = static_cast<Label*>( getChildByTag( TAG_FireAmountView ) );
			label->setString( StringUtils::format( "Fire Count : %d", mCurrentFireAmount ) );
		}
		void StageNodeScene::onKeyPressed( EventKeyboard::KeyCode keycode, Event* /*event*/ )
		{
			switch( keycode )
			{
			case EventKeyboard::KeyCode::KEY_ESCAPE:
				helper::BackToThePreviousScene::MoveBack();
				return;

			case EventKeyboard::KeyCode::KEY_SPACE:
			{
				Vec2 offset;

				for( int i = 0; i < mCurrentFireAmount; ++i )
				{
					Vec2 dir = Vec2( mStageConfig.GetStageRect().getMaxX(), mStageConfig.GetStageRect().getMaxY() ) - mStageConfig.GetStageRect().origin;
					dir.normalize();
					dir.scale( mCurrentMoveSpeed );
					mStageNode->RequestBulletAction(
						mStageConfig.GetCenter() - Vec2( mStageConfig.GetBulletGenerateRadiusMin(), mStageConfig.GetBulletGenerateRadiusMin() ) + offset
						, dir
					);

					offset.y += 2.f;
				}
			}
			break;

			case EventKeyboard::KeyCode::KEY_Q:
				mCurrentMoveSpeed += 1;
				updateMoveSpeedView();
				break;
			case EventKeyboard::KeyCode::KEY_W:
				mCurrentMoveSpeed = std::max( 1, mCurrentMoveSpeed - 1 );
				updateMoveSpeedView();
				break;

			case EventKeyboard::KeyCode::KEY_A:
				mCurrentFireAmount += 1;
				updateFireAmountView();
				break;
			case EventKeyboard::KeyCode::KEY_S:
				mCurrentFireAmount = std::max( 1, mCurrentFireAmount - 1 );
				updateFireAmountView();
				break;

			default:
				CCLOG( "Key Code : %d", keycode );
			}

			mKeyCodeCollector.onKeyPressed( keycode );
		}

		void StageNodeScene::onKeyReleased( EventKeyboard::KeyCode keycode, Event* /*event*/ )
		{
			mKeyCodeCollector.onKeyReleased( keycode );
		}
	}
}
