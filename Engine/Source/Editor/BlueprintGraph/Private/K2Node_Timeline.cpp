// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#include "BlueprintGraphPrivatePCH.h"
#include "Editor/GraphEditor/Public/DiffResults.h"
#include "Kismet2NameValidators.h"

#include "KismetCompiler.h"

#define LOCTEXT_NAMESPACE "K2Node_Timeline"

/////////////////////////////////////////////////////
// UK2Node_Timeline

UK2Node_Timeline::UK2Node_Timeline(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bAutoPlay = false;
	bLoop = false;
	bReplicated = false;
}

static FString PlayPinName(TEXT("Play"));
static FString PlayFromStartPinName(TEXT("PlayFromStart"));
static FString StopPinName(TEXT("Stop"));
static FString UpdatePinName(TEXT("Update"));
static FString ReversePinName(TEXT("Reverse"));
static FString ReverseFromEndPinName(TEXT("ReverseFromEnd"));
static FString FinishedPinName(TEXT("Finished"));
static FString NewTimePinName(TEXT("NewTime"));
static FString SetNewTimePinName(TEXT("SetNewTime"));
static FString DirectionPinName(TEXT("Direction"));

namespace 
{
	UEdGraphPin* GetPin (const UK2Node_Timeline* Timeline, const FString& PinName, EEdGraphPinDirection DesiredDirection) 
	{
		UEdGraphPin* Pin = Timeline->FindPin(PinName);
		check(Pin != NULL);
		check(Pin->Direction == DesiredDirection);
		return Pin;
	}
}

void UK2Node_Timeline::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	bCanRenameNode = 1;

	CreatePin(EGPD_Input, K2Schema->PC_Exec, TEXT(""), NULL, false, false, PlayPinName);
	CreatePin(EGPD_Input, K2Schema->PC_Exec, TEXT(""), NULL, false, false, PlayFromStartPinName);
	CreatePin(EGPD_Input, K2Schema->PC_Exec, TEXT(""), NULL, false, false, StopPinName);
	CreatePin(EGPD_Input,  K2Schema->PC_Exec, TEXT(""), NULL, false, false, ReversePinName);
	CreatePin(EGPD_Input,  K2Schema->PC_Exec, TEXT(""), NULL, false, false, ReverseFromEndPinName);

	CreatePin(EGPD_Output, K2Schema->PC_Exec, TEXT(""), NULL, false, false, UpdatePinName);
	CreatePin(EGPD_Output, K2Schema->PC_Exec, TEXT(""), NULL, false, false, FinishedPinName);

	CreatePin(EGPD_Input, K2Schema->PC_Exec, TEXT(""), NULL, false, false, SetNewTimePinName);

	UEdGraphPin* NewPositionPin = CreatePin(EGPD_Input, K2Schema->PC_Float, TEXT(""), NULL, false, false, NewTimePinName);
	NewPositionPin->DefaultValue = NewPositionPin->AutogeneratedDefaultValue = TEXT("0.0");

	CreatePin(EGPD_Output, K2Schema->PC_Byte, TEXT(""), FTimeline::GetTimelineDirectionEnum(), false, false, DirectionPinName);

	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint);

	UTimelineTemplate* Timeline = Blueprint->FindTimelineTemplateByVariableName(TimelineName);
	if(Timeline)
	{
		for(int32 i=0; i<Timeline->FloatTracks.Num(); i++)
		{
			FTTFloatTrack& FloatTrack = Timeline->FloatTracks[i];
			CreatePin(EGPD_Output, K2Schema->PC_Float, TEXT(""), NULL, false, false, FloatTrack.TrackName.ToString());			
		}

		for(int32 i=0; i<Timeline->VectorTracks.Num(); i++)
		{
			FTTVectorTrack& VectorTrack = Timeline->VectorTracks[i];
			UScriptStruct* VectorStruct = FindObjectChecked<UScriptStruct>(UObject::StaticClass(), TEXT("Vector"));
			CreatePin(EGPD_Output, K2Schema->PC_Struct, TEXT(""), VectorStruct, false, false, VectorTrack.TrackName.ToString());			
		}

		for(int32 i=0; i<Timeline->LinearColorTracks.Num(); i++)
		{
			FTTLinearColorTrack& LinearColorTrack = Timeline->LinearColorTracks[i];
			UScriptStruct* LinearColorStruct = FindObjectChecked<UScriptStruct>(UObject::StaticClass(), TEXT("LinearColor"));
			CreatePin(EGPD_Output, K2Schema->PC_Struct, TEXT(""), LinearColorStruct, false, false, LinearColorTrack.TrackName.ToString());			
		}

		for(int32 i=0; i<Timeline->EventTracks.Num(); i++)
		{
			FTTEventTrack& EventTrack = Timeline->EventTracks[i];
			CreatePin(EGPD_Output, K2Schema->PC_Exec, TEXT(""), NULL, false, false, EventTrack.TrackName.ToString());			
		}

		// cache play status
		bAutoPlay = Timeline->bAutoPlay;
		bLoop = Timeline->bLoop;
	}

	Super::AllocateDefaultPins();
}

