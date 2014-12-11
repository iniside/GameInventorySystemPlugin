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
	//we override this in each component to make sure that spawned widget will have right pointer to widget assigned.
	virtual void InitializeComponent() override;

	virtual TArray<FGISSlotInfo> GetInventoryArray() override;

	virtual void AddItemToInventory(class UGISItemData* ItemIn) override;

	virtual void ServerAddItemToInventory_Implementation(class UGISItemData* ItemIn) override;
	virtual bool ServerAddItemToInventory_Validate(class UGISItemData* ItemIn) override;

	virtual void AddItemOnSlot(int32 TargetSlotIndex, const FGISSlotInfo& TargetSlotType, int32 LastSlotIndex, const FGISSlotInfo& LastSlotType) override;

	virtual void ServerAddItemOnSlot_Implementation(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType) override;
	virtual bool ServerAddItemOnSlot_Validate(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType) override;

//private:
//	void InitializeInventory();
//	void InitializeInventoryTabs();
};



