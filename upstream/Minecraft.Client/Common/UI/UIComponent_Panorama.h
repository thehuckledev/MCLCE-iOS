#pragma once

#include "UIScene.h"

class UIComponent_Panorama : public UIScene
{
private:
	bool m_bShowingDay;
	void EnsurePanoramaTexturesLoaded();
	void DrawPanoramaBackgroundQuad(S32 width, S32 height);

	int m_texPanoramaDay = -1;
	int m_texPanoramaNight = -1;

	bool m_bPanoramaTexturesLoaded = false;

	wstring m_panoramaTextureRoot;
	wstring m_panoramaTextureRootKey;
	float m_panoramaAspect = 1.0f;
	DWORD m_lastScrollTickMs = 0;
	float m_panoramaScroll = 0.0f;

	float m_panoramaScrollSpeed = 0.0001f / 16.6667f;
	float m_panoramaBlurSigma = 0.5f;

	bool m_panoramaGridEnabled = false;
	float m_panoramaGridSizeX = 1.0f;
	float m_panoramaGridSizeY = 1.0f;

	int m_panoramaNativeWidth = 0;
	int m_panoramaNativeHeight = 0;
	
	bool m_panoramaGridScalingNearest = false;            

public:
	UIComponent_Panorama(int iPad, void *initData, UILayer *parentLayer);

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	virtual EUIScene getSceneType() { return eUIComponent_Panorama;}
	virtual bool isPanoramaOverrideScene() { return true; }
	virtual bool isPanoramaDayOverride() { return m_bShowingDay; }

	// Returns true if this scene handles input
	virtual bool stealsFocus() { return false; }

	// Returns true if this scene has focus for the pad passed in
	virtual bool hasFocus(int iPad) { return false; }

	virtual void tick();

	// RENDERING
	virtual void render(S32 width, S32 height, C4JRender::eViewportType viewport);

private:
	void setPanorama(bool isDay);
};
