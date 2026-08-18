#pragma once
#include "QuadrupedModel.h"

class PolarBearModel : public QuadrupedModel
{
public:
	PolarBearModel();
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
