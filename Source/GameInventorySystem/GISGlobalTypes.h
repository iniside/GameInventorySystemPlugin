#pragma once

#include "GISGlobalTypes.generated.h"

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FPickupItemData
{
	GENERATED_USTRUCT_BODY()
};

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FPickupItemContainer
{
	GENERATED_USTRUCT_BODY()
};

//hmp. This actually doesn't lend itself as extensible solution...
UENUM(BlueprintType)
namespace EGISSlotType
{
	enum Type
	{
		Inventory,
		Hotbar,
		Character,

		MAX_INVALID //always last.
	};
}

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISSlotSwapInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
		int32 LastSlotIndex;
	UPROPERTY(BlueprintReadOnly)
		class UGISItemData* LastSlotData;
	UPROPERTY(BlueprintReadOnly)
		TWeakObjectPtr<class UGISInventoryBaseComponent> LastSlotComponent;
	UPROPERTY(BlueprintReadOnly)
		int32 TargetSlotIndex;
	UPROPERTY(BlueprintReadOnly)
		class UGISItemData* TargetSlotData;
	UPROPERTY(BlueprintReadOnly)
		TWeakObjectPtr<class UGISInventoryBaseComponent> TargetSlotComponent;
};

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISSlotInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
		int32 SlotIndex;
	UPROPERTY(BlueprintReadWrite)
		class UGISItemData* ItemData;

	UPROPERTY(BlueprintReadWrite)
		TWeakObjectPtr<class UGISInventoryBaseComponent> CurrentInventoryComponent;

	static const int32 SlotTypeID = 0;

	virtual bool IsOfType(int32 SlotTypeIDIn) const { return FGISSlotInfo::SlotTypeID == SlotTypeIDIn; }
};

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISInventorySlot : public FGISSlotInfo
{
	GENERATED_USTRUCT_BODY()
public:
	static const int32 SlotTypeID = 1;

	virtual bool IsOfType(int32 SlotTypeIDIn) const { return FGISInventorySlot::SlotTypeID == SlotTypeIDIn; }
};

USTRUCT(BlueprintType)
struct GAMEINVENTORYSYSTEM_API FGISItemHotbar : public FGISSlotInfo
{
	GENERATED_USTRUCT_BODY()
public:
	//unique per derived struct.
	static const int32 SlotTypeID = 2;

	virtual bool IsOfType(int32 SlotTypeIDIn) const { return FGISItemHotbar::SlotTypeID == SlotTypeIDIn; }
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
