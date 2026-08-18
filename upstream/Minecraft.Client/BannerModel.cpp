#include "stdafx.h"
#include "BannerModel.h"
#include "ModelPart.h"

BannerModel::BannerModel()
{
	texWidth = 64;
	texHeight = 64;

	bannerSlate = new ModelPart(this, 0, 0);
	bannerSlate->addBox(-10.0f, 0.0f, -2.0f, 20, 40, 1, 0.0f);

	bannerStand = new ModelPart(this, 44, 0);
	bannerStand->addBox(-1.0f, -30.0f, -1.0f, 2, 42, 2, 0.0f);

	bannerTop = new ModelPart(this, 0, 42);
	bannerTop->addBox(-10.0f, -32.0f, -1.0f, 20, 2, 2, 0.0f);

	bannerStand->compile(1.0f / 16.0f);
	bannerTop->compile(1.0f / 16.0f);
}

void BannerModel::renderBanner(bool useCompiled)
{
	bannerSlate->render(1.0f / 16.0f, false);
	bannerStand->render(1.0f / 16.0f, useCompiled);
	bannerTop->render(1.0f / 16.0f, useCompiled);
}
