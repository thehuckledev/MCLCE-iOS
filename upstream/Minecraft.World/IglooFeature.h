#pragma once

#include "Feature.h"

class IglooFeature : public Feature
{
public:
	bool place(Level *level, Random *random, int x, int y, int z) override;

private:
	void placeBasement(Level *level, Random *random, int x, int y, int z);
};
