#pragma once

#include "Animal.h"
#include "MobGroupData.h"

class DamageSource;

class PolarBear : public Animal
{
public:
	eINSTANCEOF GetType() { return eTYPE_POLARBEAR; }
	static Entity *create(Level *level) { return new PolarBear(level); }

private:
	static const int CUB_AGE = -23488;

	float clientSideStandAnimationO;
	float clientSideStandAnimation;
	int warningSoundTicks;
	int cubDefenseCooldown;

public:
	PolarBear(Level *level);

	virtual bool useNewAi();

protected:
	virtual void registerAttributes();

public:
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
	virtual bool isFood(shared_ptr<ItemInstance> itemInstance);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual int getDeathLoot();
	virtual void playStepSound(int xt, int yt, int zt, int t);

public:
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual bool isStanding();
	void setStanding(bool standing);
	float getStandingAnimationScale(float a);

	void playWarningSound();
	virtual void setTarget(shared_ptr<LivingEntity> target);

	virtual void aiStep();
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);

private:
	bool m_standing;
};

class PolarBearGroupData : public MobGroupData
{
};
