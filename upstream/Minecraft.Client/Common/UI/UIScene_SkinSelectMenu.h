#pragma once
#include "../../../Minecraft.World/Definitions.h"
#include "UIScene.h"
#include "UIControl_PlayerSkinPreview.h"
#include "UIControl_MultiList.h"
#include "UIControl_BitmapIcon.h"

class UIScene_SkinSelectMenu : public UIScene
{
private:
	static const WCHAR *wchDefaultNamesA[eDefaultSkins_Count];

	// 4J Stu - How many to show on each side of the main control
	static const BYTE sidePreviewControls = 2;

#ifdef __PSVITA__
	enum ETouchInput
	{
		ETouchInput_TabLeft = 10,
		ETouchInput_TabRight,
		ETouchInput_TabCenter,
		ETouchInput_IggyCharacters,
		
		ETouchInput_Count,
	};
#endif

	enum ESkinSelectNavigation
	{
		eSkinNavigation_Pack,
		eSkinNavigation_Skin,
		
		eSkinNavigation_Count,
	};

	enum ECharacters
	{
		eCharacter_Current = 0,
		eCharacter_Next1 = 1,
		eCharacter_Next2 = 2,
		eCharacter_Previous1 = 4,
		eCharacter_Previous2 = 5,

		eCharacter_COUNT = 7,
	};

	UIControl_PlayerSkinPreview m_characters[eCharacter_COUNT];
	UIControl_Label m_labelSkinName, m_labelSkinOrigin;
	UIControl m_controlBlocked, m_controlFavourite, m_controlLocked, m_controlSelected;
	UIControl_Label m_labelPackName, m_labelPackType;
	UIControl_MultiList m_controlSkinButtonList;
	UIControl_BitmapIcon m_controlTexturePackIcon;
	UIControl m_controlIggyCharacters, m_controlTimer;
	IggyName m_funcSetPlayerCharacterSelected, m_funcSetCharacterLocked;
	IggyName m_funcSetCharacterFavourite, m_funcSetCharacterBlocked;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_controlBlocked, "Blocked" )
		UI_MAP_ELEMENT( m_controlFavourite, "Favourite" )
		UI_MAP_ELEMENT( m_controlLocked, "Locked" )
		UI_MAP_ELEMENT( m_controlSelected, "Selected" )

		UI_MAP_ELEMENT( m_labelSkinName, "SkinTitle1" )
		UI_MAP_ELEMENT( m_labelSkinOrigin, "SkinTitle2" )
		UI_MAP_ELEMENT( m_labelPackName, "Pack_Name" )
		UI_MAP_ELEMENT( m_labelPackType, "Pack_Type" )

		UI_MAP_ELEMENT( m_controlTexturePackIcon, "TexturePackIcon" )
		UI_MAP_ELEMENT( m_controlSkinButtonList, "SkinButtonList" )

		UI_MAP_ELEMENT( m_controlTimer, "Timer" )

		// 4J Stu - These aren't really used a AS3 controls, but adding here means that they get ticked by the scene
		UI_MAP_ELEMENT( m_controlIggyCharacters, "IggyCharacters" )
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlIggyCharacters )
			UI_MAP_ELEMENT( m_characters[eCharacter_Current], "iggy_Character0" )
			UI_MAP_ELEMENT( m_characters[eCharacter_Next1], "iggy_Character1" )
			UI_MAP_ELEMENT( m_characters[eCharacter_Next2], "iggy_Character2" )
			UI_MAP_ELEMENT( m_characters[eCharacter_Previous1], "iggy_Character6" )
			UI_MAP_ELEMENT( m_characters[eCharacter_Previous2], "iggy_Character5" )
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_NAME( m_funcSetPlayerCharacterSelected, L"SetPlayerCharacterSelected" )
		UI_MAP_NAME( m_funcSetCharacterLocked, L"SetCharacterLocked" )
		UI_MAP_NAME( m_funcSetCharacterFavourite, L"SetCharacterFavourite" )
		UI_MAP_NAME( m_funcSetCharacterBlocked, L"SetCharacterBlocked" )
	UI_END_MAP_ELEMENTS_AND_NAMES()

	DLCPack *m_currentPack;
	DWORD m_packIndex, m_skinIndex;
	DWORD m_originalSkinId;
	wstring m_currentSkinPath, m_selectedSkinPath, m_selectedCapePath;
	vector<SKIN_BOX *> *m_vAdditionalSkinBoxes;
	vector<SKIN_OFFSET *> *m_vSkinOffsets;

	bool m_bSlidingSkins, m_bAnimatingMove;
	ESkinSelectNavigation m_currentNavigation;

	bool m_bNoSkinsToShow;
	DWORD m_currentPackCount;
	bool m_bIgnoreInput;
	bool m_bSkinIndexChanged;
	bool m_bFocusDirty;
	bool m_bNeedButtonListRefresh;
	bool m_bHasTexturePack;
	int m_iTexturePackIndex;

	S32 m_iTouchXStart;
	bool m_bTouchScrolled;
public:
	UIScene_SkinSelectMenu(int iPad, void *initData, UILayer *parentLayer);
#ifdef __PSVITA__
	virtual ~UIScene_SkinSelectMenu() { DeleteCriticalSection(&m_DLCInstallCS); }
#endif

	virtual void tick();
	
	virtual void updateTooltips();
	virtual void updateComponents();

	virtual EUIScene getSceneType() { return eUIScene_SkinSelectMenu;}

	virtual void handleAnimationEnd();

	virtual void handleFocusChange(F64 controlId, F64 childId);

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

private:
	void handleSkinIndexChanged();
	int getNextSkinIndex(DWORD sourceIndex);
	int getPreviousSkinIndex(DWORD sourceIndex);

	TEXTURE_NAME getTextureId(int skinIndex);
	
	void handlePackIndexChanged();
	void updatePackDisplay();
	int getNextPackIndex(DWORD sourceIndex);
	int getPreviousPackIndex(DWORD sourceIndex);

	void setCharacterSelected(bool selected);
	void setCharacterLocked(bool locked);
	void setCharacterFavourite(bool favourite);
	void setCharacterBlocked(bool blocked);
	void SetSkinPackButtonList();
	void setPackLabel();
	int relativePackIndex(DWORD base, int offset);
	void handlePress(F64 controlId, F64 childId);
	void setActivePackIndex();
	bool registerTexture(const wstring &texturePath);

	virtual void HandleDLCMountingComplete();
	virtual void HandleDLCInstalled();
#ifdef _XBOX_ONE
	virtual void HandleDLCLicenseChange();
#endif

	void showNotOnlineDialog(int iPad);

	static int UnlockSkinReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int RenableInput(LPVOID lpVoid, int, int);
	void AddFavoriteSkin(int iPad,int iSkinID);

	void InputActionOK(unsigned int iPad);
	void InputActionFavorite(unsigned int iPad);
	void SetFavoriteSkin(unsigned int pad, int skinId);
#ifdef __PSVITA__
	virtual void handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased);
#endif //__PSVITA__
	virtual void handleReload();

#ifdef __ORBIS__
	bool m_bErrorDialogRunning;
#endif

#ifdef __PSVITA__
	static int	MustSignInReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int PSNSignInReturned(void* pParam, bool bContinue, int iPad);
#endif


#ifdef __PSVITA__
	CRITICAL_SECTION m_DLCInstallCS;		// to prevent a race condition between the install and the mounted callback
#endif
};