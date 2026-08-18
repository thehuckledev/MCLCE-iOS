#include "stdafx.h"
#include "HumanoidModel.h"
#include "../Minecraft.World/Mth.h"
#include "../Minecraft.World/Player.h"
#include "ModelPart.h"
#include "Cube.h"

// 4J added 

ModelPart * HumanoidModel::AddOrRetrievePart(SKIN_BOX *pBox)
{
	ModelPart *pAttachTo=nullptr;
	float scale=0;

	switch(pBox->ePart)
	{
	case eBodyPart_Head:
		pAttachTo=head;
		break;
	case eBodyPart_Body:
		pAttachTo=body;
		break;
	case eBodyPart_Arm0:
		pAttachTo=arm0;
		break;
	case eBodyPart_Arm1:
		pAttachTo=arm1;
		break;
	case eBodyPart_Leg0:
		pAttachTo=leg0;
		break;
	case eBodyPart_Leg1:
		pAttachTo=leg1;
		break;
	case eBodyPart_Jacket:
		pAttachTo=jacket;
		scale=0.25;
		break;
	case eBodyPart_Sleeve0:
		pAttachTo=sleeve0;
		scale=0.25;
		break;
	case eBodyPart_Sleeve1:
		pAttachTo=sleeve1;
		scale=0.25;
		break;
	case eBodyPart_Pants0:
		pAttachTo=pants0;
		scale=0.25;
		break;
	case eBodyPart_Pants1:
		pAttachTo=pants1;
		scale=0.25;
		break;
	case eBodyPart_Headwear:
		pAttachTo=hair;
		break;
	case eBodyPart_Waist:
		pAttachTo=waist;
		break;
	case eBodyPart_Belt:
		pAttachTo=belt;
		break;
	case eBodyPart_BodyArmor:
		pAttachTo=bodyArmor;
		break;
	case eBodyPart_ArmArmor0:
		pAttachTo=armArmor0;
		break;
	case eBodyPart_ArmArmor1:
		pAttachTo=armArmor1;
		break;
	case eBodyPart_Legging0:
		pAttachTo=legging0;
		break;
	case eBodyPart_Legging1:
		pAttachTo=legging1;
		break;
	case eBodyPart_Sock0:
		pAttachTo=sock0;
		break;
	case eBodyPart_Sock1:
		pAttachTo=sock1;
		break;
	case eBodyPart_Boot0:
		pAttachTo=boot0;
		break;
	case eBodyPart_Boot1:
		pAttachTo=boot1;
		break;
	}

	// check if this box has a declared scale then add it
	if (pBox->fS != 0) scale = pBox->fS;

	// first check this box doesn't already exist
	ModelPart *pNewBox = pAttachTo->retrieveChild(pBox);

	if(pNewBox)
	{
		if((pNewBox->getfU()!=static_cast<int>(pBox->fU)) || (pNewBox->getfV()!=static_cast<int>(pBox->fV)))
		{
			app.DebugPrintf("HumanoidModel::AddOrRetrievePart - Box geometry was found, but with different uvs\n");
			pNewBox=nullptr;
		}
	}
	if(pNewBox==nullptr)
	{
		//app.DebugPrintf("HumanoidModel::AddOrRetrievePart - Adding box to model part\n");

		pNewBox = new ModelPart(this, static_cast<int>(pBox->fU), static_cast<int>(pBox->fV));
		pNewBox->visible=false;
		if (pBox->fM > 0) pNewBox->bMirror = true; // check if this box has the mirror flag
		pNewBox->hideWithArmor = (unsigned int)pBox->fA; // add the "hide when armor is worn" bit flags
		pNewBox->addHumanoidBox(pBox->fX, pBox->fY, pBox->fZ, pBox->fW, pBox->fH, pBox->fD, scale);
		// 4J-PB - don't compile here, since the lighting isn't set up. It'll be compiled on first use.
		//pNewBox->compile(1.0f/16.0f);
		pAttachTo->addChild(pNewBox);
	}

	return pNewBox;
}

