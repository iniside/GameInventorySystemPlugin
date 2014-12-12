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
		int32 InventorySize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray <FGISSlotsInTab> InitialTabInfo;

	/*
		Which item types, this component will accept.

		Useful if you have multiple inventory components on single actor, and want to make sure that they will accept
		only specific items.

		If nothing specified, nothing will be accepted, so make sure to add something!
	*/
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<class UGISItemData>> AccepectedItems;

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
		TArray<FGISSlotInfo> ItemsInInventory;
	/*
	Initial take on inventory tabs.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Inventory")
		FGISInventoryTab Tabs;

	UPROPERTY(EditAnywhere, Instanced)
		TSubclassOf<class UGISContainerBaseWidget> InventoryContainerClass;

	/*
		Types of slot used in this container.
	*/
	UPROPERTY(EditAnywhere, meta = (ExposeOnSpawn))
		TSubclassOf<class UGISSlotBaseWidget> SlotClass;

	UPROPERTY(BlueprintReadOnly)
	class UGISContainerBaseWidget* InventoryContainer;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FGISOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FGISOnItemSlotSwapped OnItemSlotSwapped;


	virtual void InitializeComponent() override;
	virtual void PostInitProperties() override;

	/*
		To easily get current inventory array for widget initialization.
	*/
	virtual TArray<FGISSlotInfo> GetInventoryArray();
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


	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs) override;

	/*
		Invenory UObject replication support

	*/
private:
	void InitializeInventory();
	void InitializeInventoryTabs();
};



