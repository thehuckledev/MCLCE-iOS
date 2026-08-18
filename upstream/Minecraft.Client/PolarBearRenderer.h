#pragma once
#include "MobRenderer.h"

class PolarBearRenderer : public MobRenderer
{
private:
	static ResourceLocation POLARBEAR_LOCATION;

public:
	PolarBearRenderer(Model *model, float shadow);

	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
