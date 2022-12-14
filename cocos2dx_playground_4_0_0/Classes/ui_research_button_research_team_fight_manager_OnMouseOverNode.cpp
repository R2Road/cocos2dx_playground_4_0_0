#include "ui_research_button_research_team_fight_manager_OnMouseOverNode.h"

#include <new>
#include <numeric>

#include "2d/CCDrawNode.h"
#include "2d/CCLayer.h"
#include "2d/CCTweenFunction.h"

#include "cpg_node_PivotNode.h"

USING_NS_CC;

namespace ui_research
{
	namespace button_research
	{
		namespace team_fight_manager
		{
			OnMouseOverNode::OnMouseOverNode() : mRotateNode( nullptr ), mElapsedTime( 0.f )
			{}

			OnMouseOverNode* OnMouseOverNode::create( const cocos2d::Size& size )
			{
				auto ret = new ( std::nothrow ) OnMouseOverNode();
				if( !ret || !ret->init( size ) )
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

			bool OnMouseOverNode::init( const cocos2d::Size& size )
			{
				//
				// 
				//
				setContentSize( size );
				setBackGroundColorType( ui::Layout::BackGroundColorType::SOLID );
				setBackGroundColor( Color3B( 218, 143, 133 ) );
				setClippingEnabled( true );

				//
				// Pivot
				//
				{
					auto pivot_node = cpg_node::PivotNode::create();
					addChild( pivot_node, std::numeric_limits<int>::max() );
				}

				//
				// Polygon
				//
				{
					const float required_height = std::sqrt( pow( size.width * 0.5f, 2 ) + pow( size.height * 0.5f, 2 ) );

					Vec2 points[3] = { Vec2::ZERO, Vec2( -required_height, required_height ), Vec2( 0.f, required_height ) };

					auto draw_node = DrawNode::create();
					draw_node->drawPolygon( points, 3, Color4F::WHITE, 0.f, Color4F::WHITE );
					draw_node->setPosition( Vec2( getContentSize().width * 0.5f, getContentSize().height * 0.5f ) );
					addChild( draw_node, 0 );

					mRotateNode = draw_node;
				}

				//
				// Layer
				//
				{
					auto layer = LayerColor::create( Color4B::BLACK, size.width - 2.f, size.height - 2.f );
					layer->setVisible( true );
					layer->setPosition( 1.f, 1.f );
					addChild( layer, 1 );
				}

				return true;
			}

			void OnMouseOverNode::update4Rotation( float dt )
			{
				mElapsedTime += dt;
				if( 1.f < mElapsedTime )
				{
					mElapsedTime = mElapsedTime - static_cast<int>( mElapsedTime );
				}

				tweenfunc::quadraticIn( mElapsedTime );

				mRotateNode->setRotation( -360.f * tweenfunc::quadraticInOut( mElapsedTime ) );
			}

			void OnMouseOverNode::setVisible( bool visible )
			{
				ui::Layout::setVisible( visible );

				if( !visible )
				{
					unschedule( CC_SCHEDULE_SELECTOR( OnMouseOverNode::update4Rotation ) );
				}
				else
				{
					mElapsedTime = 0.f;
					schedule( CC_SCHEDULE_SELECTOR( OnMouseOverNode::update4Rotation ) );
				}
			}
		}
	}
}
