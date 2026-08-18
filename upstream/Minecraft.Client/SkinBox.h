#pragma once

enum eBodyPart
{
	eBodyPart_Unknown=0,
	eBodyPart_Head,
	eBodyPart_Body,
	eBodyPart_Arm0,
	eBodyPart_Arm1,
	eBodyPart_Leg0,
	eBodyPart_Leg1,
	eBodyPart_Headwear,
	eBodyPart_Jacket,
	eBodyPart_Sleeve0,
	eBodyPart_Sleeve1,
	eBodyPart_Pants0,
	eBodyPart_Pants1,
	eBodyPart_Waist,
	eBodyPart_Legging0,
	eBodyPart_Legging1,
	eBodyPart_Sock0,
	eBodyPart_Sock1,
	eBodyPart_Boot0,
	eBodyPart_Boot1,
	eBodyPart_ArmArmor0,
	eBodyPart_ArmArmor1,
	eBodyPart_BodyArmor,
	eBodyPart_Belt
};

typedef struct 
{
	eBodyPart ePart;
	float fX,fY,fZ,fW,fH,fD,fU,fV,fA,fM,fS;
}
SKIN_BOX;
