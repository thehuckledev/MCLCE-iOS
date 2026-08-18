#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsAudioMenu.h"

UIScene_SettingsAudioMenu::UIScene_SettingsAudioMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

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

UIScene_SettingsAudioMenu::~UIScene_SettingsAudioMenu()
{
}

wstring UIScene_SettingsAudioMenu::getMoviePath()
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

void UIScene_SettingsAudioMenu::tick()
{
	if(m_bNeedsMultiListPopulate)
	{
		m_bNeedsMultiListPopulate = false;
		m_multiList.setupControl(this, m_rootPath, "MultiList");
		m_controls.push_back(&m_multiList);
		m_multiList.clearList();
		m_multiList.init(eControl_MultiList);

		WCHAR TempString[256];
		int musicVol = app.GetGameSettings(m_iPad, eGameSetting_MusicVolume);
		int soundVol = app.GetGameSettings(m_iPad, eGameSetting_SoundFXVolume);
		bool caveSounds = app.GetGameSettings(m_iPad, eGameSetting_CaveSounds) != 0;
		bool minecartSounds = app.GetGameSettings(m_iPad, eGameSetting_MinecartSounds) != 0;
		bool gameChat = app.GetGameSettings(m_iPad, eGameSetting_GameChat) != 0;

		swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_MUSIC), musicVol);
		m_multiList.AddNewSlider(TempString, eControl_Music, 0, 100, 1, musicVol);

		swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_SOUND), soundVol);
		m_multiList.AddNewSlider(TempString, eControl_Sound, 0, 100, 1, soundVol);
		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_CAVE_SOUNDS), eControl_CaveSounds, caveSounds);
		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_MINECART_SOUNDS), eControl_MinecartSounds, minecartSounds);
		m_multiList.AddNewCheckbox(L"Game Chat", eControl_GameChat, gameChat);
		m_multiList.EnableItem(eControl_GameChat, false);

		IggyName funcDoVert = registerFastName(L"DoVerticalResizeCheck");
		IggyName funcHideDesc = registerFastName(L"HideDescription");
		IggyDataValue result;
		IggyPlayerCallMethodRS(getMovie(), &result, m_rootPath, funcDoVert, 0, nullptr);
		doHorizontalResizeCheck();
		IggyPlayerCallMethodRS(getMovie(), &result, m_rootPath, funcHideDesc, 0, nullptr);
		m_multiList.HighlightItem(eControl_Sound);
		m_multiList.HighlightItem(eControl_Music);
	}

	if(m_bPendingSliderUpdate)
	{
		m_bPendingSliderUpdate = false;
		m_multiList.SetSliderValue(m_iPendingSliderId, m_iPendingSliderValue);

		WCHAR TempString[256];
		switch(m_iPendingSliderId)
		{
		case eControl_Music:
			app.SetGameSettings(m_iPad, eGameSetting_MusicVolume, m_iPendingSliderValue);
			swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_MUSIC), m_iPendingSliderValue);
			m_multiList.SetSliderLabel(eControl_Music, TempString);
			break;
		case eControl_Sound:
			app.SetGameSettings(m_iPad, eGameSetting_SoundFXVolume, m_iPendingSliderValue);
			swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_SOUND), m_iPendingSliderValue);
			m_multiList.SetSliderLabel(eControl_Sound, TempString);
			break;
		}
	}

	UIScene::tick();
	m_bInitialPopulateDone = true;
}

void UIScene_SettingsAudioMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsAudioMenu::updateComponents()
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

void UIScene_SettingsAudioMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
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

void UIScene_SettingsAudioMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	int sliderIdInt = static_cast<int>(sliderId);
	int value = static_cast<int>(currentValue);

	m_multiList.handleSliderMove(sliderIdInt, value);

	switch(sliderIdInt)
	{
	case eControl_Music:
	case eControl_Sound:
		m_bPendingSliderUpdate = true;
		m_iPendingSliderId = sliderIdInt;
		m_iPendingSliderValue = value;
		break;
	}
}

void UIScene_SettingsAudioMenu::handleCheckboxToggled(F64 controlId, bool selected)
{
	if(m_bInitialPopulateDone)
		ui.PlayUISFX(eSFX_Press);

	switch(static_cast<int>(controlId))
	{
	case eControl_CaveSounds:
		app.SetGameSettings(m_iPad, eGameSetting_CaveSounds, selected ? 1 : 0);
		break;
	case eControl_MinecartSounds:
		app.SetGameSettings(m_iPad, eGameSetting_MinecartSounds, selected ? 1 : 0);
		break;
	}
}

void UIScene_SettingsAudioMenu::handlePress(F64 controlId, F64 childId)
{
	ui.PlayUISFX(eSFX_Press);
}

void UIScene_SettingsAudioMenu::setGameSettings()
{
	app.SetGameSettings(m_iPad, eGameSetting_MusicVolume, m_multiList.GetSliderValue(eControl_Music));
	app.SetGameSettings(m_iPad, eGameSetting_SoundFXVolume, m_multiList.GetSliderValue(eControl_Sound));
	app.SetGameSettings(m_iPad, eGameSetting_CaveSounds, m_multiList.GetCheckboxValue(eControl_CaveSounds) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_MinecartSounds, m_multiList.GetCheckboxValue(eControl_MinecartSounds) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_GameChat, m_multiList.GetCheckboxValue(eControl_GameChat) ? 1 : 0);
}

void UIScene_SettingsAudioMenu::handleGainFocus(bool navBack)
{
	if(navBack)
	{
		m_bNeedsMultiListPopulate = true;
		m_bInitialPopulateDone = false;
	}
}
