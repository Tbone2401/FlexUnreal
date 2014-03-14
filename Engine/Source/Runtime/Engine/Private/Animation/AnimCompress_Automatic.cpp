// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#include "EnginePrivate.h"
#include "AnimationUtils.h"


UAnimCompress_Automatic::UAnimCompress_Automatic(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Description = TEXT("Automatic");
	MaxEndEffectorError = 1.0f;
	bTryFixedBitwiseCompression = true;
	bTryPerTrackBitwiseCompression = true;
	bTryLinearKeyRemovalCompression = true;
	bTryIntervalKeyRemoval = true;
	bRunCurrentDefaultCompressor = false;
	bAutoReplaceIfExistingErrorTooGreat = false;
	bRaiseMaxErrorToExisting = false;
}


void UAnimCompress_Automatic::DoReduction(UAnimSequence* AnimSeq, const TArray<FBoneData>& BoneData)
{
#if WITH_EDITORONLY_DATA
	FAnimationUtils::CompressAnimSequenceExplicit(
		AnimSeq,
		MaxEndEffectorError,
		false, // bOutput
		bRunCurrentDefaultCompressor,
		bAutoReplaceIfExistingErrorTooGreat,
		bRaiseMaxErrorToExisting,
		bTryFixedBitwiseCompression,
		bTryPerTrackBitwiseCompression,
		bTryLinearKeyRemovalCompression,
		bTryIntervalKeyRemoval);
	AnimSeq->CompressionScheme = static_cast<UAnimCompress*>( StaticDuplicateObject( AnimSeq->CompressionScheme, AnimSeq, TEXT("None")) );
#endif // WITH_EDITORONLY_DATA
}
