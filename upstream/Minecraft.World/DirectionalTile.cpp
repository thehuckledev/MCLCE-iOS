#include "stdafx.h"

#include "DirectionalTile.h"

DirectionalTile::DirectionalTile(int id, Material *material, bool isSolidRender) : Tile(id, material, isSolidRender)
{
	setLightBlock(0);
}

int DirectionalTile::getDirection(int data)
{
	return data & DIRECTION_MASK;
}