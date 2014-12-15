// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "../GISItemData.h"
#include "../GISPickupActor.h"

#include "GISLootSlotBaseWidget.h"

UGISLootSlotBaseWidget::UGISLootSlotBaseWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


UTexture2D* UGISLootSlotBaseWidget::GetIcon() const
{
	if (LootSlotInfo.SlotData)
	{
		return LootSlotInfo.SlotData->GetImage();
	}
	return nullptr;
}


FEventReply UGISLootSlotBaseWidget::OnMouseButtonDown_Implementation(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (LootSlotInfo.OwningPickupActor)
		{
			LootSlotInfo.OwningPickupActor->LootSingleItem(LootSlotInfo.SlotIndex);
		}
	}
	return Reply;
}