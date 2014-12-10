// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GISGlobalTypes.h"
#include "GISInventoryBaseComponent.h"
#include "GISActionBarComponent.generated.h"

UCLASS(hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), editinlinenew, meta = (BlueprintSpawnableComponent))
class GAMEINVENTORYSYSTEM_API UGISActionBarComponent : public UGISInventoryBaseComponent
{
	GENERATED_UCLASS_BODY()
public:
	//test for hotbar
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Hotbar")
		TArray<FGISItemHotbar> HotBarButtons;

	virtual void InitializeComponent() override;

	virtual void AddItemOnSlot(int32 TargetSlotIndex, const FGISSlotInfo& TargetSlotType, int32 LastSlotIndex, const FGISSlotInfo& LastSlotType) override;

	virtual void ServerAddItemOnSlot_Implementation(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType) override;
	virtual bool ServerAddItemOnSlot_Validate(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType) override;

	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs) override;
};



