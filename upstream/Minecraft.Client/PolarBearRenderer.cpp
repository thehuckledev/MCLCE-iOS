#include "stdafx.h"
#include "PolarBearRenderer.h"

ResourceLocation PolarBearRenderer::POLARBEAR_LOCATION = ResourceLocation(TN_MOB_POLARBEAR);

PolarBearRenderer::PolarBearRenderer(Model *model, float shadow) : MobRenderer(model, shadow)
{
}

void PolarBearRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(_mob, x, y, z, rot, a);
}

ResourceLocation *PolarBearRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return &POLARBEAR_LOCATION;
}
