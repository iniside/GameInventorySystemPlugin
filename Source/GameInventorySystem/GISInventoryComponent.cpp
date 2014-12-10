// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "GISGlobalTypes.h"
#include "GISItemData.h"
#include "Widgets/GISContainerBase.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "GISInventoryComponent.h"

UGISInventoryComponent::UGISInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	InventorySize = 50;
	InitializeInventory();
}

void UGISInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (InventoryContainerClass)
	{
		InventoryContainer = ConstructObject<UGISContainerBase>(InventoryContainerClass, this);
		if (InventoryContainer)
		{
			ULocalPlayer* Player = World->GetFirstLocalPlayerFromController(); //temporary
			InventoryContainer->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
			InventoryContainer->Initialize();
			InventoryContainer->InventoryComponent = this;
		}
	}
}

void UGISInventoryComponent::AddItemToInventory(class UGISItemData* ItemIn)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerAddItemToInventory(ItemIn);
	}
	else
	{
		for (FGISInventorySlot& Item : ItemsInInventory)
		{
			if (Item.ItemData == nullptr)
			{
				Item.ItemData = ItemIn;
				ClientUpdateInventory(Item.SlotIndex, Item.ItemData);
				return;
			}
		}
	}
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
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerAddItemOnSlot(TargetSlotIndex, TargetSlotType, LastSlotIndex, LastSlotType);
	}
	else
	{
		//frankly this looks ugly shit.
		/*
		This is how it works.
		Each widget slot have assigned struct, which describe it's type.

		This struct doesn't do anything in particular beyond holdind it's type in int32
		pointer to data store object, and index in array.
		But it can be easily extended to add your own functionality.

		When you drag item from inventory, last slot type is derived from last widget slot,
		and it's passed to this function.
		Here we check slot type, and determine interaction between slots (how TargetSlot
		will interact with last slot).

		It's more or less generic way to code interaction between various elements of UI.
		You can have your normal inventory which store items.
		You can have another inventory which store abilities.
		And at very least you can have inventory, which is essentialy hotbar.
		For example you might not want to let use drag items from inventory to hotbar and to
		ability inventory ;).
		Ot you mihgt want to allow user to drag items from item and ability inventory to hotbar,
		but not between them.
		*/

		if (ItemsInInventory[TargetSlotIndex].ItemData == nullptr)
		{
			UGISItemData* TargetItem = ItemsInInventory[LastSlotIndex].ItemData;
			UGISItemData* LastItem = ItemsInInventory[TargetSlotIndex].ItemData;

			ItemsInInventory[TargetSlotIndex].ItemData = TargetItem;
			ItemsInInventory[LastSlotIndex].ItemData = nullptr;

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
			UGISItemData* TargetItem = ItemsInInventory[LastSlotIndex].ItemData;
			UGISItemData* LastItem = ItemsInInventory[TargetSlotIndex].ItemData;

			ItemsInInventory[TargetSlotIndex].ItemData = TargetItem;
			ItemsInInventory[LastSlotIndex].ItemData = LastItem;

			FGISSlotSwapInfo SlotSwapInfo;
			SlotSwapInfo.LastSlotIndex = LastSlotIndex;
			SlotSwapInfo.LastSlotData = LastItem;
			SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
			SlotSwapInfo.TargetSlotIndex = TargetSlotIndex;
			SlotSwapInfo.TargetSlotData = TargetItem;
			SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
			ClientSlotSwap(SlotSwapInfo);
		}
		//if (TargetSlotType.IsOfType(FGISInventorySlot::SlotTypeID))
		//{
		//	FGISInventorySlot* const TargetSlot = (FGISInventorySlot*)&TargetSlotType;

		//	if (LastSlotType.IsOfType(FGISInventorySlot::SlotTypeID))
		//	{
		//		FGISInventorySlot* const LastSlot = (FGISInventorySlot*)&LastSlotType;
		//		if (ItemsInInventory[TargetSlotIndex].ItemData == nullptr)
		//		{
		//			UGISItemData* TargetItem = ItemsInInventory[LastSlotIndex].ItemData;
		//			UGISItemData* LastItem = ItemsInInventory[TargetSlotIndex].ItemData;

		//			ItemsInInventory[TargetSlotIndex].ItemData = TargetItem;
		//			ItemsInInventory[LastSlotIndex].ItemData = nullptr;
		//			ClientSlotSwap(LastSlotIndex, LastItem, TargetSlotIndex, TargetItem);
		//		}
		//		else
		//		{
		//			UGISItemData* TargetItem = ItemsInInventory[LastSlotIndex].ItemData;
		//			UGISItemData* LastItem = ItemsInInventory[TargetSlotIndex].ItemData;

		//			ItemsInInventory[TargetSlotIndex].ItemData = TargetItem;
		//			ItemsInInventory[LastSlotIndex].ItemData = LastItem;
		//			ClientSlotSwap(LastSlotIndex, LastItem, TargetSlotIndex, TargetItem);
		//		}
		//	}
		//	else if (LastSlotType.IsOfType(FGISItemHotbar::SlotTypeID))
		//	{
		//		FGISItemHotbar* const LasttSlot = (FGISItemHotbar*)&LastSlotType;
		//	}
		//}
		//if (TargetSlotType.IsOfType(FGISItemHotbar::SlotTypeID))
		//{
		//	FGISItemHotbar* const TargetSlot = (FGISItemHotbar*)&TargetSlotType;

		//	if (LastSlotType.IsOfType(FGISInventorySlot::SlotTypeID))
		//	{
		//		FGISInventorySlot* const LastSlot = (FGISInventorySlot*)&LastSlotType;
		//	}
		//	else if (LastSlotType.IsOfType(FGISItemHotbar::SlotTypeID))
		//	{
		//		FGISItemHotbar* const LasttSlot = (FGISItemHotbar*)&LastSlotType;
		//	}
		//}
		//if (ItemsInInventory[TargetSlot].Item == nullptr)
		//{
		//	UGISItemData* TargetItem = ItemsInInventory[LastSlot].Item;
		//	UGISItemData* LastItem = ItemsInInventory[TargetSlot].Item;

		//	ItemsInInventory[TargetSlot].Item = TargetItem;
		//	ItemsInInventory[LastSlot].Item = nullptr;
		//	ClientSlotSwap(LastSlot, LastItem, TargetSlot, TargetItem);
		//}
		//else
		//{
		//	UGISItemData* TargetItem = ItemsInInventory[LastSlot].Item;
		//	UGISItemData* LastItem = ItemsInInventory[TargetSlot].Item;

		//	ItemsInInventory[TargetSlot].Item = TargetItem;
		//	ItemsInInventory[LastSlot].Item = LastItem;
		//	ClientSlotSwap(LastSlot, LastItem, TargetSlot, TargetItem);
		//}
	}
}

void UGISInventoryComponent::ServerAddItemOnSlot_Implementation(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType)
{
	AddItemOnSlot(TargetSlot, TargetSlotType, LastSlot, LastSlotType);
}
bool UGISInventoryComponent::ServerAddItemOnSlot_Validate(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType)
{
	return true;
}

void UGISInventoryComponent::InitializeInventory()
{
	for (int32 Index = 0; Index < InventorySize; Index++)
	{
		FGISInventorySlot newItem;
		//newItem.LastSlotIndex = INDEX_NONE;
		newItem.SlotIndex = Index;
		newItem.ItemData = nullptr;
		newItem.CurrentInventoryComponent = this;
		ItemsInInventory.Add(newItem);
	}
}

void UGISInventoryComponent::InitializeInventoryTabs()
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
	PostInventoryInitialized();
}

void UGISInventoryComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UGISInventoryComponent, ItemsInInventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGISInventoryComponent, Tabs, COND_OwnerOnly);
}

bool UGISInventoryComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const FGISInventorySlot& item : ItemsInInventory)
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
void UGISInventoryComponent::GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs)
{
	for (const FGISInventorySlot& item : ItemsInInventory)
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