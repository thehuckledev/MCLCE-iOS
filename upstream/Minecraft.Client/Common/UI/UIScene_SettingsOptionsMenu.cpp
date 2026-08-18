#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsOptionsMenu.h"

namespace
{
	void FormatAutosaveSliderLabel(WCHAR *buffer, size_t bufferSize, int value, int controlType)
	{
		const bool fasterAutosave = (controlType == 0 || controlType == 1 || controlType == 4); // windows, xbox one and ps4

		if(value == 0)
		{
			swprintf(buffer, bufferSize, L"%ls", app.GetString(IDS_SLIDER_AUTOSAVE_OFF));
		}
		else
		{
			const int displayValue = fasterAutosave ? value : value * 15;
			const wstring unitText = (displayValue == 1) ? L"Min" : app.GetString(IDS_MINUTES);
			swprintf(buffer, bufferSize, L"%ls: %d %ls", app.GetString(IDS_SLIDER_AUTOSAVE), displayValue, unitText.c_str());
		}
	}
}

int UIScene_SettingsOptionsMenu::m_iDifficultyTitleSettingA[4]=
{
	IDS_DIFFICULTY_TITLE_PEACEFUL,
	IDS_DIFFICULTY_TITLE_EASY,
	IDS_DIFFICULTY_TITLE_NORMAL,
	IDS_DIFFICULTY_TITLE_HARD
};

UIScene_SettingsOptionsMenu::UIScene_SettingsOptionsMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// m_bNavigateToLanguageSelector = false;

	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_bNotInGame = (Minecraft::GetInstance()->level==nullptr);
	m_bNeedsMultiListPopulate = true;
	m_bNavigateToLanguageSelector = false;
	m_bInitialPopulateDone = false;
	m_bPendingSliderUpdate = false;
	m_iPendingSliderId = 0;
	m_iPendingSliderValue = 0;

	doHorizontalResizeCheck();

	if(app.GetLocalPlayerCount()>1)
	{
#if TO_BE_IMPLEMENTED
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
#endif
	}
}

UIScene_SettingsOptionsMenu::~UIScene_SettingsOptionsMenu()
{
}

wstring UIScene_SettingsOptionsMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"MultilistMenuSplit";
	}
	else
	{
		return L"MultilistMenu";
	}
}

void UIScene_SettingsOptionsMenu::tick()
{
	if(m_bNeedsMultiListPopulate)
	{
		m_bNeedsMultiListPopulate = false;
		m_multiList.setupControl(this, m_rootPath, "MultiList");
		m_controls.push_back(&m_multiList);
		m_multiList.clearList();
		m_multiList.init(eControl_MultiList);

		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_VERTICAL_SPLIT_SCREEN), eControl_VerticalSplitscreen, (app.GetGameSettings(m_iPad,eGameSetting_SplitScreenVertical)!=0));
		m_multiList.AddNewCheckbox(app.GetString(IDS_VIEW_BOBBING), eControl_ViewBob, (app.GetGameSettings(m_iPad,eGameSetting_ViewBob)!=0));
		m_multiList.AddNewCheckbox(app.GetString(IDS_HINTS), eControl_Hints, (app.GetGameSettings(m_iPad,eGameSetting_Hints)!=0));
		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_DEATH_MESSAGES), eControl_DeathMessages, (app.GetGameSettings(m_iPad,eGameSetting_DeathMessages)!=0));
		m_multiList.AddNewCheckbox(L"Passive Chunk Loading", eControl_PassiveChunkLoading, (app.GetGameSettings(m_iPad,eGameSetting_PassiveChunkLoading)!=0));

		if(m_bNotInGame)
		{
			m_multiList.AddNewButton(app.GetString(IDS_LANGUAGE_SELECTOR), eControl_Languages);
		}

		WCHAR TempString[256];

		int autosaveVal = app.GetGameSettings(m_iPad,eGameSetting_Autosave);
		FormatAutosaveSliderLabel(TempString, 256, autosaveVal, app.GetGameSettings(m_iPad, eGameSetting_ControlType));
		m_multiList.AddNewSlider(TempString, eControl_Autosave, 0, 8, 1, autosaveVal);

		swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_SENSITIVITY_INGAME), app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame));
		m_multiList.AddNewSlider(TempString, eControl_Sensitivity_InGame, 0, 200, 1, app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame));

		int diffVal = app.GetGameSettings(m_iPad,eGameSetting_Difficulty);
		swprintf(TempString, 256, L"%ls: %ls", app.GetString(IDS_SLIDER_DIFFICULTY), app.GetString(m_iDifficultyTitleSettingA[diffVal]));
		m_multiList.AddNewSlider(TempString, eControl_Difficulty, 0, 3, 1, diffVal);

		IggyName funcDoVert = registerFastName(L"DoVerticalResizeCheck");
		IggyName funcHideDesc = registerFastName(L"HideDescription");
		IggyDataValue result;
		IggyPlayerCallMethodRS(getMovie(), &result, m_rootPath, funcDoVert, 0, nullptr);
		doHorizontalResizeCheck();
		IggyPlayerCallMethodRS(getMovie(), &result, m_rootPath, funcHideDesc, 0, nullptr);
		m_multiList.HighlightItem(eControl_ViewBob);
		m_multiList.HighlightItem(eControl_VerticalSplitscreen);
	}

	if(m_bNavigateToLanguageSelector)
	{
		m_bNavigateToLanguageSelector = false;
		setGameSettings();
		ui.NavigateToScene(m_iPad, eUIScene_LanguageSelector);
	}

	if(m_bPendingSliderUpdate)
	{
		m_bPendingSliderUpdate = false;
		m_multiList.SetSliderValue(m_iPendingSliderId, m_iPendingSliderValue);

		WCHAR TempString[256];
		switch(m_iPendingSliderId)
		{
		case eControl_Autosave:
			app.SetGameSettings(m_iPad, eGameSetting_Autosave, m_iPendingSliderValue);
			app.SetAutosaveTimerTime();
			FormatAutosaveSliderLabel(TempString, 256, m_iPendingSliderValue, app.GetGameSettings(m_iPad, eGameSetting_ControlType));
			m_multiList.SetSliderLabel(eControl_Autosave, TempString);
			break;
		case eControl_Sensitivity_InGame:
			app.SetGameSettings(m_iPad, eGameSetting_Sensitivity_InGame, m_iPendingSliderValue);
			swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_SENSITIVITY_INGAME), m_iPendingSliderValue);
			m_multiList.SetSliderLabel(eControl_Sensitivity_InGame, TempString);
			break;
		case eControl_Difficulty:
			app.SetGameSettings(m_iPad, eGameSetting_Difficulty, m_iPendingSliderValue);
			swprintf(TempString, 256, L"%ls: %ls", app.GetString(IDS_SLIDER_DIFFICULTY), app.GetString(m_iDifficultyTitleSettingA[m_iPendingSliderValue]));
			m_multiList.SetSliderLabel(eControl_Difficulty, TempString);
			break;
		}
	}

	UIScene::tick();
	m_bInitialPopulateDone = true;
}

