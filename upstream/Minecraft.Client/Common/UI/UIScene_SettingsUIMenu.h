#pragma once

#include "UIScene.h"
#include "UIControl_MultiList.h"

class UIScene_SettingsUIMenu : public UIScene
{
protected:
	static int m_iControlTypeSettingA[6];
private:
	enum EControls
	{
		eControl_MultiList = 0,
		eControl_DisplayHUD = 1,
		eControl_DisplayHand = 2,
		eControl_ShowTooltips = 3,
		eControl_DisplayAnimatedCharacter = 4,
		eControl_InGameGamertags = 5,
		eControl_ShowSplitscreenGamertags = 6,
		eControl_ShowClassicCrafting = 7,
		eControl_HideSaveSizeBar = 8,
		eControl_InterfaceOpacity = 9,
		eControl_SensitivityInMenu = 10,
		eControl_UISize = 11,
		eControl_UISizeSplitscreen = 12,
		eControl_ControlType = 13,
	};

	UIControl_MultiList m_multiList;
	bool m_bNeedsMultiListPopulate;
	bool m_bInitialPopulateDone;
	bool m_bPendingSliderUpdate;
	int m_iPendingSliderId;
	int m_iPendingSliderValue;
	bool m_bControlTypeChanged;
	bool m_bNotInGame;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
	UI_END_MAP_ELEMENTS_AND_NAMES()

public:
	UIScene_SettingsUIMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_SettingsUIMenu();

	virtual EUIScene getSceneType() { return eUIScene_SettingsUIMenu;}

	virtual void tick();
	virtual void updateTooltips();
	virtual void updateComponents();
	virtual void handleGainFocus(bool navBack);

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleSliderMove(F64 sliderId, F64 currentValue);
	virtual void handleCheckboxToggled(F64 controlId, bool selected);
	virtual void handlePress(F64 controlId, F64 childId);

	void setGameSettings();
};
