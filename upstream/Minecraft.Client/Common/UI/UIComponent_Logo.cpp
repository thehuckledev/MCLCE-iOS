#include "stdafx.h"
#include "UI.h"
#include "UIComponent_Logo.h"

UIComponent_Logo::UIComponent_Logo(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer), m_bFullscreenProgressLogoRefreshed(false)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
}

bool UIComponent_Logo::needsReloaded()
{
	if(!hasMovie())
	{
		return true;
	}

	UIScene *topScene = ui.GetTopScene(m_iPad);
	const bool isSettingsMenuActive = topScene && topScene->getSceneType() == eUIScene_SettingsUIMenu;
	if(isSettingsMenuActive)
	{
		return false;
	}

	const bool isFullscreenProgressActive = topScene && topScene->getSceneType() == eUIScene_FullscreenProgress;
	if(isFullscreenProgressActive && !m_bFullscreenProgressLogoRefreshed)
	{
		m_bFullscreenProgressLogoRefreshed = true;
		return true;
	}
	if(!isFullscreenProgressActive)
	{
		m_bFullscreenProgressLogoRefreshed = false;
	}

	const bool shouldUse1080 = UIScene::SceneShouldUse1080p(getMoviePath());
	if(shouldUse1080 && m_loadedResolution != eSceneResolution_1080)
		return true;
	if(!shouldUse1080 && m_loadedResolution == eSceneResolution_1080)
		return true;

	return false;
}

wstring UIComponent_Logo::getMoviePath()
{
	switch( m_parentLayer->getViewport() )
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		return L"ComponentLogoSplit";
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		return L"ComponentLogo";
		break;
	}
}