void UIScene_SettingsOptionsMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsOptionsMenu::updateComponents()
{
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);
	if(bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
		m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);

		if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
		else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
	}
}

void UIScene_SettingsOptionsMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);
	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			setGameSettings();
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		sendInputToMovie(key, repeat, pressed, released);
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_SettingsOptionsMenu::handlePress(F64 controlId, F64 childId)
{
	//CD - Added for audio
	ui.PlayUISFX(eSFX_Press);

	switch(static_cast<int>(childId))
	{
	case eControl_Languages:
		m_bNavigateToLanguageSelector = true;
		break;
	}
}

void UIScene_SettingsOptionsMenu::handleCheckboxToggled(F64 controlId, bool selected)
{
	if(m_bInitialPopulateDone)
		ui.PlayUISFX(eSFX_Press);
}

void UIScene_SettingsOptionsMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	int sliderIdInt = static_cast<int>(sliderId);
	int value = static_cast<int>(currentValue);

	m_multiList.handleSliderMove(sliderIdInt, value);

	switch(sliderIdInt)
	{
	case eControl_Autosave:
	case eControl_Sensitivity_InGame:
	case eControl_Difficulty:
		m_bPendingSliderUpdate = true;
		m_iPendingSliderId = sliderIdInt;
		m_iPendingSliderValue = value;
		break;
	}
}

void UIScene_SettingsOptionsMenu::setGameSettings()
{
	bool bSplitChanged = (app.GetGameSettings(m_iPad,eGameSetting_SplitScreenVertical)!=(m_multiList.GetCheckboxValue(eControl_VerticalSplitscreen)?1:0));

	app.SetGameSettings(m_iPad,eGameSetting_SplitScreenVertical,m_multiList.GetCheckboxValue(eControl_VerticalSplitscreen)?1:0);
	app.SetGameSettings(m_iPad,eGameSetting_ViewBob,m_multiList.GetCheckboxValue(eControl_ViewBob)?1:0);
	app.SetGameSettings(m_iPad,eGameSetting_Hints,m_multiList.GetCheckboxValue(eControl_Hints)?1:0);
	app.SetGameSettings(m_iPad,eGameSetting_DeathMessages,m_multiList.GetCheckboxValue(eControl_DeathMessages)?1:0);
	app.SetGameSettings(m_iPad,eGameSetting_PassiveChunkLoading,m_multiList.GetCheckboxValue(eControl_PassiveChunkLoading)?1:0);
	app.SetGameSettings(m_iPad,eGameSetting_Autosave,m_multiList.GetSliderValue(eControl_Autosave));
	app.SetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame,m_multiList.GetSliderValue(eControl_Sensitivity_InGame));
	app.SetGameSettings(m_iPad,eGameSetting_Difficulty,m_multiList.GetSliderValue(eControl_Difficulty));

	if(bSplitChanged && app.GetLocalPlayerCount()==2)
	{
		ui.CloseAllPlayersScenes();
	}
}

void UIScene_SettingsOptionsMenu::handleGainFocus(bool navBack)
{
	if(navBack)
	{
		m_bNeedsMultiListPopulate = true;
		m_bInitialPopulateDone = false;
	}
}
	// handled = true;
	// if the splitscreen vertical/horizontal has changed, need to update the scenes

	// 4J-PB - don't action changes here or we might write to the profile on backing out here and then get a change in the settings all, and write again on backing out there
	//app.CheckGameSettingsChanged(true,pInputData->UserIndex);