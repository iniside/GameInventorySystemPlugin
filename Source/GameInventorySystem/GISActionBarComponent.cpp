// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "GISGlobalTypes.h"
#include "GISItemData.h"
#include "Widgets/GISContainerBaseWidget.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "GISActionBarComponent.h"

UGISActionBarComponent::UGISActionBarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	for (int32 Index = 0; Index < 8; Index++)
	{
		FGISItemHotbar newItem;
		//newItem.LastSlotIndex = INDEX_NONE;
		newItem.SlotIndex = Index;
		newItem.ItemData = nullptr;
		newItem.CurrentInventoryComponent = this;
		HotBarButtons.Add(newItem);
	}
}

void UGISActionBarComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (InventoryContainerClass)
	{
		InventoryContainer = ConstructObject<UGISContainerBaseWidget>(InventoryContainerClass, this);
		if (InventoryContainer)
		{
			ULocalPlayer* Player = World->GetFirstLocalPlayerFromController();
			InventoryContainer->SetPlayerContext(FLocalPlayerContext(Player));
			InventoryContainer->Initialize();
			InventoryContainer->InventoryComponent = this;

			//call last
			InventoryContainer->InitializeContainer();
		}
	}
}

void UGISActionBarComponent::AddItemOnSlot(int32 TargetSlotIndex, const FGISSlotInfo& TargetSlotType, int32 LastSlotIndex, const FGISSlotInfo& LastSlotType)
{
	if (TargetSlotType.CurrentInventoryComponent != LastSlotType.CurrentInventoryComponent)
	{
		//RemoveItem() from last slot
		//add item on current slot.
		//call back to client with info which components/slots exchanged.
		//to allow client update widgets.
	}

	if (HotBarButtons[TargetSlotIndex].ItemData == nullptr)
	{
		UGISItemData* TargetItem = HotBarButtons[LastSlotIndex].ItemData;
		UGISItemData* LastItem = HotBarButtons[TargetSlotIndex].ItemData;

		HotBarButtons[TargetSlotIndex].ItemData = TargetItem;
		HotBarButtons[LastSlotIndex].ItemData = nullptr;
		FGISSlotSwapInfo SlotSwapInfo;
		SlotSwapInfo.LastSlotIndex = LastSlotIndex;
		SlotSwapInfo.LastSlotData = LastItem;
		SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
		SlotSwapInfo.TargetSlotIndex = TargetSlotIndex;
		SlotSwapInfo.TargetSlotData = TargetItem;
		SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
		ClientSlotSwap(SlotSwapInfo);
	}
	else
	{
		UGISItemData* TargetItem = HotBarButtons[LastSlotIndex].ItemData;
		UGISItemData* LastItem = HotBarButtons[TargetSlotIndex].ItemData;

		HotBarButtons[TargetSlotIndex].ItemData = TargetItem;
		HotBarButtons[LastSlotIndex].ItemData = LastItem;
		FGISSlotSwapInfo SlotSwapInfo;
		SlotSwapInfo.LastSlotIndex = LastSlotIndex;
		SlotSwapInfo.LastSlotData = LastItem;
		SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
		SlotSwapInfo.TargetSlotIndex = TargetSlotIndex;
		SlotSwapInfo.TargetSlotData = TargetItem;
		SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
		ClientSlotSwap(SlotSwapInfo);
	}
}

void UGISActionBarComponent::ServerAddItemOnSlot_Implementation(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType)
{
	AddItemOnSlot(TargetSlot, TargetSlotType, LastSlot, LastSlotType);
}
bool UGISActionBarComponent::ServerAddItemOnSlot_Validate(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType)
{
	return true;
}

void UGISActionBarComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UGISActionBarComponent, HotBarButtons, COND_OwnerOnly);
}
bool UGISActionBarComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const FGISItemHotbar& item : HotBarButtons)
	{
		if (item.ItemData)
		{
			WroteSomething |= Channel->ReplicateSubobject(const_cast<UGISItemData*>(item.ItemData), *Bunch, *RepFlags);
		}
	}
	return WroteSomething;
}
void UGISActionBarComponent::GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs)
{
	for (const FGISItemHotbar& item : HotBarButtons)
	{
		if (item.ItemData && item.ItemData->IsNameStableForNetworking())
		{
			Objs.Add(const_cast<UGISItemData*>(item.ItemData));
		}
	}
}