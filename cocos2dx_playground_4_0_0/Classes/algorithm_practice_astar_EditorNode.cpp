#include "algorithm_practice_astar_EditorNode.h"

#include <new>
#include <numeric>

#include "2d/CCLabel.h"
#include "base/CCDirector.h"
#include "base/CCEventListenerKeyboard.h"
#include "base/CCEventDispatcher.h"
#include "ui/UIButton.h"

#include "algorithm_practice_astar_Grid4TileMap.h"

#include "cpg_SStream.h"
#include "cpg_StringTable.h"
#include "cpg_ui_ToolBarNode.h"

#include "step_defender_game_TileMapNode.h"

USING_NS_CC;

namespace
{
	cpg::Point GetTilePoint( algorithm_practice_astar::eCellType cell_type )
	{
		switch( cell_type )
		{
		case algorithm_practice_astar::eCellType::Road:
			return cpg::Point{ 0, 0 };

		case algorithm_practice_astar::eCellType::Wall:
			return cpg::Point{ 1, 0 };

		default:
			return cpg::Point{ 0, 0 };
		}
	}
}

namespace algorithm_practice_astar
{
	EditorNode::EditorNode(
		const Config config
		, Grid4TileMap* const grid_4_tile_map
		, step_defender::game::TileMapNode* const tile_map_node
		, Node* const entry_point_indocator_node
		, Node* const exit_point_indocator_node
		, const cpg::TileSheetConfiguration& tile_sheet_configuration
	) :
		mKeyboardListener( nullptr )

		, mConfig( config )

		, mGrid4TileMap( grid_4_tile_map )
		, mTileMapNode( tile_map_node )
		, mEntryPointIndicatorNode( entry_point_indocator_node )
		, mExitPointIndicatorNode( exit_point_indocator_node )
		, mPosition2GridIndexConverter( 1, 1 )
		, mTileSheetConfiguration( tile_sheet_configuration )

		, mToolIndex( eToolIndex::Wall )
		, mGridDebugViewNode( nullptr )
	{}

