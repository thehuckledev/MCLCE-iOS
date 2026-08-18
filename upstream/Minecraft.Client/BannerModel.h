#pragma once
#include "Model.h"

class ModelPart;

class BannerModel : public Model
{
public:
	ModelPart *bannerSlate;
	ModelPart *bannerStand;
	ModelPart *bannerTop;

	BannerModel();
	void renderBanner(bool useCompiled);
};
