#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.damagesource.h"
#include "SharedMonsterAttributes.h"
#include "PolarBear.h"
#include "../Minecraft.Client/Textures.h"
#include "SoundTypes.h"

PolarBear::PolarBear(Level *level) : Animal(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	m_standing = false;
	clientSideStandAnimation = clientSideStandAnimationO = 0.0f;
	warningSoundTicks = 0;
	cubDefenseCooldown = 0;

	setSize(1.3f, 1.4f);

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new MeleeAttackGoal(this, 1.25, true));
	goalSelector.addGoal(1, new PanicGoal(this, 2.0));
	goalSelector.addGoal(4, new FollowParentGoal(this, 1.25));
	goalSelector.addGoal(5, new RandomStrollGoal(this, 1.0));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(7, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
}

bool PolarBear::useNewAi()
{
	return true;
}

void PolarBear::registerAttributes()
{
	Animal::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(20);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25f);

	getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6);
}

shared_ptr<AgableMob> PolarBear::getBreedOffspring(shared_ptr<AgableMob> target)
{
	return std::make_shared<PolarBear>(level);
}

bool PolarBear::isFood(shared_ptr<ItemInstance> itemInstance)
{
	return false;
}

int PolarBear::getAmbientSound()
{
	return isBaby() ? eSoundType_MOB_POLARBEAR_BABY_AMBIENT : eSoundType_MOB_POLARBEAR_AMBIENT;
}

int PolarBear::getHurtSound()
{
	return eSoundType_MOB_POLARBEAR_HURT;
}

int PolarBear::getDeathSound()
{
	return eSoundType_MOB_POLARBEAR_DEATH;
}

void PolarBear::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_POLARBEAR_STEP, 0.15f, 1.0f);
}

void PolarBear::playWarningSound()
{
	if (warningSoundTicks <= 0)
	{
		playSound(eSoundType_MOB_POLARBEAR_WARNING, 1.0f, getVoicePitch());
		warningSoundTicks = 40;
	}
}

void PolarBear::setTarget(shared_ptr<LivingEntity> newTarget)
{
	shared_ptr<LivingEntity> old = getTarget();
	Animal::setTarget(newTarget);
	if (newTarget != nullptr && old == nullptr && !isBaby())
	{
		playWarningSound();
	}
}

int PolarBear::getDeathLoot()
{
	return Item::fish_Id;
}

bool PolarBear::doHurtTarget(shared_ptr<Entity> target)
{
	if (isBaby())
	{
		return false;
	}
	float dmg = (float)getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue();
	return target->hurt(DamageSource::mobAttack(dynamic_pointer_cast<LivingEntity>(shared_from_this())), dmg);
}

bool PolarBear::isStanding()
{
	return m_standing;
}

void PolarBear::setStanding(bool standing)
{
	m_standing = standing;
}

float PolarBear::getStandingAnimationScale(float a)
{
	return (clientSideStandAnimationO + (clientSideStandAnimation - clientSideStandAnimationO) * a) / 6.0f;
}

void PolarBear::aiStep()
{
	Animal::aiStep();

	if (level->isClientSide)
	{
		clientSideStandAnimationO = clientSideStandAnimation;
		if (isStanding())
		{
			clientSideStandAnimation = Mth::clamp(clientSideStandAnimation + 1.0f, 0.0f, 6.0f);
		}
		else
		{
			clientSideStandAnimation = Mth::clamp(clientSideStandAnimation - 1.0f, 0.0f, 6.0f);
		}
	}

	if (warningSoundTicks > 0)
	{
		--warningSoundTicks;
	}

	if (!level->isClientSide && !isBaby() && getTarget() == nullptr)
	{
		if (cubDefenseCooldown > 0)
		{
			--cubDefenseCooldown;
		}
		else
		{
			cubDefenseCooldown = 10;
			bool cubNear = false;
			vector<shared_ptr<Entity> > *bears = level->getEntitiesOfClass(typeid(PolarBear), bb->grow(8.0, 4.0, 8.0));
			for (auto &it : *bears)
			{
				shared_ptr<PolarBear> other = dynamic_pointer_cast<PolarBear>(it);
				if (other != nullptr && other.get() != this && other->isBaby())
				{
					cubNear = true;
					break;
				}
			}
			delete bears;
			if (cubNear)
			{
				shared_ptr<Entity> p = level->getNearestAttackablePlayer(shared_from_this(), 16.0);
				if (p != nullptr)
				{
					setTarget(dynamic_pointer_cast<LivingEntity>(p));
				}
			}
		}
	}
}

MobGroupData *PolarBear::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Animal::finalizeMobSpawn(groupData);

	if (groupData == nullptr || dynamic_cast<PolarBearGroupData *>(groupData) == nullptr)
	{
		groupData = new PolarBearGroupData();
	}
	else
	{
		setAge(CUB_AGE);
	}

	return groupData;
}