void UK2Node_Timeline::DestroyNode()
{
	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint);
	UTimelineTemplate* Timeline = Blueprint->FindTimelineTemplateByVariableName(TimelineName);
	if(Timeline)
	{
		FBlueprintEditorUtils::RemoveTimeline(Blueprint, Timeline, true);

		// Move template object out of the way so that we can potentially create a timeline with the same name either through a paste or a new timeline action
		Timeline->Rename(NULL, GetTransientPackage(), (Blueprint->bIsRegeneratingOnLoad ? REN_ForceNoResetLoaders : REN_None));
	}

	Super::DestroyNode();
}

void UK2Node_Timeline::PostPasteNode()
{
	Super::PostPasteNode();

	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint);

	UTimelineTemplate* OldTimeline = NULL;

	//find the template with same UUID
	for(TObjectIterator<UTimelineTemplate> It;It;++It)
	{
		UTimelineTemplate* Template = *It;
		if(Template->TimelineGuid == TimelineGuid)
		{
			OldTimeline = Template;
			break;
		}
	}

	// Make sure TimelineName is unique, and we allocate a new timeline template object for this node
	TimelineName = FBlueprintEditorUtils::FindUniqueTimelineName(Blueprint);

	if(!OldTimeline)
	{
		if (UTimelineTemplate* Template = FBlueprintEditorUtils::AddNewTimeline(Blueprint, TimelineName))
		{
			bAutoPlay = Template->bAutoPlay;
			bLoop = Template->bLoop;
		}
	}
	else
	{
		check(NULL != Blueprint->GeneratedClass);
		Blueprint->Modify();
		const FName TimelineTemplateName = *UTimelineTemplate::TimelineVariableNameToTemplateName(TimelineName);
		UTimelineTemplate* Template = DuplicateObject<UTimelineTemplate>(OldTimeline, Blueprint->GeneratedClass, *TimelineTemplateName.ToString());
		bAutoPlay = Template->bAutoPlay;
		bLoop = Template->bLoop;
		Template->SetFlags(RF_Transactional);
		Blueprint->Timelines.Add(Template);

		// Fix up timeline tracks to point to the proper location.  When duplicated, they're still parented to their old blueprints because we don't have the appropriate scope.  Note that we never want to fix up external curve asset references
		{
			for( auto TrackIt = Template->FloatTracks.CreateIterator(); TrackIt; ++TrackIt )
			{
				FTTFloatTrack& Track = *TrackIt;
				if(Track.CurveFloat->GetOuter()->IsA(UBlueprint::StaticClass()))
				{
					Track.CurveFloat->Rename(*Template->MakeUniqueCurveName(Track.CurveFloat), Blueprint, REN_DontCreateRedirectors);
				}
			}

			for( auto TrackIt = Template->EventTracks.CreateIterator(); TrackIt; ++TrackIt )
			{
				FTTEventTrack& Track = *TrackIt;
				if(Track.CurveKeys->GetOuter()->IsA(UBlueprint::StaticClass()))
				{
					Track.CurveKeys->Rename(*Template->MakeUniqueCurveName(Track.CurveKeys), Blueprint, REN_DontCreateRedirectors);
				}
			}

			for( auto TrackIt = Template->VectorTracks.CreateIterator(); TrackIt; ++TrackIt )
			{
				FTTVectorTrack& Track = *TrackIt;
				if(Track.CurveVector->GetOuter()->IsA(UBlueprint::StaticClass()))
				{
					Track.CurveVector->Rename(*Template->MakeUniqueCurveName(Track.CurveVector), Blueprint, REN_DontCreateRedirectors);
				}
			}

			for( auto TrackIt = Template->LinearColorTracks.CreateIterator(); TrackIt; ++TrackIt )
			{
				FTTLinearColorTrack& Track = *TrackIt;
				if(Track.CurveLinearColor->GetOuter()->IsA(UBlueprint::StaticClass()))
				{
					Track.CurveLinearColor->Rename(*Template->MakeUniqueCurveName(Track.CurveLinearColor), Blueprint, REN_DontCreateRedirectors);
				}
			}
		}

		FBlueprintEditorUtils::ValidateBlueprintChildVariables(Blueprint, TimelineName);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}
}

