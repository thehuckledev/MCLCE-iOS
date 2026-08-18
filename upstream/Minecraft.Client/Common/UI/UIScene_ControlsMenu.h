#pragma once

#include "UIScene.h"

class UIScene_ControlsMenu : public UIScene
{
private:
	enum EControl
	{
		// Buttons must be first three controls here
		eControl_Button0,
		eControl_Button1,
		eControl_Button2,
		eControl_InvertLook,
		eControl_Southpaw,
		eControl_SafeCam,
		eControl_ABSwap,
	};

	enum EPadButtons
	{	
		e_PadBack=0,
		e_PadLT,
		e_PadLB,
		e_PadDPadLeft,
		e_PadDPadRight,
		e_PadDPadUp,
		e_PadDPadDown,
		e_PadLS_1,
		e_PadLS_2,
		e_PadStart,		
		e_PadRT,
		e_PadRB,
		e_PadY,
		e_PadB,
		e_PadA,
		e_PadX,
		e_PadRS_1,
		e_PadRS_2,
		e_PadTouch,

		e_PadCOUNT,
	};

	int m_iSchemeTextA[3];	
	int m_iCurrentNavigatedControlsLayout;
	bool m_bCreativeMode;
	bool m_bLayoutChanged;

	UIControl_Label m_labelCurrentLayout;
	UIControl_Label m_labelVersion;
	UIControl_Button m_buttonLayouts[3];
	UIControl_CheckBox m_checkboxInvert, m_checkboxSouthpaw, m_checkboxSafeCam, m_checkboxAbswap;
	IggyName m_funcSetPlatform, m_funcSetControllerLayout;
	IggyName m_funcSetLineAndText, m_funcClearAllKeyLines;
	IggyName m_funcSetABSwapCheckBox, m_funcRemoveSafeSprint;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)

#ifndef __PSVITA__
#ifdef __ORBIS__
	if (!InputManager.UsingRemoteVita())
#endif
	{
		UI_MAP_ELEMENT( m_labelCurrentLayout, "CurrentLayout")

		UI_MAP_ELEMENT( m_buttonLayouts[0], "Button1")
		UI_MAP_ELEMENT( m_buttonLayouts[1], "Button2")
		UI_MAP_ELEMENT( m_buttonLayouts[2], "Button3")
	}
#endif

		UI_MAP_ELEMENT( m_checkboxInvert, "InvertLook")
		UI_MAP_ELEMENT( m_checkboxSouthpaw, "SouthPaw")
		UI_MAP_ELEMENT( m_checkboxSafeCam, "SafeCam")
		UI_MAP_ELEMENT( m_checkboxAbswap, "ABSwap")

		UI_MAP_NAME( m_funcSetPlatform, L"SetPlatform")
		UI_MAP_NAME( m_funcSetControllerLayout, L"SetControllerLayout")
		UI_MAP_NAME( m_funcSetLineAndText, L"SetLineAndText")
		UI_MAP_NAME( m_funcClearAllKeyLines, L"ClearAllKeyLines")
		UI_MAP_NAME( m_funcSetABSwapCheckBox, L"SetABSwapCheckBox")
		UI_MAP_NAME( m_funcRemoveSafeSprint, L"RemoveSafeSprint")
		UI_MAP_ELEMENT( m_labelVersion, "Version")
	UI_END_MAP_ELEMENTS_AND_NAMES()
public:
	UIScene_ControlsMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_ControlsMenu;}
	
	virtual void updateTooltips();
	virtual void tick();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleCheckboxToggled(F64 controlId, bool selected);	
	virtual void handlePress(F64 controlId, F64 childId);
	virtual void handleFocusChange(F64 controlId, F64 childId);

private:
	void PositionText(int iPad,int iTextID, unsigned char ucAction);
	void PositionTextDirect(int iPad,int iTextID, int iControlDetailsIndex, bool bShow);
	void PositionAllText(int iPad);
};