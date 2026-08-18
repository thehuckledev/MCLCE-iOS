#pragma once
#include "IceTile.h"

class Random;
class Icon;
class IconRegister;

class FrostedIceTile : public IceTile
{
public:
	static const int MAX_AGE = 3;

	FrostedIceTile(int id);

	virtual void registerIcons(IconRegister *iconRegister);
	virtual Icon *getTexture(int face, int data);

	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual void neighborChanged(Level *level, int x, int y, int z, int neighborId);
	virtual int getResourceCount(Random *random);
	virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data);

private:
	Icon *m_ageIcons[4];

	bool slightlyMelt(Level *level, int x, int y, int z, Random *random, bool spreadToNeighbours);
	void meltToWater(Level *level, int x, int y, int z);
	int countIceNeighbours(Level *level, int x, int y, int z);
	int randomTickDelay(Random *random);
};
