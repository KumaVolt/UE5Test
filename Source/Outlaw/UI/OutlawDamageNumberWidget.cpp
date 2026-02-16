// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawDamageNumberWidget.h"

void UOutlawDamageNumberWidget::InitDamageNumber(float Amount, bool bCrit, FVector WorldLocation)
{
	OnDamageNumberInit(Amount, bCrit);
}