void HumanoidModel::_init(float g, float yOffset, int texWidth, int texHeight, bool slim, bool isArmor)
{
	this->texWidth = texWidth;
	this->texHeight = texHeight;

	m_is64x64 = texHeight == 64;

	jacket = nullptr;
	sleeve0 = nullptr;
	sleeve1 = nullptr;
	pants0 = nullptr;
	pants1 = nullptr;

	waist = nullptr;
	belt = nullptr;
	bodyArmor = nullptr;
	armArmor0 = nullptr;
	armArmor1 = nullptr;
	legging0 = nullptr;
	legging1 = nullptr;
	sock0 = nullptr;
	sock1 = nullptr;
	boot0 = nullptr;
	boot1 = nullptr;

	m_fYOffset=yOffset;
    cloak = new ModelPart(this, 0, 0);
    cloak->addHumanoidBox(-5, -0, -1, 10, 16, 1, g); // Cloak

	elytraRight = new ModelPart(this, 22, 0);
	elytraRight->addHumanoidBox(-10.0f, 0.0f, 0.0f, 10, 20, 2, 0.0f);
	elytraRight->setPos(5.0f, 0.0f + yOffset, 0.0f); // Wing Left

	elytraLeft = new ModelPart(this, 22, 0);
	elytraLeft->bMirror = true;
	elytraLeft->addHumanoidBox(0.0f, 0.0f, 0.0f, 10, 20, 2, 0.0f);
	elytraLeft->setPos(-5.0f, 0.0f + yOffset, 0.0f); // Wing Right

    ear = new ModelPart(this, 24, 0);
    ear->addHumanoidBox(-3, -6, -1, 6, 6, 1, g); // Ear
        
    head = new ModelPart(this, 0, 0);
    head->addHumanoidBox(-4, -8, -4, 8, 8, 8, g); // Head
    head->setPos(0, 0 + yOffset, 0);

    hair = new ModelPart(this, 32, 0);
    hair->addHumanoidBox(-4, -8, -4, 8, 8, 8, g + 0.5f); // Head
    hair->setPos(0, 0 + yOffset, 0);

    body = new ModelPart(this, 16, 16);
    body->addHumanoidBox(-4, 0, -2, 8, 12, 4, g); // Body
    body->setPos(0, 0 + yOffset, 0);

	if (m_is64x64)
	{
		arm0 = new ModelPart(this, 24 + 16, 16);
		arm1 = new ModelPart(this, 16 + 16, 48);

		sleeve0 = new ModelPart(this, 24 + 16, 32);
		sleeve1 = new ModelPart(this, 32 + 16, 48);

		if (!slim)
		{
			sleeve0->addHumanoidBox(-3, -2, -2, 4, 12, 4, g + 0.25f); // Sleeve0
			sleeve1->addHumanoidBox(-1, -2, -2, 4, 12, 4, g + 0.25f); // Sleeve1
		}
		else if (slim)
		{
			sleeve0->addHumanoidBox(-2, -2, -2, 3, 12, 4, g + 0.25f); // Sleeve0 Slim
			sleeve1->addHumanoidBox(-1, -2, -2, 3, 12, 4, g + 0.25f); // Sleeve1 Slim
		}

		jacket = new ModelPart(this, 16, 32);
		jacket->addHumanoidBox(-4, 0, -2, 8, 12, 4, g + 0.25f);
		jacket->setPos(0, 0 + yOffset, 0);

		sleeve0->setPos(-5, 2 + yOffset, 0);
		sleeve1->setPos(5, 2 + yOffset, 0);

		waist = new ModelPart(this, 0, 0);
		waist->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // Waist
		waist->setPos(0, 0 + yOffset, 0);
		belt = new ModelPart(this, 0, 0);
		belt->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // Belt
		belt->setPos(0, 0 + yOffset, 0);
		bodyArmor = new ModelPart(this, 0, 0);
		bodyArmor->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // BodyArmor
		bodyArmor->setPos(0, 0 + yOffset, 0);
		armArmor0 = new ModelPart(this, 0, 0);
		armArmor0->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // ArmArmor0
		armArmor0->setPos(-5, 2 + yOffset, 0);
		armArmor1 = new ModelPart(this, 0, 0);
		armArmor1->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // ArmArmor1
		armArmor1->setPos(5, 2 + yOffset, 0);
		legging0 = new ModelPart(this, 0, 0);
		legging0->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // Legging0
		legging0->setPos(-1.9, 12 + yOffset, 0);
		legging1 = new ModelPart(this, 0, 0);
		legging1->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // Legging1
		legging1->setPos(1.9, 12 + yOffset, 0);
		sock0 = new ModelPart(this, 0, 0);
		sock0->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // Sock0
		sock0->setPos(-1.9, 12 + yOffset, 0);
		sock1 = new ModelPart(this, 0, 0);
		sock1->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // Sock1
		sock1->setPos(1.9, 12 + yOffset, 0);
		boot0 = new ModelPart(this, 0, 0);
		boot0->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // Boot0
		boot0->setPos(-1.9, 12 + yOffset, 0);
		boot1 = new ModelPart(this, 0, 0);
		boot1->addHumanoidBox(0, 0, 0, 0, 0, 0, g); // Boot1
		boot1->setPos(1.9, 12 + yOffset, 0);
	}
	else if (!m_is64x64)
	{
		arm0 = new ModelPart(this, 24 + 16, 16);
		arm1 = new ModelPart(this, 24 + 16, 16);
		arm1->bMirror = true;
	}

	if (!slim)
	{
		arm0->addHumanoidBox(-3, -2, -2, 4, 12, 4, g); // Arm0
		arm1->addHumanoidBox(-1, -2, -2, 4, 12, 4, g); // Arm1
	}
	else if (slim)
	{
		arm0->addHumanoidBox(-2, -2, -2, 3, 12, 4, g); // Arm0 Slim
		arm1->addHumanoidBox(-1, -2, -2, 3, 12, 4, g); // Arm1 Slim
	}

	arm0->setPos(-5, 2 + yOffset, 0);
	arm1->setPos(5, 2 + yOffset, 0);

	leg0 = new ModelPart(this, 0, 16);
	if (m_is64x64)
	{
		leg1 = new ModelPart(this, 16, 48);

		pants0 = new ModelPart(this, 0, 32);
		pants0->addHumanoidBox(-2, 0, -2, 4, 12, 4, g + 0.25f); // Pants0
		pants0->setPos(-1.9, 12 + yOffset, 0);

		pants1 = new ModelPart(this, 0, 48);
		pants1->addHumanoidBox(-2, 0, -2, 4, 12, 4, g + 0.25f); // Pants1
		pants1->setPos(1.9, 12 + yOffset, 0);
	}
	else if (!m_is64x64)
	{
		leg1 = new ModelPart(this, 0, 16);
		leg1->bMirror = true;
	}

    leg0->addHumanoidBox(-2, 0, -2, 4, 12, 4, g); // Leg0
    leg0->setPos(-1.9, 12 + yOffset, 0);

    leg1->addHumanoidBox(-2, 0, -2, 4, 12, 4, g); // Leg1
    leg1->setPos(1.9, 12 + yOffset, 0);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	// 4J Stu - Not just performance, but alpha+depth tests don't work right unless we compile here
	cloak->compile(1.0f/16.0f);
	elytraLeft->compile(1.0f / 16.0f);
	elytraRight->compile(1.0f / 16.0f);
	ear->compile(1.0f/16.0f);
	head->compile(1.0f/16.0f);
	body->compile(1.0f/16.0f);
	arm0->compile(1.0f/16.0f);
	arm1->compile(1.0f/16.0f);
	leg0->compile(1.0f/16.0f);
	leg1->compile(1.0f/16.0f);
	hair->compile(1.0f/16.0f);

	if (m_is64x64)
	{
		jacket->compile(1.0f/16.0f);
		sleeve0->compile(1.0f/16.0f);
		sleeve1->compile(1.0f/16.0f);
		pants0->compile(1.0f/16.0f);
		pants1->compile(1.0f/16.0f);
		waist->compile(1.0f/16.0f);
		belt->compile(1.0f/16.0f);
		bodyArmor->compile(1.0f/16.0f);
		armArmor0->compile(1.0f/16.0f);
		armArmor1->compile(1.0f/16.0f);
		legging0->compile(1.0f/16.0f);
		legging1->compile(1.0f/16.0f);
		sock0->compile(1.0f/16.0f);
		sock1->compile(1.0f/16.0f);
		boot0->compile(1.0f/16.0f);
		boot1->compile(1.0f/16.0f);
	}

	holdingLeftHand=0;
	holdingRightHand=0;
	sneaking=false;
	idle=false;
	bowAndArrow=false;
	elytraFlying = false;
	elytraCrouching = false;
	m_isArmor = isArmor;

	// 4J added
	eating = false;
	eating_t = 0.0f;
	eating_swing = 0.0f;
	m_uiAnimOverrideBitmask = 0L;
}


HumanoidModel::HumanoidModel() : Model()
{
	_init(0, 0, 64, 32, false, false);
}

HumanoidModel::HumanoidModel(float g) : Model()
{
	_init(g, 0, 64, 32, false, false);
}

HumanoidModel::HumanoidModel(float g, bool isArmor) : Model()
{
	_init(g, 0, 64, 32, false, isArmor);
}

HumanoidModel::HumanoidModel(float g, float yOffset, int texWidth, int texHeight) : Model()
{
	_init(g, yOffset, texWidth, texHeight, false, false);
}

HumanoidModel::HumanoidModel(float g, float yOffset, int texWidth, int texHeight, bool slim) : Model()
{
	_init(g, yOffset, texWidth, texHeight, slim, false);
}

void HumanoidModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	float headOffsets[3] = {0};
	float bodyOffsets[3] = {0};
	float arm0Offsets[3] = {0};
	float arm1Offsets[3] = {0};
	float leg0Offsets[3] = {0};
	float leg1Offsets[3] = {0};

	if(entity != nullptr)
	{
		m_uiAnimOverrideBitmask=entity->getAnimOverrideBitmask();

		// Reset offsets so they don't leak onto other skins - Langtanium
		head->translateX = 0;
		head->translateY = 0;
		head->translateZ = 0;
		hair->translateX = 0;
		hair->translateY = 0;
		hair->translateZ = 0;
		body->translateX = 0;
		body->translateY = 0;
		body->translateZ = 0;
		arm0->translateX = 0;
		arm0->translateY = 0;
		arm0->translateZ = 0;
		arm1->translateX = 0;
		arm1->translateY = 0;
		arm1->translateZ = 0;
		leg0->translateX = 0;
		leg0->translateY = 0;
		leg0->translateZ = 0;
		leg1->translateX = 0;
		leg1->translateY = 0;
		leg1->translateZ = 0;
		if (m_is64x64)
		{
			jacket->translateX = 0;
			jacket->translateY = 0;
			jacket->translateZ = 0;
			sleeve0->translateX = 0;
			sleeve0->translateY = 0;
			sleeve0->translateZ = 0;
			sleeve1->translateX = 0;
			sleeve1->translateY = 0;
			sleeve1->translateZ = 0;
			pants0->translateX = 0;
			pants0->translateY = 0;
			pants0->translateZ = 0;
			pants1->translateX = 0;
			pants1->translateY = 0;
			pants1->translateZ = 0;
			bodyArmor->translateX = 0;
			bodyArmor->translateY = 0;
			bodyArmor->translateZ = 0;
			waist->translateX = 0;
			waist->translateY = 0;
			waist->translateZ = 0;
			belt->translateX = 0;
			belt->translateY = 0;
			belt->translateZ = 0;
			armArmor0->translateX = 0;
			armArmor0->translateY = 0;
			armArmor0->translateZ = 0;
			armArmor1->translateX = 0;
			armArmor1->translateY = 0;
			armArmor1->translateZ = 0;
			legging0->translateX = 0;
			legging0->translateY = 0;
			legging0->translateZ = 0;
			legging1->translateX = 0;
			legging1->translateY = 0;
			legging1->translateZ = 0;
			sock0->translateX = 0;
			sock0->translateY = 0;
			sock0->translateZ = 0;
			sock1->translateX = 0;
			sock1->translateY = 0;
			sock1->translateZ = 0;
			boot0->translateX = 0;
			boot0->translateY = 0;
			boot0->translateZ = 0;
			boot1->translateX = 0;
			boot1->translateY = 0;
			boot1->translateZ = 0;
		}

		shared_ptr<Player> player = dynamic_pointer_cast<Player>(entity);
		vector<SKIN_OFFSET *>* pSkinOffsets = nullptr;
		if (player != nullptr)
			pSkinOffsets = player->GetSkinOffsets();
		if (pSkinOffsets != nullptr)
		{
			for( SKIN_OFFSET *pSkinOffset : *pSkinOffsets )
			{
				switch (pSkinOffset->ePart)
				{
				case eBodyOffset_Head:
					if (pSkinOffset->fD == 1 && head->translateX == 0)
					{
						head->translateX = pSkinOffset->fO / 16.0f;
						hair->translateX = pSkinOffset->fO / 16.0f;
						headOffsets[0] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 2 && head->translateY == 0)
					{
						head->translateY = pSkinOffset->fO / 16.0f;
						hair->translateY = pSkinOffset->fO / 16.0f;
						headOffsets[1] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 3 && head->translateZ == 0)
					{
						head->translateZ = pSkinOffset->fO / 16.0f;
						hair->translateZ = pSkinOffset->fO / 16.0f;
						headOffsets[2] = pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Body:
					if (pSkinOffset->fD == 1 && body->translateX == 0)
					{
						body->translateX = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							jacket->translateX = pSkinOffset->fO / 16.0f;
							bodyArmor->translateX = pSkinOffset->fO / 16.0f;
							waist->translateX = pSkinOffset->fO / 16.0f;
							belt->translateX = pSkinOffset->fO / 16.0f;
						}
						bodyOffsets[0] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 2 && body->translateY == 0)
					{
						body->translateY = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							jacket->translateY = pSkinOffset->fO / 16.0f;
							bodyArmor->translateY = pSkinOffset->fO / 16.0f;
							waist->translateY = pSkinOffset->fO / 16.0f;
							belt->translateY = pSkinOffset->fO / 16.0f;
						}
						bodyOffsets[1] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 3 && body->translateZ == 0)
					{
						body->translateZ = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							jacket->translateZ = pSkinOffset->fO / 16.0f;
							bodyArmor->translateZ = pSkinOffset->fO / 16.0f;
							waist->translateZ = pSkinOffset->fO / 16.0f;
							belt->translateZ = pSkinOffset->fO / 16.0f;
						}
						bodyOffsets[2] = pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Arm0:
					if (pSkinOffset->fD == 1 && arm0->translateX == 0)
					{
						arm0->translateX = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							sleeve0->translateX = pSkinOffset->fO / 16.0f;
							armArmor0->translateX = pSkinOffset->fO / 16.0f;
						}
						arm0Offsets[0] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 2 && arm0->translateY == 0)
					{
						arm0->translateY = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							sleeve0->translateY = pSkinOffset->fO / 16.0f;
							armArmor0->translateY = pSkinOffset->fO / 16.0f;
						}
						arm0Offsets[1] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 3 && arm0->translateZ == 0)
					{
						arm0->translateZ = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							sleeve0->translateZ = pSkinOffset->fO / 16.0f;
							armArmor0->translateZ = pSkinOffset->fO / 16.0f;
						}
						arm0Offsets[2] = pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Arm1:
					if (pSkinOffset->fD == 1 && arm1->translateX == 0)
					{
						arm1->translateX = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							sleeve1->translateX = pSkinOffset->fO / 16.0f;
							armArmor1->translateX = pSkinOffset->fO / 16.0f;
						}
						arm1Offsets[0] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 2 && arm1->translateY == 0)
					{
						arm1->translateY = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							sleeve1->translateY = pSkinOffset->fO / 16.0f;
							armArmor1->translateY = pSkinOffset->fO / 16.0f;
						}
						arm1Offsets[1] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 3 && arm1->translateZ == 0)
					{
						arm1->translateZ = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							sleeve1->translateZ = pSkinOffset->fO / 16.0f;
							armArmor1->translateZ = pSkinOffset->fO / 16.0f;
						}
						arm1Offsets[2] = pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Leg0:
					if (pSkinOffset->fD == 1 && leg0->translateX == 0)
					{
						leg0->translateX = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							pants0->translateX = pSkinOffset->fO / 16.0f;
							legging0->translateX = pSkinOffset->fO / 16.0f;
							sock0->translateX = pSkinOffset->fO / 16.0f;
							boot0->translateX = pSkinOffset->fO / 16.0f;
						}
						leg0Offsets[0] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 2 && leg0->translateY == 0)
					{
						leg0->translateY = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							pants0->translateY = pSkinOffset->fO / 16.0f;
							legging0->translateY = pSkinOffset->fO / 16.0f;
							sock0->translateY = pSkinOffset->fO / 16.0f;
							boot0->translateY = pSkinOffset->fO / 16.0f;
						}
						leg0Offsets[1] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 3 && leg0->translateZ == 0)
					{
						leg0->translateZ = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							pants0->translateZ = pSkinOffset->fO / 16.0f;
							legging0->translateZ = pSkinOffset->fO / 16.0f;
							sock0->translateZ = pSkinOffset->fO / 16.0f;
							boot0->translateZ = pSkinOffset->fO / 16.0f;
						}
						leg0Offsets[2] = pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Leg1:
					if (pSkinOffset->fD == 1 && leg1->translateX == 0)
					{
						leg1->translateX = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							pants1->translateX = pSkinOffset->fO / 16.0f;
							legging1->translateX = pSkinOffset->fO / 16.0f;
							sock1->translateX = pSkinOffset->fO / 16.0f;
							boot1->translateX = pSkinOffset->fO / 16.0f;
						}
						leg1Offsets[0] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 2 && leg1->translateY == 0)
					{
						leg1->translateY = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							pants1->translateY = pSkinOffset->fO / 16.0f;
							legging1->translateY = pSkinOffset->fO / 16.0f;
							sock1->translateY = pSkinOffset->fO / 16.0f;
							boot1->translateY = pSkinOffset->fO / 16.0f;
						}
						leg1Offsets[1] = pSkinOffset->fO / 16.0f;
					}
					else if (pSkinOffset->fD == 3 && leg1->translateZ == 0)
					{
						leg1->translateZ = pSkinOffset->fO / 16.0f;
						if (m_is64x64)
						{
							pants1->translateZ = pSkinOffset->fO / 16.0f;
							legging1->translateZ = pSkinOffset->fO / 16.0f;
							sock1->translateZ = pSkinOffset->fO / 16.0f;
							boot1->translateZ = pSkinOffset->fO / 16.0f;
						}
						leg1Offsets[2] = pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Helmet:
					if (m_isArmor)
					{
						if (pSkinOffset->fD == 1 && head->translateX == headOffsets[0])
						{
							head->translateX += pSkinOffset->fO / 16.0f;
							hair->translateX += pSkinOffset->fO / 16.0f;
						}
						else if (pSkinOffset->fD == 2 && head->translateY == headOffsets[1])
						{
							head->translateY += pSkinOffset->fO / 16.0f;
							hair->translateY += pSkinOffset->fO / 16.0f;
						}
						else if (pSkinOffset->fD == 3 && head->translateZ == headOffsets[2])
						{
							head->translateZ += pSkinOffset->fO / 16.0f;
							hair->translateZ += pSkinOffset->fO / 16.0f;
						}
					}
					break;
				case eBodyOffset_BodyArmor:
					if (m_isArmor && !body->isArmorPart2)
					{
						if (pSkinOffset->fD == 1 && body->translateX == bodyOffsets[0])
							body->translateX += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 2 && body->translateY == bodyOffsets[1])
							body->translateY += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 3 && body->translateZ == bodyOffsets[2])
							body->translateZ += pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_ArmArmor0:
					if (m_isArmor)
					{
						if (pSkinOffset->fD == 1 && arm0->translateX == arm0Offsets[0])
							arm0->translateX += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 2 && arm0->translateY == arm0Offsets[1])
							arm0->translateY += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 3 && arm0->translateZ == arm0Offsets[2])
							arm0->translateZ += pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_ArmArmor1:
					if (m_isArmor)
					{
						if (pSkinOffset->fD == 1 && arm1->translateX == arm1Offsets[0])
							arm1->translateX += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 2 && arm1->translateY == arm1Offsets[1])
							arm1->translateY += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 3 && arm1->translateZ == arm1Offsets[2])
							arm1->translateZ += pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Waist:
					if (m_isArmor && body->isArmorPart2)
					{
						if (pSkinOffset->fD == 1 && body->translateX == bodyOffsets[0])
							body->translateX += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 2 && body->translateY == bodyOffsets[1])
							body->translateY += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 3 && body->translateZ == bodyOffsets[2])
							body->translateZ += pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Legging0:
					if (m_isArmor && leg0->isArmorPart2)
					{
						if (pSkinOffset->fD == 1 && leg0->translateX == leg0Offsets[0])
							leg0->translateX += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 2 && leg0->translateY == leg0Offsets[1])
							leg0->translateY += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 3 && leg0->translateZ == leg0Offsets[2])
							leg0->translateZ += pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Legging1:
					if (m_isArmor && leg1->isArmorPart2)
					{
						if (pSkinOffset->fD == 1 && leg1->translateX == leg1Offsets[0])
							leg1->translateX += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 2 && leg1->translateY == leg1Offsets[1])
							leg1->translateY += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 3 && leg1->translateZ == leg1Offsets[2])
							leg1->translateZ += pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Boot0:
					if (m_isArmor && !leg0->isArmorPart2)
					{
						if (pSkinOffset->fD == 1 && leg0->translateX == leg0Offsets[0])
							leg0->translateX += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 2 && leg0->translateY == leg0Offsets[1])
							leg0->translateY += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 3 && leg0->translateZ == leg0Offsets[2])
							leg0->translateZ += pSkinOffset->fO / 16.0f;
					}
					break;
				case eBodyOffset_Boot1:
					if (m_isArmor && !leg1->isArmorPart2)
					{
						if (pSkinOffset->fD == 1 && leg1->translateX == leg1Offsets[0])
							leg1->translateX += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 2 && leg1->translateY == leg1Offsets[1])
							leg1->translateY += pSkinOffset->fO / 16.0f;
						else if (pSkinOffset->fD == 3 && leg1->translateZ == leg1Offsets[2])
							leg1->translateZ += pSkinOffset->fO / 16.0f;
					}
					break;
				}
			}
		}
	}

	setupAnim(time, r, bob, yRot, xRot, scale, entity, m_uiAnimOverrideBitmask);

	if (young)
	{
		float ss = 2.0f;
		glPushMatrix();
		glScalef(1.5f / ss, 1.5f / ss, 1.5f / ss);
		glTranslatef(0, 16 * scale, 0);
		head->render(scale, usecompiled);
		glPopMatrix();
		glPushMatrix();
		glScalef(1 / ss, 1 / ss, 1 / ss);
		glTranslatef(0, 24 * scale, 0);
		body->render(scale, usecompiled);
		arm0->render(scale, usecompiled);
		arm1->render(scale, usecompiled);
		leg0->render(scale, usecompiled);
		leg1->render(scale, usecompiled);
		hair->render(scale, usecompiled);

		if (m_is64x64)
		{
			jacket->render(scale, usecompiled);
			sleeve0->render(scale, usecompiled);
			sleeve1->render(scale, usecompiled);
			pants0->render(scale, usecompiled);
			pants1->render(scale, usecompiled);
		}

		glPopMatrix();
	}
	else
	{
		head->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderHead))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorHead))>0||!m_isArmor));
		body->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderTorso))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorTorso))>0||!m_isArmor));
		arm0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderArm0))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorArm0))>0||!m_isArmor));
		arm1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderArm1))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorArm1))>0||!m_isArmor));
		leg0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderLeg0))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorLeg0))>0||!m_isArmor));
		leg1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderLeg1))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorLeg1))>0||!m_isArmor));
		hair->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderHair))>0);
		if (m_is64x64)
		{
			jacket->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderJacket))>0);
			sleeve0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderSleeve0))>0);
			sleeve1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderSleeve1))>0);
			pants0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderPants0))>0);
			pants1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderPants1))>0);
			waist->render(scale, usecompiled);
			belt->render(scale, usecompiled);
			bodyArmor->render(scale, usecompiled);
			armArmor0->render(scale, usecompiled);
			armArmor1->render(scale, usecompiled);
			legging0->render(scale, usecompiled);
			legging1->render(scale, usecompiled);
			sock0->render(scale, usecompiled);
			sock1->render(scale, usecompiled);
			boot0->render(scale, usecompiled);
			boot1->render(scale, usecompiled);
		}
	}
}

