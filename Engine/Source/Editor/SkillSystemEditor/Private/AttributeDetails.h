// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"

class IPropertyHandle;
class FDetailWidgetRow;
class IDetailLayoutBuilder;

DECLARE_LOG_CATEGORY_EXTERN(LogAttributeDetails, Log, All);

class FAttributeDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) OVERRIDE;

	TSharedPtr<FString> GetPropertyType() const;

	TArray<TSharedPtr<FString>> PropertyOptions;

	TSharedPtr<IPropertyHandle> MyProperty;

	void OnChangeProperty(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo);	
};


class FAttributePropertyDetails : public IStructCustomization
{
public:
	static TSharedRef<IStructCustomization> MakeInstance();

	/** IStructCustomization interface */
	virtual void CustomizeStructHeader( TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IStructCustomizationUtils& StructCustomizationUtils ) OVERRIDE;
	virtual void CustomizeStructChildren( TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IStructCustomizationUtils& StructCustomizationUtils ) OVERRIDE;              

private:

	TSharedPtr<IPropertyHandle> MyProperty;
	TArray<TSharedPtr<FString>> PropertyOptions;

	TSharedPtr<FString>	GetPropertyType() const;

	void OnChangeProperty(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo);
};

class FScalableFloatDetails : public IStructCustomization
{
public:
	static TSharedRef<IStructCustomization> MakeInstance();


private:

	virtual void CustomizeStructHeader( TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IStructCustomizationUtils& StructCustomizationUtils ) OVERRIDE;
	virtual void CustomizeStructChildren( TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IStructCustomizationUtils& StructCustomizationUtils ) OVERRIDE;              

	bool IsEditable( ) const;

	TSharedPtr<IPropertyHandle> ValueProperty;
	TSharedPtr<IPropertyHandle> CurveTableHandleProperty;
	TSharedPtr<IPropertyHandle> CurveTableProperty;
	TSharedPtr<IPropertyHandle> RowNameProperty;
};


class FFlexTableRowHandleDetails : public IStructCustomization
{
public:
	static TSharedRef<IStructCustomization> MakeInstance();

	bool IsEditable() const;

private:

	virtual void CustomizeStructHeader( TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IStructCustomizationUtils& StructCustomizationUtils ) OVERRIDE;
	virtual void CustomizeStructChildren( TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IStructCustomizationUtils& StructCustomizationUtils ) OVERRIDE;
	
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<IPropertyHandle> DataTablePropertyHandle;
	
	TSharedPtr<IPropertyHandle> BaseStringPropertyHandle;
	TSharedPtr<IPropertyHandle> RowStringPropertyHandle;

	TSharedPtr<IPropertyHandle> ParentArrayProperty;

	FString LookForName;
	

	TArray<TSharedPtr<FString> > RowNames;

	TSharedPtr<SComboButton> RowNameComboButton;
	TSharedPtr<SListView<TSharedPtr<FString> > > RowNameComboListView;
	TSharedPtr<FString> CurrentSelectedItem;

	TSharedRef<SWidget> GetListContent();
	void OnCurveTableChanged();

	void OnFilterTextChanged( const FText& InFilterText );

	FString GetRowNameComboBoxContentText( ) const;

	void OnSelectionChanged( TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo );

	TSharedRef<ITableRow> HandleRowNameComboBoxGenarateWidget( TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable );


	void OnTest();
};
