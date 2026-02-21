// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomClassDefinition.h"
#include "AtomSkillTreeNodeDefinition.h"

UAtomClassDefinition::UAtomClassDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float UAtomClassDefinition::GetAttributeBaseValueAtLevel(FGameplayAttribute Attribute, int32 Level) const
{
	float Total = 0.0f;
	for (const FAtomStatGrowthEntry& Entry : StatGrowthTable)
	{
		if (Entry.Attribute == Attribute)
		{
			Total += Entry.ValuePerLevel * static_cast<float>(Level);
		}
	}
	return Total;
}

UAtomSkillTreeNodeDefinition* UAtomClassDefinition::FindSkillNode(FGameplayTag NodeTag) const
{
	for (const TObjectPtr<UAtomSkillTreeNodeDefinition>& Node : SkillTreeNodes)
	{
		if (Node && Node->NodeTag == NodeTag)
		{
			return Node;
		}
	}
	return nullptr;
}
