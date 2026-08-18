#include "stdafx.h"
#include "MinecartSoundInstance.h"
#include "Minecart.h"
#include "net.minecraft.world.level.h"
#include "../Minecraft.Client/Minecraft.h"
#include "../Minecraft.Client/Common/Audio/SoundEngine.h"

// MinecartSoundInstance

MinecartSoundInstance::MinecartSoundInstance(shared_ptr<Minecart> minecart)
	: m_minecart(minecart), m_bIsCurrentlyPlaying(false), m_sound(nullptr), m_volume(0.0f), m_pitch(1.0f)
{
}

MinecartSoundInstance::~MinecartSoundInstance()
{
	if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
	{
		Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
	}
	m_sound = nullptr;
}

void MinecartSoundInstance::tick()
{
	if (!m_minecart || m_minecart->removed)
	{
		if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
			m_sound = nullptr;
		}
		m_bIsCurrentlyPlaying = false;
		return;
	}

	// minecart sound functionality check
	if (!app.GetGameSettings(0, eGameSetting_MinecartSounds))
	{
		if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
			m_sound = nullptr;
		}
		m_bIsCurrentlyPlaying = false;
		return;
	}

	// volume + pitch calculations
    // relative to minecart velocity
	double xd = m_minecart->xd;
	double zd = m_minecart->zd;
	double velocity = sqrt(xd * xd + zd * zd);

	if (velocity >= 0.01)
	{
		float clampedVel = (float)(velocity > 1.0 ? 1.0 : (velocity < 0.0 ? 0.0 : velocity));
		m_volume = clampedVel * 0.75f;
		m_pitch = 1.0f;

		if (!m_bIsCurrentlyPlaying)
		{
			m_bIsCurrentlyPlaying = true;
			if (Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
			{
				m_sound = Minecraft::GetInstance()->soundEngine->startLoopingSound(L"mob.minecart.rolling", (float)m_minecart->x, (float)m_minecart->y, (float)m_minecart->z, m_volume, m_pitch, true);
			}
		}
		else if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->updateLoopingSound(m_sound, (float)m_minecart->x, (float)m_minecart->y, (float)m_minecart->z, m_volume, m_pitch);
		}
	}
	else
	{
		if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
			m_sound = nullptr;
		}
		m_volume = 0.0f;
		m_pitch = 0.0f;
		m_bIsCurrentlyPlaying = false;
	}
}

// RidingMinecartSoundInstance

RidingMinecartSoundInstance::RidingMinecartSoundInstance(shared_ptr<Minecart> minecart)
	: m_minecart(minecart), m_bIsCurrentlyPlaying(false), m_sound(nullptr), m_volume(0.0f), m_pitch(0.0f)
{
}

RidingMinecartSoundInstance::~RidingMinecartSoundInstance()
{
	if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
	{
		Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
	}
	m_sound = nullptr;
}

void RidingMinecartSoundInstance::tick()
{
	if (!m_minecart || m_minecart->removed)
	{
		if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
			m_sound = nullptr;
		}
		m_bIsCurrentlyPlaying = false;
		return;
	}

	// minecart sound functionality check
	if (!app.GetGameSettings(0, eGameSetting_MinecartSounds))
	{
		if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
			m_sound = nullptr;
		}
		m_bIsCurrentlyPlaying = false;
		return;
	}

	// minecart passenger check
	if (m_minecart->rider.lock() == nullptr)
	{
		if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
			m_sound = nullptr;
		}
		m_bIsCurrentlyPlaying = false;
		return;
	}

	// volume + pitch calculations
    // relative to minecart velocity
	double xd = m_minecart->xd;
	double zd = m_minecart->zd;
	double velocity = sqrt(xd * xd + zd * zd);

	if (velocity >= 0.01)
	{
		float clampedVel = (float)(velocity > 1.0 ? 1.0 : (velocity < 0.0 ? 0.0 : velocity));
		m_volume = clampedVel * 0.75f;
		m_pitch = 1.0f;

		if (!m_bIsCurrentlyPlaying)
		{
			m_bIsCurrentlyPlaying = true;
			if (Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
			{
				m_sound = Minecraft::GetInstance()->soundEngine->startLoopingSound(L"mob.minecart.inside", (float)m_minecart->x, (float)m_minecart->y, (float)m_minecart->z, m_volume, m_pitch, false);
			}
		}
		else if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->updateLoopingSound(m_sound, (float)m_minecart->x, (float)m_minecart->y, (float)m_minecart->z, m_volume, m_pitch);
		}
	}
	else
	{
		if (m_sound && Minecraft::GetInstance() && Minecraft::GetInstance()->soundEngine)
		{
			Minecraft::GetInstance()->soundEngine->stopLoopingSound(m_sound);
			m_sound = nullptr;
		}
		m_volume = 0.0f;
		m_bIsCurrentlyPlaying = false;
	}
}
