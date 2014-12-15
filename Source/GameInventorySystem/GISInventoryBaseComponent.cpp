// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "GISGlobalTypes.h"

#include "GISItemData.h"
#include "IGISPickupItem.h"
#include "GISPickupActor.h"

#include "Widgets/GISContainerBaseWidget.h"
#include "Widgets/GISLootContainerBaseWidget.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "GISInventoryBaseComponent.h"

UGISInventoryBaseComponent::UGISInventoryBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	bAutoRegister = true;
}


void UGISInventoryBaseComponent::InitializeComponent()
{
	Super::InitializeComponent();
	//if (!bHasBeenInitialized)
	//{
		ENetRole CurrentRole = GetOwnerRole();
		ENetMode CurrentNetMode = GetNetMode();


		if (CurrentRole == ROLE_Authority || CurrentNetMode == ENetMode::NM_Standalone)
			InitializeInventoryTabs();

		if (CurrentRole < ROLE_Authority || CurrentNetMode == ENetMode::NM_Standalone)
		{
			if (InventoryContainerClass)
			{
				UObject* Outer = GetWorld()->GetGameInstance() ? StaticCast<UObject*>(GetWorld()->GetGameInstance()) : StaticCast<UObject*>(GetWorld());
				InventoryContainer = ConstructObject<UGISContainerBaseWidget>(InventoryContainerClass, Outer);
				if (InventoryContainer)
				{
					ULocalPlayer* Player = World->GetFirstLocalPlayerFromController(); //temporary
					InventoryContainer->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
					InventoryContainer->Initialize();
					InventoryContainer->InventoryComponent = this;

					//call last
					InventoryContainer->InitializeContainer();
				}
			}

			UObject* Outer = GetWorld()->GetGameInstance() ? StaticCast<UObject*>(GetWorld()->GetGameInstance()) : StaticCast<UObject*>(GetWorld());
			if (LootWidgetClass)
			{
				LootWidget = ConstructObject<UGISLootContainerBaseWidget>(LootWidgetClass, Outer);
				if (LootWidget)
				{
					ULocalPlayer* Player = GetWorld()->GetFirstLocalPlayerFromController(); //temporary
					LootWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
					LootWidget->Initialize();
					LootWidget->SetVisibility(ESlateVisibility::Hidden);
				}
			}

			OnItemLooted.AddDynamic(this, &UGISInventoryBaseComponent::ConstructLootPickingWidget);
		}

		if (CurrentRole == ROLE_Authority || CurrentNetMode == ENetMode::NM_Standalone)
			ClientLoadInventory();
	//}
}
void UGISInventoryBaseComponent::OnRep_InventoryCreated()
{
	OnInventoryLoaded.Broadcast();
	//if (SlotUpdateInfo.SlotComponent.IsValid())
	//	SlotUpdateInfo.SlotData = SlotUpdateInfo.SlotComponent->Tabs.InventoryTabs[SlotUpdateInfo.TabIndex].TabSlots[SlotUpdateInfo.SlotIndex].ItemData;
	//OnItemAdded.Broadcast(SlotUpdateInfo);
}

void UGISInventoryBaseComponent::OnRep_SlotUpdate()
{
//	if (SlotUpdateInfo.SlotComponent.IsValid())
//		SlotUpdateInfo.SlotData = SlotUpdateInfo.SlotComponent->Tabs.InventoryTabs[SlotUpdateInfo.TabIndex].TabSlots[SlotUpdateInfo.SlotIndex].ItemData;
	OnItemAdded.Broadcast(SlotUpdateInfo);
}

void UGISInventoryBaseComponent::OnRep_SlotSwap()
{
	OnItemSlotSwapped.Broadcast(SlotSwapInfo);
}
void UGISInventoryBaseComponent::PostInitProperties()
{
	Super::PostInitProperties();
	
}

FGISInventoryTab UGISInventoryBaseComponent::GetInventoryTabs()
{
	return Tabs;
}

void UGISInventoryBaseComponent::PickItem(AActor* PickupItemIn)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerPickItem(PickupItemIn);
	}
	else
	{
		IIGISPickupItem* pickupItem = Cast<IIGISPickupItem>(PickupItemIn);
		if (pickupItem)
		{
			//do something, I'm not sure what yet.
		}
	}
}

