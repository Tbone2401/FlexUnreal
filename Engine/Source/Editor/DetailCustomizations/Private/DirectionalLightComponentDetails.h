// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

class FDirectionalLightComponentDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) OVERRIDE;

private:

	bool IsLightMovable() const;
	bool IsLightStationary() const;

	TSharedPtr<IPropertyHandle> MobilityProperty;

};
