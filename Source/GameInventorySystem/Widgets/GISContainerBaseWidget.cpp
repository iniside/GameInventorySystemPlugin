// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "GameInventorySystem.h"

#include "GISSlotBaseWidget.h"
#include "GISItemBaseWidget.h"

#include "../GISGlobalTypes.h"
#include "../GISInventoryBaseComponent.h"

#include "GISContainerBaseWidget.h"

UGISContainerBaseWidget::UGISContainerBaseWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UGISContainerBaseWidget::InitializeContainer()
{
	if (InventoryComponent)
	{
		TArray<FGISSlotInfo> ItemInfos = InventoryComponent->GetInventoryArray();
		int32 IndexCounter = 0;
		for (const FGISSlotInfo& slot : ItemInfos)
		{
			if (SlotClass)
			{
				UGISSlotBaseWidget* SlotWidget = ConstructObject<UGISSlotBaseWidget>(SlotClass, this);
				if (SlotWidget)
				{
					ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
					SlotWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
					SlotWidget->Initialize();
					SlotWidget->SlotInfo = slot;
					SlotWidget->GISItemClass = ItemClass;

					InventorySlots.Add(SlotWidget);
					//call last
					//SlotWidget->InitializeContainer();
				}
			}
			IndexCounter++;
		}
		IndexCounter = 0;
		//bind functions to delegates:
		InventoryComponent->OnItemAdded.AddDynamic(this, &UGISContainerBaseWidget::Widget_OnItemAdded);
		InventoryComponent->OnItemSlotSwapped.AddDynamic(this, &UGISContainerBaseWidget::Widget_OnItemSlotSwapped);
	}
}

void UGISContainerBaseWidget::Widget_OnItemAdded(int32 NewSlot, class UGISItemData* ItemDataIn)
{
	if (ItemDataIn)
	{
		UGISItemBaseWidget* ItemWidget = ConstructObject<UGISItemBaseWidget>(ItemClass, this);
		if (ItemWidget && InventoryComponent)
		{
			ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
			ItemWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
			ItemWidget->Initialize();
			//ItemWidget->LastSlotInfo = SlotInfo;
		}
		//TArray<FName> SlotNames;
		//InventorySlots[NewSlot]->GetSlotNames(SlotNames);
		UWidget* superWidget = InventorySlots[NewSlot]->GetWidgetFromName(TEXT("OverlaySlot"));
		
		UOverlay* overlay = Cast<UOverlay>(superWidget);
		if (overlay)
		{
			int32 childCount = overlay->GetChildrenCount();
			if (childCount > 1)
			{
				overlay->RemoveChildAt(childCount - 1);
			}
			overlay->AddChild(ItemWidget);
		}
		
		//if (SlotNames.Num() > 0)
		//{
		//}
		//UNamedSlot* NamedWidet = Cast<UNamedSlot>(InventorySlots[NewSlot]);
		//if (NamedWidet)
		//{
		//	int32 childCount = NamedWidet->GetChildrenCount();
		//	NamedWidet->RemoveChildAt(childCount - 1);
		//	NamedWidet->AddChild(ItemWidget);
		//}
	}
}
void UGISContainerBaseWidget::Widget_OnItemSlotSwapped(const FGISSlotSwapInfo& SlotSwapInfo)
{
	if (SlotSwapInfo.LastSlotComponent == SlotSwapInfo.TargetSlotComponent)
	{
		if (!SlotSwapInfo.LastSlotData)
		{
			RemoveItem(SlotSwapInfo);
			AddItem(SlotSwapInfo);
		}
	}
	//so we targeted different component with out drop action.
	//we need to handle it! but how...
	if (SlotSwapInfo.LastSlotComponent != SlotSwapInfo.TargetSlotComponent)
	{
		if (SlotSwapInfo.LastSlotComponent.IsValid() && SlotSwapInfo.TargetSlotComponent.IsValid())
		{
			//actually, probabaly need separate functions, as there might be more
			//complex scases like actuall swapping items, instead of puting it
			//in empty slot in another component.
			SlotSwapInfo.LastSlotComponent->InventoryContainer->RemoveItem(SlotSwapInfo);
			SlotSwapInfo.TargetSlotComponent->InventoryContainer->AddItem(SlotSwapInfo);
		}
	}
	UGISContainerBaseWidget* awesomeTest = this;
	float lolo = 10;
}

