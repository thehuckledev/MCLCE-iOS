#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsGraphicsMenu.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "../../GameRenderer.h"

#ifdef _WINDOWS64
extern bool g_bVSync;
extern void SetExclusiveFullscreen(bool enabled);
#endif

namespace
{
    constexpr int FOV_MIN = 70;
    constexpr int FOV_MAX = 110;
    constexpr int FOV_SLIDER_MAX = 100;

	int ClampFov(int value)
	{
		if (value < FOV_MIN) return FOV_MIN;
		if (value > FOV_MAX) return FOV_MAX;
		return value;
	}

    [[maybe_unused]]
    int FovToSliderValue(float fov)
	{
		const int clampedFov = ClampFov(static_cast<int>(fov + 0.5f));
		return ((clampedFov - FOV_MIN) * FOV_SLIDER_MAX) / (FOV_MAX - FOV_MIN);
	}

	int sliderValueToFov(int sliderValue)
	{
		if (sliderValue < 0) sliderValue = 0;
		if (sliderValue > FOV_SLIDER_MAX) sliderValue = FOV_SLIDER_MAX;
		return FOV_MIN + ((sliderValue * (FOV_MAX - FOV_MIN)) / FOV_SLIDER_MAX);
	}
}

int UIScene_SettingsGraphicsMenu::LevelToDistance(int level)
{
	static const int table[6] = {2,4,8,16,32,64};
	if(level < 0) level = 0;
	if(level > 5) level = 5;
	return table[level];
}

int UIScene_SettingsGraphicsMenu::DistanceToLevel(int dist)
{
    static const int table[6] = {2,4,8,16,32,64};
    for(int i = 0; i < 6; i++){
        if(table[i] == dist)
            return i;
    }
    return 3;
}

UIScene_SettingsGraphicsMenu::UIScene_SettingsGraphicsMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

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

UIScene_SettingsGraphicsMenu::~UIScene_SettingsGraphicsMenu()
{
}

wstring UIScene_SettingsGraphicsMenu::getMoviePath()
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

void UIScene_SettingsGraphicsMenu::tick()
{
	if (m_bNeedsMultiListPopulate)
	{
		m_bNeedsMultiListPopulate = false;
		m_multiList.setupControl(this, m_rootPath, "MultiList");
		m_controls.push_back(&m_multiList);
		m_multiList.clearList();
		m_multiList.init(eControl_MultiList);

		WCHAR TempString[256];

		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_RENDER_CLOUDS), eControl_Clouds, (app.GetGameSettings(m_iPad, eGameSetting_Clouds) != 0));
		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_CUSTOM_SKIN_ANIM), eControl_CustomSkinAnim, (app.GetGameSettings(m_iPad, eGameSetting_CustomSkinAnim) != 0));
		m_multiList.AddNewCheckbox(app.GetString(IDS_CHECKBOX_RENDER_BEDROCKFOG), eControl_BedrockFog, (app.GetGameSettings(m_iPad, eGameSetting_BedrockFog) != 0));

#ifdef _WINDOWS64
		m_multiList.AddNewCheckbox(L"VSync", eControl_VSync, (app.GetGameSettings(m_iPad, eGameSetting_VSync) != 0));
		m_multiList.AddNewCheckbox(L"Fullscreen", eControl_ExclusiveFullscreen, (app.GetGameSettings(m_iPad, eGameSetting_ExclusiveFullscreen) != 0));
