#include "stdafx.h"
#include "../App_enums.h"
#include "UI.h"
#include "UIScene_SettingsUIMenu.h"

int UIScene_SettingsUIMenu::m_iControlTypeSettingA[6]=
{
	IDS_CONTROLTYPE_KBM,
	IDS_CONTROLTYPE_XBOXONE,
	IDS_CONTROLTYPE_XBOX360,
	// IDS_CONTROLTYPE_VITA,
	IDS_CONTROLTYPE_PLAYSTATION3,
	IDS_CONTROLTYPE_PLAYSTATION4,
	IDS_CONTROLTYPE_WIIU,
	// IDS_CONTROLTYPE_SWITCH,
};

UIScene_SettingsUIMenu::UIScene_SettingsUIMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	m_bControlTypeChanged = false;
	m_bNotInGame = (Minecraft::GetInstance()->level == nullptr);
	m_bNeedsMultiListPopulate = true;
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

UIScene_SettingsUIMenu::~UIScene_SettingsUIMenu()
{
}

wstring UIScene_SettingsUIMenu::getMoviePath()
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

void UIScene_SettingsUIMenu::tick()
{
	if(m_bNeedsMultiListPopulate)
	{
		m_bNeedsMultiListPopulate = false;
		m_multiList.setupControl(this, m_rootPath, "MultiList");
		m_controls.push_back(&m_multiList);
		m_multiList.clearList();
		m_multiList.init(eControl_MultiList);

		bool bPrimaryPlayer = (ProfileManager.GetPrimaryPad() == m_iPad);
		bool bInGame = !m_bNotInGame;

		WCHAR TempString[256];

		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_DISPLAY_HUD), eControl_DisplayHUD, (app.GetGameSettings(m_iPad, eGameSetting_DisplayHUD) != 0));
		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_DISPLAY_HAND), eControl_DisplayHand, (app.GetGameSettings(m_iPad, eGameSetting_DisplayHand) != 0));

		int opacityVal = app.GetGameSettings(m_iPad, eGameSetting_InterfaceOpacity);
		swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_INTERFACEOPACITY), opacityVal);
		m_multiList.AddNewSlider(TempString, eControl_InterfaceOpacity, 0, 100, 1, opacityVal);

		m_multiList.AddNewCheckbox(app.GetString(IDS_IN_GAME_TOOLTIPS), eControl_ShowTooltips, (app.GetGameSettings(m_iPad, eGameSetting_Tooltips) != 0));
		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_ANIMATED_CHARACTER), eControl_DisplayAnimatedCharacter, (app.GetGameSettings(m_iPad, eGameSetting_AnimatedCharacter) != 0));

		int sensitivityVal = app.GetGameSettings(m_iPad, eGameSetting_Sensitivity_InMenu);
		swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_SENSITIVITY_INMENU), sensitivityVal);
		m_multiList.AddNewSlider(TempString, eControl_SensitivityInMenu, 0, 200, 1, sensitivityVal);

		if(bPrimaryPlayer)
		{
			m_multiList.AddNewCheckbox(app.GetString(IDS_IN_GAME_GAMERTAGS), eControl_InGameGamertags, (app.GetGameSettings(m_iPad, eGameSetting_GamertagsVisible) != 0));
		}

		if(!bInGame || bPrimaryPlayer)
		{
			m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_DISPLAY_SPLITSCREENGAMERTAGS), eControl_ShowSplitscreenGamertags, (app.GetGameSettings(m_iPad, eGameSetting_DisplaySplitscreenGamertags) != 0));
		}

		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_CLASSICCRAFTING), eControl_ShowClassicCrafting, (app.GetGameSettings(m_iPad, eGameSetting_ClassicCrafting) != 0));
		m_multiList.AddNewCheckbox(L"Hide world disk space bar", eControl_HideSaveSizeBar, (app.GetGameSettings(m_iPad, eGameSetting_HideSaveSizeBar) != 0));

		int uiSizeVal = app.GetGameSettings(m_iPad, eGameSetting_UISize) + 1;
		swprintf(TempString, 256, L"%ls: %d", app.GetString(IDS_SLIDER_UISIZE), uiSizeVal);
		m_multiList.AddNewSlider(TempString, eControl_UISize, 1, 3, 1, uiSizeVal);

		int uiSizeSplitVal = app.GetGameSettings(m_iPad, eGameSetting_UISizeSplitscreen) + 1;
		swprintf(TempString, 256, L"%ls: %d", app.GetString(IDS_SLIDER_UISIZESPLITSCREEN), uiSizeSplitVal);
		m_multiList.AddNewSlider(TempString, eControl_UISizeSplitscreen, 1, 3, 1, uiSizeSplitVal);

		if(!bInGame)
		{
			int controlTypeVal = app.GetGameSettings(m_iPad, eGameSetting_ControlType);
			swprintf(TempString, 256, L"%ls: %ls", app.GetString(IDS_SLIDER_CONTROLTYPE), app.GetString(m_iControlTypeSettingA[controlTypeVal]));
			m_multiList.AddNewSlider(TempString, eControl_ControlType, 0, 5, 1, controlTypeVal);
		}

		IggyName funcDoVert = registerFastName(L"DoVerticalResizeCheck");
		IggyName funcHideDesc = registerFastName(L"HideDescription");
		IggyDataValue result;
		IggyPlayerCallMethodRS(getMovie(), &result, m_rootPath, funcDoVert, 0, nullptr);
		doHorizontalResizeCheck();
		IggyPlayerCallMethodRS(getMovie(), &result, m_rootPath, funcHideDesc, 0, nullptr);
		m_multiList.HighlightItem(eControl_DisplayHUD);
	}

	if(m_bPendingSliderUpdate)
	{
		m_bPendingSliderUpdate = false;
		m_multiList.SetSliderValue(m_iPendingSliderId, m_iPendingSliderValue);

		WCHAR TempString[256];
		switch(m_iPendingSliderId)
		{
		case eControl_InterfaceOpacity:
			app.SetGameSettings(m_iPad, eGameSetting_InterfaceOpacity, m_iPendingSliderValue);
			swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_INTERFACEOPACITY), m_iPendingSliderValue);
			m_multiList.SetSliderLabel(eControl_InterfaceOpacity, TempString);
			break;
		case eControl_SensitivityInMenu:
			app.SetGameSettings(m_iPad, eGameSetting_Sensitivity_InMenu, m_iPendingSliderValue);
			swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_SENSITIVITY_INMENU), m_iPendingSliderValue);
			m_multiList.SetSliderLabel(eControl_SensitivityInMenu, TempString);
			break;
		case eControl_UISize:
			app.SetGameSettings(m_iPad, eGameSetting_UISize, m_iPendingSliderValue - 1);
			ui.UpdateSelectedItemPos(m_iPad);
			swprintf(TempString, 256, L"%ls: %d", app.GetString(IDS_SLIDER_UISIZE), m_iPendingSliderValue);
			m_multiList.SetSliderLabel(eControl_UISize, TempString);
			break;
		case eControl_UISizeSplitscreen:
			app.SetGameSettings(m_iPad, eGameSetting_UISizeSplitscreen, m_iPendingSliderValue - 1);
			ui.UpdateSelectedItemPos(m_iPad);
			swprintf(TempString, 256, L"%ls: %d", app.GetString(IDS_SLIDER_UISIZESPLITSCREEN), m_iPendingSliderValue);
			m_multiList.SetSliderLabel(eControl_UISizeSplitscreen, TempString);
			break;
		case eControl_ControlType:
			app.SetGameSettings(m_iPad, eGameSetting_ControlType, m_iPendingSliderValue);
			m_bControlTypeChanged = true;
			swprintf(TempString, 256, L"%ls: %ls", app.GetString(IDS_SLIDER_CONTROLTYPE), app.GetString(m_iControlTypeSettingA[m_iPendingSliderValue]));
			m_multiList.SetSliderLabel(eControl_ControlType, TempString);
			break;
		}
	}

	UIScene::tick();
	m_bInitialPopulateDone = true;
}

