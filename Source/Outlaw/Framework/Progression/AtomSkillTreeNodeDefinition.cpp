// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomSkillTreeNodeDefinition.h"
#include "AbilitySystem/AtomAbilitySet.h"

UAtomSkillTreeNodeDefinition::UAtomSkillTreeNodeDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAtomAbilitySet* UAtomSkillTreeNodeDefinition::GetAbilitySetForRank(int32 Rank) const
{
	const int32 Index = Rank - 1;
	if (PerRankAbilitySets.IsValidIndex(Index) && PerRankAbilitySets[Index])
	{
		return PerRankAbilitySets[Index];
	}
	return GrantedAbilitySet;
}
