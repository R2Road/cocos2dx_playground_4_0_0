#include "ui_research_type_effect_ColorScene.h"

#include <algorithm>
#include <new>
#include <numeric>

#include "2d/CCLabel.h"
#include "2d/CCLayer.h"
#include "2d/CCSprite.h"
#include "base/CCDirector.h"
#include "base/CCEventListenerKeyboard.h"
#include "base/CCEventListenerMouse.h"
#include "base/CCEventDispatcher.h"
#include "base/ccUTF8.h"

#include "cpg_SStream.h"
#include "cpg_StringTable.h"

USING_NS_CC;

namespace
{
	const int TAG_TextView = 20140416;
	const int TAG_DelayView = 20160528;
}

namespace ui_research
{
	namespace type_effect
	{
		ColorScene::ColorScene( const helper::FuncSceneMover& back_to_the_previous_scene_callback ) :
			helper::BackToThePreviousScene( back_to_the_previous_scene_callback )
			, mKeyboardListener( nullptr )
			, mTypeDelay( 0.05f )
			, mElapsedTime( 0.f )
			, mLetterIndicator( 0 )
		{}

		Scene* ColorScene::create( const helper::FuncSceneMover& back_to_the_previous_scene_callback )
		{
			auto ret = new ( std::nothrow ) ColorScene( back_to_the_previous_scene_callback );
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

		bool ColorScene::init()
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
				ss << "+ " << getTitle();
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[ESC] : Return to Root";
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[SPACE] : Start Type";
				ss << cpg::linefeed;
				ss << "[A] : End Type";
				ss << cpg::linefeed;
				ss << cpg::linefeed;
				ss << "[Arrow Up] : Delay - Increase";
				ss << cpg::linefeed;
				ss << "[Arrow Uown] : Delay - Decrease";

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
				auto background_layer = LayerColor::create( Color4B( 50, 75, 112, 255 ) );
				addChild( background_layer, std::numeric_limits<int>::min() );
			}

			//
			// Delay View
			//
			{
				auto label = Label::createWithTTF( "", cpg::StringTable::GetFontPath(), 14 );
				label->setTag( TAG_DelayView );
				label->setColor( Color3B::GREEN );
				label->setAnchorPoint( Vec2( 1.f, 1.f ) );
				label->setPosition( Vec2(
					visibleOrigin.x + visibleSize.width
					, visibleOrigin.y + visibleSize.height
				) );
				addChild( label, std::numeric_limits<int>::max() );

				updateDelayView();
			}

			//
			// Research
			//
			{
				const std::u32string u32_string = U"????????????????????????????\n?????????? ??????????\n?????????? ?????????? ???????? 1234567890\n?????? ?????? ???? ??????";
				std::string utf8_string;
				StringUtils::UTF32ToUTF8( u32_string, utf8_string );

				auto label = Label::createWithTTF( utf8_string, cpg::StringTable::GetFontPath(), 15, Size::ZERO, TextHAlignment::CENTER );
				label->setTag( TAG_TextView );
				label->setPosition( Vec2(
					visibleOrigin.x + ( visibleSize.width * 0.5f )
					, visibleOrigin.y + ( visibleSize.height * 0.5f )
				) );
				addChild( label, std::numeric_limits<int>::max() );

				// build letters
				Color3B letter_color = Color3B::WHITE;
				for( int i = 0; label->getStringLength() > i; ++i )
				{
					switch( i )
					{
					case 10:
						letter_color = Color3B::RED;
						break;
					case 20:
						letter_color = Color3B::BLACK;
						break;
					case 30:
						letter_color = Color3B::ORANGE;
						break;
					case 40:
						letter_color = Color3B::MAGENTA;
						break;
					case 50:
						letter_color = Color3B::GREEN;
						break;
					case 60:
						letter_color = Color3B::YELLOW;
						break;
					}

					auto letter = label->getLetter( i );
					if( letter )
					{
						letter->setColor( letter_color );
					}
				}
			}

			return true;
		}

		void ColorScene::onEnter()
		{
			Scene::onEnter();

			assert( !mKeyboardListener );
			mKeyboardListener = EventListenerKeyboard::create();
			mKeyboardListener->onKeyPressed = CC_CALLBACK_2( ColorScene::onKeyPressed, this );
			getEventDispatcher()->addEventListenerWithSceneGraphPriority( mKeyboardListener, this );
		}
		void ColorScene::onExit()
		{
			assert( mKeyboardListener );
			getEventDispatcher()->removeEventListener( mKeyboardListener );
			mKeyboardListener = nullptr;

			Scene::onExit();
		}

		void ColorScene::HideLetters()
		{
			auto label = static_cast<Label*>( getChildByTag( TAG_TextView ) );
			for( auto letter : label->getChildren() )
			{
				letter->setVisible( false );
			}
		}
		void ColorScene::StartType()
		{
			if( isScheduled( SEL_SCHEDULE( &ColorScene::updateForType ) ) )
			{
				EndType();
			}

			HideLetters();
			schedule( SEL_SCHEDULE( &ColorScene::updateForType ) );
		}
		void ColorScene::EndType()
		{
			unscheduleAllCallbacks();

			mElapsedTime = 0.f;
			mLetterIndicator = 0;
		}
		void ColorScene::updateForType( float dt )
		{
			mElapsedTime += dt;
			if( mTypeDelay > mElapsedTime )
			{
				return;
			}

			mElapsedTime = 0.f;

			auto label = static_cast<Label*>( getChildByTag( TAG_TextView ) );
			auto letter = label->getLetter( mLetterIndicator );
			if( letter )
			{
				letter->setVisible( true );
			}

			++mLetterIndicator;
			if( label->getStringLength() <= mLetterIndicator )
			{
				EndType();
			}
		}

		void ColorScene::updateDelayView()
		{
			auto label = static_cast<Label*>( getChildByTag( TAG_DelayView ) );
			label->setString( StringUtils::format( "Delay : %.2f", mTypeDelay ) );
		}

		void ColorScene::onKeyPressed( EventKeyboard::KeyCode keycode, Event* /*event*/ )
		{
			if( EventKeyboard::KeyCode::KEY_ESCAPE == keycode )
			{
				helper::BackToThePreviousScene::MoveBack();
				return;
			}

			switch( keycode )
			{
			case EventKeyboard::KeyCode::KEY_SPACE:
				StartType();
				break;
			case EventKeyboard::KeyCode::KEY_A:
				EndType();
				break;

			case EventKeyboard::KeyCode::KEY_UP_ARROW:
				mTypeDelay += 0.01f;
				updateDelayView();
				break;
			case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
				mTypeDelay = std::max( 0.f, mTypeDelay - 0.01f );
				updateDelayView();
				break;
			}
		}
	}
}
