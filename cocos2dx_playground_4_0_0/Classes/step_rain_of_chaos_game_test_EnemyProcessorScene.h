#pragma once

#include "2d/CCScene.h"
#include "ui/UIWidget.h"

#include "helper_BackToThePreviousScene.h"
#include "step_rain_of_chaos_game_EnemyNode.h"
#include "step_rain_of_chaos_game_StageConfig.h"

namespace step_rain_of_chaos
{
	namespace game
	{
		class StageNode;
	}

	namespace game_test
	{
		class EnemyProcessorScene : public cocos2d::Scene, private helper::BackToThePreviousScene
		{
		private:
			struct NameNPackage
			{
				const char* Name = nullptr;
				game::EnemyNode::EnemyProcessorContainer Package;
			};
			using PackageContainer = std::vector<NameNPackage>;

			EnemyProcessorScene( const helper::FuncSceneMover& back_to_the_previous_scene_callback );

		public:
			static const char* getTitle() { return "Game Test : Enemy Processor"; }
			static cocos2d::Scene* create( const helper::FuncSceneMover& back_to_the_previous_scene_callback );

		private:
			bool init() override;

		public:
			void onEnter() override;
			void onExit() override;

		private:
			void onKeyPressed( cocos2d::EventKeyboard::KeyCode keycode, cocos2d::Event* event );
			void onStartButton( cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType touch_event_type );
			void onTargetButton( cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType touch_event_type );
			void onPackageSelect( cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType touch_event_type );

		private:
			cocos2d::EventListenerKeyboard* mKeyboardListener;

			cocos2d::Vec2 mStartButton_MoveOffset;
			cocos2d::Vec2 mPlayerLastTouchPosition;

			game::StageConfig mStageConfig;
			game::StageNode* mStageNode;

			cocos2d::Node* mStartNode;
			cocos2d::Node* mTargetNode;

			PackageContainer mPackgeContainer;
			PackageContainer::value_type* mCurrentPackage;
		};
	}
}
