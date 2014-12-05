#pragma once

#include "BPNode_CreateItemData.generated.h"
/*
	This is base class for item container used in InventoryComponent, to store items.

	How you use this container is up to you. I assume that item container will contain only TSubclassOf<>
	properties, which will in turn reference proper items (like weapons, construction elements, 
	consumable, armor items etc), along with any accompaning data for them (like upgrades, stats mods,
	count of how many items player have etc).

	If you so desire, you can of course just extend any item from this class but I do not recommend it.
*/
UCLASS(BlueprintType, Blueprintable)
class GAMEINVENTORYSYSTEM_API UBPNode_CreateItemData : public UObject
{
	GENERATED_UCLASS_BODY()


};
