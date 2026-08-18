#pragma once

#include "UIScene.h"
#include "UIControl_MultiList.h"

class UIScene_SettingsGraphicsMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_MultiList = 0,
		eControl_Clouds = 1,
		eControl_BedrockFog = 2,
		eControl_CustomSkinAnim = 3,
		eControl_VSync = 4,
		eControl_ExclusiveFullscreen = 5,
		eControl_RenderDistance = 6,
		eControl_Gamma = 7,
		eControl_FOV = 8,
	};

	UIControl_MultiList m_multiList;
	bool m_bNeedsMultiListPopulate;
	bool m_bInitialPopulateDone;
	bool m_bPendingSliderUpdate;
	int m_iPendingSliderId;
	int m_iPendingSliderValue;

	bool m_bNotInGame;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
	UI_END_MAP_ELEMENTS_AND_NAMES()

public:
	UIScene_SettingsGraphicsMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_SettingsGraphicsMenu();

	virtual EUIScene getSceneType() { return eUIScene_SettingsGraphicsMenu;}

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

	static int LevelToDistance(int dist);

	static int DistanceToLevel(int dist);
};