// This code is similar to what's above, but allows skin offsets to work in the skin select menu - Langtanium
void HumanoidModel::renderUI(float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled, vector<SKIN_OFFSET *> *skinOffsets)
{
	// Reset offsets so they don't leak onto other skins - Langtanium
	head->translateX = 0;
	head->translateY = 0;
	head->translateZ = 0;
	hair->translateX = 0;
	hair->translateY = 0;
	hair->translateZ = 0;
	body->translateX = 0;
	body->translateY = 0;
	body->translateZ = 0;
	arm0->translateX = 0;
	arm0->translateY = 0;
	arm0->translateZ = 0;
	arm1->translateX = 0;
	arm1->translateY = 0;
	arm1->translateZ = 0;
	leg0->translateX = 0;
	leg0->translateY = 0;
	leg0->translateZ = 0;
	leg1->translateX = 0;
	leg1->translateY = 0;
	leg1->translateZ = 0;
	if (m_is64x64)
	{
		jacket->translateX = 0;
		jacket->translateY = 0;
		jacket->translateZ = 0;
		sleeve0->translateX = 0;
		sleeve0->translateY = 0;
		sleeve0->translateZ = 0;
		sleeve1->translateX = 0;
		sleeve1->translateY = 0;
		sleeve1->translateZ = 0;
		pants0->translateX = 0;
		pants0->translateY = 0;
		pants0->translateZ = 0;
		pants1->translateX = 0;
		pants1->translateY = 0;
		pants1->translateZ = 0;
		bodyArmor->translateX = 0;
		bodyArmor->translateY = 0;
		bodyArmor->translateZ = 0;
		waist->translateX = 0;
		waist->translateY = 0;
		waist->translateZ = 0;
		belt->translateX = 0;
		belt->translateY = 0;
		belt->translateZ = 0;
		armArmor0->translateX = 0;
		armArmor0->translateY = 0;
		armArmor0->translateZ = 0;
		armArmor1->translateX = 0;
		armArmor1->translateY = 0;
		armArmor1->translateZ = 0;
		legging0->translateX = 0;
		legging0->translateY = 0;
		legging0->translateZ = 0;
		legging1->translateX = 0;
		legging1->translateY = 0;
		legging1->translateZ = 0;
		sock0->translateX = 0;
		sock0->translateY = 0;
		sock0->translateZ = 0;
		sock1->translateX = 0;
		sock1->translateY = 0;
		sock1->translateZ = 0;
		boot0->translateX = 0;
		boot0->translateY = 0;
		boot0->translateZ = 0;
		boot1->translateX = 0;
		boot1->translateY = 0;
		boot1->translateZ = 0;
	}

	if (skinOffsets != nullptr)
	{
		for( SKIN_OFFSET *pSkinOffset : *skinOffsets )
		{
			switch (pSkinOffset->ePart)
			{
			case eBodyOffset_Head:
				if (pSkinOffset->fD == 1 && head->translateX == 0)
				{
					head->translateX = pSkinOffset->fO / 16.0f;
					hair->translateX = pSkinOffset->fO / 16.0f;
				}
				else if (pSkinOffset->fD == 2 && head->translateY == 0)
				{
					head->translateY = pSkinOffset->fO / 16.0f;
					hair->translateY = pSkinOffset->fO / 16.0f;
				}
				else if (pSkinOffset->fD == 3 && head->translateZ == 0)
				{
					head->translateZ = pSkinOffset->fO / 16.0f;
					hair->translateZ = pSkinOffset->fO / 16.0f;
				}
				break;
			case eBodyOffset_Body:
				if (pSkinOffset->fD == 1 && body->translateX == 0)
				{
					body->translateX = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						jacket->translateX = pSkinOffset->fO / 16.0f;
						bodyArmor->translateX = pSkinOffset->fO / 16.0f;
						waist->translateX = pSkinOffset->fO / 16.0f;
						belt->translateX = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 2 && body->translateY == 0)
				{
					body->translateY = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						jacket->translateY = pSkinOffset->fO / 16.0f;
						bodyArmor->translateY = pSkinOffset->fO / 16.0f;
						waist->translateY = pSkinOffset->fO / 16.0f;
						belt->translateY = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 3 && body->translateZ == 0)
				{
					body->translateZ = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						jacket->translateZ = pSkinOffset->fO / 16.0f;
						bodyArmor->translateZ = pSkinOffset->fO / 16.0f;
						waist->translateZ = pSkinOffset->fO / 16.0f;
						belt->translateZ = pSkinOffset->fO / 16.0f;
					}
				}
				break;
			case eBodyOffset_Arm0:
				if (pSkinOffset->fD == 1 && arm0->translateX == 0)
				{
					arm0->translateX = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						sleeve0->translateX = pSkinOffset->fO / 16.0f;
						armArmor0->translateX = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 2 && arm0->translateY == 0)
				{
					arm0->translateY = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						sleeve0->translateY = pSkinOffset->fO / 16.0f;
						armArmor0->translateY = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 3 && arm0->translateZ == 0)
				{
					arm0->translateZ = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						sleeve0->translateZ = pSkinOffset->fO / 16.0f;
						armArmor0->translateZ = pSkinOffset->fO / 16.0f;
					}
				}
				break;
			case eBodyOffset_Arm1:
				if (pSkinOffset->fD == 1 && arm1->translateX == 0)
				{
					arm1->translateX = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						sleeve1->translateX = pSkinOffset->fO / 16.0f;
						armArmor1->translateX = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 2 && arm1->translateY == 0)
				{
					arm1->translateY = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						sleeve1->translateY = pSkinOffset->fO / 16.0f;
						armArmor1->translateY = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 3 && arm1->translateZ == 0)
				{
					arm1->translateZ = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						sleeve1->translateZ = pSkinOffset->fO / 16.0f;
						armArmor1->translateZ = pSkinOffset->fO / 16.0f;
					}
				}
				break;
			case eBodyOffset_Leg0:
				if (pSkinOffset->fD == 1 && leg0->translateX == 0)
				{
					leg0->translateX = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						pants0->translateX = pSkinOffset->fO / 16.0f;
						legging0->translateX = pSkinOffset->fO / 16.0f;
						sock0->translateX = pSkinOffset->fO / 16.0f;
						boot0->translateX = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 2 && leg0->translateY == 0)
				{
					leg0->translateY = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						pants0->translateY = pSkinOffset->fO / 16.0f;
						legging0->translateY = pSkinOffset->fO / 16.0f;
						sock0->translateY = pSkinOffset->fO / 16.0f;
						boot0->translateY = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 3 && leg0->translateZ == 0)
				{
					leg0->translateZ = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						pants0->translateZ = pSkinOffset->fO / 16.0f;
						legging0->translateZ = pSkinOffset->fO / 16.0f;
						sock0->translateZ = pSkinOffset->fO / 16.0f;
						boot0->translateZ = pSkinOffset->fO / 16.0f;
					}
				}
				break;
			case eBodyOffset_Leg1:
				if (pSkinOffset->fD == 1 && leg1->translateX == 0)
				{
					leg1->translateX = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						pants1->translateX = pSkinOffset->fO / 16.0f;
						legging1->translateX = pSkinOffset->fO / 16.0f;
						sock1->translateX = pSkinOffset->fO / 16.0f;
						boot1->translateX = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 2 && leg1->translateY == 0)
				{
					leg1->translateY = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						pants1->translateY = pSkinOffset->fO / 16.0f;
						legging1->translateY = pSkinOffset->fO / 16.0f;
						sock1->translateY = pSkinOffset->fO / 16.0f;
						boot1->translateY = pSkinOffset->fO / 16.0f;
					}
				}
				else if (pSkinOffset->fD == 3 && leg1->translateZ == 0)
				{
					leg1->translateZ = pSkinOffset->fO / 16.0f;
					if (m_is64x64)
					{
						pants1->translateZ = pSkinOffset->fO / 16.0f;
						legging1->translateZ = pSkinOffset->fO / 16.0f;
						sock1->translateZ = pSkinOffset->fO / 16.0f;
						boot1->translateZ = pSkinOffset->fO / 16.0f;
					}
				}
				break;
			}
		}
	}

	setupAnim(time, r, bob, yRot, xRot, scale, nullptr, m_uiAnimOverrideBitmask);

	head->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderHead))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorHead))>0||!m_isArmor));
	body->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderTorso))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorTorso))>0||!m_isArmor));
	arm0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderArm0))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorArm0))>0||!m_isArmor));
	arm1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderArm1))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorArm1))>0||!m_isArmor));
	leg0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderLeg0))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorLeg0))>0||!m_isArmor));
	leg1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderLeg1))>0&&(!(m_uiAnimOverrideBitmask&(1<<eAnim_RenderArmorLeg1))>0||!m_isArmor));
	hair->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderHair))>0);
	if (m_is64x64)
	{
		jacket->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderJacket))>0);
		sleeve0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderSleeve0))>0);
		sleeve1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderSleeve1))>0);
		pants0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderPants0))>0);
		pants1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderPants1))>0);
		waist->render(scale, usecompiled);
		belt->render(scale, usecompiled);
		bodyArmor->render(scale, usecompiled);
		armArmor0->render(scale, usecompiled);
		armArmor1->render(scale, usecompiled);
		legging0->render(scale, usecompiled);
		legging1->render(scale, usecompiled);
		sock0->render(scale, usecompiled);
		sock1->render(scale, usecompiled);
		boot0->render(scale, usecompiled);
		boot1->render(scale, usecompiled);
	}
}

void HumanoidModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	//bool bIsAttacking = (attackTime > -9990.0f);
	
	{
		head->yRot = yRot / (float) (180.0f / PI);
		head->xRot = xRot / (float) (180.0f / PI);
		hair->yRot = head->yRot;
		hair->xRot = head->xRot;
		body->z = 0.0f;

		// Does the skin have an override for anim?

		if(uiBitmaskOverrideAnim&(1<<eAnim_ArmsDown))
		{
			arm0->xRot=0.0f;
			arm1->xRot=0.0f;
			arm0->zRot = 0.0f;
			arm1->zRot = 0.0f;
		}
		else if(uiBitmaskOverrideAnim&(1<<eAnim_ArmsOutFront))
		{
			arm0->xRot=-HALF_PI;
			arm1->xRot=-HALF_PI;
			arm0->zRot = 0.0f;
			arm1->zRot = 0.0f;
		}
		else if(uiBitmaskOverrideAnim&(1<<eAnim_SingleArms))
		{
			arm0->xRot = (Mth::cos(time * 0.6662f + PI) * 2.0f) * r * 0.5f;
			arm1->xRot = (Mth::cos(time * 0.6662f + PI) * 2.0f) * r * 0.5f;
			arm0->zRot = 0.0f;
			arm1->zRot = 0.0f;
		}
		// 4J-PB - Weeping Angel - does't look good holding something in the arm that's up
		else if((uiBitmaskOverrideAnim&(1<<eAnim_StatueOfLiberty)) && (holdingRightHand==0) && (attackTime==0.0f))
		{
			arm0->xRot = -PI;
			arm0->zRot = -0.3f;
			arm1->xRot = ( Mth::cos(time * 0.6662f) * 2.0f) * r * 0.5f;
			arm1->zRot = 0.0f;
		}
		else
		{
			arm0->xRot = (Mth::cos(time * 0.6662f + PI) * 2.0f) * r * 0.5f;
			arm1->xRot = ( Mth::cos(time * 0.6662f) * 2.0f) * r * 0.5f;
			arm0->zRot = 0.0f;
			arm1->zRot = 0.0f;
		}
		//        arm0.zRot = ((float) (util.Mth.cos(time * 0.2312f) + 1) * 1) * r;


		//        arm1.zRot = ((float) (util.Mth.cos(time * 0.2812f) - 1) * 1) * r;


		leg0->yRot = 0.0f;
		leg1->yRot = 0.0f;

		if (riding)
		{
			if ((uiBitmaskOverrideAnim&(1<<eAnim_SmallModel)) == 0)
			{
				arm0->xRot += -HALF_PI * 0.4f;
				arm1->xRot += -HALF_PI * 0.4f;
				leg0->xRot = -HALF_PI * 0.8f;
				leg1->xRot = -HALF_PI * 0.8f;
				leg0->yRot = HALF_PI * 0.2f;
				leg1->yRot = -HALF_PI * 0.2f;
			}
			else
			{
				arm0->xRot += -HALF_PI * 0.4f;
				arm1->xRot += -HALF_PI * 0.4f;
				leg0->xRot = -HALF_PI * 0.4f;
				leg1->xRot = -HALF_PI * 0.4f;
			}
		}
		else if(idle && !sneaking )
		{
			leg0->xRot = -HALF_PI;
			leg1->xRot = -HALF_PI;
			leg0->yRot = HALF_PI * 0.2f;
			leg1->yRot = -HALF_PI * 0.2f;
		}		
		else if(uiBitmaskOverrideAnim&(1<<eAnim_NoLegAnim))
		{
			leg0->xRot=0.0f;
			leg0->zRot=0.0f;
			leg1->xRot=0.0f;
			leg1->zRot=0.0f;
			leg0->yRot = 0.0f;
			leg1->yRot = 0.0f;
		}
		else if(uiBitmaskOverrideAnim&(1<<eAnim_SingleLegs))
		{
			leg0->xRot = ( Mth::cos(time * 0.6662f) * 1.4f) * r;
			leg1->xRot = ( Mth::cos(time * 0.6662f) * 1.4f) * r;
		}
		else
		{
			leg0->xRot = ( Mth::cos(time * 0.6662f) * 1.4f) * r;
			leg1->xRot = ( Mth::cos(time * 0.6662f + PI) * 1.4f) * r;
		}

		if (holdingLeftHand != 0) 
		{
			arm1->xRot = arm1->xRot * 0.5f - HALF_PI * 0.2f * holdingLeftHand;
		}
		if (holdingRightHand != 0) 
		{
			arm0->xRot = arm0->xRot * 0.5f - HALF_PI * 0.2f * holdingRightHand;
		}

		arm0->yRot = 0.0f;
		arm1->yRot = 0.0f;

		if (attackTime > -9990.0f)
		{
			float swing = attackTime;
			body->yRot = Mth::sin(sqrt(swing) * PI * 2.0f) * 0.2f;
			arm0->z = Mth::sin(body->yRot) * 5.0f;
			arm0->x = -Mth::cos(body->yRot) * 5.0f;
			arm1->z = -Mth::sin(body->yRot) * 5.0f;
			arm1->x = Mth::cos(body->yRot) * 5.0f;
			arm0->yRot += body->yRot;
			arm1->yRot += body->yRot;
			arm1->xRot += body->yRot;

			swing = 1.0f - attackTime;
			swing *= swing;
			swing *= swing;
			swing = 1.0f - swing;
			float aa = Mth::sin(swing * PI);
			float bb = Mth::sin(attackTime * PI) * -(head->xRot - 0.7f) * 0.75f;
			arm0->xRot -= aa * 1.2f + bb;	// 4J - changed 1.2 -> 1.2f
			arm0->yRot += body->yRot * 2.0f;

			if((uiBitmaskOverrideAnim&(1<<eAnim_StatueOfLiberty))&& (holdingRightHand==0) && (attackTime==0.0f))
			{
				arm0->zRot -= Mth::sin(attackTime * PI) * -0.4f;
			}
			else
			{
				arm0->zRot = Mth::sin(attackTime * PI) * -0.4f;
			}
		}

		// 4J added
		if( eating )
		{
			// These factors are largely lifted from ItemInHandRenderer to try and keep the 3rd person eating animation as similar as possible
			float is = 1 - eating_swing;
			is = is * is * is;
			is = is * is * is;
			is = is * is * is;
			float iss = 1 - is;
			arm0->xRot = - Mth::abs(Mth::cos(eating_t / 4.0f * PI) * 0.1f) * (eating_swing > 0.2 ? 1.0f : 0.0f) * 2.0f;		// This factor is the chomping bit (conditional factor is so that he doesn't eat whilst the food is being pulled away at the end)
			arm0->yRot -= iss * 0.5f;																			// This factor and the following to the general arm movement through the life of the swing
			arm0->xRot -= iss * 1.2f;
		}

		if (sneaking)
		{
			if(uiBitmaskOverrideAnim&(1<<eAnim_SmallModel))
			{
				body->xRot = -0.5f;
				leg0->xRot -= 0.0f;
				leg1->xRot -= 0.0f;
				arm0->xRot += 0.4f;
				arm1->xRot += 0.4f;
				leg0->z = -4.0f;
				leg1->z = -4.0f;
				body->z = 2.0f;
				body->y = 0.0f;
				arm0->y = 2.0f;
				arm1->y = 2.0f;
				leg0->y = +9.0f;
				leg1->y = +9.0f;
				head->y = +1.0f;
				hair->y = +1.0f;
				ear->y = +1.0f;
				cloak->y = 0.0f;
			}
			else
			{
				body->xRot = 0.5f;
				leg0->xRot -= 0.0f;
				leg1->xRot -= 0.0f;
				arm0->xRot += 0.4f;
				arm1->xRot += 0.4f;
				leg0->z = +4.0f;
				leg1->z = +4.0f;
				body->y = 0.0f;
				arm0->y = 2.0f;
				arm1->y = 2.0f;
				leg0->y = +9.0f;
				leg1->y = +9.0f;
				head->y = +1.0f;
				hair->y = +1.0f;
				ear->y = +1.0f;
				cloak->y = 0.0f;
			}
		}
		else
		{
			body->xRot = 0.0f;
			leg0->z = 0.1f;
			leg1->z = 0.1f;

			if(!riding && idle)
			{
				leg0->y = 22.0f;
				leg1->y = 22.0f;
				body->y = 10.0f;
				arm0->y = 12.0f;
				arm1->y = 12.0f;
				head->y = 10.0f;
				hair->y = 10.0f;
				ear->y = 11.0f;
				cloak->y = 10.0f;
			}
			else
			{
				leg0->y = 12.0f;
				leg1->y = 12.0f;
				body->y = 0.0f;
				arm0->y = 2.0f;
				arm1->y = 2.0f;
				head->y = 0.0f;
				hair->y = 0.0f;
				ear->y = 1.0f;
				cloak->y = 0.0f;
			}
		}

		arm0->zRot += ((Mth::cos(bob * 0.09f)) * 0.05f + 0.05f);
		arm1->zRot -= ((Mth::cos(bob * 0.09f)) * 0.05f + 0.05f);
		arm0->xRot += ((Mth::sin(bob * 0.067f)) * 0.05f);
		arm1->xRot -= ((Mth::sin(bob * 0.067f)) * 0.05f);

		if (bowAndArrow) 
		{
			float attack2 = 0.0f;
			float attack = 0.0f;

			arm0->zRot = 0.0f;
			arm1->zRot = 0.0f;
			arm0->yRot = -(0.1f - attack2 * 0.6f) + head->yRot;
			arm1->yRot = +(0.1f - attack2 * 0.6f) + head->yRot + 0.4f;
			arm0->xRot = -HALF_PI + head->xRot;
			arm1->xRot = -HALF_PI + head->xRot;
			arm0->xRot -= attack2 * 1.2f - attack * 0.4f;
			arm1->xRot -= attack2 * 1.2f - attack * 0.4f;
			arm0->zRot += ((float) (Mth::cos(bob * 0.09f)) * 0.05f + 0.05f);
			arm1->zRot -= ((float) (Mth::cos(bob * 0.09f)) * 0.05f + 0.05f);
			arm0->xRot += ((float) (Mth::sin(bob * 0.067f)) * 0.05f);
			arm1->xRot -= ((float) (Mth::sin(bob * 0.067f)) * 0.05f);
		}

		if (elytraFlying)
		{
			if (elytraCrouching)
			{
				arm0->xRot = PI;  arm0->yRot = 0.0f; arm0->zRot = 0.0f; arm0->y = 2.0f;
				if (sleeve0) { sleeve0->xRot = PI; sleeve0->yRot = 0.0f; sleeve0->zRot = 0.0f; sleeve0->y = 2.0f; }

				arm1->xRot = 0.0f; arm1->yRot = 0.0f; arm1->zRot = 0.0f; arm1->y = 2.0f;
				if (sleeve1) { sleeve1->xRot = 0.0f; sleeve1->yRot = 0.0f; sleeve1->zRot = 0.0f; sleeve1->y = 2.0f; }

				leg0->xRot = 0.0f; leg0->yRot = 0.0f; leg0->zRot = 0.0f;
				leg1->xRot = 0.0f; leg1->yRot = 0.0f; leg1->zRot = 0.0f;
				if (pants0) { pants0->xRot = 0.0f; pants0->yRot = 0.0f; pants0->zRot = 0.0f; }
				if (pants1) { pants1->xRot = 0.0f; pants1->yRot = 0.0f; pants1->zRot = 0.0f; }
			}
			else
			{
				float elytraTime = (float)(entity->tickCount) * 0.3f;
				float spd2 = (float)(entity->xd * entity->xd + entity->yd * entity->yd + entity->zd * entity->zd);
				float fDamp = spd2 / 0.2f;
				fDamp = fDamp * fDamp * fDamp;
				if (fDamp < 1.0f) fDamp = 1.0f;

				float armAmp = 2.0f * r * 0.5f / fDamp;
				float legAmp = 1.4f * r / fDamp;

				arm0->xRot = Mth::cos(elytraTime + PI) * armAmp;
				arm0->yRot = 0.0f; arm0->zRot = 0.0f; arm0->y = 2.0f;
				if (sleeve0) { sleeve0->xRot = arm0->xRot; sleeve0->yRot = 0.0f; sleeve0->zRot = 0.0f; sleeve0->y = 2.0f; }

				if (attackTime > -9990.0f)
				{
					float elytraSwing = 1.0f - attackTime;
					elytraSwing *= elytraSwing; elytraSwing *= elytraSwing;
					elytraSwing = 1.0f - elytraSwing;
					float elytraAA = Mth::sin(elytraSwing * PI);
					float elytraBB = Mth::sin(attackTime * PI) * -(head->xRot - 0.7f) * 0.75f;
					arm0->xRot -= elytraAA * 1.2f + elytraBB;
					arm0->zRot = Mth::sin(attackTime * PI) * -0.4f;
					if (sleeve0) { sleeve0->xRot -= elytraAA * 1.2f + elytraBB; sleeve0->zRot = Mth::sin(attackTime * PI) * -0.4f; }
				}

				arm1->xRot = Mth::cos(elytraTime) * armAmp;
				arm1->yRot = 0.0f; arm1->zRot = 0.0f; arm1->y = 2.0f;
				if (sleeve1) { sleeve1->xRot = arm1->xRot; sleeve1->yRot = 0.0f; sleeve1->zRot = 0.0f; sleeve1->y = 2.0f; }

				leg0->xRot = Mth::cos(elytraTime) * legAmp;
				leg0->yRot = 0.0f; leg0->zRot = 0.0f;
				leg1->xRot = Mth::cos(elytraTime + PI) * legAmp;
				leg1->yRot = 0.0f; leg1->zRot = 0.0f;
				if (pants0) { pants0->xRot = leg0->xRot; pants0->yRot = 0.0f; pants0->zRot = 0.0f; }
				if (pants1) { pants1->xRot = leg1->xRot; pants1->yRot = 0.0f; pants1->zRot = 0.0f; }
			}

			body->xRot = 0.0f;
			body->z = 0.0f;


			head->xRot = -(float)(PI / 4.0f);   
			hair->xRot = head->xRot;
		}

		if (jacket != 0)
		{
			jacket->x = body->x;
			jacket->y = body->y;
			jacket->z = body->z;
			jacket->xRot = body->xRot;
			jacket->yRot = body->yRot;
		}
		if (sleeve0 != 0)
		{
			sleeve0->x = arm0->x;
			sleeve0->y = arm0->y;
			sleeve0->z = arm0->z;
			sleeve0->xRot = arm0->xRot;
			sleeve0->yRot = arm0->yRot;
			sleeve0->zRot = arm0->zRot;
		}
		if (sleeve1 != 0)
		{
			sleeve1->x = arm1->x;
			sleeve1->y = arm1->y;
			sleeve1->z = arm1->z;
			sleeve1->xRot = arm1->xRot;
			sleeve1->yRot = arm1->yRot;
			sleeve1->zRot = arm1->zRot;
		}
		if (pants0 != 0)
		{
			pants0->x = leg0->x;
			pants0->y = leg0->y;
			pants0->z = leg0->z;
			pants0->xRot = leg0->xRot;
			pants0->yRot = leg0->yRot;
			pants0->zRot = leg0->zRot;
		}
		if (pants1 != 0)
		{
			pants1->x = leg1->x;
			pants1->y = leg1->y;
			pants1->z = leg1->z;
			pants1->xRot = leg1->xRot;
			pants1->yRot = leg1->yRot;
			pants1->zRot = leg1->zRot;
		}
		if (waist != 0)
		{
			waist->x = body->x;
			waist->y = body->y;
			waist->z = body->z;
			waist->xRot = body->xRot;
			waist->yRot = body->yRot;
		}
		if (belt != 0)
		{
			belt->x = body->x;
			belt->y = body->y;
			belt->z = body->z;
			belt->xRot = body->xRot;
			belt->yRot = body->yRot;
		}
		if (bodyArmor != 0)
		{
			bodyArmor->x = body->x;
			bodyArmor->y = body->y;
			bodyArmor->z = body->z;
			bodyArmor->xRot = body->xRot;
			bodyArmor->yRot = body->yRot;
		}
		if (armArmor0 != 0)
		{
			armArmor0->x = arm0->x;
			armArmor0->y = arm0->y;
			armArmor0->z = arm0->z;
			armArmor0->xRot = arm0->xRot;
			armArmor0->yRot = arm0->yRot;
			armArmor0->zRot = arm0->zRot;
		}
		if (armArmor1 != 0)
		{
			armArmor1->x = arm1->x;
			armArmor1->y = arm1->y;
			armArmor1->z = arm1->z;
			armArmor1->xRot = arm1->xRot;
			armArmor1->yRot = arm1->yRot;
			armArmor1->zRot = arm1->zRot;
		}
		if (legging0 != 0)
		{
			legging0->x = leg0->x;
			legging0->y = leg0->y;
			legging0->z = leg0->z;
			legging0->xRot = leg0->xRot;
			legging0->yRot = leg0->yRot;
			legging0->zRot = leg0->zRot;
		}
		if (legging1 != 0)
		{
			legging1->x = leg1->x;
			legging1->y = leg1->y;
			legging1->z = leg1->z;
			legging1->xRot = leg1->xRot;
			legging1->yRot = leg1->yRot;
			legging1->zRot = leg1->zRot;
		}
		if (sock0 != 0)
		{
			sock0->x = leg0->x;
			sock0->y = leg0->y;
			sock0->z = leg0->z;
			sock0->xRot = leg0->xRot;
			sock0->yRot = leg0->yRot;
			sock0->zRot = leg0->zRot;
		}
		if (sock1 != 0)
		{
			sock1->x = leg1->x;
			sock1->y = leg1->y;
			sock1->z = leg1->z;
			sock1->xRot = leg1->xRot;
			sock1->yRot = leg1->yRot;
			sock1->zRot = leg1->zRot;
		}
		if (boot0 != 0)
		{
			boot0->x = leg0->x;
			boot0->y = leg0->y;
			boot0->z = leg0->z;
			boot0->xRot = leg0->xRot;
			boot0->yRot = leg0->yRot;
			boot0->zRot = leg0->zRot;
		}
		if (boot1 != 0)
		{
			boot1->x = leg1->x;
			boot1->y = leg1->y;
			boot1->z = leg1->z;
			boot1->xRot = leg1->xRot;
			boot1->yRot = leg1->yRot;
			boot1->zRot = leg1->zRot;
		}
	}
}

