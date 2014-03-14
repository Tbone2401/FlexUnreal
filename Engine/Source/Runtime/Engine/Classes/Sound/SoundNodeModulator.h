// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

 
#pragma once
#include "SoundNodeModulator.generated.h"

/** 
 * Defines a random volume and pitch modification when a sound starts
 */
UCLASS(hidecategories=Object, editinlinenew, MinimalAPI, meta=( DisplayName="Modulator" ))
class USoundNodeModulator : public USoundNode
{
	GENERATED_UCLASS_BODY()

	/* The lower bound of pitch (1.0 is no change). */
	UPROPERTY(EditAnywhere, Category=Modulation )
	float PitchMin;

	/* The upper bound of pitch (1.0 is no change). */
	UPROPERTY(EditAnywhere, Category=Modulation )
	float PitchMax;

	/* The lower bound of volume (1.0 is no change). */
	UPROPERTY(EditAnywhere, Category=Modulation )
	float VolumeMin;

	/* The upper bound of volume (1.0 is no change). */
	UPROPERTY(EditAnywhere, Category=Modulation )
	float VolumeMax;

public:
	// Begin USoundNode interface. 
	virtual void ParseNodes( FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances ) OVERRIDE;
	virtual FString GetUniqueString() const OVERRIDE;
	// End USoundNode interface. 
};



