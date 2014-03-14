// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

class SGraphNodeK2Composite : public SGraphNodeK2Base
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeK2Composite){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UK2Node_Composite* InNode);

	// SGraphNode interface
	virtual void UpdateGraphNode() OVERRIDE;
	virtual TSharedPtr<SToolTip> GetComplexTooltip() OVERRIDE;
	// End of SGraphNode interface
protected:
	virtual UEdGraph* GetInnerGraph() const;

private:
	FString GetPreviewCornerText() const;
	FString GetTooltipTextForNode() const;

	TSharedRef<SWidget> CreateNodeBody();
};