void HumanoidModel::renderHair(float scale,bool usecompiled)
{
    hair->yRot = head->yRot;
    hair->xRot = head->xRot;
    hair->render(scale,usecompiled);
}

void HumanoidModel::renderEars(float scale,bool usecompiled)
{
    ear->yRot = head->yRot;
    ear->xRot = head->xRot;
    ear->x=0;
    ear->y=0;
    ear->render(scale,usecompiled);
}

void HumanoidModel::renderCloak(float scale,bool usecompiled)
{
	cloak->render(scale,usecompiled);
}
void HumanoidModel::renderElytra(float scale, bool usecompiled)
{
	elytraRight->render(scale, usecompiled);
	elytraLeft->render(scale, usecompiled);
}

void HumanoidModel::render(HumanoidModel *model, float scale, bool usecompiled)
{
    head->yRot = model->head->yRot;
    head->y = model->head->y;
    head->xRot = model->head->xRot;
    hair->y = head->y;
    hair->yRot = head->yRot;
    hair->xRot = head->xRot;

    body->yRot = model->body->yRot;

	if (jacket)
		jacket->yRot = model->jacket->yRot;
							
    arm0->xRot = model->arm0->xRot;
    arm0->yRot = model->arm0->yRot;
    arm0->zRot = model->arm0->zRot;

	if (sleeve0)
	{
		sleeve0->xRot = model->sleeve0->xRot;
		sleeve0->yRot = model->sleeve0->yRot;
		sleeve0->zRot = model->sleeve0->zRot;
	}
						  
    arm1->xRot = model->arm1->xRot;
    arm1->yRot = model->arm1->yRot;
    arm1->zRot = model->arm1->zRot;

	if (sleeve1)
	{
		sleeve1->xRot = model->sleeve1->xRot;
		sleeve1->yRot = model->sleeve1->yRot;
		sleeve1->zRot = model->sleeve1->zRot;
	}
						  
    leg0->xRot = model->leg0->xRot;
    leg1->xRot = model->leg1->xRot;

	if (pants0)
		pants0->xRot = model->pants0->xRot;

	if (pants1)
		pants1->xRot = model->pants1->xRot;

	head->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderHead))>0);
	body->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderTorso))>0);
	arm0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderArm0))>0);
	arm1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderArm1))>0);
	leg0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderLeg0))>0);
	leg1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderLeg1))>0);
	hair->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderHair))>0);

	if (jacket)
		jacket->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderJacket))>0);
	if (sleeve0)
		sleeve0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderSleeve0))>0);
	if (sleeve1)
		sleeve1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderSleeve1))>0);
	if (pants0)
		pants0->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderPants0))>0);
	if (pants1)
		pants1->render(scale, usecompiled,(m_uiAnimOverrideBitmask&(1<<eAnim_DisableRenderPants1))>0);
}

void HumanoidModel::setAllVisible(bool v) {
   
    head->visible  = v;
    hair->visible  = v;
    body->visible  = v;
    arm1->visible  = v;
    arm0->visible  = v;
    leg0->visible  = v;
    leg1->visible  = v;
}

void HumanoidModel::translateToHandItem(float scale) {
   
    arm1->translateTo(scale);
}

bool HumanoidModel::IsBodyPartDisabled(animbits bit) {
    return (m_uiAnimOverrideBitmask & (1u << bit)) != 0;
}

void HumanoidModel::copyPropertiesFrom(HumanoidModel* other) {
    if (!other) return;
   
    idle          = other->idle;
    sneaking      = other->sneaking;
    bowAndArrow   = other->bowAndArrow;
    eating        = other->eating;
    eating_t      = other->eating_t;
    eating_swing  = other->eating_swing;
    holdingLeftHand  = other->holdingLeftHand;
    holdingRightHand = other->holdingRightHand;
    m_uiAnimOverrideBitmask = other->m_uiAnimOverrideBitmask;
    m_fYOffset    = other->m_fYOffset;
}