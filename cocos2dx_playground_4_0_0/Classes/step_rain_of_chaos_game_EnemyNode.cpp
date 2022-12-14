#include "step_rain_of_chaos_game_EnemyNode.h"

#include <new>
#include <numeric>
#include <utility>

#include "2d/CCSprite.h"
#include "audio/include/AudioEngine.h"
#include "base/CCDirector.h"

#include "step_mole_AnimationComponent.h"
#include "step_mole_CircleCollisionComponent.h"
#include "step_rain_of_chaos_game_AnimationInfoContainer.h"

USING_NS_CC;

namespace step_rain_of_chaos
{
	namespace game
	{
		EnemyNode::EnemyNode( const ProcessEndCallback& process_end_callback, const RequestBulletCallback& request_bullet_callback ) :
			mProcessEndCallback( process_end_callback )
			, mRequestBulletCallback( request_bullet_callback )

			, mViewNode( nullptr )

			, mProcessorContainer( nullptr )
			, mCurrentProcessor()
			, mSpawnInfoContainer()
		{
			mSpawnInfoContainer.reserve( 100 );
		}

		EnemyNode* EnemyNode::create(
			const float radius
			, const DebugConfig debug_config
			, const step_mole::CircleCollisionComponentConfig& circle_collision_component_config
			, const ProcessEndCallback& process_end_callback
			, const RequestBulletCallback& request_bullet_callback
		)
		{
			CCASSERT( process_end_callback, "" );
			CCASSERT( request_bullet_callback, "" );

			auto ret = new ( std::nothrow ) EnemyNode( process_end_callback, request_bullet_callback );
			if( !ret || !ret->init( radius, debug_config, circle_collision_component_config ) )
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

		bool EnemyNode::init(
			const float radius
			, const DebugConfig debug_config
			, const step_mole::CircleCollisionComponentConfig& circle_collision_component_config
		)
		{
			if( !Node::init() )
			{
				return false;
			}

			//
			// Pivot
			//
			if( debug_config.bShowPivot )
			{
				auto pivot = Sprite::createWithSpriteFrameName( "helper_pivot.png" );
				pivot->setScale( _director->getContentScaleFactor() );
				addChild( pivot, std::numeric_limits<int>::max() );
			}

			//
			// View
			//
			{
				const auto& animation_info_container = step_rain_of_chaos::game::GetEnemyAnimationInfoContainer();

				auto view_node = Sprite::createWithSpriteFrameName( animation_info_container[0].SpriteFrameNames[0] );
				view_node->setScale( _director->getContentScaleFactor() );
				addChild( view_node );
				{
					// Animation Component
					auto animation_component = step_mole::AnimationComponent::create( animation_info_container );
					view_node->addComponent( animation_component );
					animation_component->PlayAnimation( cpg_animation::eIndex::idle );
				}

				mViewNode = view_node;
			}

			//
			// Collision
			//
			{
				// Collision Component
				auto circle_collision_component = step_mole::CircleCollisionComponent::create( radius, Vec2::ZERO, circle_collision_component_config );
				addComponent( circle_collision_component );
			}

			return true;
		}

		void EnemyNode::update4Processor( float dt )
		{
			if( mProcessorContainer->end() == mCurrentProcessor )
			{
				StopProcess();
				mProcessEndCallback();
				return;
			}

			mSpawnInfoContainer.clear();

			if( !( *mCurrentProcessor )->Update( dt ) )
			{
				++mCurrentProcessor;
				if( mProcessorContainer->end() != mCurrentProcessor )
				{
					( *mCurrentProcessor )->Enter();
				}
			}

			if( !mSpawnInfoContainer.empty() )
			{
				for( const auto& s : mSpawnInfoContainer )
				{
					Vec2 dir = s.MoveDirection;
					dir.normalize();
					dir.scale( 150.f );
					mRequestBulletCallback( s.StartPosition, dir );
				}

				static std::random_device rd;
				static std::mt19937 randomEngine( rd() );
				static std::uniform_int_distribution<> dist( 0, 1 );

				AudioEngine::play2d(
					0 == dist( randomEngine ) ? "sounds/fx/shoot_004.ogg" : "sounds/fx/shoot_006.ogg"
					, false
					, 0.1f
				);
			}
		}

		void EnemyNode::ShowView()
		{
			mViewNode->setVisible( true );
		}
		void EnemyNode::HideView()
		{
			mViewNode->setVisible( false );
		}

		void EnemyNode::StartProcess( EnemyProcessorContainer* enemy_processor_container )
		{
			StopProcess();

			mProcessorContainer = enemy_processor_container;
			mCurrentProcessor = mProcessorContainer->begin();

			( *mCurrentProcessor )->Enter();

			schedule( CC_SCHEDULE_SELECTOR( EnemyNode::update4Processor ) );
		}
		void EnemyNode::StopProcess()
		{
			unschedule( CC_SCHEDULE_SELECTOR( EnemyNode::update4Processor ) );
		}
	}
}