bool UK2Node_Timeline::CanPasteHere(const UEdGraph* TargetGraph, const UEdGraphSchema* Schema) const
{
	if(Super::CanPasteHere(TargetGraph, Schema))
	{
		UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
		if(Blueprint)
		{
			const UEdGraphSchema_K2* K2Schema = Cast<UEdGraphSchema_K2>(Schema);
			check(K2Schema);

			const bool bSupportsEventGraphs = FBlueprintEditorUtils::DoesSupportEventGraphs(Blueprint);
			const bool bAllowEvents = K2Schema->GetGraphType(TargetGraph) == GT_Ubergraph && bSupportsEventGraphs &&
				Blueprint->BlueprintType != BPTYPE_MacroLibrary;

			if(bAllowEvents)
			{
				return true;
			}
			else
			{
				bool bCompositeOfUbberGraph = false;

				//If the composite has a ubergraph in its outer, it is allowed to have timelines
				if (bSupportsEventGraphs && K2Schema->IsCompositeGraph(TargetGraph))
				{
					while (TargetGraph)
					{
						if (UK2Node_Composite* Composite = Cast<UK2Node_Composite>(TargetGraph->GetOuter()))
						{
							TargetGraph =  Cast<UEdGraph>(Composite->GetOuter());
						}
						else if (K2Schema->GetGraphType(TargetGraph) == GT_Ubergraph)
						{
							bCompositeOfUbberGraph = true;
							break;
						}
						else
						{
							TargetGraph = Cast<UEdGraph>(TargetGraph->GetOuter());
						}
					}
				}
				return bCompositeOfUbberGraph;
			}
		}
	}

	return false;
}

FLinearColor UK2Node_Timeline::GetNodeTitleColor() const
{
	return FLinearColor(1.0f, 0.51f, 0.0f);
}

FString UK2Node_Timeline::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return TimelineName.ToString();
}

UEdGraphPin* UK2Node_Timeline::GetDirectionPin() const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	check(NULL != K2Schema);

	UEdGraphPin* Pin = FindPin(DirectionPinName);
	if(Pin && K2Schema)
	{
		const bool bIsOutput = (EGPD_Output == Pin->Direction);
		const bool bProperType = (K2Schema->PC_Byte == Pin->PinType.PinCategory);
		const bool bProperSubCategoryObj = (Pin->PinType.PinSubCategoryObject == FTimeline::GetTimelineDirectionEnum());
		if(bIsOutput && bProperType && bProperSubCategoryObj)
		{
			return Pin;
		}
	}
	return NULL;
}