void UGISInventoryBaseComponent::ServerPickItem_Implementation(AActor* PickupItemIn)
{
	PickItem(PickupItemIn);
}
bool UGISInventoryBaseComponent::ServerPickItem_Validate(AActor* PickupItemIn)
{
	return true;
}
void UGISInventoryBaseComponent::AddItemToInventory(class UGISItemData* ItemIn)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerAddItemToInventory(ItemIn);
	}
	else
	{
		//add item to first empty slot in first matching tab.
		for (FGISTabInfo& TabInfo : Tabs.InventoryTabs)
		{
			for (FGISSlotInfo& Slot : TabInfo.TabSlots)
			{
				if (Slot.ItemData == nullptr)
				{
					Slot.ItemData = ItemIn;
					//Slot.ItemData->OnItemRemovedFromSlot();
					SlotUpdateInfo.TabIndex = TabInfo.TabIndex;
					SlotUpdateInfo.SlotIndex = Slot.SlotIndex;
					SlotUpdateInfo.SlotData = Slot.ItemData;
					SlotUpdateInfo.SlotComponent = this;
					if (GetNetMode() == ENetMode::NM_Standalone)
						OnItemAdded.Broadcast(SlotUpdateInfo);

					ClientUpdateInventory(SlotUpdateInfo);
					return;
				}
			}
		}
	}
}

void UGISInventoryBaseComponent::ServerAddItemToInventory_Implementation(class UGISItemData* ItemIn)
{
	AddItemToInventory(ItemIn);
}

bool UGISInventoryBaseComponent::ServerAddItemToInventory_Validate(class UGISItemData* ItemIn)
{
	return true;
}


void UGISInventoryBaseComponent::AddItemOnSlot(const FGISSlotInfo& TargetSlotType, const FGISSlotInfo& LastSlotType)
{
	//Before we start swapping item, let's check if tags match!
	if (!TargetSlotType.CurrentInventoryComponent->RequiredTags.MatchesAny(LastSlotType.CurrentInventoryComponent->OwnedTags, false))
		return;
	
	//next check should be against item tags, but that's later!


	//Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotTabIndex].ItemData
	if (TargetSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData == nullptr)
	{

		UGISItemData* TargetItem = LastSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[LastSlotType.SlotTabIndex].TabSlots[LastSlotType.SlotIndex].ItemData;
		UGISItemData* LastItem = TargetSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData; //Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData;

		TargetSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData = TargetItem;
		LastSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[LastSlotType.SlotTabIndex].TabSlots[LastSlotType.SlotIndex].ItemData = nullptr;
		
		TargetItem->OnItemAddedToSlot();
		//FGISSlotSwapInfo SlotSwapInfo;

		SlotSwapInfo.LastSlotIndex = LastSlotType.SlotIndex;
		SlotSwapInfo.LastTabIndex = LastSlotType.SlotTabIndex;
		SlotSwapInfo.LastSlotData = LastItem;
		SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
		SlotSwapInfo.TargetSlotIndex = TargetSlotType.SlotIndex;
		SlotSwapInfo.TargetTabIndex = TargetSlotType.SlotTabIndex;
		SlotSwapInfo.TargetSlotData = TargetItem;
		SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
		if (GetNetMode() == ENetMode::NM_Standalone)
			OnItemSlotSwapped.Broadcast(SlotSwapInfo);
		ClientSlotSwap(SlotSwapInfo);
	}
	else
	{
		UGISItemData* TargetItem = LastSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[LastSlotType.SlotTabIndex].TabSlots[LastSlotType.SlotIndex].ItemData;
		UGISItemData* LastItem = TargetSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData; //Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData;

		TargetSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[TargetSlotType.SlotTabIndex].TabSlots[TargetSlotType.SlotIndex].ItemData = TargetItem;
		LastSlotType.CurrentInventoryComponent->Tabs.InventoryTabs[LastSlotType.SlotTabIndex].TabSlots[LastSlotType.SlotIndex].ItemData = LastItem;
		TargetItem->OnItemAddedToSlot();
		LastItem->OnItemAddedToSlot();

		//FGISSlotSwapInfo SlotSwapInfo;
		SlotSwapInfo.LastSlotIndex = LastSlotType.SlotIndex;
		SlotSwapInfo.LastTabIndex = LastSlotType.SlotTabIndex;
		SlotSwapInfo.LastSlotData = LastItem;
		SlotSwapInfo.LastSlotComponent = LastSlotType.CurrentInventoryComponent;
		SlotSwapInfo.TargetSlotIndex = TargetSlotType.SlotIndex;
		SlotSwapInfo.TargetTabIndex = TargetSlotType.SlotTabIndex;
		SlotSwapInfo.TargetSlotData = TargetItem;
		SlotSwapInfo.TargetSlotComponent = TargetSlotType.CurrentInventoryComponent;
		if (GetNetMode() == ENetMode::NM_Standalone)
			OnItemSlotSwapped.Broadcast(SlotSwapInfo);
		ClientSlotSwap(SlotSwapInfo);
	}
}