void UGISContainerBaseWidget::AddItem(const FGISSlotSwapInfo& SlotSwapInfo)
{
	if (!SlotSwapInfo.LastSlotData)
	{
		UGISItemBaseWidget* ItemWidget = ConstructObject<UGISItemBaseWidget>(ItemClass, this);
		if (ItemWidget && InventoryComponent)
		{
			ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
			ItemWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
			ItemWidget->Initialize();
			//ItemWidget->LastSlotInfo = SlotInfo;
		}
		UWidget* superWidget = InventorySlots[SlotSwapInfo.TargetSlotIndex]->GetWidgetFromName(DropSlottName);

		UOverlay* overlay = Cast<UOverlay>(superWidget);
		if (overlay)
		{
			overlay->AddChild(ItemWidget);
		}
	}
	else
	{
		//construct target and last, since this is for test one will do just as well.
		UGISItemBaseWidget* ItemWidget = ConstructObject<UGISItemBaseWidget>(ItemClass, this);
		if (ItemWidget && InventoryComponent)
		{
			ULocalPlayer* Player = InventoryComponent->GetWorld()->GetFirstLocalPlayerFromController(); //temporary
			ItemWidget->SetPlayerContext(FLocalPlayerContext(Player)); //temporary
			ItemWidget->Initialize();
			//ItemWidget->LastSlotInfo = SlotInfo;
		}
		UWidget* lastSlotWidget = InventorySlots[SlotSwapInfo.LastSlotIndex]->GetWidgetFromName(DropSlottName);
		UWidget* targetSlotWidget = InventorySlots[SlotSwapInfo.TargetSlotIndex]->GetWidgetFromName(DropSlottName);
		
		UOverlay* lastSlotOverlay = Cast<UOverlay>(lastSlotWidget);
		if (lastSlotOverlay)
		{
			lastSlotOverlay->AddChild(ItemWidget);
		}

		UOverlay* targetSlotOverlay = Cast<UOverlay>(targetSlotWidget);
		if (targetSlotOverlay)
		{
			targetSlotOverlay->AddChild(ItemWidget);
		}
	}
}
void UGISContainerBaseWidget::RemoveItem(const FGISSlotSwapInfo& SlotSwapInfo)
{
	if (!SlotSwapInfo.LastSlotData)
	{
		UWidget* superWidget = InventorySlots[SlotSwapInfo.LastSlotIndex]->GetWidgetFromName(DropSlottName);

		//this it bit fiddly since the widget which will contain our widget must be last
		//and out item widget must be last child within this widget
		UOverlay* overlay = Cast<UOverlay>(superWidget);
		if (overlay)
		{
			int32 childCount = overlay->GetChildrenCount();
			if (childCount > 1)
			{
				overlay->RemoveChildAt(childCount - 1);
			}
		}
	}
	else
	{
		UWidget* lastSlotWidget = InventorySlots[SlotSwapInfo.LastSlotIndex]->GetWidgetFromName(DropSlottName);
		UWidget* targetSlotWidget = InventorySlots[SlotSwapInfo.TargetSlotIndex]->GetWidgetFromName(DropSlottName);
		//this it bit fiddly since the widget which will contain our widget must be last
		//and out item widget must be last child within this widget
		UOverlay* lastSlotOverlay = Cast<UOverlay>(lastSlotWidget);
		if (lastSlotOverlay)
		{
			int32 childCount = lastSlotOverlay->GetChildrenCount();
			if (childCount > 1)
			{
				lastSlotOverlay->RemoveChildAt(childCount - 1);
			}
		}
		UOverlay* targetSlotOverlay = Cast<UOverlay>(targetSlotWidget);
		if (lastSlotOverlay)
		{
			int32 childCount = targetSlotOverlay->GetChildrenCount();
			if (childCount > 1)
			{
				targetSlotOverlay->RemoveChildAt(childCount - 1);
			}
		}
	}
}