// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomLevelingConfig.h"

UAtomLevelingConfig::UAtomLevelingConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

int32 UAtomLevelingConfig::GetMaxLevel() const
{
	return LevelTable.Num();
}

int32 UAtomLevelingConfig::GetXPForLevel(int32 Level) const
{
	const int32 Index = Level - 1;
	if (!LevelTable.IsValidIndex(Index))
	{
		return 0;
	}
	return LevelTable[Index].RequiredXP;
}

int32 UAtomLevelingConfig::GetLevelForXP(int32 TotalXP) const
{
	int32 Level = 1;
	for (int32 i = 0; i < LevelTable.Num(); ++i)
	{
		if (TotalXP >= LevelTable[i].RequiredXP)
		{
			Level = i + 1;
		}
		else
		{
			break;
		}
	}
	return Level;
}

int32 UAtomLevelingConfig::GetSkillPointsForLevel(int32 Level) const
{
	const int32 Index = Level - 1;
	if (!LevelTable.IsValidIndex(Index))
	{
		return 0;
	}

	const int32 Points = LevelTable[Index].SkillPointsAwarded;
	return (Points > 0) ? Points : DefaultSkillPointsPerLevel;
}

int32 UAtomLevelingConfig::GetTotalSkillPointsForLevel(int32 Level) const
{
	int32 Total = 0;
	for (int32 i = 1; i <= Level; ++i)
	{
		Total += GetSkillPointsForLevel(i);
	}
	return Total;
}