UEdGraphPin* UK2Node_Timeline::GetPlayPin() const
{
	return GetPin(this, PlayPinName, EGPD_Input);
}

UEdGraphPin* UK2Node_Timeline::GetPlayFromStartPin() const
{
	return GetPin(this, PlayFromStartPinName, EGPD_Input);
}

UEdGraphPin* UK2Node_Timeline::GetStopPin() const
{
	return GetPin(this, StopPinName, EGPD_Input);
}

UEdGraphPin* UK2Node_Timeline::GetReversePin() const
{
	return GetPin(this, ReversePinName, EGPD_Input);
}

UEdGraphPin* UK2Node_Timeline::GetReverseFromEndPin() const
{
	return GetPin(this, ReverseFromEndPinName, EGPD_Input);
}

UEdGraphPin* UK2Node_Timeline::GetUpdatePin() const
{
	return GetPin(this, UpdatePinName, EGPD_Output);
}

UEdGraphPin* UK2Node_Timeline::GetFinishedPin() const
{
	return GetPin(this, FinishedPinName, EGPD_Output);
}

UEdGraphPin* UK2Node_Timeline::GetNewTimePin() const
{
	return GetPin(this, NewTimePinName, EGPD_Input);
}

UEdGraphPin* UK2Node_Timeline::GetSetNewTimePin() const
{
	return GetPin(this, SetNewTimePinName, EGPD_Input);
}

bool UK2Node_Timeline::RenameTimeline (const FString& NewName)
{
	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint);

	FName NewTimelineName(*NewName);
	if (FBlueprintEditorUtils::RenameTimeline(Blueprint, TimelineName, NewTimelineName))
	{
		// Clear off any existing error message now the timeline has been renamed
		this->ErrorMsg.Empty();
		this->bHasCompilerMessage = false;

		return true;
	}
	return false;
}

void UK2Node_Timeline::PrepareForCopying() 
{
	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint);
	//Set the GUID so we can identify which timeline template the copied node should use
	UTimelineTemplate* Template  = Blueprint->FindTimelineTemplateByVariableName(TimelineName);
	check(Template);
	TimelineGuid = Template->TimelineGuid =  FGuid::NewGuid(); //refresh the guid 
}

//Determine if all the tracks contained with both arrays are identical
template<class T>
void FindExactTimelineDifference(struct FDiffResults& Results, FDiffSingleResult Result, const TArray<T>& Tracks1, const TArray<T>& Tracks2, FString TrackTypeStr)
{
	if(Tracks1.Num() != Tracks2.Num())
	{
		FString NodeName = *Result.Node1->GetNodeTitle(ENodeTitleType::ListView);
		Result.Diff = EDiffType::TIMELINE_NUM_TRACKS;
		Result.ToolTip =  FString::Printf(*LOCTEXT("DIF_TimelineNumTracksToolTip", "The number of %s tracks in Timeline '%s' has changed").ToString(), *TrackTypeStr, *NodeName);
		Result.DisplayColor = FLinearColor(0.05f,0.261f,0.775f);
		Result.DisplayString = FString::Printf(*LOCTEXT("DIF_TimelineNumTracks", "%s Track Count '%s'").ToString(), *TrackTypeStr, *NodeName);
		Results.Add(Result);
		return;
	}

	for(int32 i = 0;i<Tracks1.Num();++i)
	{
		if(!(Tracks1[i] == Tracks2[i]))
		{
			FName TrackName = Tracks2[i].TrackName;
			FString NodeName = *Result.Node1->GetNodeTitle(ENodeTitleType::ListView);
			Result.Diff = EDiffType::TIMELINE_TRACK_MODIFIED;
			Result.ToolTip =  FString::Printf(*LOCTEXT("DIF_TimelineTrackModifiedToolTip", "Track '%s' of Timeline '%s' was Modified").ToString(), *TrackName.ToString(),  *NodeName);
			Result.DisplayColor = FLinearColor(0.75f,0.1f,0.15f);
			Result.DisplayString = FString::Printf(*LOCTEXT("DIF_TimelineTrackModified", "Track Modified '%s'").ToString(), *TrackName.ToString());;
			Results.Add(Result);
			break;
		}
	}
}

