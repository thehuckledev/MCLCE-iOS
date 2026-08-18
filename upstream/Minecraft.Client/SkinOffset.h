#pragma once

enum eBodyOffset
{
	eBodyOffset_Unknown=0,
	eBodyOffset_Head,
	eBodyOffset_Body,
	eBodyOffset_Arm0,
	eBodyOffset_Arm1,
	eBodyOffset_Leg0,
	eBodyOffset_Leg1,
	eBodyOffset_Headwear,
	eBodyOffset_Jacket,
	eBodyOffset_Sleeve0,
	eBodyOffset_Sleeve1,
	eBodyOffset_Pants0,
	eBodyOffset_Pants1,
	eBodyOffset_Helmet,
	eBodyOffset_Waist,
	eBodyOffset_Legging0,
	eBodyOffset_Legging1,
	eBodyOffset_Sock0,
	eBodyOffset_Sock1,
	eBodyOffset_Boot0,
	eBodyOffset_Boot1,
	eBodyOffset_ArmArmor1,
	eBodyOffset_ArmArmor0,
	eBodyOffset_BodyArmor,
	eBodyOffset_Belt,
	eBodyOffset_Tool0,
	eBodyOffset_Tool1
	
};

enum eOffsetDirection
{
	eOffsetDirection_Unknown=0,
	eOffsetDirection_X,
	eOffsetDirection_Y,
	eOffsetDirection_Z
};

typedef struct 
{
	eBodyOffset ePart;
	float fD, fO;
}
SKIN_OFFSET;
