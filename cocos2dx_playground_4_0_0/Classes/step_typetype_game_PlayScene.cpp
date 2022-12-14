#include "step_typetype_game_PlayScene.h"

#include <new>
#include <numeric>
#include <sstream>

#include "2d/CCLabel.h"
#include "2d/CCLayer.h"
#include "audio/include/AudioEngine.h"
#include "base/CCDirector.h"
#include "base/CCEventListenerKeyboard.h"
#include "base/CCEventDispatcher.h"

#include "step_typetype_game_Constant.h"
#include "step_typetype_game_IndicatorViewNode.h"
#include "step_typetype_game_ResultScene.h"
#include "step_typetype_game_StageViewNode.h"
#include "step_typetype_game_TitleScene.h"

USING_NS_CC;

namespace step_typetype
{
	namespace game
	{
		PlayScene::PlayScene() :
			mKeyboardListener( nullptr )

			, mCurrentStageLength( 4u )
			, mStage( GameConfig.StageMaxLength )
			, mStageViewNode( nullptr )
			, mIndicatorViewNode( nullptr )
			, mNextStageIndicatorNode( nullptr )

			, mElapsedTime( 0.f )
			, mAudioID_forBGM( -1 )
		{}

		Scene* PlayScene::create()
		{
			auto ret = new ( std::nothrow ) PlayScene();
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

		bool PlayScene::init()
		{
			if( !Scene::init() )
			{
				return false;
			}

			const auto visibleSize = _director->getVisibleSize();
			const auto visibleOrigin = _director->getVisibleOrigin();


			//
			// Summury
			//
			{
				std::stringstream ss;
				ss << "[ESC] : Return to Title";

				auto label = Label::createWithTTF( ss.str(), "fonts/NanumSquareR.ttf", 6, Size::ZERO, TextHAlignment::LEFT );
				label->setAnchorPoint( Vec2( 0.f, 1.f ) );
				label->setPosition(
					visibleOrigin
					+ Vec2( 0.f, visibleSize.height )
				);
				addChild( label, std::numeric_limits<int>::max() );
			}

			//
			// Background
			//
			{
				auto background_layer = LayerColor::create( Color4B( 79, 10, 5, 255 ) );
				addChild( background_layer, std::numeric_limits<int>::min() );
			}

			//
			// BGM License
			//
			{
				auto label = Label::createWithTTF(
					"BGM : Empty Space\nAuthor : tcarisland\nLicense : CC-BY 4.0\nFrom : https://opengameart.org/"
					, "fonts/NanumSquareR.ttf", 10, Size::ZERO, TextHAlignment::RIGHT
				);
				label->setColor( Color3B::GREEN );
				label->setAnchorPoint( Vec2( 1.f, 1.f ) );
				label->setPosition(
					visibleOrigin
					+ Vec2( visibleSize.width, visibleSize.height )
				);
				addChild( label, std::numeric_limits<int>::max() );
			}

			//
			// Stage View Node
			//
			{
				mStageViewNode = StageViewNode::create( GameConfig.StageMaxLength, StageViewNode::Config{ false } );
				mStageViewNode->setPosition(
					visibleOrigin
					+ Vec2( visibleSize.width * 0.5f, visibleSize.height * 0.5f )
				);
				addChild( mStageViewNode );
			}

			//
			// Indicator View
			//
			{
				mIndicatorViewNode = game::IndicatorViewNode::create( game::IndicatorViewNode::Config{ false, false } );
				mIndicatorViewNode->setPosition(
					visibleOrigin
					+ Vec2( visibleSize.width * 0.5f, visibleSize.height * 0.5f )
				);
				addChild( mIndicatorViewNode, 1 );
			}

			//
			// Next Stage Indicator
			//
			{
				mNextStageIndicatorNode = Label::createWithTTF( "ENTER", "fonts/NanumSquareR.ttf", 10 );
				mNextStageIndicatorNode->setColor( Color3B::GREEN );
				mNextStageIndicatorNode->setPosition(
					visibleOrigin
					+ Vec2( visibleSize.width * 0.5f, visibleSize.height * 0.5f )
				);
				addChild( mNextStageIndicatorNode, 2 );
			}

			//
			// Setup
			//
			mStage.Reset( mCurrentStageLength );
			mStageViewNode->Reset( mStage );
			mIndicatorViewNode->Reset( mCurrentStageLength );
			mNextStageIndicatorNode->setVisible( false );

			scheduleUpdate();

			return true;
		}

		void PlayScene::onEnter()
		{
			Scene::onEnter();

			mAudioID_forBGM = AudioEngine::play2d( "sounds/bgm/EmpySpace.ogg", true, 0.1f );

			assert( !mKeyboardListener );
			mKeyboardListener = EventListenerKeyboard::create();
			mKeyboardListener->onKeyPressed = CC_CALLBACK_2( PlayScene::onKeyPressed, this );
			getEventDispatcher()->addEventListenerWithSceneGraphPriority( mKeyboardListener, this );
		}
		void PlayScene::update( float dt )
		{
			mElapsedTime += dt;
			Scene::update( dt );
		}
		void PlayScene::onExit()
		{
			AudioEngine::stop( mAudioID_forBGM );

			assert( mKeyboardListener );
			getEventDispatcher()->removeEventListener( mKeyboardListener );
			mKeyboardListener = nullptr;

			Scene::onExit();
		}

		void PlayScene::onKeyPressed( EventKeyboard::KeyCode keycode, Event* /*event*/ )
		{
			if( EventKeyboard::KeyCode::KEY_ESCAPE == keycode )
			{
				_director->replaceScene( step_typetype::game::TitleScene::create() );
				return;
			}

			//
			// Game Process
			//
			if( !mStage.IsStageClear() )
			{
				if( EventKeyboard::KeyCode::KEY_A <= keycode && EventKeyboard::KeyCode::KEY_Z >= keycode )
				{
					static const char offset = static_cast<char>( EventKeyboard::KeyCode::KEY_A ) - 65; // 65 == 'A'

					const auto target_letter_code = static_cast<char>( keycode ) - offset;
					const auto target_letter_pos = mStage.GetIndicator_Current();
					if( mStage.RequestLetterDie( target_letter_code ) )
					{
						mStageViewNode->RequestLetterDie( target_letter_pos );
						mIndicatorViewNode->SetIndicatorPosition( target_letter_pos + 1u );

						AudioEngine::play2d( "sounds/fx/jump_001.ogg", false, 0.1f );

						if( mStage.IsStageClear() )
						{
							mIndicatorViewNode->setVisible( false );
							mNextStageIndicatorNode->setVisible( true );
						}
					}
					else
					{
						AudioEngine::play2d( "sounds/fx/damaged_001.ogg", false, 0.1f );
					}
				}
			}
			else if( EventKeyboard::KeyCode::KEY_ENTER == keycode )
			{
				mCurrentStageLength += 2u;
				AudioEngine::play2d( "sounds/fx/powerup_001.ogg", false, 0.1f );

				if( mCurrentStageLength < mStage.GetLength_MAX() ) // go next stage
				{
					mStage.Reset( mCurrentStageLength );
					mStageViewNode->Reset( mStage );

					mIndicatorViewNode->setVisible( true );
					mIndicatorViewNode->Reset( mCurrentStageLength );

					mNextStageIndicatorNode->setVisible( false );
				}
				else // game clear
				{
					_director->replaceScene( step_typetype::game::ResultScene::create( mElapsedTime ) );
				}
			}
		}
	}
}