void UK2Node_Timeline::FindDiffs( class UEdGraphNode* OtherNode, struct FDiffResults& Results )  
{
	UK2Node_Timeline* Timeline1 = this;
	UK2Node_Timeline* Timeline2 = Cast<UK2Node_Timeline>(OtherNode);

	UBlueprint* Blueprint1 = Timeline1->GetBlueprint();
	int32 Index1 = FBlueprintEditorUtils::FindTimelineIndex(Blueprint1,Timeline1->TimelineName);

	UBlueprint* Blueprint2 = Timeline2->GetBlueprint();
	int32 Index2 = FBlueprintEditorUtils::FindTimelineIndex(Blueprint2,Timeline2->TimelineName);
	if(Index1 != INDEX_NONE && Index2 != INDEX_NONE)
	{
		UTimelineTemplate* Template1 = Blueprint1->Timelines[Index1];
		UTimelineTemplate* Template2 = Blueprint2->Timelines[Index2];

		FDiffSingleResult Diff;
		Diff.Node1 = Timeline2;
		Diff.Node2 = Timeline1;

		if(Template1->bAutoPlay != Template2->bAutoPlay)
		{
			Diff.Diff = EDiffType::TIMELINE_AUTOPLAY;
			FString NodeName = *GetNodeTitle(ENodeTitleType::ListView);
			Diff.ToolTip =  FString::Printf(*LOCTEXT("DIF_TimelineAutoPlayToolTip", "Timeline '%s' had its AutoPlay state changed").ToString(), *NodeName);
			Diff.DisplayColor = FLinearColor(0.15f,0.61f,0.15f);
			Diff.DisplayString = FString::Printf(*LOCTEXT("DIF_TimelineAutoPlay", "Timeline AutoPlay Changed '%s'").ToString(), *NodeName);
			Results.Add(Diff);
		}
		if(Template1->bLoop != Template2->bLoop)
		{
			Diff.Diff = EDiffType::TIMELINE_LOOP;
			FString NodeName = *GetNodeTitle(ENodeTitleType::ListView);
			Diff.ToolTip =  FString::Printf(*LOCTEXT("DIF_TimelineLoopingToolTip", "Timeline '%s' had its looping state changed").ToString(), *NodeName);
			Diff.DisplayColor = FLinearColor(0.75f,0.1f,0.75f);
			Diff.DisplayString =  FString::Printf(*LOCTEXT("DIF_TimelineLooping", "Timeline Loop Changed '%s'").ToString(), *NodeName);
			Results.Add(Diff);
		}
		if(Template1->TimelineLength != Template2->TimelineLength)
		{
			FString NodeName = *GetNodeTitle(ENodeTitleType::ListView);
			Diff.Diff = EDiffType::TIMELINE_LENGTH;
			Diff.ToolTip = FString::Printf(*LOCTEXT("DIF_TimelineLengthToolTip", "Length of Timeline '%s' has changed. Was %f, but is now %f").ToString(), *NodeName,Template1->TimelineLength, Template2->TimelineLength);
			Diff.DisplayColor = FLinearColor(0.25f,0.1f,0.15f);
			Diff.DisplayString =  FString::Printf(*LOCTEXT("DIF_TimelineLength", "Timeline Length '%s' [%f -> %f]").ToString(), *NodeName, Template1->TimelineLength, Template2->TimelineLength);
			Results.Add(Diff);
		}

		//something specific inside has changed
		if(Diff.Diff == EDiffType::NO_DIFFERENCE)
		{
			FindExactTimelineDifference(Results, Diff, Template1->EventTracks, Template2->EventTracks, LOCTEXT("Event", "Event").ToString());
			FindExactTimelineDifference(Results, Diff, Template1->FloatTracks, Template2->FloatTracks, LOCTEXT("Float", "Float").ToString());
			FindExactTimelineDifference(Results, Diff, Template1->VectorTracks, Template2->VectorTracks,  LOCTEXT("Vector", "Vector").ToString() );
		}
		
	}
}

