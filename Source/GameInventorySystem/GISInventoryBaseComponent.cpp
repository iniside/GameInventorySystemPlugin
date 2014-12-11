// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "GISGlobalTypes.h"

#include "GISItemData.h"
#include "IGISPickupItem.h"
#include "Widgets/GISContainerBaseWidget.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "GISInventoryBaseComponent.h"

UGISInventoryBaseComponent::UGISInventoryBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
}


void UGISInventoryBaseComponent::InitializeComponent()
{
	Super::InitializeComponent();
	InitializeInventory();
	if (InventoryContainerClass)
	{
		InventoryContainer = ConstructObject<UGISContainerBaseWidget>(InventoryContainerClass, this);
		if (InventoryContainer)
		{
			ULocalPlayer* Player = World->GetFirstLocalPlayerFromController(); //temporary
			InventoryContainer->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
			InventoryContainer->Initialize();
			InventoryContainer->InventoryComponent = this;

			//call last
			InventoryContainer->InitializeContainer();
		}
	}
}
void UGISInventoryBaseComponent::PostInitProperties()
{
	Super::PostInitProperties();
	
}

TArray<FGISSlotInfo> UGISInventoryBaseComponent::GetInventoryArray()
{
	//TArray<FGISSlotInfo> emptyArray;
	return ItemsInInventory;
}

void UGISInventoryBaseComponent::PickItem(AActor* PickupItemIn)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerPickItem(PickupItemIn);
	}
	else
	{
		IIGISPickupItem* pickupItem = Cast<IIGISPickupItem>(PickupItemIn);
		if (pickupItem)
		{
			//do something, I'm not sure what yet.
		}
	}
}

void UGISInventoryBaseComponent::ServerPickItem_Implementation(AActor* PickupItemIn)
{
	PickItem(PickupItemIn);
}
bool UGISInventoryBaseComponent::ServerPickItem_Validate(AActor* PickupItemIn)
{
	return true;
}
void UGISInventoryBaseComponent::AddItemToInventory(class UGISItemData* ItemIn)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerAddItemToInventory(ItemIn);
	}
	else
	{
		//add new item to first free slot.
		for (FGISSlotInfo& slot : ItemsInInventory)
		{
			if (slot.ItemData == nullptr)
			{
				slot.ItemData = ItemIn;
				ClientUpdateInventory(slot.SlotIndex, slot.ItemData);
				return;
			}
		}
	}
}

void UGISInventoryBaseComponent::ServerAddItemToInventory_Implementation(class UGISItemData* ItemIn)
{
	AddItemToInventory(ItemIn);
}

bool UGISInventoryBaseComponent::ServerAddItemToInventory_Validate(class UGISItemData* ItemIn)
{
	return true;
}


void UGISInventoryBaseComponent::AddItemOnSlot(int32 TargetSlotIndex, const FGISSlotInfo& TargetSlotType, int32 LastSlotIndex, const FGISSlotInfo& LastSlotType)
{
	if (ItemsInInventory[TargetSlotType.SlotIndex].ItemData == nullptr)
	{
		UGISItemData* TargetItem = ItemsInInventory[LastSlotType.SlotIndex].ItemData;
		UGISItemData* LastItem = ItemsInInventory[TargetSlotType.SlotIndex].ItemData;

		ItemsInInventory[TargetSlotType.SlotIndex].ItemData = TargetItem;
		ItemsInInventory[LastSlotType.SlotIndex].ItemData = nullptr;

		FGISSlotSwapInfo SlotSwapInfo;

		SlotSwapInfo.LastSlotIndex = LastSlotType.SlotIndex;
		SlotSwapInfo.LastSlotData = LastItem;
		SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
		SlotSwapInfo.TargetSlotIndex = TargetSlotType.SlotIndex;
		SlotSwapInfo.TargetSlotData = TargetItem;
		SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
		ClientSlotSwap(SlotSwapInfo);
	}
	else
	{
		UGISItemData* TargetItem = ItemsInInventory[LastSlotType.SlotIndex].ItemData;
		UGISItemData* LastItem = ItemsInInventory[TargetSlotType.SlotIndex].ItemData;

		ItemsInInventory[TargetSlotType.SlotIndex].ItemData = TargetItem;
		ItemsInInventory[LastSlotType.SlotIndex].ItemData = LastItem;

		FGISSlotSwapInfo SlotSwapInfo;
		SlotSwapInfo.LastSlotIndex = LastSlotType.SlotIndex;
		SlotSwapInfo.LastSlotData = LastItem;
		SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
		SlotSwapInfo.TargetSlotIndex = TargetSlotType.SlotIndex;
		SlotSwapInfo.TargetSlotData = TargetItem;
		SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
		ClientSlotSwap(SlotSwapInfo);
	}
}

