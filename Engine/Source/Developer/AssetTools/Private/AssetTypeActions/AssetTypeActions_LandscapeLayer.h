// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

class FAssetTypeActions_LandscapeLayer : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const OVERRIDE { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LandscapeLayer", "Landscape Layer"); }
	virtual FColor GetTypeColor() const OVERRIDE { return FColor(128,192,255); }
	virtual UClass* GetSupportedClass() const OVERRIDE { return ULandscapeLayerInfoObject::StaticClass(); }
	virtual uint32 GetCategories() OVERRIDE { return EAssetTypeCategories::Misc; }
};