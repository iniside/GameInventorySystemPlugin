// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "GISGlobalTypes.h"
#include "GISItemData.h"
#include "Widgets/GISContainerBaseWidget.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "GISInventoryComponent.h"

UGISInventoryComponent::UGISInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UGISInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

TArray<FGISSlotInfo> UGISInventoryComponent::GetInventoryArray()
{
	return ItemsInInventory;
}

void UGISInventoryComponent::AddItemToInventory(class UGISItemData* ItemIn)
{

}

void UGISInventoryComponent::ServerAddItemToInventory_Implementation(class UGISItemData* ItemIn)
{
	AddItemToInventory(ItemIn);
}

bool UGISInventoryComponent::ServerAddItemToInventory_Validate(class UGISItemData* ItemIn)
{
	return true;
}


void UGISInventoryComponent::AddItemOnSlot(int32 TargetSlotIndex, const FGISSlotInfo& TargetSlotType, int32 LastSlotIndex, const FGISSlotInfo& LastSlotType)
{

}

void UGISInventoryComponent::ServerAddItemOnSlot_Implementation(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType)
{
	AddItemOnSlot(TargetSlot, TargetSlotType, LastSlot, LastSlotType);
}
bool UGISInventoryComponent::ServerAddItemOnSlot_Validate(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType)
{
	return true;
}