	EditorNode* EditorNode::create(
		const Config config
		, Grid4TileMap* const grid_4_tile_map
		, step_defender::game::TileMapNode* const tile_map_node
		, Node* const entry_point_indocator_node
		, Node* const exit_point_indocator_node
		, const cpg::TileSheetConfiguration& tile_sheet_configuration
	)
	{
		auto ret = new ( std::nothrow ) EditorNode( config, grid_4_tile_map, tile_map_node, entry_point_indocator_node, exit_point_indocator_node, tile_sheet_configuration );
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

	bool EditorNode::init()
	{
		if( !Node::init() )
		{
			return false;
		}

		const auto visibleOrigin = _director->getVisibleOrigin();
		const auto visibleSize = _director->getVisibleSize();
		const Vec2 visibleCenter(
			visibleOrigin.x + ( visibleSize.width * 0.5f )
			, visibleOrigin.y + ( visibleSize.height * 0.5f )
		);

		//
		// Summury
		//
		{
			std::stringstream ss;
			ss << cpg::linefeed;
			ss << cpg::linefeed;
			ss << "[R] : " << "Reset";
			ss << cpg::linefeed;
			ss << cpg::linefeed;
			ss << "[Mouse] : " << "Edit Grid";

			auto label = Label::createWithTTF( ss.str(), cpg::StringTable::GetFontPath(), 7 );
			label->setAnchorPoint( Vec2( 0.f, 1.f ) );
			label->setPosition(
				visibleOrigin
				+ Vec2( 0.f, visibleSize.height )
				- Vec2( 0.f, 20.f )
			);
			addChild( label, std::numeric_limits<int>::max() );
		}

		//
		// Setup Grid Index Converter
		//
		mPosition2GridIndexConverter = cpg::Position2GridIndexConverter(
			mTileSheetConfiguration.GetTileWidth()
			, mTileSheetConfiguration.GetTileHeight()
		);

		//
		// UI 4 Edit
		//
		{
			auto mUI4Edit = Node::create();
			addChild( mUI4Edit, std::numeric_limits<int>::max() );

			//
			// Tool Bar - for Tool
			//
			{
				auto tool_bar_node = cpg_ui::ToolBarNode::create( ui::Layout::Type::VERTICAL, Size( 70.f, 20.f ) );
				mUI4Edit->addChild( tool_bar_node );

				tool_bar_node->AddTool( eToolIndex::Wall, "Wall Tile", 10, std::bind( &EditorNode::onToolSelect, this, eToolIndex::Wall ) );
				tool_bar_node->AddTool( eToolIndex::Road, "Road Tile", 10, std::bind( &EditorNode::onToolSelect, this, eToolIndex::Road ) );
				tool_bar_node->AddTool( eToolIndex::Entry, "Entry Point", 10, std::bind( &EditorNode::onToolSelect, this, eToolIndex::Entry ) );
				tool_bar_node->AddTool( eToolIndex::Exit, "Exit Point", 10, std::bind( &EditorNode::onToolSelect, this, eToolIndex::Exit ) );

				tool_bar_node->setPosition(
					visibleOrigin
					+ Vec2( visibleSize.width, visibleSize.height )
					+ Vec2( -tool_bar_node->getContentSize().width, -tool_bar_node->getContentSize().height )
				);

				// Set Indicator
				tool_bar_node->SelectTool( mToolIndex );
			}
		}

		//
		// Touch Node
		//
		{
			auto button = ui::Button::create( "guide_01_0.png", "guide_01_4.png", "guide_01_2.png", ui::Widget::TextureResType::PLIST );
			button->setAnchorPoint( Vec2::ZERO );
			button->setScale9Enabled( true );
			button->setContentSize( mTileMapNode->getContentSize() + Size( 4.f, 4.f ) );
			button->setPosition(
				visibleCenter
				- Vec2( button->getContentSize().width * 0.5f, button->getContentSize().height * 0.5f )
			);
			button->addTouchEventListener( CC_CALLBACK_2( EditorNode::onUpdateTile, this ) );
			addChild( button, std::numeric_limits<int>::max() );
		}

		//
		// Grid Debug View Node
		//
		{
			cpg::TileSheetConfiguration tile_sheet_config;
			CCASSERT( tile_sheet_config.Load( "datas/algorithm_practice/algorithm_practice_tile_sheet_config_02.json" ), "Failed - Load Tile Sheet Configuration" );

			mGridDebugViewNode = step_defender::game::TileMapNode::create(
				step_defender::game::TileMapNode::Config{ mConfig.MapWidth, mConfig.MapHeight }
				, tile_sheet_config
			);
			mGridDebugViewNode->setPosition(
				visibleOrigin
				+ Vec2( visibleSize.width * 0.5f, visibleSize.height )
				- Vec2( mGridDebugViewNode->getContentSize().width * 0.5f, mGridDebugViewNode->getContentSize().height )
				- Vec2( 0.f, 2.f )
			);
			addChild( mGridDebugViewNode );
		}

		//
		// Setup
		//
		resetView();

		return true;
	}

	void EditorNode::onEnter()
	{
		Node::onEnter();

		assert( !mKeyboardListener );
		mKeyboardListener = EventListenerKeyboard::create();
		mKeyboardListener->onKeyPressed = CC_CALLBACK_2( EditorNode::onKeyPressed, this );
		mKeyboardListener->setEnabled( isVisible() );
		getEventDispatcher()->addEventListenerWithSceneGraphPriority( mKeyboardListener, this );
	}
	void EditorNode::onExit()
	{
		assert( mKeyboardListener );
		getEventDispatcher()->removeEventListener( mKeyboardListener );
		mKeyboardListener = nullptr;

		Node::onExit();
	}


	void EditorNode::setVisible( bool visible )
	{
		Node::setVisible( visible );

		if( mKeyboardListener )
		{
			mKeyboardListener->setEnabled( visible );
		}
	}


	void EditorNode::onToolSelect( const int tool_index )
	{
		mToolIndex = tool_index;
		CCLOG( "Tool Index : %d", mToolIndex );
	}
	void EditorNode::onGridClear()
	{
		//
		// Reset Grid
		//
		mGrid4TileMap->SetEntryPoint( cpg::Point{ 0, 0 } );
		mGrid4TileMap->SetExitPoint( cpg::Point{ 1, 0 } );
		for( auto& t : *mGrid4TileMap )
		{
			t = eCellType::Road;
		}

		//
		// Reset View
		//
		resetView();
	}


	void EditorNode::onUpdateTile( Ref* sender, ui::Widget::TouchEventType touch_event_type )
	{
		auto button = static_cast<ui::Button*>( sender );

		Vec2 pos;
		if( ui::Widget::TouchEventType::BEGAN == touch_event_type )
		{
			pos = mTileMapNode->convertToNodeSpace( button->getTouchBeganPosition() );
		}
		else if( ui::Widget::TouchEventType::MOVED == touch_event_type )
		{
			pos = mTileMapNode->convertToNodeSpace( button->getTouchMovePosition() );
		}
		else //if( ui::Widget::TouchEventType::ENDED == touch_event_type || ui::Widget::TouchEventType::CANCELED == touch_event_type )
		{
			pos = mTileMapNode->convertToNodeSpace( button->getTouchEndPosition() );
		}

		const auto point = mPosition2GridIndexConverter.Position2Point( pos.x, pos.y );
		CCLOG( "A : %d, %d", point.x, point.y );

		if( 0 > point.x || mConfig.MapWidth <= point.x || 0 > point.y || mConfig.MapHeight <= point.y )
		{
			return;
		}

		//
		// Put Tile
		//
		switch( mToolIndex )
		{
		case eToolIndex::Wall:
			if( mGrid4TileMap->GetEntryPoint() != point )
			{
				mGrid4TileMap->SetCellType( point.x, point.y, eCellType::Wall );

				const auto tile_point = GetTilePoint( eCellType::Wall );
				mTileMapNode->UpdateTile( point.x, point.y, tile_point.x, tile_point.y );

				updateDebugView();
			}
			break;
		case eToolIndex::Road:
			if( mGrid4TileMap->GetEntryPoint() != point )
			{
				mGrid4TileMap->SetCellType( point.x, point.y, eCellType::Road );

				const auto tile_point = GetTilePoint( eCellType::Road );
				mTileMapNode->UpdateTile( point.x, point.y, tile_point.x, tile_point.y );

				updateDebugView();
			}
			break;
		case eToolIndex::Entry:
			if( mGrid4TileMap->GetExitPoint() != point )
			{

				mGrid4TileMap->SetEntryPoint( point );
				updateEntryPointView();

				const auto tile_point = GetTilePoint( eCellType::Road );
				mTileMapNode->UpdateTile( point.x, point.y, tile_point.x, tile_point.y );

				updateDebugView();
			}
			break;
		case eToolIndex::Exit:
			if( mGrid4TileMap->GetEntryPoint() != point )
			{
				mGrid4TileMap->SetExitPoint( point );
				updateExitPointView();

				const auto tile_point = GetTilePoint( eCellType::Road );
				mTileMapNode->UpdateTile( point.x, point.y, tile_point.x, tile_point.y );

				updateDebugView();
			}
			break;

		default:
			CCASSERT( false, "Invalid Tool Index" );
		}
	}


	void EditorNode::resetView()
	{
		for( std::size_t gy = 0; mGrid4TileMap->GetHeight() > gy; ++gy )
		{
			for( std::size_t gx = 0; mGrid4TileMap->GetWidth() > gx; ++gx )
			{
				const auto& cell_type = mGrid4TileMap->GetCellType( gx, gy );
				const auto tile_point = GetTilePoint( cell_type );

				mTileMapNode->UpdateTile( gx, gy, tile_point.x, tile_point.y );
			}
		}
		updateDebugView();
		updateEntryPointView();
		updateExitPointView();
	}
	void EditorNode::updateDebugView()
	{
		for( int gy = 0; mConfig.MapHeight > gy; ++gy )
		{
			for( int gx = 0; mConfig.MapWidth > gx; ++gx )
			{
				if( eCellType::Road == mGrid4TileMap->GetCellType( gx, gy ) )
				{
					mGridDebugViewNode->UpdateTile( gx, gy, 0, 0 );
				}
				else
				{
					mGridDebugViewNode->UpdateTile( gx, gy, 1, 0 );
				}
			}
		}
	}
	void EditorNode::updateEntryPointView()
	{
		mEntryPointIndicatorNode->setPosition(
			mTileMapNode->getPosition()
			+ Vec2( mTileSheetConfiguration.GetTileWidth() * mGrid4TileMap->GetEntryPoint().x, mTileSheetConfiguration.GetTileHeight() * mGrid4TileMap->GetEntryPoint().y )
		);
	}
	void EditorNode::updateExitPointView()
	{
		mExitPointIndicatorNode->setPosition(
			mTileMapNode->getPosition()
			+ Vec2( mTileSheetConfiguration.GetTileWidth() * mGrid4TileMap->GetExitPoint().x, mTileSheetConfiguration.GetTileHeight() * mGrid4TileMap->GetExitPoint().y )
		);
	}


	void EditorNode::onKeyPressed( EventKeyboard::KeyCode key_code, Event* /*event*/ )
	{
		if( EventKeyboard::KeyCode::KEY_R == key_code )
		{
			onGridClear();
		}
	}
}
