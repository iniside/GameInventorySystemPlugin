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
	InitializeInventoryTabs();
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
	TArray<FGISSlotInfo> emptyArray;
	return emptyArray;
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
		//add item to first empty slot in first matching tab.
		for (FGISTabInfo& TabInfo : Tabs.InventoryTabs)
		{
			for (FGISSlotInfo& Slot : TabInfo.TabSlots)
			{
				if (Slot.ItemData == nullptr)
				{
					Slot.ItemData = ItemIn;
					FGISSlotUpdateData SlotUpdateData;
					SlotUpdateData.TabIndex = TabInfo.TabIndex;
					SlotUpdateData.SlotIndex = Slot.SlotIndex;
					SlotUpdateData.SlotData = Slot.ItemData;
					SlotUpdateData.SlotComponent = this;
					ClientUpdateInventory(SlotUpdateData);
					return;
				}
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


void UGISInventoryBaseComponent::AddItemOnSlot(const FGISSlotInfo& TargetSlotType, const FGISSlotInfo& LastSlotType)
{
	//Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotTabIndex].ItemData
	if (Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData == nullptr)
	{
		UGISItemData* TargetItem = Tabs.InventoryTabs[LastSlotType.SlotTabIndex].TabSlots[LastSlotType.SlotIndex].ItemData;
		UGISItemData* LastItem = Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData;

		Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData = TargetItem;
		Tabs.InventoryTabs[LastSlotType.SlotTabIndex].TabSlots[LastSlotType.SlotIndex].ItemData = nullptr;

		FGISSlotSwapInfo SlotSwapInfo;

		SlotSwapInfo.LastSlotIndex = LastSlotType.SlotIndex;
		SlotSwapInfo.LastTabIndex = LastSlotType.SlotTabIndex;
		SlotSwapInfo.LastSlotData = LastItem;
		SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
		SlotSwapInfo.TargetSlotIndex = TargetSlotType.SlotIndex;
		SlotSwapInfo.TargetTabIndex = TargetSlotType.SlotTabIndex;
		SlotSwapInfo.TargetSlotData = TargetItem;
		SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
		ClientSlotSwap(SlotSwapInfo);
	}
	else
	{
		UGISItemData* TargetItem = Tabs.InventoryTabs[LastSlotType.SlotTabIndex].TabSlots[LastSlotType.SlotIndex].ItemData;
		UGISItemData* LastItem = Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData;

		Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData = TargetItem;
		Tabs.InventoryTabs[LastSlotType.SlotTabIndex].TabSlots[LastSlotType.SlotIndex].ItemData = LastItem;

		FGISSlotSwapInfo SlotSwapInfo;
		SlotSwapInfo.LastSlotIndex = LastSlotType.SlotIndex;
		SlotSwapInfo.LastTabIndex = LastSlotType.SlotTabIndex;
		SlotSwapInfo.LastSlotData = LastItem;
		SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
		SlotSwapInfo.TargetSlotIndex = TargetSlotType.SlotIndex;
		SlotSwapInfo.TargetTabIndex = TargetSlotType.SlotTabIndex;
		SlotSwapInfo.TargetSlotData = TargetItem;
		SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
		ClientSlotSwap(SlotSwapInfo);
	}
}

void UGISInventoryBaseComponent::ServerAddItemOnSlot_Implementation(const FGISSlotInfo& TargetSlotType, const FGISSlotInfo& LastSlotType)
{
	AddItemOnSlot(TargetSlotType, LastSlotType);
}
bool UGISInventoryBaseComponent::ServerAddItemOnSlot_Validate(const FGISSlotInfo& TargetSlotType, const FGISSlotInfo& LastSlotType)
{
	return true;
}

void UGISInventoryBaseComponent::ClientUpdateInventory_Implementation(const FGISSlotUpdateData& SlotUpdateInfo)
{
	OnItemAdded.Broadcast(SlotUpdateInfo);
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

	DOREPLIFETIME_CONDITION(UGISInventoryBaseComponent, Tabs, COND_OwnerOnly);
}

bool UGISInventoryBaseComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

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
			SlotInfo.SlotTabIndex = counter;
			SlotInfo.CurrentInventoryComponent = this;
			SlotInfo.ItemData = nullptr;
			TabInfo.TabSlots.Add(SlotInfo);
		}

		Tabs.InventoryTabs.Add(TabInfo);
		counter++;
	}
}