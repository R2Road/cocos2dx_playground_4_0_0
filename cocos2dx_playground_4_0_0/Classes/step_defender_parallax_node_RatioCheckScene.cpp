#include "step_defender_parallax_node_RatioCheckScene.h"

#include <new>
#include <numeric>

#include "2d/CCLabel.h"
#include "2d/CCLayer.h"
#include "2d/CCParallaxNode.h"
#include "2d/CCSprite.h"
#include "base/CCDirector.h"
#include "base/CCEventListenerKeyboard.h"
#include "base/CCEventDispatcher.h"

#include "cpg_SStream.h"
#include "cpg_StringTable.h"

USING_NS_CC;

namespace
{
	const int TAG_Parallax = 10000;
	const float ScrollSpeed = 300.f;
}

namespace step_defender
{
	namespace parallax_node
	{
		RatioCheckScene::RatioCheckScene( const helper::FuncSceneMover& back_to_the_previous_scene_callback ) :
			helper::BackToThePreviousScene( back_to_the_previous_scene_callback )
			, mKeyboardListener( nullptr )
			, mKeyCodeCollector()
		{}

		Scene* RatioCheckScene::create( const helper::FuncSceneMover& back_to_the_previous_scene_callback )
		{
			auto ret = new ( std::nothrow ) RatioCheckScene( back_to_the_previous_scene_callback );
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

		bool RatioCheckScene::init()
		{
			if( !Scene::init() )
			{
				return false;
			}

			const auto visibleOrigin = _director->getVisibleOrigin();
			const auto visibleSize = _director->getVisibleSize();

			//
			// Summury
			//
			{
				std::stringstream ss;
				ss << "+ " << getTitle();
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[ESC] : Return to Root";
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[Arrow L/R] : Do Parralax";

				auto label = Label::createWithTTF( ss.str(), cpg::StringTable::GetFontPath(), 10, Size::ZERO, TextHAlignment::LEFT );
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
				auto background_layer = LayerColor::create( Color4B( 7, 39, 43, 255 ) );
				addChild( background_layer, std::numeric_limits<int>::min() );
			}

			//
			// Explain
			//
			{
				auto label = Label::createWithTTF( "+ Do Scroll and Check Green Line", cpg::StringTable::GetFontPath(), 10 );
				label->setAnchorPoint( Vec2( 1.f, 1.f ) );
				label->setColor( Color3B::GREEN );
				label->setPosition( Vec2(
					visibleOrigin.x + visibleSize.width
					, visibleOrigin.y + visibleSize.height
				) );
				addChild( label, std::numeric_limits<int>::max() );
			}

			//
			// Test Setup
			//
			{
				const Size TotalContentSize( visibleSize.width * 2, visibleSize.height );
				setContentSize( TotalContentSize );

				auto background_root_node = ParallaxNode::create();
				background_root_node->setTag( TAG_Parallax );
				addChild( background_root_node );

				//
				// Background 1
				//
				{
					const float parallax_rate = 0.6f;
					const float part_width = 100.f;
					const float part_height = TotalContentSize.height * 0.75f;

					auto background_node = Node::create();
					background_root_node->addChild( background_node, 1, Vec2( parallax_rate, 1.f ), Vec2::ZERO );

					//
					// # Summury
					// - "... + visibleSize.width;" is need for scroll limit
					// - Background node tails meet on position x( "-TotalContentSize.width" )
					// - Do run and Show green line
					//
					const auto background_width = ( TotalContentSize.width * parallax_rate ) + visibleSize.width;
					const auto div_result = std::div( static_cast<int>( background_width ), part_width );
					Color4B current_color;
					for( int i = 0, end = div_result.quot + ( div_result.rem > 0 ? 1 : 0 ); end > i; ++i )
					{
						if( ( i & 1 ) == 0 )
						{
							current_color = Color4B::RED;
						}
						else
						{
							current_color = Color4B::BLUE;
						}

						auto layer = LayerColor::create( current_color, part_width, part_height );
						layer->setPositionX( i * part_width );
						background_node->addChild( layer );
					}

					//
					// Tail Guide
					//
					{
						auto layer = LayerColor::create( Color4B::GREEN, 5.f, part_height );
						layer->setPositionX( TotalContentSize.width * parallax_rate );
						background_node->addChild( layer, 1 );
					}
				}

				//
				// Background 2
				//
				{
					const float parallax_rate = 0.8f;
					const float part_width = 50.f;
					const float part_height = TotalContentSize.height * 0.5f;

					auto background_node = Node::create();
					background_root_node->addChild( background_node, 2, Vec2( parallax_rate, 1.f ), Vec2::ZERO );

					const auto background_width = ( TotalContentSize.width * parallax_rate ) + visibleSize.width;
					const auto div_result = std::div( static_cast<int>( background_width ), part_width );
					Color4B current_color;
					for( int i = 0, end = div_result.quot + ( div_result.rem > 0 ? 1 : 0 ); end > i; ++i )
					{
						if( ( i & 1 ) == 0 )
						{
							current_color = Color4B::YELLOW;
						}
						else
						{
							current_color = Color4B::ORANGE;
						}

						auto layer = LayerColor::create( current_color, part_width, part_height );
						layer->setPositionX( i * part_width );
						background_node->addChild( layer );
					}

					//
					// Tail Guide
					//
					{
						auto layer = LayerColor::create( Color4B::GREEN, 5.f, part_height );
						layer->setPositionX( TotalContentSize.width * parallax_rate );
						background_node->addChild( layer, 1 );
					}
				}

				//
				// Background 3
				//
				{
					const float parallax_rate = 1.f;
					const float part_width = 25.f;
					const float part_height = TotalContentSize.height * 0.25f;

					auto background_node = Node::create();
					background_root_node->addChild( background_node, 2, Vec2( parallax_rate, 1.f ), Vec2::ZERO );

					const auto background_width = ( TotalContentSize.width * parallax_rate ) + visibleSize.width;
					const auto div_result = std::div( static_cast<int>( background_width ), part_width );
					Color4B current_color;
					for( int i = 0, end = div_result.quot + ( div_result.rem > 0 ? 1 : 0 ); end > i; ++i )
					{
						if( ( i & 1 ) == 0 )
						{
							current_color = Color4B::WHITE;
						}
						else
						{
							current_color = Color4B::BLACK;
						}

						auto layer = LayerColor::create( current_color, part_width, part_height );
						layer->setPositionX( i * part_width );
						background_node->addChild( layer );
					}

					//
					// Tail Guide
					//
					{
						auto layer = LayerColor::create( Color4B::GREEN, 5.f, part_height );
						layer->setPositionX( TotalContentSize.width * parallax_rate );
						background_node->addChild( layer, 1 );
					}
				}
			}

			schedule( CC_SCHEDULE_SELECTOR( RatioCheckScene::update4Move ) );

			return true;
		}

		void RatioCheckScene::onEnter()
		{
			Scene::onEnter();

			assert( !mKeyboardListener );
			mKeyboardListener = EventListenerKeyboard::create();
			mKeyboardListener->onKeyPressed = CC_CALLBACK_2( RatioCheckScene::onKeyPressed, this );
			mKeyboardListener->onKeyReleased = CC_CALLBACK_2( RatioCheckScene::onKeyReleased, this );
			getEventDispatcher()->addEventListenerWithSceneGraphPriority( mKeyboardListener, this );
		}
		void RatioCheckScene::onExit()
		{
			assert( mKeyboardListener );
			getEventDispatcher()->removeEventListener( mKeyboardListener );
			mKeyboardListener = nullptr;

			Scene::onExit();
		}

		void RatioCheckScene::update4Move( float delta_time )
		{
			if( mKeyCodeCollector.isActiveKey( EventKeyboard::KeyCode::KEY_RIGHT_ARROW ) )
			{
				auto background_node = getChildByTag( TAG_Parallax );

				const auto new_position = background_node->getPositionX() + ( -ScrollSpeed * delta_time );
				if( -getContentSize().width < new_position )
				{
					background_node->setPositionX( new_position );
				}
				else
				{
					background_node->setPositionX( -getContentSize().width );
				}
			}

			if( mKeyCodeCollector.isActiveKey( EventKeyboard::KeyCode::KEY_LEFT_ARROW ) )
			{
				auto background_node = getChildByTag( TAG_Parallax );

				const auto new_position = background_node->getPositionX() + ( ScrollSpeed * delta_time );
				if( 0.f > new_position )
				{
					background_node->setPositionX( new_position );
				}
				else
				{
					background_node->setPositionX( 0.f );
				}
			}
		}

		void RatioCheckScene::onKeyPressed( EventKeyboard::KeyCode key_code, Event* /*event*/ )
		{
			if( EventKeyboard::KeyCode::KEY_ESCAPE == key_code )
			{
				helper::BackToThePreviousScene::MoveBack();
				return;
			}

			mKeyCodeCollector.onKeyPressed( key_code );
		}
		void RatioCheckScene::onKeyReleased( EventKeyboard::KeyCode key_code, Event* /*event*/ )
		{
			mKeyCodeCollector.onKeyReleased( key_code );
		}
	}
}
