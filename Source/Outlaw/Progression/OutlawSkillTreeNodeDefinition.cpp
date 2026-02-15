// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawSkillTreeNodeDefinition.h"
#include "AbilitySystem/OutlawAbilitySet.h"

UOutlawSkillTreeNodeDefinition::UOutlawSkillTreeNodeDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UOutlawAbilitySet* UOutlawSkillTreeNodeDefinition::GetAbilitySetForRank(int32 Rank) const
{
	const int32 Index = Rank - 1;
	if (PerRankAbilitySets.IsValidIndex(Index) && PerRankAbilitySets[Index])
	{
		return PerRankAbilitySets[Index];
	}
	return GrantedAbilitySet;
}
