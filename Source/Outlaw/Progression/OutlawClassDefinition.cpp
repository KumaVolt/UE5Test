// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawClassDefinition.h"
#include "OutlawSkillTreeNodeDefinition.h"

UOutlawClassDefinition::UOutlawClassDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float UOutlawClassDefinition::GetAttributeBaseValueAtLevel(FGameplayAttribute Attribute, int32 Level) const
{
	float Total = 0.0f;
	for (const FOutlawStatGrowthEntry& Entry : StatGrowthTable)
	{
		if (Entry.Attribute == Attribute)
		{
			Total += Entry.ValuePerLevel * static_cast<float>(Level);
		}
	}
	return Total;
}

UOutlawSkillTreeNodeDefinition* UOutlawClassDefinition::FindSkillNode(FGameplayTag NodeTag) const
{
	for (const TObjectPtr<UOutlawSkillTreeNodeDefinition>& Node : SkillTreeNodes)
	{
		if (Node && Node->NodeTag == NodeTag)
		{
			return Node;
		}
	}
	return nullptr;
}
