// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GISInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISItemDataWrapper
{
	GENERATED_USTRUCT_BODY()
public:
	int32 LastItemIndex;
	//we don't want to change this property in blueprint. Not intentionally, nor by accident.
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentItemIndex;
	/*
		We don't care about item type. Get CDO of object and Cast it to check
		if it implement interface, which is used to retrieve properties out of object for display.
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Item")
	UGISItemData* Item;
};

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISSlotInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
		int8 SlotIndex;
	UPROPERTY(BlueprintReadWrite)
		UGISItemData* ItemData;
};

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISTabInfo
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY(BlueprintReadOnly)
		int8 TabIndex;

	UPROPERTY(BlueprintReadWrite)
		int8 NumberOfSlots;

	UPROPERTY(BlueprintReadOnly)
		TArray<FGISSlotInfo> TabSlots;
};

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISInventoryTab
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
		int8 NumberOfTabs;

	UPROPERTY(BlueprintReadOnly)
		TArray<FGISTabInfo> InventoryTabs;
};

USTRUCT()
struct FGISSlotsInTab
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfSlots;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGISOnItemAdded, int32, NewSlot, class UGISItemData*, ItemDataOut);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGISOnItemSlotSwapped, int32, LastSlotIndex, class UGISItemData*, LastSlotData, int32, TargetSlot, class UGISItemData*, TargetSlotData);

UCLASS(hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), editinlinenew, meta = (BlueprintSpawnableComponent))
class GAMEINVENTORYSYSTEM_API UGISInventoryComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY()
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
		TArray<FGISItemDataWrapper> ItemsInInventory;

	/*
		Initial take on inventory tabs.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Inventory")
		FGISInventoryTab Tabs;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FGISOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FGISOnItemSlotSwapped OnItemSlotSwapped;



	virtual void InitializeComponent() override;
	virtual void PostInitProperties() override;
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
		void AddItemOnSlot(int32 TargetSlot, int32 LastSlot);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddItemOnSlot(int32 TargetSlot, int32 LastSlot);
	/*
		Heyyy client, would you be so nice, and update this slot with this item ? Thanks!

		On more serious note, it will just call OnItemAdded delegate on client.
	*/
	UFUNCTION(Client, Reliable)
		void ClientUpdateInventory(int32 NewSlot, class UGISItemData* ItemDataOut);


	UFUNCTION(Client, Reliable)
		void ClientSlotSwap(int32 LastSlotIndex, class UGISItemData* LastSlotData, int32 TargetSlot, class UGISItemData* TargetSlotData);


	void PostInventoryInitialized();

	/*
		Invenory UObject replication support

	*/
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs) override;


private:
	void InitializeInventory();
	void InitializeInventoryTabs();
};



