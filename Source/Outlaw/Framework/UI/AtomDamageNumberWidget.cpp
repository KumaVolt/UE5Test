// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomDamageNumberWidget.h"

void UAtomDamageNumberWidget::InitDamageNumber(float Amount, bool bCrit, FVector WorldLocation)
{
	OnDamageNumberInit(Amount, bCrit);
}
