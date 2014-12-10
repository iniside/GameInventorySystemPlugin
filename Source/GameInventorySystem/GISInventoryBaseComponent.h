// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GISGlobalTypes.h"
#include "GISInventoryBaseComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGISOnItemAdded, int32, NewSlot, class UGISItemData*, ItemDataOut);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGISOnItemSlotSwapped, const FGISSlotSwapInfo&, SlotSwapInfo);

UCLASS(hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), editinlinenew, meta = (BlueprintSpawnableComponent))
class GAMEINVENTORYSYSTEM_API UGISInventoryBaseComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY(EditAnywhere)
		TSubclassOf<class UGISContainerBase> InventoryContainerClass;

	UPROPERTY(BlueprintReadOnly)
	class UGISContainerBase* InventoryContainer;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FGISOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FGISOnItemSlotSwapped OnItemSlotSwapped;

	virtual void InitializeComponent() override;
	virtual void PostInitProperties() override;
	/*
		Initial draft for picking up items from world.


		First you need to find item. This part is somewhat game specific, you need use radial sweep,
		line trace etc, to find items.

		The important part, is the fact that trace should happen ON SERVER, to make sure that
		item is in that place.

		Once item has been found, we will call PickItem function and pass Actor pointer to it.
		There is no reason to use UObject as UObjects can't be placed on level.

		Pickable actor should implement special interface, 
		which I have not yet decided how it will look ;/.
	*/
	UFUNCTION(BlueprintCallable, Category = "Game InventorySystem")
		void PickItem(AActor* PickupItemIn);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerPickItem(AActor* PickupItemIn);
		

	/* some changes for git.
	Testing Function1
	Technically you never should call this function from client, in client-server environment. NEVER
	EVER.

	It's here now just for testing. Once invetory plugin is fully operational, I will remove it.
	and make it authority callable only.
	*/
	UFUNCTION(BlueprintCallable, Category="Game Inventory System")
		virtual void AddItemToInventory(class UGISItemData* ItemIn);

	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerAddItemToInventory(class UGISItemData* ItemIn);


	UFUNCTION(BlueprintCallable, Category = "Game Inventory System")
		virtual void AddItemOnSlot(int32 TargetSlotIndex, const FGISSlotInfo& TargetSlotType, int32 LastSlotIndex, const FGISSlotInfo& LastSlotType);

	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerAddItemOnSlot(int32 TargetSlot, const FGISSlotInfo& TargetSlotType, int32 LastSlot, const FGISSlotInfo& LastSlotType);
	/*
		Heyyy client, would you be so nice, and update this slot with this item ? Thanks!

		On more serious note, it will just call OnItemAdded delegate on client.
	*/
	UFUNCTION(Client, Reliable)
		void ClientUpdateInventory(int32 NewSlot, class UGISItemData* ItemDataOut);


	UFUNCTION(Client, Reliable)
		void ClientSlotSwap(const FGISSlotSwapInfo& SlotSwapInfo);


	void PostInventoryInitialized();

	/*
		Invenory UObject replication support

	*/

};



