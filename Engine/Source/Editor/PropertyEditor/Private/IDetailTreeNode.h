// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

namespace ENodeVisibility
{
	enum Type
	{
		// Hidden but can be visible if parent is visible due to filtering
		HiddenDueToFiltering,
		// Never visible no matter what
		ForcedHidden,
		// Always visible
		Visible,
	};
};

/**
 * Base class of all nodes in the detail tree                                                              
 */
class IDetailTreeNode
{
public:

	virtual ~IDetailTreeNode() {}

	/** @return The details view that this node is in */
	virtual SDetailsView& GetDetailsView() const = 0;

	/**
	 * Generates the widget representing this node
	 *
	 * @param OwnerTable		The table owner of the widget being generated
	 * @param PropertyUtilities	Property utilities to help generate widgets
	 */
	virtual TSharedRef< ITableRow > GenerateNodeWidget( const TSharedRef<STableViewBase>& OwnerTable, const FDetailColumnSizeData& ColumnSizeData, const TSharedRef<IPropertyUtilities>& PropertyUtilities ) = 0;

	/**
	 * Filters this nodes visibility based on the provided filter strings                                                              
	 */
	virtual void FilterNode( const FDetailFilter& InFilter ) = 0;

	/**
	 * Gets child tree nodes 
	 *
	 * @param OutChildren	The array to add children to
	 */
	virtual void GetChildren( TArray< TSharedRef<IDetailTreeNode> >& OutChildren ) = 0;

	/**
	 * Called when the item is expanded in the tree                                                              
	 */
	virtual void OnItemExpansionChanged( bool bIsExpanded ) = 0;

	/**
	 * @return Whether or not the tree node should be expanded                                                              
	 */
	virtual bool ShouldBeExpanded() const = 0;

	/**
	 * @return the visibility of this node in the tree
	 */
	virtual ENodeVisibility::Type GetVisibility() const = 0;

	/**
	 * Called each frame if the node requests that it should be ticked                                                              
	 */
	virtual void Tick( float DeltaTime ) = 0;
	
	/**
	 * @return true to ignore this node for visibility in the tree and only examine children                                                              
	 */
	virtual bool ShouldShowOnlyChildren() const = 0;

	/**
	 * The identifier name of the node                                                              
	 */
	virtual FName GetNodeName() const = 0;
};

