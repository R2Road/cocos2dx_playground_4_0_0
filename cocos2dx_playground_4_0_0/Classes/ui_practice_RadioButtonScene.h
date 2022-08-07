#pragma once

#include "2d/CCScene.h"
#include "ui/UIWidget.h"

#include "helper_BackToThePreviousScene.h"

namespace ui_practice
{
	class RadioButtonScene : public cocos2d::Scene, private helper::BackToThePreviousScene
	{
	private:
		RadioButtonScene( const helper::FuncSceneMover& back_to_the_previous_scene_callback );

	public:
		static const char* getTitle() { return "Radio Button"; }
		static cocos2d::Scene* create( const helper::FuncSceneMover& back_to_the_previous_scene_callback );

	private:
		bool init() override;

	public:
		void onEnter() override;
		void onExit() override;

	private:
		void onRadioButton( cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType touchEventType );
		void updateStatusView( const int radio_button_tag );

		void onKeyPressed( cocos2d::EventKeyboard::KeyCode keycode, cocos2d::Event* event );

	private:
		cocos2d::EventListenerKeyboard* mKeyboardListener;
	};
}