void UGISInventoryBaseComponent::ServerAddItemOnSlot_Implementation(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType)
{
	AddItemOnSlot(TargetSlot, TargetSlotType, LastSlot, LastSlotType);
}
bool UGISInventoryBaseComponent::ServerAddItemOnSlot_Validate(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType)
{
	return true;
}

void UGISInventoryBaseComponent::ClientUpdateInventory_Implementation(int32 NewSlot, class UGISItemData* ItemDataOut)
{
	OnItemAdded.Broadcast(NewSlot, ItemDataOut);
}
void UGISInventoryBaseComponent::ClientSlotSwap_Implementation(const FGISSlotSwapInfo& SlotSwapInfo)
{
	OnItemSlotSwapped.Broadcast(SlotSwapInfo);
}


void UGISInventoryBaseComponent::PostInventoryInitialized()
{

}

void UGISInventoryBaseComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UGISInventoryBaseComponent, ItemsInInventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGISInventoryBaseComponent, Tabs, COND_OwnerOnly);
}

bool UGISInventoryBaseComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const FGISSlotInfo& item : ItemsInInventory)
	{
		if (item.ItemData)
		{
			WroteSomething |= Channel->ReplicateSubobject(const_cast<UGISItemData*>(item.ItemData), *Bunch, *RepFlags);
		}
	}

	for (const FGISTabInfo& TabInfo : Tabs.InventoryTabs)
	{
		for (const FGISSlotInfo& SlotItem : TabInfo.TabSlots)
		{
			if (SlotItem.ItemData)
			{
				WroteSomething |= Channel->ReplicateSubobject(const_cast<UGISItemData*>(SlotItem.ItemData), *Bunch, *RepFlags);
			}
		}
	}
	return WroteSomething;
}
void UGISInventoryBaseComponent::GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs)
{
	for (const FGISSlotInfo& item : ItemsInInventory)
	{
		if (item.ItemData && item.ItemData->IsNameStableForNetworking())
		{
			Objs.Add(const_cast<UGISItemData*>(item.ItemData));
		}
	}
	for (const FGISTabInfo& TabInfo : Tabs.InventoryTabs)
	{
		for (const FGISSlotInfo& SlotItem : TabInfo.TabSlots)
		{
			if (SlotItem.ItemData)
			{
				Objs.Add(const_cast<UGISItemData*>(SlotItem.ItemData));
			}
		}
	}
}

void UGISInventoryBaseComponent::InitializeInventory()
{
	for (int32 Index = 0; Index < InventorySize; Index++)
	{
		FGISSlotInfo newItem;
		//newItem.LastSlotIndex = INDEX_NONE;
		newItem.SlotIndex = Index;
		newItem.ItemData = nullptr;
		newItem.CurrentInventoryComponent = this;
		ItemsInInventory.Add(newItem);
	}
}

void UGISInventoryBaseComponent::InitializeInventoryTabs()
{
	int8 counter = 0;
	for (const FGISSlotsInTab& SlotInfo : InitialTabInfo)
	{

		FGISTabInfo TabInfo;
		TabInfo.NumberOfSlots = SlotInfo.NumberOfSlots;
		TabInfo.TabIndex = counter;

		for (int32 Index = 0; Index < SlotInfo.NumberOfSlots; Index++)
		{
			FGISSlotInfo SlotInfo;
			SlotInfo.SlotIndex = Index;
			SlotInfo.ItemData = nullptr;
			TabInfo.TabSlots.Add(SlotInfo);
		}

		Tabs.InventoryTabs.Add(TabInfo);
		counter++;
	}
}