#pragma once

#include "UIScene.h"
#include "UIControl_MultiList.h"

class UIScene_SettingsOptionsMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_MultiList = 0,
		eControl_VerticalSplitscreen = 1,
		eControl_ViewBob = 2,
		eControl_Hints = 3,
		eControl_DeathMessages = 4,
		eControl_PassiveChunkLoading = 5,
		eControl_Languages = 6,
		eControl_Autosave = 7,
		eControl_Sensitivity_InGame = 8,
		eControl_Difficulty = 9,
	};

	UIControl_MultiList m_multiList;
	bool m_bNeedsMultiListPopulate;
	bool m_bNavigateToLanguageSelector;
	bool m_bInitialPopulateDone;
	bool m_bPendingSliderUpdate;
	int m_iPendingSliderId;
	int m_iPendingSliderValue;

	static int m_iDifficultyTitleSettingA[4];

	bool m_bNotInGame;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
	UI_END_MAP_ELEMENTS_AND_NAMES()

	//bool m_bNotInGame;
	// bool m_bMashUpWorldsUnhideOption;
	// bool m_bNavigateToLanguageSelector;

public:
	UIScene_SettingsOptionsMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_SettingsOptionsMenu();

	virtual EUIScene getSceneType() { return eUIScene_SettingsOptionsMenu;}
	
	virtual void tick();

	virtual void updateTooltips();
	virtual void updateComponents();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
	virtual void handlePress(F64 controlId, F64 childId);
	virtual void handleSliderMove(F64 sliderId, F64 currentValue);
	virtual void handleCheckboxToggled(F64 controlId, bool selected);
	virtual void handleGainFocus(bool navBack);

	void setGameSettings();
};
