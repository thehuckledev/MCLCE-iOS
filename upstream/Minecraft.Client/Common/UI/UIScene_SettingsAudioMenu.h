#pragma once

#include "UIScene.h"
#include "UIControl_MultiList.h"

class UIScene_SettingsAudioMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_MultiList = 0,
		eControl_Music = 1,
		eControl_Sound = 2,
		eControl_CaveSounds = 3,
		eControl_MinecartSounds = 4,
		eControl_GameChat = 5
	};

	UIControl_MultiList m_multiList;
	bool m_bNeedsMultiListPopulate;
	bool m_bInitialPopulateDone;
	bool m_bPendingSliderUpdate;
	int m_iPendingSliderId;
	int m_iPendingSliderValue;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
	UI_END_MAP_ELEMENTS_AND_NAMES()

public:
	UIScene_SettingsAudioMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_SettingsAudioMenu();

	virtual EUIScene getSceneType() { return eUIScene_SettingsAudioMenu;}
	
	virtual void tick();
	virtual void updateTooltips();
	virtual void updateComponents();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleSliderMove(F64 sliderId, F64 currentValue);
	virtual void handleCheckboxToggled(F64 controlId, bool selected);
	virtual void handlePress(F64 controlId, F64 childId);
	virtual void handleGainFocus(bool navBack);

	void setGameSettings();
};
