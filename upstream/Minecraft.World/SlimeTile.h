#pragma once
#include "HalfTransparentTile.h"

class Random;

class SlimeTile : public HalfTransparentTile
{
public:
	SlimeTile(int id);
	virtual int getRenderLayer();
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
	virtual int getRenderShape();
	virtual bool isSolidRender();
	virtual int getPistonPushReaction();

	// slime block logic
	virtual void fallOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity, float fallDistance);
	virtual void updateEntityAfterFallOn(Level *level, shared_ptr<Entity> entity);
    virtual void stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
};
