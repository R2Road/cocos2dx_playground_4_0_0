#include "cocos_research_update_SequenceScene.h"

#include <new>
#include <numeric>

#include "2d/CCLabel.h"
#include "2d/CCLayer.h"
#include "base/CCDirector.h"
#include "base/CCEventListenerKeyboard.h"
#include "base/CCEventDispatcher.h"

#include "cpg_SStream.h"
#include "cpg_StringTable.h"

USING_NS_CC;

namespace
{
	class TestChildNode : public cocos2d::Node
	{
	private:
		TestChildNode( std::string& update_log ) : mUpdateLog( update_log ) {}

	public:
		static Node* create( std::string& update_log )
		{
			auto ret = new ( std::nothrow ) TestChildNode( update_log );
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

		void update( float )
		{
			mUpdateLog += "priority ==== TestChildNode::update\n";
		}

	private:
		std::string& mUpdateLog;
	};
}

namespace cocos_research_update
{
	SequenceScene::SequenceScene( const helper::FuncSceneMover& back_to_the_previous_scene_callback ) :
		helper::BackToThePreviousScene( back_to_the_previous_scene_callback )
		, mKeyboardListener( nullptr )

		, mLabel4Log( nullptr )
		, mUpdateLog()

		, mTestNode( nullptr )

		, mbInputBlock( false )
	{}

	Scene* SequenceScene::create( const helper::FuncSceneMover& back_to_the_previous_scene_callback )
	{
		auto ret = new ( std::nothrow ) SequenceScene( back_to_the_previous_scene_callback );
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

	bool SequenceScene::init()
	{
		if( !Scene::init() )
		{
			return false;
		}

		const auto visibleSize = _director->getVisibleSize();
		const auto visibleOrigin = _director->getVisibleOrigin();
		const Vec2 visibleCenter(
			visibleOrigin.x + ( visibleSize.width * 0.5f )
			, visibleOrigin.y + ( visibleSize.height * 0.5f )
		);

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
			ss << "[SPACE] : Test Update 4 Me";
			ss << cpg::linefeed;
			ss << cpg::linefeed;
			ss << "[Z] : Test Update With Child Node : Scene First";
			ss << cpg::linefeed;
			ss << "[X] : Test Update With Child Node : Child First";

			auto label = Label::createWithTTF( ss.str(), cpg::StringTable::GetFontPath(), 8 );
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
			auto background_layer = LayerColor::create( Color4B( 5, 29, 81, 255 ) );
			addChild( background_layer, std::numeric_limits<int>::min() );
		}

		//
		// Research
		//
		{
			mLabel4Log = Label::createWithTTF( "Waiting", cpg::StringTable::GetFontPath(), 8, Size::ZERO, TextHAlignment::CENTER );
			mLabel4Log->setPosition( visibleCenter );
			addChild( mLabel4Log );

			mTestNode = TestChildNode::create( mUpdateLog );
			addChild( mTestNode );
		}


		return true;
	}

	void SequenceScene::onEnter()
	{
		Scene::onEnter();

		assert( !mKeyboardListener );
		mKeyboardListener = EventListenerKeyboard::create();
		mKeyboardListener->onKeyPressed = CC_CALLBACK_2( SequenceScene::onKeyPressed, this );
		getEventDispatcher()->addEventListenerWithSceneGraphPriority( mKeyboardListener, this );
	}
	void SequenceScene::onExit()
	{
		assert( mKeyboardListener );
		getEventDispatcher()->removeEventListener( mKeyboardListener );
		mKeyboardListener = nullptr;

		Scene::onExit();
	}

	void SequenceScene::update( float )
	{
		mUpdateLog += "priority ==== SequenceScene::update\n";

		// Scene::update( dt ); - not need, update 4 component
	}
	void SequenceScene::test_Update( float )
	{
		mUpdateLog += "custom selectors ==== SequenceScene::test_Update\n";
	}
	void SequenceScene::test_UpdateOnce( float )
	{
		mUpdateLog += "custom selectors ==== SequenceScene::test_UpdateOnce\n";
	}
	void SequenceScene::test_UpdateEnd( float )
	{
		mUpdateLog += "custom selectors ==== SequenceScene::test_UpdateEnd\n";

		mUpdateLog += "\n\nscheduleUpdate is priority 0";
		mUpdateLog += "\n\ncustom selectors is follow insert sequence";

		mLabel4Log->setString( mUpdateLog );

		mUpdateLog.clear();

		unscheduleAllCallbacks();
		mTestNode->unscheduleAllCallbacks();

		mbInputBlock = false;
	}

	void SequenceScene::onKeyPressed( EventKeyboard::KeyCode keycode, Event* /*event*/ )
	{
		if( mbInputBlock )
		{
			return;
		}

		switch( keycode )
		{
		case EventKeyboard::KeyCode::KEY_ESCAPE:
			helper::BackToThePreviousScene::MoveBack();
			return;

		case EventKeyboard::KeyCode::KEY_SPACE:
			schedule( CC_SCHEDULE_SELECTOR( SequenceScene::test_Update ) );
			scheduleOnce( CC_SCHEDULE_SELECTOR( SequenceScene::test_UpdateOnce ), 0.f );
			scheduleUpdate();

			scheduleOnce( CC_SCHEDULE_SELECTOR( SequenceScene::test_UpdateEnd ), 0.f );
			mbInputBlock = true;
			return;

		case EventKeyboard::KeyCode::KEY_Z:
			scheduleUpdate();
			mTestNode->scheduleUpdate();

			scheduleOnce( CC_SCHEDULE_SELECTOR( SequenceScene::test_UpdateEnd ), 0.f );
			mbInputBlock = true;
			return;

		case EventKeyboard::KeyCode::KEY_X:
			mTestNode->scheduleUpdate();
			scheduleUpdate();

			scheduleOnce( CC_SCHEDULE_SELECTOR( SequenceScene::test_UpdateEnd ), 0.f );
			mbInputBlock = true;
			return;

		default:
			CCLOG( "Key Code : %d", keycode );
		}
	}
}