void UGISInventoryBaseComponent::ServerAddItemOnSlot_Implementation(const FGISSlotInfo& TargetSlotType, const FGISSlotInfo& LastSlotType)
{
	AddItemOnSlot(TargetSlotType, LastSlotType);
}
bool UGISInventoryBaseComponent::ServerAddItemOnSlot_Validate(const FGISSlotInfo& TargetSlotType, const FGISSlotInfo& LastSlotType)
{
	return true;
}

void UGISInventoryBaseComponent::RemoveItem(const FGISSlotInfo& TargetSlotType)
{

}

void UGISInventoryBaseComponent::LootItems(class AGISPickupActor* LootContainer)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLootItems(LootContainer);
	}
	else
	{
		if (!LootContainer)
			return;

		LootContainer->SetOwner(GetOwner());

		for (UGISItemData* Item : LootContainer->ItemToLoot)
		{
			AddItemToInventory(Item);
		}
	}
}
void UGISInventoryBaseComponent::ServerLootItems_Implementation(class AGISPickupActor* LootContainer)
{
	LootItems(LootContainer);
}
bool UGISInventoryBaseComponent::ServerLootItems_Validate(class AGISPickupActor* LootContainer)
{
	return true;
}


void UGISInventoryBaseComponent::GetLootContainer(class AGISPickupActor* LootContainer)
{
	//if (GetOwnerRole() < ROLE_Authority)
	//{
	//	ServerGetLootContainer(LootContainer);
	//}
	//else
//	{
	LootContainer->SetOwner(GetOwner());
		CurrentPickupActor = LootContainer;
		ConstructLootPickingWidget();
//	}
}

void UGISInventoryBaseComponent::ConstructLootPickingWidget()
{
	if (CurrentPickupActor)
	{
		if (LootWidget)
		{
			CurrentPickupActor->InteractingInventory = this;
			int32 ItemCount = CurrentPickupActor->ItemToLoot.Num();
			TArray<FGISLootSlotInfo> LootSlotInfos;
			for (int32 Index = 0; Index < ItemCount; Index++)
			{
				FGISLootSlotInfo LootInfo;
				LootInfo.SlotIndex = Index;
				LootInfo.SlotData = CurrentPickupActor->ItemToLoot[Index];
				LootInfo.OwningPickupActor = CurrentPickupActor;
				LootSlotInfos.Add(LootInfo);
			}
			LootWidget->ItemsInfos = LootSlotInfos;
			LootWidget->OwningPickupActor = CurrentPickupActor;
			LootWidget->InitializeLootWidget();
			LootWidget->SetVisibility(ESlateVisibility::Visible);
		}
		//LootContainer->InteractingInventory = this;
		//LootContainer->OpenLootWindow();
	}
}

void UGISInventoryBaseComponent::LootAllItems(class AGISPickupActor* LootContainer)
{
	//if (GetOwnerRole() < ROLE_Authority)
	//{
	//	SeverLootOneItem
	//}
	//else
	//{
		if (LootContainer)
		{
			for (UGISItemData* Item : LootContainer->ItemToLoot)
			{
				AddItemToInventory(Item);
			}
		}
	//}
}
void UGISInventoryBaseComponent::LootOneItem(int32 ItemIndex, class AGISPickupActor* LootContainer)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		SeverLootOneItem(ItemIndex, LootContainer);
	}
	else
	{
		if (LootContainer)
		{
			AddItemToInventory(LootContainer->ItemToLoot[ItemIndex]);
			//ok we removed one item. We need to rconstruct widgets, indexes etc, to make sure arry
			//have proper indexes in first place.
			LootContainer->ItemToLoot.RemoveAt(ItemIndex, 1, true);
			//reconstruct widget.
			ClientReconstructLootWidget();
		}
	}
}
void UGISInventoryBaseComponent::SeverLootOneItem_Implementation(int32 ItemIndex, class AGISPickupActor* LootContainer)
{
	LootOneItem(ItemIndex, LootContainer);
}
bool UGISInventoryBaseComponent::SeverLootOneItem_Validate(int32 ItemIndex, class AGISPickupActor* LootContainer)
{
	return true;
}


