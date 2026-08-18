#pragma once
#include "Model.h"
#include "SkinOffset.h"
class HumanoidModel : public Model
{
public:
    ModelPart* head, * hair, * body, * jacket, * arm0, * sleeve0, * arm1, * sleeve1, * leg0, * pants0, * leg1, * pants1, * ear, * cloak;
	ModelPart* waist, * belt, * bodyArmor, * armArmor0, * armArmor1, * legging0, * legging1, * sock0, * sock1, * boot0, * boot1;
    ModelPart* elytraLeft, * elytraRight;
    int holdingLeftHand;
    int holdingRightHand;
    bool idle;
    bool sneaking;
    bool bowAndArrow;
    bool eating;		
    float eating_t;			
    float eating_swing;	
    bool elytraFlying;    
    bool elytraCrouching; 
    bool m_isArmor;
	bool m_is64x64;
    unsigned int m_uiAnimOverrideBitmask; 
    float m_fYOffset;
    enum animbits
    {
        eAnim_ArmsDown = 0,
        eAnim_ArmsOutFront,
        eAnim_NoLegAnim,
        eAnim_HasIdle,
        eAnim_ForceAnim, 
        eAnim_SingleLegs,
        eAnim_SingleArms,
        eAnim_StatueOfLiberty, 
        eAnim_DontRenderArmour, 
        eAnim_NoBobbing, 
        eAnim_DisableRenderHead,
        eAnim_DisableRenderArm0,
        eAnim_DisableRenderArm1,
        eAnim_DisableRenderTorso,
        eAnim_DisableRenderLeg0,
        eAnim_DisableRenderLeg1,
        eAnim_DisableRenderHair,
        eAnim_SmallModel,
		eAnim_WideModel,
		eAnim_SlimModel,
		eAnim_DisableRenderSleeve1,
		eAnim_DisableRenderSleeve0,
		eAnim_DisableRenderPants1,
		eAnim_DisableRenderPants0,
		eAnim_DisableRenderJacket,
		eAnim_RenderArmorHead,
		eAnim_RenderArmorArm0,
		eAnim_RenderArmorArm1,
		eAnim_RenderArmorTorso,
		eAnim_RenderArmorLeg0,
		eAnim_RenderArmorLeg1,
		eAnim_Dinnerbone
    };


    static const unsigned int m_staticBitmaskIgnorePlayerCustomAnimSetting =
        (1 << HumanoidModel::eAnim_ForceAnim) |
        (1 << HumanoidModel::eAnim_DisableRenderArm0) |
        (1 << HumanoidModel::eAnim_DisableRenderArm1) |
        (1 << HumanoidModel::eAnim_DisableRenderTorso) |
        (1 << HumanoidModel::eAnim_DisableRenderLeg0) |
        (1 << HumanoidModel::eAnim_DisableRenderLeg1) |
        (1 << HumanoidModel::eAnim_DisableRenderHair) |
        (1 << HumanoidModel::eAnim_DisableRenderJacket) |
        (1 << HumanoidModel::eAnim_DisableRenderSleeve0) |
        (1 << HumanoidModel::eAnim_DisableRenderSleeve1) |
        (1 << HumanoidModel::eAnim_DisableRenderPants0) |
        (1 << HumanoidModel::eAnim_DisableRenderPants1);

    void _init(float g, float yOffset, int texWidth, int texHeight, bool slim, bool isArmor);

    HumanoidModel();
    HumanoidModel(float g);
    HumanoidModel(float g, bool isArmor);
    HumanoidModel(float g, float yOffset, int texWidth, int texHeight);
    HumanoidModel(float g, float yOffset, int texWidth, int texHeight, bool slim);

    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	virtual void renderUI(float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled, vector<SKIN_OFFSET *> *skinOffsets);
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim = 0);
    void renderHair(float scale, bool usecompiled);
    void renderEars(float scale, bool usecompiled);
    void renderCloak(float scale, bool usecompiled);
    void renderElytra(float scale, bool usecompiled);

    void render(HumanoidModel* model, float scale, bool usecompiled);

    ModelPart* AddOrRetrievePart(SKIN_BOX* pBox);

 
    void setAllVisible(bool visible);

 
    void translateToHandItem(float scale);


    void copyPropertiesFrom(HumanoidModel* other);


    bool IsBodyPartDisabled(animbits bit);
};