void UK2Node_Timeline::OnRenameNode(const FString& NewName)
{
	RenameTimeline(NewName);
}

TSharedPtr<class INameValidatorInterface> UK2Node_Timeline::MakeNameValidator() const
{
	return MakeShareable(new FKismetNameValidator(GetBlueprint(), TimelineName));
}

FNodeHandlingFunctor* UK2Node_Timeline::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FNodeHandlingFunctor(CompilerContext);
}

void UK2Node_Timeline::ExpandForPin(UEdGraphPin* TimelinePin, const FName PropertyName, FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	if (TimelinePin && TimelinePin->LinkedTo.Num() > 0)
	{
		UK2Node_VariableGet* GetVarNode = CompilerContext.SpawnIntermediateNode<UK2Node_VariableGet>(this, SourceGraph);
		GetVarNode->VariableReference.SetSelfMember(PropertyName);
		GetVarNode->AllocateDefaultPins();
		UEdGraphPin* ValuePin = GetVarNode->GetValuePin();
		if (NULL != ValuePin)
		{
			CompilerContext.CheckConnectionResponse(CompilerContext.GetSchema()->MovePinLinks(*TimelinePin, *ValuePin), this);
		}
		else
		{
			CompilerContext.MessageLog.Error(*LOCTEXT("ExpandForPin_Error", "ExpandForPin error, no property found for @@").ToString(), TimelinePin);
		}
	}
}

void UK2Node_Timeline::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	UBlueprint* Blueprint = GetBlueprint();
	check(Blueprint);

	UTimelineTemplate* Timeline = Blueprint->FindTimelineTemplateByVariableName(TimelineName);
	if(Timeline)
	{
		ExpandForPin(GetDirectionPin(), Timeline->GetDirectionPropertyName(), CompilerContext, SourceGraph);

		for(int32 i=0; i<Timeline->FloatTracks.Num(); i++)
		{
			const FName TrackName = Timeline->FloatTracks[i].TrackName;
			ExpandForPin(FindPin(TrackName.ToString()), Timeline->GetTrackPropertyName(TrackName), CompilerContext, SourceGraph);
		}

		for(int32 i=0; i<Timeline->VectorTracks.Num(); i++)
		{
			const FName TrackName = Timeline->VectorTracks[i].TrackName;
			ExpandForPin(FindPin(TrackName.ToString()), Timeline->GetTrackPropertyName(TrackName), CompilerContext, SourceGraph);
		}

		for(int32 i=0; i<Timeline->LinearColorTracks.Num(); i++)
		{
			const FName TrackName = Timeline->LinearColorTracks[i].TrackName;
			ExpandForPin(FindPin(TrackName.ToString()), Timeline->GetTrackPropertyName(TrackName), CompilerContext, SourceGraph);
		}
	}
}

FString UK2Node_Timeline::GetTooltip() const
{
	return LOCTEXT("TimelineTooltip", "Timeline node allows values to be keyframed over time.\nDouble click to open timeline editor.").ToString();
}

FString UK2Node_Timeline::GetDocumentationExcerptName() const
{
	return TEXT("UK2Node_Timeline");
}

#undef LOCTEXT_NAMESPACE
