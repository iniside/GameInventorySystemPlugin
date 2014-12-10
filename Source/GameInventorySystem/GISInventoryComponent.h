// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GISGlobalTypes.h"
#include "GISInventoryBaseComponent.h"
#include "GISInventoryComponent.generated.h"

UCLASS(hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), editinlinenew, meta = (BlueprintSpawnableComponent))
class GAMEINVENTORYSYSTEM_API UGISInventoryComponent : public UGISInventoryBaseComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn))
		int32 InventorySize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray <FGISSlotsInTab> InitialTabInfo;

	/*
	Indicates if items can be activated directly in invetory window.
	Useful if you want to prevent player from activating items in invetory. For example
	healing potions, or something.
	*/
	UPROPERTY(EditAnywhere, Category = "Inventory Options")
		bool bCanActivateItemInInventory;
	/*
	I probabaly need to wrap it into struct, to better support drag&drop, sorting, item swapping
	etc.
	Because right now I have no way of really knowing, which position item is, or on what
	position it was previously, because order of Dynamic Array is not guaranteed.

	Index order of this array should be stable. It's not sorted in anyway, but also items never change position inside it.
	Only data contained within struct is changed, while struct itself remain intact.

	Once this is done and working, refactor this into Tabs.
	Inventory can have multiple Tabs(Bags ?), and each tab can have X slots.
	*/
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Inventory")
		TArray<FGISInventorySlot> ItemsInInventory;
	/*
	Initial take on inventory tabs.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Inventory")
		FGISInventoryTab Tabs;

	//we override this in each component to make sure that spawned widget will have right pointer to widget assigned.
	virtual void InitializeComponent() override;

	virtual void AddItemToInventory(class UGISItemData* ItemIn) override;

	virtual void ServerAddItemToInventory_Implementation(class UGISItemData* ItemIn) override;
	virtual bool ServerAddItemToInventory_Validate(class UGISItemData* ItemIn) override;

	virtual void AddItemOnSlot(int32 TargetSlotIndex, const FGISSlotInfo& TargetSlotType, int32 LastSlotIndex, const FGISSlotInfo& LastSlotType) override;

	virtual void ServerAddItemOnSlot_Implementation(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType) override;
	virtual bool ServerAddItemOnSlot_Validate(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType) override;

	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs) override;

private:
	void InitializeInventory();
	void InitializeInventoryTabs();
};



