#pragma once
#include "Blueprint/UserWidget.h"
#include "../GISGlobalTypes.h"
#include "GISContainerBaseWidget.generated.h"
/*
	This is base class for item container used in InventoryComponent, to store items.

	How you use this container is up to you. I assume that item container will contain only TSubclassOf<>
	properties, which will in turn reference proper items (like weapons, construction elements, 
	consumable, armor items etc), along with any accompaning data for them (like upgrades, stats mods,
	count of how many items player have etc).

	If you so desire, you can of course just extend any item from this class but I do not recommend it.
*/
UCLASS()
class GAMEINVENTORYSYSTEM_API UGISContainerBaseWidget : public UUserWidget
{
	GENERATED_UCLASS_BODY()
public:
	virtual void InitializeContainer();

	/*
		Types of slot used in this container.
	*/
	UPROPERTY(EditAnywhere, meta = (ExposeOnSpawn))
		TSubclassOf<class UGISSlotBaseWidget> SlotClass;

	/*
		Types if item widget, which can be contained in slot.
	*/
	UPROPERTY(EditAnywhere, meta = (ExposeOnSpawn))
		TSubclassOf<class UGISItemBaseWidget> ItemClass;

	/*
		Name of drop slot, which will contain ItemWidget.

	*/
	UPROPERTY(EditAnywhere, meta = (ExposeOnSpawn))
		FName DropSlottName;

	/**
		Inventory which owns this widget
	*/
	UPROPERTY(BlueprintReadOnly)
	class UGISInventoryBaseComponent* InventoryComponent;

	/*
		Base widget doesn't care how slots are displyed, it will just create list of slots
		upon creation, so you can then display it however you want, by making derived widget in 
		blueprint.
	*/
	/**
		List of slots, for this container;
	*/
	UPROPERTY(BlueprintReadWrite)
		TArray<class UGISSlotBaseWidget*> InventorySlots;

	//could make it virtual, but don't want to expose it to blueprint
	//ideally blueprint user shouldn't know that this even exist.
	//in anycase they are only for client side interaction

	UFUNCTION()
	void Widget_OnItemAdded(int32 NewSlot, class UGISItemData* ItemDataIn);
	
	UFUNCTION()
	void Widget_OnItemSlotSwapped(const FGISSlotSwapInfo& SlotSwapInfo);

	void AddItem(const FGISSlotSwapInfo& SlotSwapInfo);
	void RemoveItem(const FGISSlotSwapInfo& SlotSwapInfo);
};