void UIScene_SettingsUIMenu::updateTooltips()
{
	ui.SetTooltips(m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsUIMenu::updateComponents()
{
	bool bNotInGame = (Minecraft::GetInstance()->level == nullptr);
	if(bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, true);
		m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, false);

		if(app.GetLocalPlayerCount() == 1) m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
		else m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, false);
	}
}

void UIScene_SettingsUIMenu::handleGainFocus(bool navBack)
{
	if(navBack)
	{
		m_bNeedsMultiListPopulate = true;
		m_bInitialPopulateDone = false;
	}
}

void UIScene_SettingsUIMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			const bool reloadControlTypeSkin = m_bControlTypeChanged;
			setGameSettings();
			navigateBack();
			if(reloadControlTypeSkin)
			{
				ui.ReloadSkin();
			}
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

void UIScene_SettingsUIMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	int sliderIdInt = static_cast<int>(sliderId);
	int value = static_cast<int>(currentValue);

	m_multiList.handleSliderMove(sliderIdInt, value);

	switch(sliderIdInt)
	{
	case eControl_InterfaceOpacity:
	case eControl_SensitivityInMenu:
	case eControl_UISize:
	case eControl_UISizeSplitscreen:
	case eControl_ControlType:
		m_bPendingSliderUpdate = true;
		m_iPendingSliderId = sliderIdInt;
		m_iPendingSliderValue = value;
		break;
	}
}

