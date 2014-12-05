// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GISInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISItemDataWrapper
{
	GENERATED_USTRUCT_BODY()
public:
	int32 LastItemIndex;
	int32 CurrentItemIndex;
	/*
		We don't care about item type. Get CDO of object and Cast it to check
		if it implement interface, which is used to retrieve properties out of object for display.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TSubclassOf<UObject> Item;
}

UCLASS(hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), editinlinenew, meta = (BlueprintSpawnableComponent))
class GAMEINVENTORYSYSTEM_API UGISInventoryComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:
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
<<<<<<< HEAD

		Index order of this array should be stable. It's not sorted in anyway, but also items never change position inside it.
		Only data contained within struct is changed, while struct itself remain intact.
	*/
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Inventory")
		TArray<FGISItemDataWrapper> ItemsInInventory;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FGISOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FGISOnItemSlotSwapped OnItemSlotSwapped;


	/* some changes for git.
	Testing Function1
	Technically you never should call this function from client, in client-server environment. NEVER
	EVER.

	It's here now just for testing. Once invetory plugin is fully operational, I will remove it.
	and make it authority callable only.
	*/
	UFUNCTION(BlueprintCallable, Category="Game Inventory System")
	void AddItemToInventory(class UGISItemData* ItemIn);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddItemToInventory(class UGISItemData* ItemIn);


	UFUNCTION(BlueprintCallable, Category = "Game Inventory System")
		void AddItemOnSlot(int32 TargetSlot, int32 LastSlot, class UGISItemData* ItemData);

	/*
		Heyyy client, would you be so nice, and update this slot with this item ? Thanks!

		On more serious note, it will just call OnItemAdded delegate on client.
	*/
	UFUNCTION(Client, Reliable)
		void ClientUpdateInventory(int32 NewSlot, class UGISItemData* ItemDataOut);


	UFUNCTION(Client, Reliable)
		void ClientSlotSwap(int32 LastSlotIndex, class UGISItemData* LastSlotData, int32 TargetSlot, class UGISItemData* TargetSlotData);

	/*
		Invenory UObject replication support

	*/
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs) override;
};



