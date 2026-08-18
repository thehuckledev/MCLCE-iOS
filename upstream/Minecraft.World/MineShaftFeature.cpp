#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "net.minecraft.world.level.biome.h"
#include "JavaMath.h"
#include "Mth.h"

const wstring MineShaftFeature::OPTION_CHANCE = L"chance";

MineShaftFeature::MineShaftFeature()
{
	chance = 0.01;
}

wstring MineShaftFeature::getFeatureName()
{
	return L"Mineshaft";
}

MineShaftFeature::MineShaftFeature(unordered_map<wstring, wstring> options)
{
	chance = 0.01;

	for(auto& option : options)
	{
		if (option.first.compare(OPTION_CHANCE) == 0)
		{
			chance = Mth::getDouble(option.second, chance);
		}
	}
}

bool MineShaftFeature::isFeatureChunk(int x, int z, bool bIsSuperflat)
{
	bool forcePlacement = false;
	LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
	if( levelGenOptions != nullptr )
	{
		forcePlacement = levelGenOptions->isFeatureChunk(x,z,eFeature_Mineshaft);
	}

	double effectiveChance = chance;
	if (level != nullptr)
	{
		Biome *b = level->getBiome(x * 16, z * 16);
		if (b == Biome::mesa || b == Biome::mesaPlateauF || b == Biome::mesaPlateau ||
		    b == Biome::mesaBryce || b == Biome::mesaPlateauFM || b == Biome::mesaPlateauM)
			effectiveChance = 0.9;
	}

	return forcePlacement || (random->nextDouble() < effectiveChance && random->nextInt(80) < max(abs(x), abs(z)));
}

StructureStart *MineShaftFeature::createStructureStart(int x, int z)
{
	// 4J added
	app.AddTerrainFeaturePosition(eTerrainFeature_Mineshaft,x,z);

	return new MineShaftStart(level, random, x, z);
}