void UIScene_SettingsUIMenu::handleCheckboxToggled(F64 controlId, bool selected)
{
	if(m_bInitialPopulateDone)
		ui.PlayUISFX(eSFX_Press);
}

void UIScene_SettingsUIMenu::handlePress(F64 controlId, F64 childId)
{
	ui.PlayUISFX(eSFX_Press);
}

void UIScene_SettingsUIMenu::setGameSettings()
{
	bool bPrimaryPlayer = (ProfileManager.GetPrimaryPad() == m_iPad);
	bool bInGame = !m_bNotInGame;

	app.SetGameSettings(m_iPad, eGameSetting_DisplayHUD, m_multiList.GetCheckboxValue(eControl_DisplayHUD) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_DisplayHand, m_multiList.GetCheckboxValue(eControl_DisplayHand) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_Tooltips, m_multiList.GetCheckboxValue(eControl_ShowTooltips) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_AnimatedCharacter, m_multiList.GetCheckboxValue(eControl_DisplayAnimatedCharacter) ? 1 : 0);

	if(bPrimaryPlayer)
	{
		app.SetGameSettings(m_iPad, eGameSetting_GamertagsVisible, m_multiList.GetCheckboxValue(eControl_InGameGamertags) ? 1 : 0);
	}

	if(!bInGame || bPrimaryPlayer)
	{
		app.SetGameSettings(m_iPad, eGameSetting_DisplaySplitscreenGamertags, m_multiList.GetCheckboxValue(eControl_ShowSplitscreenGamertags) ? 1 : 0);
	}

	app.SetGameSettings(m_iPad, eGameSetting_ClassicCrafting, m_multiList.GetCheckboxValue(eControl_ShowClassicCrafting) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_HideSaveSizeBar, m_multiList.GetCheckboxValue(eControl_HideSaveSizeBar) ? 1 : 0);

	app.SetGameSettings(m_iPad, eGameSetting_InterfaceOpacity, m_multiList.GetSliderValue(eControl_InterfaceOpacity));
	app.SetGameSettings(m_iPad, eGameSetting_Sensitivity_InMenu, m_multiList.GetSliderValue(eControl_SensitivityInMenu));
	app.SetGameSettings(m_iPad, eGameSetting_UISize, m_multiList.GetSliderValue(eControl_UISize) - 1);
	app.SetGameSettings(m_iPad, eGameSetting_UISizeSplitscreen, m_multiList.GetSliderValue(eControl_UISizeSplitscreen) - 1);

	if(!bInGame)
	{
		app.SetGameSettings(m_iPad, eGameSetting_ControlType, m_multiList.GetSliderValue(eControl_ControlType));
	}
}
