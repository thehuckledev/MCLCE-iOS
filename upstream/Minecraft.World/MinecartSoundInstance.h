#pragma once

#include <memory>

class Minecart;
class SoundEngine;
struct MiniAudioSound;

// minecart rolling sound
class MinecartSoundInstance
{
protected:
	shared_ptr<Minecart> m_minecart;
	bool m_bIsCurrentlyPlaying;
	MiniAudioSound* m_sound;
	float m_volume;
	float m_pitch;

public:
	MinecartSoundInstance(shared_ptr<Minecart> minecart);
	virtual ~MinecartSoundInstance();

	virtual void tick();
	bool isCurrentlyPlaying() const { return m_bIsCurrentlyPlaying; }
};

// minecart passenger sound
class RidingMinecartSoundInstance
{
protected:
	shared_ptr<Minecart> m_minecart;
	bool m_bIsCurrentlyPlaying;
	MiniAudioSound* m_sound;
	float m_volume;
	float m_pitch;

public:
	RidingMinecartSoundInstance(shared_ptr<Minecart> minecart);
	virtual ~RidingMinecartSoundInstance();

	virtual void tick();
	bool isCurrentlyPlaying() const { return m_bIsCurrentlyPlaying; }
};
