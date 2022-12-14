#pragma once

#include "helper_BackToThePreviousScene.h"
#include "2d/CCScene.h"

namespace cocos_research_image
{
	class BasicScene : public cocos2d::Scene, private helper::BackToThePreviousScene
	{
	private:
		BasicScene( const helper::FuncSceneMover& back_to_the_previous_scene_callback );

	public:
		static const char* getTitle() { return "Image : Basic"; }
		static cocos2d::Scene* create( const helper::FuncSceneMover& back_to_the_previous_scene_callback );

	private:
		bool init() override;

	public:
		void onEnter() override;
		void onExit() override;

	private:
		void onKeyPressed( cocos2d::EventKeyboard::KeyCode keycode, cocos2d::Event* event );

	private:
		cocos2d::EventListenerKeyboard* mKeyboardListener;
	};
}
