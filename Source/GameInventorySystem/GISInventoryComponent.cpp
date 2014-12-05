// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "GISItemData.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "GISInventoryComponent.h"

UGISInventoryComponent::UGISInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventorySize = 50;
	InitializeInventory();
}


void UGISInventoryComponent::AddItemToInventory(class UGISItemData* ItemIn)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerAddItemToInventory(ItemIn);
	}
	else
	{
		for (FGISItemDataWrapper& Item : ItemsInInventory)
		{
			if (Item.Item == nullptr)
			{
				Item.Item = ItemIn;
				ClientUpdateInventory(Item.CurrentItemIndex, Item.Item);
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


void UGISInventoryComponent::AddItemOnSlot(int32 TargetSlot, int32 LastSlot)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerAddItemOnSlot(TargetSlot, LastSlot);
	}
	else
	{
		if (ItemsInInventory[TargetSlot].Item == nullptr)
		{
			UGISItemData* TargetItem = ItemsInInventory[LastSlot].Item;
			UGISItemData* LastItem = ItemsInInventory[TargetSlot].Item;

			ItemsInInventory[TargetSlot].Item = TargetItem;

			ClientSlotSwap(LastSlot, LastItem, TargetSlot, TargetItem);
		}
		else
		{
			UGISItemData* TargetItem = ItemsInInventory[LastSlot].Item;
			UGISItemData* LastItem = ItemsInInventory[TargetSlot].Item;

			ItemsInInventory[TargetSlot].Item = TargetItem;
			ItemsInInventory[LastSlot].Item = LastItem;
			ClientSlotSwap(LastSlot, LastItem, TargetSlot, TargetItem);
		}
	}
}

void UGISInventoryComponent::ServerAddItemOnSlot_Implementation(int32 TargetSlot, int32 LastSlot)
{
	AddItemOnSlot(TargetSlot, LastSlot);
}
bool UGISInventoryComponent::ServerAddItemOnSlot_Validate(int32 TargetSlot, int32 LastSlot)
{
	return true;
}

void UGISInventoryComponent::ClientUpdateInventory_Implementation(int32 NewSlot, class UGISItemData* ItemDataOut)
{
	OnItemAdded.Broadcast(NewSlot, ItemDataOut);
}
void UGISInventoryComponent::ClientSlotSwap_Implementation(int32 LastSlotIndex, class UGISItemData* LastSlotData, int32 TargetSlot, class UGISItemData* TargetSlotData)
{
	OnItemSlotSwapped.Broadcast(LastSlotIndex, LastSlotData, TargetSlot, TargetSlotData);
}
void UGISInventoryComponent::InitializeInventory()
{
	for (int32 Index; Index < InventorySize; Index++)
	{
		FGISItemDataWrapper newItem;
		newItem.CurrentItemIndex = Index;
		newItem.Item = nullptr;

		ItemsInInventory.Add(newItem);
	}
}
bool UGISInventoryComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const FGISItemDataWrapper& item : ItemsInInventory)
	{
		if (item.Item)
		{
			WroteSomething |= Channel->ReplicateSubobject(const_cast<UGISItemData*>(item.Item), *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}
void UGISInventoryComponent::GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs)
{
	for (const FGISItemDataWrapper& item : ItemsInInventory)
	{
		if (item.Item && item.Item->IsNameStableForNetworking())
		{
			Objs.Add(const_cast<UGISItemData*>(item.Item));
		}
	}
}