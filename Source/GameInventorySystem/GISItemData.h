#pragma once

#include "GISItemData.generated.h"
/*
	This is base class for item container used in InventoryComponent, to store items.

	How you use this container is up to you. I assume that item container will contain only TSubclassOf<>
	properties, which will in turn reference proper items (like weapons, construction elements, 
	consumable, armor items etc), along with any accompaning data for them (like upgrades, stats mods,
	count of how many items player have etc).

	If you so desire, you can of course just extend any item from this class but I do not recommend it.
*/
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class GAMEINVENTORYSYSTEM_API UGISItemData : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(EditAnywhere)
		bool bCanBeAddedToHotbar;

	bool IsNameStableForNetworking() const override;

	bool IsSupportedForNetworking() const override
	{
		return true;
	}
	void SetNetAddressable();

	/*
		Called when item is added to slot. 
		Might for example spawn objects from data in item, if you need object to be interactable.
	*/
	virtual bool OnItemAddedToSlot() { return false; }
	virtual bool OnItemRemovedFromSlot() { return false; }

	/*
		Called when item is added to inventory. And only then.
	*/
	virtual bool OnItemAddedToInventory() { return false; }

	/*
		Override when you need to activate item. For example from hotbar. Or something.
	*/
	virtual void ActivateItem() {}

	/*
		Don't like this crap, but it is best way to expose it to blueprint (;.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input")
		bool OnKeyPressedDown();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input")
		bool OnKeyPressedUp();
	/*
		Below is mainly for convinience. We could just as well use Cast<> to determine class type.
		And at some point you will have to do Cast. To get item from inventory for example.
		But you don't need if you want to, for example just iterate over all items
		to check what types of items are in.
	*/
	/*
		Unique! Id of item container. 
	*/
	static const int32 ItemTypeID = 0;

	virtual int32 GetItemTypeID() const { return UGISItemData::ItemTypeID; }

	virtual bool IsOfType(int32 ItemTypeIDIn) { return UGISItemData::ItemTypeID == ItemTypeIDIn; }

protected:
	bool bNetAddressable;

};

template< class T >
T* CreateDataItem(APlayerController* OwningPlayer, UClass* UserWidgetClass)
{
	if (!UserWidgetClass->IsChildOf(UGISItemData::StaticClass()))
	{
		// TODO UMG Error?
		return nullptr;
	}

	// Assign the outer to the game instance if it exists, otherwise use the player controller's world
	UWorld* World = OwningPlayer->GetWorld();
	//UObject* Outer = World->GetGameInstance() ? StaticCast<UObject*>(World->GetGameInstance()) : StaticCast<UObject*>(World);
	UGISItemData* NewWidget = ConstructObject<UGISItemData>(UserWidgetClass, OwningPlayer);

	//NewWidget->SetPlayerContext(FLocalPlayerContext(OwningPlayer));
	//NewWidget->Initialize();

	return Cast<T>(NewWidget);
}