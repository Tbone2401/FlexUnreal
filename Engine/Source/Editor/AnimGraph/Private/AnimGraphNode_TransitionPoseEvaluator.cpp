// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "AnimGraphPrivatePCH.h"

#include "CompilerResultsLog.h"
#include "GraphEditorActions.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_TransitionPoseEvaluator

UAnimGraphNode_TransitionPoseEvaluator::UAnimGraphNode_TransitionPoseEvaluator(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}

FLinearColor UAnimGraphNode_TransitionPoseEvaluator::GetNodeTitleColor() const
{
	return FColor(200, 100, 100);
}

FString UAnimGraphNode_TransitionPoseEvaluator::GetTooltip() const
{
	return (Node.DataSource == EEvaluatorDataSource::EDS_DestinationPose) ? TEXT("Evaluates and returns the pose generated by the destination state of this transition") : TEXT("Evaluates and returns the pose generated by the setup prior to this transition firing");
}

FString UAnimGraphNode_TransitionPoseEvaluator::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::FullTitle)
	{
		return (Node.DataSource == EEvaluatorDataSource::EDS_DestinationPose) ? TEXT("Get Destination State Pose") : TEXT("Get Source State Pose");
	}

	return TEXT("Input Pose Evaluator");
}

void UAnimGraphNode_TransitionPoseEvaluator::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
	if ((Node.EvaluatorMode != EEvaluatorMode::EM_Standard) && Node.FramesToCachePose < 1)
	{
		MessageLog.Error(TEXT("@@ is set to a mode that caches the pose, but frames to cache is less then 1."), this);
	}

	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FString UAnimGraphNode_TransitionPoseEvaluator::GetNodeCategory() const 
{
	return TEXT("Transition");
}

bool UAnimGraphNode_TransitionPoseEvaluator::CanUserDeleteNode() const
{
	// Allow deleting the node if we're in the wrong kind of graph (via some accident or regression)
	return !(GetGraph()->IsA(UAnimationCustomTransitionGraph::StaticClass()));
}

void UAnimGraphNode_TransitionPoseEvaluator::GetMenuEntries(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	// Intentionally empty: Don't allow an option to create them, as they're auto-created when custom blend graphs are made
}