#endif

		int gammaVal = app.GetGameSettings(m_iPad, eGameSetting_Gamma);
		swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_GAMMA), gammaVal);
		m_multiList.AddNewSlider(TempString, eControl_Gamma, 0, 100, 1, gammaVal);

		int renderDistLevel = DistanceToLevel(app.GetGameSettings(m_iPad, eGameSetting_RenderDistance));
		swprintf(TempString, 256, L"Render Distance: %d", LevelToDistance(renderDistLevel));
		m_multiList.AddNewSlider(TempString, eControl_RenderDistance, 0, 3, 1, renderDistLevel);

		int fovSlider = app.GetGameSettings(m_iPad, eGameSetting_FOV);
		int fovDeg = sliderValueToFov(fovSlider);
		swprintf(TempString, 256, L"FOV: %d", fovDeg);
		m_multiList.AddNewSlider(TempString, eControl_FOV, 0, FOV_SLIDER_MAX, 1, fovSlider);

		IggyName funcDoVert = registerFastName(L"DoVerticalResizeCheck");
		IggyName funcHideDesc = registerFastName(L"HideDescription");
		IggyDataValue result;
		IggyPlayerCallMethodRS(getMovie(), &result, m_rootPath, funcDoVert, 0, nullptr);
		doHorizontalResizeCheck();
		IggyPlayerCallMethodRS(getMovie(), &result, m_rootPath, funcHideDesc, 0, nullptr);
		m_multiList.HighlightItem(eControl_Clouds);
	}

	if (m_bPendingSliderUpdate)
	{
		m_bPendingSliderUpdate = false;
		m_multiList.SetSliderValue(m_iPendingSliderId, m_iPendingSliderValue);

		WCHAR TempString[256];
		switch (m_iPendingSliderId)
		{
		case eControl_RenderDistance:
		{
			int dist = LevelToDistance(m_iPendingSliderValue);
			app.SetGameSettings(m_iPad, eGameSetting_RenderDistance, dist);
			Minecraft *pMinecraft = Minecraft::GetInstance();
			pMinecraft->options->viewDistance = 3 - m_iPendingSliderValue;
			swprintf(TempString, 256, L"Render Distance: %d", dist);
			m_multiList.SetSliderLabel(eControl_RenderDistance, TempString);
			break;
		}
		case eControl_Gamma:
			app.SetGameSettings(m_iPad, eGameSetting_Gamma, m_iPendingSliderValue);
			swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_GAMMA), m_iPendingSliderValue);
			m_multiList.SetSliderLabel(eControl_Gamma, TempString);
			break;
		case eControl_FOV:
		{
			int fovDeg = sliderValueToFov(m_iPendingSliderValue);
			Minecraft *pMinecraft = Minecraft::GetInstance();
			pMinecraft->gameRenderer->SetFovVal(static_cast<float>(fovDeg));
			app.SetGameSettings(m_iPad, eGameSetting_FOV, m_iPendingSliderValue);
			swprintf(TempString, 256, L"FOV: %d", fovDeg);
			m_multiList.SetSliderLabel(eControl_FOV, TempString);
			break;
		}
		}
	}

	UIScene::tick();
	m_bInitialPopulateDone = true;
}

void UIScene_SettingsGraphicsMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsGraphicsMenu::updateComponents()
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

void UIScene_SettingsGraphicsMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
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

void UIScene_SettingsGraphicsMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	int sliderIdInt = static_cast<int>(sliderId);
	int value = static_cast<int>(currentValue);

	m_multiList.handleSliderMove(sliderIdInt, value);

	switch (sliderIdInt)
	{
	case eControl_RenderDistance:
	case eControl_Gamma:
	case eControl_FOV:
		m_bPendingSliderUpdate = true;
		m_iPendingSliderId = sliderIdInt;
		m_iPendingSliderValue = value;
		break;
	}
}

void UIScene_SettingsGraphicsMenu::handleCheckboxToggled(F64 controlId, bool selected)
{
	if (m_bInitialPopulateDone)
		ui.PlayUISFX(eSFX_Press);
}

void UIScene_SettingsGraphicsMenu::handlePress(F64 controlId, F64 childId)
{
	ui.PlayUISFX(eSFX_Press);
}

void UIScene_SettingsGraphicsMenu::setGameSettings()
{
	app.SetGameSettings(m_iPad, eGameSetting_Clouds, m_multiList.GetCheckboxValue(eControl_Clouds) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_BedrockFog, m_multiList.GetCheckboxValue(eControl_BedrockFog) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_CustomSkinAnim, m_multiList.GetCheckboxValue(eControl_CustomSkinAnim) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_RenderDistance, LevelToDistance(m_multiList.GetSliderValue(eControl_RenderDistance)));
	app.SetGameSettings(m_iPad, eGameSetting_Gamma, m_multiList.GetSliderValue(eControl_Gamma));
	app.SetGameSettings(m_iPad, eGameSetting_FOV, m_multiList.GetSliderValue(eControl_FOV));

#ifdef _WINDOWS64
	app.SetGameSettings(m_iPad, eGameSetting_VSync, m_multiList.GetCheckboxValue(eControl_VSync) ? 1 : 0);
	app.SetGameSettings(m_iPad, eGameSetting_ExclusiveFullscreen, m_multiList.GetCheckboxValue(eControl_ExclusiveFullscreen) ? 1 : 0);
	g_bVSync = m_multiList.GetCheckboxValue(eControl_VSync);
	SetExclusiveFullscreen(m_multiList.GetCheckboxValue(eControl_ExclusiveFullscreen));
#endif
}

void UIScene_SettingsGraphicsMenu::handleGainFocus(bool navBack)
{
	if (navBack)
	{
		m_bNeedsMultiListPopulate = true;
		m_bInitialPopulateDone = false;
	}
}