void UGISInventoryBaseComponent::ServerGetLootContainer_Implementation(class AGISPickupActor* LootContainer)
{
	GetLootContainer(LootContainer);
}
bool UGISInventoryBaseComponent::ServerGetLootContainer_Validate(class AGISPickupActor* LootContainer)
{
	return true;
}

void UGISInventoryBaseComponent::ClientReconstructLootWidget()
{
	OnItemLooted.Broadcast();
}

void UGISInventoryBaseComponent::ClientUpdateInventory_Implementation(const FGISSlotUpdateData& SlotUpdateInfoIn)
{
	OnItemAdded.Broadcast(SlotUpdateInfoIn);
}
void UGISInventoryBaseComponent::ClientSlotSwap_Implementation(const FGISSlotSwapInfo& SlotSwapInfoIn)
{
	OnItemSlotSwapped.Broadcast(SlotSwapInfoIn);
}

void UGISInventoryBaseComponent::ClientLoadInventory_Implementation()
{
	OnInventoryLoaded.Broadcast();
}

void UGISInventoryBaseComponent::PostInventoryInitialized()
{

}

void UGISInventoryBaseComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UGISInventoryBaseComponent, Tabs, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGISInventoryBaseComponent, SlotUpdateInfo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGISInventoryBaseComponent, SlotSwapInfo, COND_OwnerOnly);
}

bool UGISInventoryBaseComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const FGISTabInfo& TabInfo : Tabs.InventoryTabs)
	{
		for (const FGISSlotInfo& SlotItem : TabInfo.TabSlots)
		{
			if (SlotItem.ItemData)
			{
				WroteSomething |= Channel->ReplicateSubobject(const_cast<UGISItemData*>(SlotItem.ItemData), *Bunch, *RepFlags);
			}
		}
	}

	if (SlotUpdateInfo.SlotData)
	{
		WroteSomething |= Channel->ReplicateSubobject(const_cast<UGISItemData*>(SlotUpdateInfo.SlotData), *Bunch, *RepFlags);
	}
	if (SlotSwapInfo.LastSlotData)
	{
		WroteSomething |= Channel->ReplicateSubobject(const_cast<UGISItemData*>(SlotSwapInfo.LastSlotData), *Bunch, *RepFlags);
	}
	if (SlotSwapInfo.TargetSlotData)
	{
		WroteSomething |= Channel->ReplicateSubobject(const_cast<UGISItemData*>(SlotSwapInfo.TargetSlotData), *Bunch, *RepFlags);
	}
	return WroteSomething;
}
void UGISInventoryBaseComponent::GetSubobjectsWithStableNamesForNetworking(TArray<UObject*>& Objs)
{
	for (const FGISTabInfo& TabInfo : Tabs.InventoryTabs)
	{
		for (const FGISSlotInfo& SlotItem : TabInfo.TabSlots)
		{
			if (SlotItem.ItemData)
			{
				Objs.Add(const_cast<UGISItemData*>(SlotItem.ItemData));
			}
		}
	}
}

void UGISInventoryBaseComponent::InitializeInventoryTabs()
{
	int8 counter = 0;
	for (const FGISSlotsInTab& SlotInfo : InitialTabInfo)
	{

		FGISTabInfo TabInfo;
		TabInfo.NumberOfSlots = SlotInfo.NumberOfSlots;
		TabInfo.TabIndex = counter;

		for (int32 Index = 0; Index < SlotInfo.NumberOfSlots; Index++)
		{
			FGISSlotInfo SlotInfo;
			SlotInfo.SlotIndex = Index;
			SlotInfo.SlotTabIndex = counter;
			SlotInfo.CurrentInventoryComponent = this;
			SlotInfo.ItemData = nullptr;
			TabInfo.TabSlots.Add(SlotInfo);
		}

		Tabs.InventoryTabs.Add(TabInfo);
		counter++;
	}
}