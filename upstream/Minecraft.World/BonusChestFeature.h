#pragma once
#include "Feature.h"

#include <string>

class Random;
class Level;

class BonusChestFeature : public Feature
{

private:
	const std::string lootTableName;

public:
	explicit BonusChestFeature(const std::string &lootTableName);

	virtual bool place(Level *level, Random *random, int x, int y, int z);
	bool place(Level *level, Random *random, int x, int y, int z, bool force);      // 4J added this method with extra force parameter
};
