#include "stdafx.h"
#include "UI.h"
#include "UIScene_SkinSelectMenu.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "TexturePackRepository.h"
#ifdef __ORBIS__
#include <error_dialog.h>
#elif defined __PSVITA__
#include <message_dialog.h>
#endif

#define SKIN_SELECT_PACK_DEFAULT 0
#define SKIN_SELECT_PACK_FAVORITES 1
//#define SKIN_SELECT_PACK_PLAYER_CUSTOM 1
#define SKIN_SELECT_MAX_DEFAULTS 2

const WCHAR *UIScene_SkinSelectMenu::wchDefaultNamesA[]=
{
	L"USE LOCALISED VERSION", // Server selected
	L"Steve",
	L"Tennis Steve",
	L"Tuxedo Steve",
	L"Athlete Steve",
	L"Scottish Steve",
	L"Prisoner Steve",
	L"Cyclist Steve",
	L"Boxer Steve",
	L"Alex",
	L"Tennis Alex",
	L"Tuxedo Alex",
	L"Athlete Alex",
	L"Swedish Alex",
	L"Prisoner Alex",
	L"Cyclist Alex",
	L"Boxer Alex",
	L"Developer Alex",
	L"Developer Steve",
};

UIScene_SkinSelectMenu::UIScene_SkinSelectMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	//m_labelSelected.init( app.GetString( IDS_SELECTED ) );

#ifdef __ORBIS__
	m_bErrorDialogRunning=false;
#endif

	m_bIgnoreInput=false;
	m_bNoSkinsToShow = false;
	
	m_currentPack = nullptr;
	m_packIndex = SKIN_SELECT_PACK_DEFAULT;
	m_skinIndex = 0;

	m_originalSkinId = app.GetPlayerSkinId(iPad);
	m_currentSkinPath = app.GetPlayerSkinName(iPad);
	m_selectedSkinPath = L"";
	m_selectedCapePath = L"";
	m_vAdditionalSkinBoxes = nullptr;
	m_vSkinOffsets = nullptr;

	m_bSlidingSkins = false;
	m_bAnimatingMove = false;
	m_bSkinIndexChanged = false;
	m_bFocusDirty = false;
	m_bNeedButtonListRefresh = true;

	m_currentNavigation = eSkinNavigation_Skin;

	m_currentPackCount = 0;

	m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Forward);

	m_characters[eCharacter_Next1].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Left);
	m_characters[eCharacter_Next2].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Left);

	m_characters[eCharacter_Previous1].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Right);
	m_characters[eCharacter_Previous2].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Right);

	m_labelSkinName.init(L"");
	//m_labelSkinOrigin.init(L"");

	//m_leftLabel = L"";
	//m_centreLabel = L"";
	//m_rightLabel = L"";

#ifdef __PSVITA__
		// initialise vita tab  controls with ids
		m_TouchTabLeft.init(ETouchInput_TabLeft);
		m_TouchTabRight.init(ETouchInput_TabRight);
		m_TouchTabCenter.init(ETouchInput_TabCenter);
		m_TouchIggyCharacters.init(ETouchInput_IggyCharacters);
#endif

	// block input if we're waiting for DLC to install. The end of dlc mounting custom message will fill the save list
	if(app.StartInstallDLCProcess(m_iPad))
	{
		// DLC mounting in progress, so disable input
		m_bIgnoreInput=true;

		m_controlTimer.setVisible( true );
		m_controlIggyCharacters.setVisible( false );
		//m_controlSkinNamePlate.setVisible( false );

		setCharacterLocked(false);
		setCharacterSelected(false);
	}
	else
	{
		m_controlTimer.setVisible( false );

		app.ValidateFavoriteSkins(m_iPad);
		setActivePackIndex();
		handlePackIndexChanged();
	}

	// Display the tooltips

#ifdef __PSVITA__
	InitializeCriticalSection(&m_DLCInstallCS);		// to prevent a race condition between the install and the mounted callback
#endif

}

void UIScene_SkinSelectMenu::updateTooltips()
{
	int iFav = -1;
	int favCount = app.GetPlayerFavoriteSkinsCount(m_iPad);

	if(m_packIndex == SKIN_SELECT_PACK_FAVORITES)
	{
		iFav = (favCount > 0) ? IDS_TOOLTIPS_REMOVE_FAVORITE : -1;
	}
	else if(m_packIndex == SKIN_SELECT_PACK_DEFAULT)
	{
		// check if current skin is in favorites
		iFav = IDS_TOOLTIPS_ADD_FAVORITE;
		for(int i = 0; i < favCount; i++)
		{
			if(app.GetPlayerFavoriteSkin(m_iPad, i) == (unsigned int)m_skinIndex)
			{
				iFav = IDS_TOOLTIPS_REMOVE_FAVORITE;
				break;
			}
		}
	}
	else if(m_currentPack != nullptr)
	{
		// dlc pack is checked via skin ID
		DLCSkinFile *skinFile = m_currentPack->getSkinFile(m_skinIndex);
		if(skinFile != nullptr)
		{
			DWORD skinId = skinFile->getSkinID();
			iFav = IDS_TOOLTIPS_ADD_FAVORITE;
			for(int i = 0; i < favCount; i++)
			{
				if(app.GetPlayerFavoriteSkin(m_iPad, i) == skinId)
				{
					iFav = IDS_TOOLTIPS_REMOVE_FAVORITE;
					break;
				}
			}
		}
	}

	ui.SetTooltips(m_iPad, m_bNoSkinsToShow?-1:IDS_TOOLTIPS_SELECT_SKIN,
		IDS_TOOLTIPS_CANCEL, -1, iFav, -1, -1, -1, -1, -1, IDS_TOOLTIPS_NAVIGATE);
}

void UIScene_SkinSelectMenu::updateComponents()
{
	m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
}

wstring UIScene_SkinSelectMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"SkinSelectMenuSplit";
	}
	else
	{
		return L"SkinSelectMenu";
	}
}

void UIScene_SkinSelectMenu::tick()
{
	UIScene::tick();

	if(m_bFocusDirty)
	{
		m_bFocusDirty = false;
		handlePackIndexChanged();
	}

	if(m_bSkinIndexChanged)
	{
		m_bSkinIndexChanged = false;
		handleSkinIndexChanged();
	}

	if(m_bAnimatingMove && !m_characters[eCharacter_Current].IsAnimatingToFacing())
	{
		handleAnimationEnd();
	}

	// check for new DLC installed

	// check for the patch error dialog
#ifdef __ORBIS__

	// process the error dialog (for a patch being available)
	if(m_bErrorDialogRunning)
	{	
		SceErrorDialogStatus stat = sceErrorDialogUpdateStatus();
		if( stat == SCE_ERROR_DIALOG_STATUS_FINISHED ) 
		{
			sceErrorDialogTerminate();
			m_bErrorDialogRunning=false;
		}
	}

#endif
}

void UIScene_SkinSelectMenu::handleAnimationEnd()
{
	if(m_bSlidingSkins)
	{
		m_bSlidingSkins = false;

		m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Forward, false);
		m_characters[eCharacter_Next1].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Left, false);
		m_characters[eCharacter_Previous1].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Right, false);

		m_bSkinIndexChanged = true;
		//handleSkinIndexChanged();

		m_bAnimatingMove = false;
	}
}

void UIScene_SkinSelectMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if (m_bIgnoreInput) return;
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
			app.CheckGameSettingsChanged(true,iPad);
			navigateBack();
			handled = true;
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		if(pressed)
		{
			InputActionOK(iPad);
			handled = true;
		}
		break;
	case ACTION_MENU_Y:
		if(pressed)
		{
			ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
			InputActionFavorite(iPad);
			handled = true;
		}
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		if(pressed)
		{
		DWORD startingIndex = m_packIndex;
			if(key == ACTION_MENU_UP)
				m_packIndex = getPreviousPackIndex(m_packIndex);
			else
				m_packIndex = getNextPackIndex(m_packIndex);

			if(startingIndex != m_packIndex)
			{
				ui.PlayUISFX(eSFX_Scroll);
				m_controlSkinButtonList.HighlightItem(m_packIndex, true);
				handlePackIndexChanged();
			}
			handled = true;
		}
		break;
	case ACTION_MENU_LEFT:
		if(pressed)
			{
				if(!m_bAnimatingMove)
				{
					ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
					ui.PlayUISFX(eSFX_Scroll);

				m_skinIndex = getPreviousSkinIndex(m_skinIndex);
					//handleSkinIndexChanged();

					m_bSlidingSkins = true;
					m_bAnimatingMove = true;

					m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Left, true);
					m_characters[eCharacter_Previous1].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Forward, true);

					// 4J Stu - Swapped nav buttons
				sendInputToMovie(ACTION_MENU_RIGHT, repeat, pressed, released);
				handled = true;
			}
		}
		break;
	case ACTION_MENU_RIGHT:
		if(pressed)
		{
			if(!m_bAnimatingMove)
			{
				ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
				ui.PlayUISFX(eSFX_Scroll);
				m_skinIndex = getNextSkinIndex(m_skinIndex);
					//handleSkinIndexChanged();

				m_bSlidingSkins = true;
				m_bAnimatingMove = true;

				m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Right, true);
				m_characters[eCharacter_Next1].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Forward, true);

					// 4J Stu - Swapped nav buttons
				sendInputToMovie(ACTION_MENU_LEFT, repeat, pressed, released);
				handled = true;
			}
		}
		break;
	case ACTION_MENU_OTHER_STICK_PRESS:
		if(pressed)
		{
			ui.PlayUISFX(eSFX_Press);
			m_characters[eCharacter_Current].ResetRotation();
			handled = true;
		}
		break;
	case ACTION_MENU_OTHER_STICK_LEFT:
		if(pressed)
		{
			m_characters[eCharacter_Current].m_incYRot = true;
			handled = true;
		}
		else if(released)
		{
			m_characters[eCharacter_Current].m_incYRot = false;
		}
		break;
	case ACTION_MENU_OTHER_STICK_RIGHT:
		if(pressed)
		{
			m_characters[eCharacter_Current].m_decYRot = true;
			handled = true;
		}
		else if(released)
		{
			m_characters[eCharacter_Current].m_decYRot = false;
		}
		break;
	case ACTION_MENU_OTHER_STICK_UP:
		if(pressed)
		{
				//m_previewControl->m_incXRot = true;
			m_characters[eCharacter_Current].CyclePreviousAnimation();
			handled = true;
		}
		break;
	case ACTION_MENU_OTHER_STICK_DOWN:
		if(pressed)
		{
				//m_previewControl->m_decXRot = true;
			m_characters[eCharacter_Current].CycleNextAnimation();
			handled = true;
		}
		break;
	}
}

void UIScene_SkinSelectMenu::InputActionOK(unsigned int iPad)
{
	ui.AnimateKeyPress(iPad, ACTION_MENU_OK, false, true, false);

	// if the profile data has been changed, then force a profile write
	// It seems we're allowed to break the 5 minute rule if it's the result of a user action
	switch(m_packIndex)
	{
	case SKIN_SELECT_PACK_DEFAULT:
		app.SetPlayerSkin(iPad, m_skinIndex);
		app.SetPlayerCape(iPad, 0);
		m_currentSkinPath = app.GetPlayerSkinName(iPad);
		m_originalSkinId = app.GetPlayerSkinId(iPad);
		setCharacterSelected(true);
		ui.PlayUISFX(eSFX_Press);
		break;
	case SKIN_SELECT_PACK_FAVORITES:
		if(app.GetPlayerFavoriteSkinsCount(iPad)>0)
		{		
			// get the pack number from the skin id
			wchar_t chars[256];
			swprintf(chars, 256, L"dlcskin%08d.png", GET_DLC_SKIN_ID_FROM_BITMASK(app.GetPlayerFavoriteSkin(iPad,m_skinIndex)));

			DLCPack *Pack=app.m_dlcManager.getPackContainingSkin(chars);	

			if(Pack)
			{
				DLCSkinFile *skinFile = Pack->getSkinFile(chars);
				app.SetPlayerSkin(iPad, skinFile->getPath());
				app.SetPlayerCape(iPad, skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape));
				setCharacterSelected(true);
				m_currentSkinPath = app.GetPlayerSkinName(iPad);
				m_originalSkinId = app.GetPlayerSkinId(iPad);
				app.SetPlayerFavoriteSkinsPos(iPad,m_skinIndex);
	}
}
		break;
	default:
		if( m_currentPack != nullptr )
		{
			bool renableInputAfterOperation = true;
			m_bIgnoreInput = true;

			DLCSkinFile *skinFile = m_currentPack->getSkinFile(m_skinIndex);

			// Is this a free skin?

			if(!skinFile->getParameterAsBool( DLCManager::e_DLCParamType_Free ))
			{
				// do we have a license?
				//if(true)
				if(!m_currentPack->hasPurchasedFile( DLCManager::e_DLCType_Skin, skinFile->getPath() ))
				{
#ifdef __ORBIS__
					// 4J-PB - Check if there is a patch for the game
					int errorCode = ProfileManager.getNPAvailability(ProfileManager.GetPrimaryPad());

					bool bPatchAvailable;
					switch(errorCode)
					{
					case SCE_NP_ERROR_LATEST_PATCH_PKG_EXIST:
					case SCE_NP_ERROR_LATEST_PATCH_PKG_DOWNLOADED:
						bPatchAvailable=true;
						break;
					default:
						bPatchAvailable=false;
						break;
					}

					if(bPatchAvailable)
					{
						int32_t ret=sceErrorDialogInitialize();
						m_bErrorDialogRunning=true;
						if (  ret==SCE_OK ) 
						{
							SceErrorDialogParam param;
							sceErrorDialogParamInitialize( &param );
							// 4J-PB - We want to display the option to get the patch now
							param.errorCode = SCE_NP_ERROR_LATEST_PATCH_PKG_DOWNLOADED;//pClass->m_errorCode;
							ret = sceUserServiceGetInitialUser( &param.userId );
							if ( ret == SCE_OK ) 
							{
								ret=sceErrorDialogOpen( &param );
								break;
							}
						}
					}
#endif

					// no
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;

#ifdef __ORBIS__
					// Check if PSN is unavailable because of age restriction
					int npAvailability = ProfileManager.getNPAvailability(iPad);
					if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
					{
						ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);
					}
					else
#endif
					// We need to upsell the full version
					if(ProfileManager.IsGuest(iPad))
					{
						// can't buy
						ui.RequestAlertMessage(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1,iPad);
					}
#if defined(__PS3__) || defined(__ORBIS__) || defined __PSVITA__
					// are we online?
					else if(!ProfileManager.IsSignedInLive(iPad))
					{
						showNotOnlineDialog(iPad);
					}
#endif
					else
					{
						// upsell
#ifdef _XBOX
						DLC_INFO *pDLCInfo = app.GetDLCInfoForTrialOfferID(m_currentPack->getPurchaseOfferId());
						ULONGLONG ullOfferID_Full;

						if(pDLCInfo!=nullptr)
						{
							ullOfferID_Full=pDLCInfo->ullOfferID_Full;
						}
						else
						{
							ullOfferID_Full=m_currentPack->getPurchaseOfferId();
						}

						// tell sentient about the upsell of the full version of the skin pack
						SentientManager.RecordUpsellPresented(iPad, eSet_UpsellID_Skin_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif
						bool bContentRestricted=false;
#if defined(__PS3__) || defined(__PSVITA__)
						ProfileManager.GetChatAndContentRestrictions(m_iPad,true,nullptr,&bContentRestricted,nullptr);
#endif
						if(bContentRestricted)
						{
#if !(defined(_XBOX) || defined(_WINDOWS64) || defined(_XBOX_ONE)) // 4J Stu - Temp to get the win build running, but so we check this for other platforms
							// you can't see the store
							UINT uiIDA[1];
							uiIDA[0]=IDS_CONFIRM_OK;
							ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);
#endif
						}
						else
						{	
							// 4J-PB - need to check for an empty store
#if defined __ORBIS__ || defined __PSVITA__ || defined __PS3__
							if(app.CheckForEmptyStore(iPad)==false)
#endif
							{
								m_bIgnoreInput = true;
								renableInputAfterOperation = false;

								UINT uiIDA[2] = { IDS_CONFIRM_OK, IDS_CONFIRM_CANCEL };
								ui.RequestAlertMessage(IDS_UNLOCK_DLC_TITLE, IDS_UNLOCK_DLC_SKIN, uiIDA, 2, iPad,&UIScene_SkinSelectMenu::UnlockSkinReturned,this);
							}
						}
					}
				}
				else
				{
					app.SetPlayerSkin(iPad, skinFile->getPath());
					app.SetPlayerCape(iPad, skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape));
					setCharacterSelected(true);
					m_currentSkinPath = app.GetPlayerSkinName(iPad);
					m_originalSkinId = app.GetPlayerSkinId(iPad);

					// push this onto the favorite list
					//AddFavoriteSkin(m_iPad,GET_DLC_SKIN_ID_FROM_BITMASK(m_originalSkinId));
				}
			}
			else
			{
				app.SetPlayerSkin(iPad, skinFile->getPath());
				app.SetPlayerCape(iPad, skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape));
				setCharacterSelected(true);
				m_currentSkinPath = app.GetPlayerSkinName(iPad);
				m_originalSkinId = app.GetPlayerSkinId(iPad);

				// push this onto the favorite list
				AddFavoriteSkin(iPad,GET_DLC_SKIN_ID_FROM_BITMASK(m_originalSkinId));
			}

			if (renableInputAfterOperation)
			{
				m_bIgnoreInput = false;
			}
		}

		ui.PlayUISFX(eSFX_Press);
		break;
	}
}

void UIScene_SkinSelectMenu::customDraw(IggyCustomDrawCallbackRegion *region)
{
	int characterId = -1;
	swscanf(static_cast<wchar_t *>(region->name),L"Character%d",&characterId);
	if (characterId == -1 || characterId >= eCharacter_COUNT)
	{
		app.DebugPrintf("Invalid character to render found\n");
	}
	else
	{
		// Setup GDraw, normal game render states and matrices
		CustomDrawData *customDrawRegion = ui.setupCustomDraw(this,region);

		// don't draw hidden characters
		if(!m_characters[characterId].isVisible())
		{
			ui.endCustomDraw(region);
			delete customDrawRegion;
			return;
		}

		delete customDrawRegion;

		//app.DebugPrintf("Scissor x0= %d, y0= %d, x1= %d, y1= %d\n", region->scissor_x0, region->scissor_y0, region->scissor_x1, region->scissor_y1);
		//app.DebugPrintf("Stencil mask= %d, stencil ref= %d, stencil write= %d\n", region->stencil_func_mask, region->stencil_func_ref, region->stencil_write_mask);
#ifdef __PS3__
		if(region->stencil_func_ref != 0) RenderManager.StateSetStencil(GL_EQUAL,region->stencil_func_ref,region->stencil_func_mask);
#elif __PSVITA__
		// AP - make sure the skins are only drawn inside the smokey panel
		if(region->stencil_func_ref != 0) RenderManager.StateSetStencil(SCE_GXM_STENCIL_FUNC_EQUAL,region->stencil_func_mask,region->stencil_write_mask);
#else
		if(region->stencil_func_ref != 0) RenderManager.StateSetStencil(GL_EQUAL,region->stencil_func_ref, region->stencil_func_mask,region->stencil_write_mask);
#endif
		m_characters[characterId].render(region);

		// Finish GDraw and anything else that needs to be finalised
		ui.endCustomDraw(region);
	}
}

void UIScene_SkinSelectMenu::handleSkinIndexChanged()
{
	BOOL showPrevious = FALSE, showNext = FALSE;
	DWORD previousIndex = 0, nextIndex = 0;
	wstring skinName = L"";
	wstring skinOrigin = L"";
	bool bSkinIsFree=false;
	bool bLicensed=false;
	DLCSkinFile *skinFile=nullptr;
	DLCPack *Pack=nullptr;
	BYTE sidePreviewControlsL,sidePreviewControlsR;
	m_bNoSkinsToShow=false;

	TEXTURE_NAME backupTexture = TN_MOB_CHAR;
	
	setCharacterSelected(false);
	setCharacterBlocked(false);

	//m_controlSkinNamePlate.setVisible( false );

	if( m_currentPack != nullptr )
	{
		skinFile = m_currentPack->getSkinFile(m_skinIndex);
		m_selectedSkinPath = skinFile->getPath();
		m_selectedCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
		m_vAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
		m_vSkinOffsets = skinFile->getOffsets();

		skinName = skinFile->getParameterAsString( DLCManager::e_DLCParamType_DisplayName );
		skinOrigin = skinFile->getParameterAsString( DLCManager::e_DLCParamType_ThemeName );

		if( m_selectedSkinPath.compare( m_currentSkinPath ) == 0 )
		{
			setCharacterSelected(true);
		}

		bSkinIsFree = skinFile->getParameterAsBool( DLCManager::e_DLCParamType_Free );
		bLicensed = m_currentPack->hasPurchasedFile( DLCManager::e_DLCType_Skin, m_selectedSkinPath );
		
		setCharacterLocked(!(bSkinIsFree || bLicensed));
		
		m_characters[eCharacter_Current].setVisible(true);
		//m_controlSkinNamePlate.setVisible( true );
	}
	else
	{	
		m_selectedSkinPath = L"";
		m_selectedCapePath = L"";
		m_vAdditionalSkinBoxes = nullptr;
		m_vSkinOffsets = nullptr;

		switch(m_packIndex)
		{
		case SKIN_SELECT_PACK_DEFAULT:
			backupTexture = getTextureId(m_skinIndex);

			if( m_skinIndex ==  eDefaultSkins_ServerSelected )
			{
				skinName = app.GetString(IDS_DEFAULT_SKINS);
			}
            else if (m_skinIndex < eDefaultSkins_Count)
            {
                skinName = wchDefaultNamesA[m_skinIndex];
            }
            else
            {
                skinName = L"";
				setCharacterSelected(true);
			}
			setCharacterLocked(false);
			setCharacterLocked(false);

			m_characters[eCharacter_Current].setVisible(true);
			//m_controlSkinNamePlate.setVisible( true );

			break;
		case SKIN_SELECT_PACK_FAVORITES:

			if(app.GetPlayerFavoriteSkinsCount(m_iPad)>0)
			{
				unsigned int favSkinId = app.GetPlayerFavoriteSkin(m_iPad,m_skinIndex);
				if(GET_IS_DLC_SKIN_FROM_BITMASK(favSkinId))
				{
					wchar_t chars[256];
					swprintf(chars, 256, L"dlcskin%08d.png", GET_DLC_SKIN_ID_FROM_BITMASK(favSkinId));

					Pack=app.m_dlcManager.getPackContainingSkin(chars);	
					if(Pack)
					{			
						skinFile = Pack->getSkinFile(chars);

						m_selectedSkinPath = skinFile->getPath();
						m_selectedCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
						m_vAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
						m_vSkinOffsets = skinFile->getOffsets();

						skinName = skinFile->getParameterAsString( DLCManager::e_DLCParamType_DisplayName );
						skinOrigin = skinFile->getParameterAsString( DLCManager::e_DLCParamType_ThemeName );

						if( m_selectedSkinPath.compare( m_currentSkinPath ) == 0 )
						{
							setCharacterSelected(true);
						}

						bSkinIsFree = skinFile->getParameterAsBool( DLCManager::e_DLCParamType_Free );
						bLicensed = Pack->hasPurchasedFile( DLCManager::e_DLCType_Skin, m_selectedSkinPath );
					
						setCharacterLocked(!(bSkinIsFree || bLicensed));
					//m_controlSkinNamePlate.setVisible( true );
					}
					else
					{
						setCharacterSelected(false);
						setCharacterBlocked(false);
						setCharacterLocked(false);
					}
				}
				else
				{
					// this fix a thingy with the default skin packs
					DWORD defaultSkinIndex = favSkinId;
					backupTexture = getTextureId(defaultSkinIndex);

					if( defaultSkinIndex == eDefaultSkins_ServerSelected )
					{
						skinName = app.GetString(IDS_DEFAULT_SKINS);
					}
                    else if (defaultSkinIndex < eDefaultSkins_Count)
                    {
                        skinName = wchDefaultNamesA[defaultSkinIndex];
                    }
                    else
                    {
                        skinName = L"";
						setCharacterSelected(true);
					}
					setCharacterLocked(false);

					m_characters[eCharacter_Current].setVisible(true);
				}
			}
			else
			{
				//disable the display
				m_characters[eCharacter_Current].setVisible(false);

				// change the tooltips
				m_bNoSkinsToShow=true;
			}
			break;
		}
	}

	m_labelSkinName.setLabel(skinName);
	m_labelSkinOrigin.setLabel(skinOrigin);


	if(m_vAdditionalSkinBoxes && m_vAdditionalSkinBoxes->size()!=0)
	{
		// add the boxes to the humanoid model, but only if we've not done this already

		vector<ModelPart *> *pAdditionalModelParts = app.GetAdditionalModelParts(skinFile->getSkinID());
		if(pAdditionalModelParts==nullptr)
		{
			pAdditionalModelParts = app.SetAdditionalSkinBoxes(skinFile->getSkinID(),m_vAdditionalSkinBoxes);
		}
	}
	
	if(m_vSkinOffsets && m_vSkinOffsets->size()!=0)
	{
		// add the skin Offsets to the humanoid model, but only if we've not done this already

		vector<SKIN_OFFSET *> *pSkinOffsets = app.GetSkinOffsets(skinFile->getSkinID());
		if(pSkinOffsets==nullptr)
		{
			pSkinOffsets = app.SetSkinOffsets(skinFile->getSkinID(),m_vSkinOffsets);
		}
	}

	if(skinFile!=nullptr)
	{
		app.SetAnimOverrideBitmask(skinFile->getSkinID(),skinFile->getAnimOverrideBitmask());
	}

	// printf("[SkinSelectMenu] Setting current character skin: path='%ls' pack=%d skinIndex=%d\n", m_selectedSkinPath.c_str(), m_packIndex, m_skinIndex);
	m_characters[eCharacter_Current].SetTexture(m_selectedSkinPath, backupTexture);
	m_characters[eCharacter_Current].SetCapeTexture(m_selectedCapePath);

	showNext = TRUE;		
	showPrevious = TRUE;
	nextIndex = getNextSkinIndex(m_skinIndex);
	previousIndex = getPreviousSkinIndex(m_skinIndex);

	wstring otherSkinPath = L"";
	wstring otherCapePath = L"";
	vector<SKIN_BOX *> *othervAdditionalSkinBoxes=nullptr;
	vector<SKIN_OFFSET *> *othervSkinOffsets=nullptr;
	wchar_t chars[256];

	// turn off all displays
	for(unsigned int i = eCharacter_Current + 1; i < eCharacter_COUNT; ++i)
	{
		m_characters[i].setVisible(false);
	}

	unsigned int uiCurrentFavoriteC=app.GetPlayerFavoriteSkinsCount(m_iPad);

	if(m_packIndex==SKIN_SELECT_PACK_FAVORITES)
	{
		// might not be enough to cycle through
		if(uiCurrentFavoriteC<((sidePreviewControls*2)+1))
		{
			if(uiCurrentFavoriteC==0)
			{
				sidePreviewControlsL=sidePreviewControlsR=0;
			}
			// might be an odd number
			else if((uiCurrentFavoriteC-1)%2==1)
			{
				sidePreviewControlsL=1+(uiCurrentFavoriteC-1)/2;
				sidePreviewControlsR=(uiCurrentFavoriteC-1)/2;
			}
			else
			{
				sidePreviewControlsL=sidePreviewControlsR=(uiCurrentFavoriteC-1)/2;
			}
		}
		else
		{
			sidePreviewControlsL=sidePreviewControlsR=sidePreviewControls;
		}
	}
	else
	{
		sidePreviewControlsL=sidePreviewControlsR=sidePreviewControls;
	}

	for(BYTE i = 0; i < sidePreviewControlsR; ++i)
	{
		if(showNext)
		{
			skinFile=nullptr;

			m_characters[eCharacter_Next1 + i].setVisible(true);

			if( m_currentPack != nullptr )
			{
				skinFile = m_currentPack->getSkinFile(nextIndex);
				otherSkinPath = skinFile->getPath();
				otherCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
				othervAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
				othervSkinOffsets = skinFile->getOffsets();
				backupTexture = TN_MOB_CHAR;
			}
			else
			{	
				otherSkinPath = L"";
				otherCapePath = L"";
				othervAdditionalSkinBoxes=nullptr;
				othervSkinOffsets=nullptr;
				switch(m_packIndex)
				{
				case SKIN_SELECT_PACK_DEFAULT:
					backupTexture = getTextureId(nextIndex);
					break;
				case SKIN_SELECT_PACK_FAVORITES:
					if(uiCurrentFavoriteC>0)
					{
						unsigned int favSkinIdNext = app.GetPlayerFavoriteSkin(m_iPad,nextIndex);
						if(GET_IS_DLC_SKIN_FROM_BITMASK(favSkinIdNext))
						{
							// get the pack number from the skin id
							swprintf(chars, 256, L"dlcskin%08d.png", GET_DLC_SKIN_ID_FROM_BITMASK(favSkinIdNext));

							Pack=app.m_dlcManager.getPackContainingSkin(chars);	
							if(Pack)
							{				
								skinFile = Pack->getSkinFile(chars);

								otherSkinPath = skinFile->getPath();
								otherCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
								othervAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
								othervSkinOffsets = skinFile->getOffsets();
							backupTexture = TN_MOB_CHAR;
							}
						}
						else
						{
							// default skin favorite
							backupTexture = getTextureId(favSkinIdNext);
							otherSkinPath = L"";
							otherCapePath = L"";
							othervAdditionalSkinBoxes = nullptr;
						}
					}
					break;
				default:
					break;
				}

			}
			if(othervAdditionalSkinBoxes && othervAdditionalSkinBoxes->size()!=0)
			{
				vector<ModelPart *> *pAdditionalModelParts = app.GetAdditionalModelParts(skinFile->getSkinID());
				if(pAdditionalModelParts==nullptr)
				{
					pAdditionalModelParts = app.SetAdditionalSkinBoxes(skinFile->getSkinID(),othervAdditionalSkinBoxes);
				}
			}
			if(othervSkinOffsets && othervSkinOffsets->size()!=0)
			{
				vector<SKIN_OFFSET *> *pSkinOffsets = app.GetSkinOffsets(skinFile->getSkinID());
				if(pSkinOffsets==nullptr)
				{
					pSkinOffsets = app.SetSkinOffsets(skinFile->getSkinID(),othervSkinOffsets);
				}
			}
			// 4J-PB - anim override needs set before SetTexture
			if(skinFile!=nullptr)
			{
				app.SetAnimOverrideBitmask(skinFile->getSkinID(),skinFile->getAnimOverrideBitmask());
			}
			m_characters[eCharacter_Next1 + i].SetTexture(otherSkinPath, backupTexture);
			m_characters[eCharacter_Next1 + i].SetCapeTexture(otherCapePath);
		}

		nextIndex = getNextSkinIndex(nextIndex);
	}

	static const ECharacters s_previousSlots[2] = { eCharacter_Previous1, eCharacter_Previous2 };

	for(BYTE i = 0; i < sidePreviewControlsL; ++i)
	{
		if(showPrevious)
		{
			skinFile = nullptr;
			ECharacters slot = s_previousSlots[i];

			m_characters[slot].setVisible(true);

			if( m_currentPack != nullptr )
			{
				skinFile = m_currentPack->getSkinFile(previousIndex);
				otherSkinPath = skinFile->getPath();
				otherCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
				othervAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
				othervSkinOffsets = skinFile->getOffsets();
				backupTexture = TN_MOB_CHAR;
			}
			else
			{	
				otherSkinPath = L"";
				otherCapePath = L"";
				othervAdditionalSkinBoxes=nullptr;
				othervSkinOffsets=nullptr;
				switch(m_packIndex)
				{
				case SKIN_SELECT_PACK_DEFAULT:
					backupTexture = getTextureId(previousIndex);
					break;
				case SKIN_SELECT_PACK_FAVORITES:
					if(uiCurrentFavoriteC>0)
					{
						unsigned int favSkinIdPrev = app.GetPlayerFavoriteSkin(m_iPad,previousIndex);
						if(GET_IS_DLC_SKIN_FROM_BITMASK(favSkinIdPrev))
						{
							// get the pack number from the skin id
							swprintf(chars, 256, L"dlcskin%08d.png", GET_DLC_SKIN_ID_FROM_BITMASK(favSkinIdPrev));

							Pack=app.m_dlcManager.getPackContainingSkin(chars);	
							if(Pack)
							{
								skinFile = Pack->getSkinFile(chars);

								otherSkinPath = skinFile->getPath();
								otherCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
								othervAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
								othervSkinOffsets = skinFile->getOffsets();
							backupTexture = TN_MOB_CHAR;
							}
						}
						else
						{
							// default skin favorite
							backupTexture = getTextureId(favSkinIdPrev);
							otherSkinPath = L"";
							otherCapePath = L"";
							othervAdditionalSkinBoxes = nullptr;
						}
					}

					break;
				default:
					break;
				}
			}
			if(othervAdditionalSkinBoxes && othervAdditionalSkinBoxes->size()!=0)
			{
				vector<ModelPart *> *pAdditionalModelParts = app.GetAdditionalModelParts(skinFile->getSkinID());
				if(pAdditionalModelParts==nullptr)
				{
					pAdditionalModelParts = app.SetAdditionalSkinBoxes(skinFile->getSkinID(),othervAdditionalSkinBoxes);
				}
			}
			if(othervSkinOffsets && othervSkinOffsets->size()!=0)
			{
				vector<SKIN_OFFSET *> *pSkinOffsets = app.GetSkinOffsets(skinFile->getSkinID());
				if(pSkinOffsets==nullptr)
				{
					pSkinOffsets = app.SetSkinOffsets(skinFile->getSkinID(),othervSkinOffsets);
				}
			}
			// 4J-PB - anim override needs set before SetTexture
			if(skinFile)
			{
				app.SetAnimOverrideBitmask(skinFile->getSkinID(),skinFile->getAnimOverrideBitmask());
			}			
			m_characters[slot].SetTexture(otherSkinPath, backupTexture);
			m_characters[slot].SetCapeTexture(otherCapePath);
		}

		previousIndex = getPreviousSkinIndex(previousIndex);
	}

	setCharacterFavourite(false);
	int favCount = app.GetPlayerFavoriteSkinsCount(m_iPad);
	if(favCount > 0)
	{
		for(int i = 0; i < favCount; i++)
		{
			unsigned int favSkinId = app.GetPlayerFavoriteSkin(m_iPad, i);
			bool bFound = false;

		if(m_currentPack != nullptr)
		{
			DLCSkinFile *favSkinFile = m_currentPack->getSkinFile(m_skinIndex);
			if(favSkinFile != nullptr && favSkinId == favSkinFile->getSkinID())
			{
				bFound = true;
			}
		}
		else if(m_packIndex == SKIN_SELECT_PACK_DEFAULT)
		{
			if(favSkinId == (unsigned int)m_skinIndex)
			{
				bFound = true;
			}
		}
		else if(m_packIndex == SKIN_SELECT_PACK_FAVORITES)
		{
			bFound = true;
		}

			if(bFound)
			{
				setCharacterFavourite(true);
				break;
			}
		}
	}

	updateTooltips();
}

TEXTURE_NAME UIScene_SkinSelectMenu::getTextureId(int skinIndex)
{
	TEXTURE_NAME texture = TN_MOB_CHAR;
	switch(skinIndex)
	{
	case eDefaultSkins_ServerSelected:
	case eDefaultSkins_Skin0:
		texture = TN_MOB_CHAR;
		break;
	case eDefaultSkins_Skin1:
		texture = TN_MOB_CHAR1;
		break;
	case eDefaultSkins_Skin2:
		texture = TN_MOB_CHAR2;
		break;
	case eDefaultSkins_Skin3:
		texture = TN_MOB_CHAR3;
		break;
	case eDefaultSkins_Skin4:
		texture = TN_MOB_CHAR4;
		break;
	case eDefaultSkins_Skin5:
		texture = TN_MOB_CHAR5;
		break;
	case eDefaultSkins_Skin6:
		texture = TN_MOB_CHAR6;
		break;
	case eDefaultSkins_Skin7:
		texture = TN_MOB_CHAR7;
		break;
	case eDefaultSkins_Skin8:
		texture = TN_MOB_ALEX;
		break;
	case eDefaultSkins_Skin9:
		texture = TN_MOB_ALEX1;
		break;
	case eDefaultSkins_Skin10:
		texture = TN_MOB_ALEX2;
		break;
	case eDefaultSkins_Skin11:
		texture = TN_MOB_ALEX3;
		break;
	case eDefaultSkins_Skin12:
		texture = TN_MOB_ALEX4;
		break;
	case eDefaultSkins_Skin13:
		texture = TN_MOB_ALEX5;
		break;
	case eDefaultSkins_Skin14:
		texture = TN_MOB_ALEX6;
		break;
	case eDefaultSkins_Skin15:
		texture = TN_MOB_ALEX7;
		break;
	case eDefaultSkins_Skin16:
		texture = TN_MOB_DEVALEX;
		break;
	case eDefaultSkins_Skin17:
		texture = TN_MOB_DEVSTEVE;
		break;
	};

	return texture;
}

int UIScene_SkinSelectMenu::getNextSkinIndex(DWORD sourceIndex)
{
	int nextSkin = sourceIndex;

	// special case for favourites
	switch(m_packIndex)
	{

	case SKIN_SELECT_PACK_FAVORITES:
		++nextSkin;
		if(nextSkin>=app.GetPlayerFavoriteSkinsCount(m_iPad))
		{
			nextSkin=0;
		}

		break;
	default:
		++nextSkin;

		if(m_packIndex == SKIN_SELECT_PACK_DEFAULT && nextSkin >= eDefaultSkins_Count)
		{
			nextSkin = eDefaultSkins_ServerSelected;
		}
		else if(m_currentPack != nullptr && nextSkin>=m_currentPack->getSkinCount())
		{
			nextSkin = 0;
		}		
		break;
	}


	return nextSkin;
}

int UIScene_SkinSelectMenu::getPreviousSkinIndex(DWORD sourceIndex)
{
	int previousSkin = sourceIndex;
	switch(m_packIndex)
	{

	case SKIN_SELECT_PACK_FAVORITES:
		if(previousSkin==0)
		{
			previousSkin = app.GetPlayerFavoriteSkinsCount(m_iPad) - 1;
		}
		else
		{
			--previousSkin;
		}		
		break;
	default:
		if(previousSkin==0)
		{
			if(m_packIndex == SKIN_SELECT_PACK_DEFAULT)
			{
				previousSkin = eDefaultSkins_Count - 1;
			}
			else if(m_currentPack != nullptr)
			{
				previousSkin = m_currentPack->getSkinCount()-1;
			}
		}
		else
		{
			--previousSkin;
		}			
		break;
	}


	return previousSkin;
}

void UIScene_SkinSelectMenu::handlePackIndexChanged()
{
	// sync m_packIndex from flash MultiList selection
	if(!m_bIgnoreInput && m_controlSkinButtonList.getItemCount() > 0)
	{
		m_packIndex = m_controlSkinButtonList.getCurrentSelection();
	}

	if(m_packIndex >= SKIN_SELECT_MAX_DEFAULTS)
	{
		m_currentPack = app.m_dlcManager.getPack(m_packIndex - SKIN_SELECT_MAX_DEFAULTS, DLCManager::e_DLCType_Skin);	
	}
	else
	{
		m_currentPack = nullptr;
	}
	m_skinIndex = 0;
	if(m_currentPack != nullptr)
	{
		bool found;
		DWORD currentSkinIndex = m_currentPack->getSkinIndexAt(m_currentSkinPath, found);
		if(found) m_skinIndex = currentSkinIndex;
	}
	else
	{
		switch(m_packIndex)
		{
		case SKIN_SELECT_PACK_DEFAULT:
			if( !GET_IS_DLC_SKIN_FROM_BITMASK(m_originalSkinId) )
			{	
				DWORD ugcSkinIndex = GET_UGC_SKIN_ID_FROM_BITMASK(m_originalSkinId);
				DWORD defaultSkinIndex = GET_DEFAULT_SKIN_ID_FROM_BITMASK(m_originalSkinId);
				if( ugcSkinIndex == 0 )
				{
					m_skinIndex = static_cast<EDefaultSkins>(defaultSkinIndex);
				}
			}	
			break;
		case SKIN_SELECT_PACK_FAVORITES:
			{
				unsigned int favCount = app.GetPlayerFavoriteSkinsCount(m_iPad);
				if(favCount > 0)
				{
					unsigned int pos = app.GetPlayerFavoriteSkinsPos(m_iPad);
					if(pos >= favCount)
					{
						pos = 0; // stale/invalid position — fall back to the first favorite
					}
					m_skinIndex = pos;
				}
			}
			break;
		default:
			break;
		}
	}
	// register pack image or whatever texture and set it on the BitmapIcon
	// control type is used to determine which default image to use if the pack image is not available
	int controlType = app.GetGameSettings(m_iPad, eGameSetting_ControlType);
	wstring controlTypeTextureName = L"default";
	
	switch(controlType)
	{
	case 2:	// Xbox 360
		controlTypeTextureName = L"xbox360";
		break;
	case 3:	// PS3
		controlTypeTextureName = L"playStation";
		break;
	case 4:	// PS4
		controlTypeTextureName = L"playStation";
		break;
	case 5:	// WiiU
		controlTypeTextureName = L"wiiU";
		break;
	default:
		controlTypeTextureName = L"default";
		break;
	}

	wstring textureName = L"";
	if(m_currentPack != nullptr)
	{
		DWORD packId = m_currentPack->GetPackId();
		// printf("Pack name: %ls, packId: %d\n", m_currentPack->getName().c_str(), packId);
		if(packId >= 1)
		{
			wchar_t packIdStr[16];
			swprintf(packIdStr, 16, L"%i", packId);
			textureName = packIdStr;
		}
	}
	if(textureName.empty() || !registerTexture(textureName))
	{
		textureName = controlTypeTextureName;
		registerTexture(textureName);
	}
	m_controlTexturePackIcon.setTextureName(textureName);

	handleSkinIndexChanged();
	SetSkinPackButtonList();
	setPackLabel();
}

void UIScene_SkinSelectMenu::updatePackDisplay()
{
	m_currentPackCount = app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin) + SKIN_SELECT_MAX_DEFAULTS;

}

void UIScene_SkinSelectMenu::handlePress(F64 controlId, F64 childId)
{
}

void UIScene_SkinSelectMenu::handleFocusChange(F64 controlId, F64 childId)
{
	if((int)controlId == 0)
	{
		m_controlSkinButtonList.updateChildFocus((int)childId);
		m_bFocusDirty = true;
	}
}

int UIScene_SkinSelectMenu::relativePackIndex(DWORD base, int offset)
{
	if(offset == 0)
	{
		if(m_bHasTexturePack && base == 2)
		{
			return m_iTexturePackIndex;
		}
		return base;
	}

	int packCount = app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin);
	int newIndex = base + offset;

	if(packCount + SKIN_SELECT_MAX_DEFAULTS < newIndex)
	{
		newIndex = SKIN_SELECT_PACK_DEFAULT;
	}
	else if(newIndex < 0)
	{
		newIndex = packCount + SKIN_SELECT_MAX_DEFAULTS;
	}

	return newIndex;
}

int UIScene_SkinSelectMenu::getNextPackIndex(DWORD sourceIndex)
{
	int nextPack = sourceIndex;
	++nextPack;
	if(nextPack > app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin) - 1 + SKIN_SELECT_MAX_DEFAULTS)
	{
		nextPack = SKIN_SELECT_PACK_DEFAULT;
	}

	return nextPack;
}

int UIScene_SkinSelectMenu::getPreviousPackIndex(DWORD sourceIndex)
{
	int previousPack = sourceIndex;
	if (previousPack == SKIN_SELECT_PACK_DEFAULT)
	{
		DWORD packCount = app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin);
		
		if (packCount > 0)
		{
			previousPack = packCount + SKIN_SELECT_MAX_DEFAULTS - 1;
		}
		else
		{
			previousPack = SKIN_SELECT_MAX_DEFAULTS - 1;
		}
	}
	else
	{
		--previousPack;
	}

	return previousPack;
}

void UIScene_SkinSelectMenu::SetSkinPackButtonList()
{
	if(!m_bNeedButtonListRefresh)
		return;

	m_bNeedButtonListRefresh = false;

	TexturePack *selectedTP = Minecraft::GetInstance()->skins->getSelected();
	DLCPack *texturePackDLC = nullptr;
	if(selectedTP != nullptr && !Minecraft::GetInstance()->skins->isUsingDefaultSkin())
	{
		texturePackDLC = selectedTP->getDLCPack();
	}

	m_controlSkinButtonList.clearList();

	m_controlSkinButtonList.AddNewButton(app.GetString(IDS_NO_SKIN_PACK), SKIN_SELECT_PACK_DEFAULT);

	m_controlSkinButtonList.AddNewButton(app.GetString(IDS_FAVORITES_SKIN_PACK), SKIN_SELECT_PACK_FAVORITES);

	m_bHasTexturePack = false;
	if(texturePackDLC != nullptr)
	{
		DWORD tpPackId = texturePackDLC->GetPackId();
		if(tpPackId > 0x3FF)
		{
			wstring tpName = texturePackDLC->getName();
			if(texturePackDLC->getSkinCount() > 0)
			{
				m_controlSkinButtonList.AddNewButton(tpName, SKIN_SELECT_MAX_DEFAULTS);
				m_bHasTexturePack = true;
			}
		}
	}
    // dlc pack buttons
	int packCount = app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin);
	int highlightIdx = m_packIndex;

	for(int i = 0; i < packCount; i++)
	{
		DLCPack *pack = app.m_dlcManager.getPack(i, DLCManager::e_DLCType_Skin);
		if(pack != nullptr)
		{
			// skip the pack that matches the active texture pack
			if(m_bHasTexturePack && texturePackDLC != nullptr)
			{
				if(pack->GetPackId() == texturePackDLC->GetPackId())
				{
					m_iTexturePackIndex = i + SKIN_SELECT_MAX_DEFAULTS;
					continue;
				}
			}

			wstring displayName = pack->getName();
			m_controlSkinButtonList.AddNewButton(displayName, i + SKIN_SELECT_MAX_DEFAULTS);

			if(m_currentPack != nullptr && pack->GetPackId() == m_currentPack->GetPackId())
			{
				highlightIdx = i + SKIN_SELECT_MAX_DEFAULTS;
			}
		}
	}

	m_controlSkinButtonList.HighlightItem(highlightIdx, true);
}

void UIScene_SkinSelectMenu::setPackLabel()
{
	int relIdx = relativePackIndex(m_packIndex, 0);

	if(relIdx == 0)
	{
		m_labelPackName.setLabel(app.GetString(IDS_NO_SKIN_PACK));
	}
	else if(relIdx == 1)
	{
		m_labelPackName.setLabel(app.GetString(IDS_FAVORITES_SKIN_PACK));
	}
	else if(relIdx >= SKIN_SELECT_MAX_DEFAULTS)
	{
		DLCPack *pack = app.m_dlcManager.getPack(relIdx - SKIN_SELECT_MAX_DEFAULTS, DLCManager::e_DLCType_Skin);
		if(pack != nullptr)
		{
			m_labelPackName.setLabel(pack->getName());
		}
		else
		{
			m_labelPackName.setLabel(L"");
		}
	}
	else
	{
		m_labelPackName.setLabel(L"");
	}

	DWORD packId = 0;
	if(m_currentPack != nullptr)
	{
		packId = m_currentPack->GetPackId();
	}

	if(packId >= 0x400)
	{
		m_labelPackType.setLabel(app.GetString(IDS_TEXTURE_PACK));
	}
	else
	{
		m_labelPackType.setLabel(app.GetString(IDS_SKIN_PACK));
	}
}

bool UIScene_SkinSelectMenu::registerTexture(const wstring &texturePath)
{
	wchar_t texPath[64];
	swprintf(texPath, 64, L"Graphics\\PackGraphics\\%ls.png", texturePath.c_str());

	if(app.hasArchiveFile(texPath))
	{
		byteArray fileData = app.getArchiveFile(texPath);
		if(fileData.data != nullptr && fileData.length > 0)
		{
			registerSubstitutionTexture(texturePath, fileData.data, fileData.length);
			return true;
		}
	}
	return false;
}

void UIScene_SkinSelectMenu::setActivePackIndex()
{
	int favCount = app.GetPlayerFavoriteSkinsCount(m_iPad);
	if(favCount == 0)
	{
		return;
	}

	if(app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin) == 0)
	{
		return;
	}

	m_currentPack = app.m_dlcManager.getPackContainingSkin(m_currentSkinPath);

	// check if current skin is in the favorites list
	for(int i = 0; i < favCount; i++)
	{
		unsigned int favSkinId = app.GetPlayerFavoriteSkin(m_iPad, i);
		if(favSkinId == m_originalSkinId)
		{
			m_packIndex = SKIN_SELECT_PACK_FAVORITES;
			return;
		}
	}
}

void UIScene_SkinSelectMenu::InputActionFavorite(unsigned int iPad)
{
	if(m_packIndex == SKIN_SELECT_PACK_DEFAULT)
	{
		SetFavoriteSkin(iPad, m_skinIndex);
		handleSkinIndexChanged();
	}
	else if(m_packIndex == SKIN_SELECT_PACK_FAVORITES)
	{
		int favCount = app.GetPlayerFavoriteSkinsCount(iPad);
		if(favCount == 0) return;

		unsigned int skinId = app.GetPlayerFavoriteSkin(iPad, m_skinIndex);

		SetFavoriteSkin(iPad, skinId);

		int newCount = app.GetPlayerFavoriteSkinsCount(iPad);
		if(newCount > 0 && newCount <= m_skinIndex)
			m_skinIndex = newCount - 1;

		handleSkinIndexChanged();
	}
	else if(m_currentPack != nullptr)
	{
		DLCSkinFile *skinFile = m_currentPack->getSkinFile(m_skinIndex);
		if(skinFile == nullptr) return;

		SetFavoriteSkin(iPad, skinFile->getSkinID());
		m_bSkinIndexChanged = true;
		return;
	}

	unsigned int pos = app.GetPlayerFavoriteSkinsPos(iPad);
	app.SetPlayerFavoriteSkinsPos(iPad, pos);
}

void UIScene_SkinSelectMenu::SetFavoriteSkin(unsigned int pad, int skinId)
{
	int favCount = app.GetPlayerFavoriteSkinsCount(pad);

	for(int i = 0; i < favCount; i++)
	{
		if(app.GetPlayerFavoriteSkin(pad, i) == (unsigned int)skinId)
		{
			for(int j = i; j < favCount - 1; j++)
			{
				app.SetPlayerFavoriteSkin(pad, j, app.GetPlayerFavoriteSkin(pad, j + 1));
			}
			app.SetPlayerFavoriteSkin(pad, favCount - 1, 0xFFFFFFFF);
			setCharacterFavourite(false);
			return;
		}
	}

	if(favCount < MAX_FAVORITE_SKINS)
	{
		app.SetPlayerFavoriteSkin(pad, favCount, skinId);
		app.SetPlayerFavoriteSkinsPos(pad, favCount);
		setCharacterFavourite(true);
	}
}

void UIScene_SkinSelectMenu::setCharacterSelected(bool selected)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = selected;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetPlayerCharacterSelected , 1 , value );
}

void UIScene_SkinSelectMenu::setCharacterLocked(bool locked)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = locked;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetCharacterLocked , 1 , value );
}

void UIScene_SkinSelectMenu::setCharacterFavourite(bool favourite)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = favourite;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetCharacterFavourite , 1 , value );
}

void UIScene_SkinSelectMenu::setCharacterBlocked(bool blocked)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = blocked;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetCharacterBlocked , 1 , value );
}

#ifdef __PSVITA__
void UIScene_SkinSelectMenu::handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased)
{
	if(bPressed)
	{
		switch(iId)
		{
		case ETouchInput_TabLeft:
		case ETouchInput_TabRight:
		case ETouchInput_TabCenter:
			// change to pack navigation if not already there!
			if(m_currentNavigation != eSkinNavigation_Pack)
			{
				ui.PlayUISFX(eSFX_Scroll);
				m_currentNavigation = eSkinNavigation_Pack;
				sendInputToMovie(ACTION_MENU_UP, false, true, false);
			}
			break;
		case ETouchInput_IggyCharacters:
			if(m_packIndex == SKIN_SELECT_PACK_FAVORITES)
			{
				if(app.GetPlayerFavoriteSkinsCount(m_iPad)==0)
				{
					// ignore this, since there are no skins being displayed
					break;
				}
			}
			// change to skin navigation if not already there!
			if(m_currentNavigation != eSkinNavigation_Skin)
			{
				ui.PlayUISFX(eSFX_Scroll);
				m_currentNavigation = eSkinNavigation_Skin;
				sendInputToMovie(ACTION_MENU_DOWN, false, true, false);
			}
			// remember touch x start
			m_iTouchXStart = x;
			m_bTouchScrolled = false;
			break;
		}
	}
	else if(bRepeat)
	{
		switch(iId)
		{
		case ETouchInput_TabLeft:
			/* no action */
			break;
		case ETouchInput_TabRight:
			/* no action */
			break;
		case ETouchInput_IggyCharacters:
			if(m_currentNavigation != eSkinNavigation_Skin)
			{
				// not in skin select mode
				break;
			}
			if(x < m_iTouchXStart - 50)
			{
				if(!m_bAnimatingMove && !m_bTouchScrolled)
				{
					ui.PlayUISFX(eSFX_Scroll);
					m_skinIndex = getNextSkinIndex(m_skinIndex);
					//handleSkinIndexChanged();

					m_bSlidingSkins = true;
					m_bAnimatingMove = true;

					m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Right, true);
					m_characters[eCharacter_Next1].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Forward, true);

					// 4J Stu - Swapped nav buttons
					sendInputToMovie(ACTION_MENU_LEFT, false, true, false);

					m_bTouchScrolled = true;
				}
			}
			else if(x > m_iTouchXStart + 50)
			{
				if(!m_bAnimatingMove && !m_bTouchScrolled)
				{
					ui.PlayUISFX(eSFX_Scroll);

					m_skinIndex = getPreviousSkinIndex(m_skinIndex);
					//handleSkinIndexChanged();

					m_bSlidingSkins = true;
					m_bAnimatingMove = true;

					m_characters[eCharacter_Current].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Left, true);
					m_characters[eCharacter_Previous1].SetFacing(UIControl_PlayerSkinPreview::e_SkinPreviewFacing_Forward, true);

					// 4J Stu - Swapped nav buttons
					sendInputToMovie(ACTION_MENU_RIGHT, false, true, false);

					m_bTouchScrolled = true;
				}
			}
			break;
		}
	}
	else if(bReleased)
	{
		switch(iId)
		{
		case ETouchInput_TabLeft:
			if( m_currentNavigation == eSkinNavigation_Pack )
			{
				ui.PlayUISFX(eSFX_Scroll);
				DWORD startingIndex = m_packIndex;
				m_packIndex = getPreviousPackIndex(m_packIndex);
				if(startingIndex != m_packIndex)
				{
					handlePackIndexChanged();
				}
			}
			break;
		case ETouchInput_TabRight:
			if( m_currentNavigation == eSkinNavigation_Pack )
			{
				ui.PlayUISFX(eSFX_Scroll);
				DWORD startingIndex = m_packIndex;
				m_packIndex = getNextPackIndex(m_packIndex);
				if(startingIndex != m_packIndex)
				{
					handlePackIndexChanged();
				}
			}
			break;
		case ETouchInput_IggyCharacters:
			if(!m_bTouchScrolled)
			{
				InputActionOK(iPad);
			}
			break;
		}
	}
}
#endif

void UIScene_SkinSelectMenu::HandleDLCInstalled()
{
#ifdef __PSVITA__
	EnterCriticalSection(&m_DLCInstallCS);	// to prevent a race condition between the install and the mounted callback
#endif

	app.DebugPrintf(4,"UIScene_SkinSelectMenu::HandleDLCInstalled\n");
	// mounted DLC may have changed
	if(app.StartInstallDLCProcess(m_iPad)==false)
	{
		// not doing a mount, so re-enable input
		app.DebugPrintf(4,"UIScene_SkinSelectMenu::HandleDLCInstalled - not doing a mount, so re-enable input\n");
		m_bIgnoreInput=false;
	}
	else
	{
		m_bIgnoreInput=true;
		m_controlTimer.setVisible( true );
		m_controlIggyCharacters.setVisible( false );
		//m_controlSkinNamePlate.setVisible( false );
	}

	// this will send a CustomMessage_DLCMountingComplete when done

#ifdef __PSVITA__
	LeaveCriticalSection(&m_DLCInstallCS);	
#endif

}


void UIScene_SkinSelectMenu::HandleDLCMountingComplete()
{	
#ifdef __PSVITA__
	EnterCriticalSection(&m_DLCInstallCS);	// to prevent a race condition between the install and the mounted callback
#endif
	app.DebugPrintf(4,"UIScene_SkinSelectMenu::HandleDLCMountingComplete\n");
	m_controlTimer.setVisible( false );
	m_controlIggyCharacters.setVisible( true );
	//m_controlSkinNamePlate.setVisible( true );

	m_packIndex = SKIN_SELECT_PACK_DEFAULT;

	if(app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin)>0)
	{
		m_currentPack = app.m_dlcManager.getPackContainingSkin(m_currentSkinPath);
		if(m_currentPack != nullptr)
		{
			bool bFound = false;
			m_packIndex = app.m_dlcManager.getPackIndex(m_currentPack,bFound,DLCManager::e_DLCType_Skin) + SKIN_SELECT_MAX_DEFAULTS;
		}
	}

	// If we have any favourites, set this to the favourites
	// first validate the favorite skins - we might have uninstalled the DLC needed for them
	app.ValidateFavoriteSkins(m_iPad);

	if(app.GetPlayerFavoriteSkinsCount(m_iPad)>0)
	{
		m_packIndex = SKIN_SELECT_PACK_FAVORITES;
	}

	handlePackIndexChanged();

	m_bIgnoreInput=false;
	app.m_dlcManager.checkForCorruptDLCAndAlert();
	bool bInGame=(Minecraft::GetInstance()->level!=nullptr);

#if TO_BE_IMPLEMENTED
	if(bInGame) XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
#endif
#ifdef __PSVITA__
	LeaveCriticalSection(&m_DLCInstallCS);	
#endif
}

void UIScene_SkinSelectMenu::showNotOnlineDialog(int iPad)
{
	// need to be signed in to live. get them to sign in to online
#if defined(__PS3__)
	SQRNetworkManager_PS3::AttemptPSNSignIn(nullptr, this);		

#elif defined(__PSVITA__)
	if(CGameNetworkManager::usingAdhocMode() && SQRNetworkManager_AdHoc_Vita::GetAdhocStatus())
	{
		// we're in adhoc mode, we really need to ask before disconnecting
		UINT uiIDA[2];
		uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
		uiIDA[1]=IDS_CANCEL;
		ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&UIScene_SkinSelectMenu::MustSignInReturned,nullptr);
	}
	else
	{
		SQRNetworkManager_Vita::AttemptPSNSignIn(nullptr, this);		
	}

#elif defined(__ORBIS__)
	SQRNetworkManager_Orbis::AttemptPSNSignIn(nullptr, this, false, iPad);

#elif defined(_DURANGO)

	UINT uiIDA[1] = { IDS_CONFIRM_OK };
	ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1, iPad );

#endif
}

int UIScene_SkinSelectMenu::UnlockSkinReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_SkinSelectMenu* pScene = static_cast<UIScene_SkinSelectMenu *>(pParam);

	if	(	(result == C4JStorage::EMessage_ResultAccept)
		&&	ProfileManager.IsSignedIn(iPad)
		)
	{
		if (ProfileManager.IsSignedInLive(iPad))
		{
#if defined(__PS3__) || defined(__ORBIS__) || defined __PSVITA__
			// need to get info on the pack to see if the user has already downloaded it

			// retrieve the store name for the skin pack
			wstring wStrPackName=pScene->m_currentPack->getName();
			const char *pchPackName=wstringtofilename(wStrPackName);
			SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo((char *)pchPackName);

			if (pSONYDLCInfo != nullptr)
			{
				char chName[42];
				char chKeyName[20];
				char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

				memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
				// find the info on the skin pack
				// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
				// So we assume the first sku for the product is the one we want

				// while the store is screwed, hardcode the sku
				//sprintf(chName,"%s-%s-%s",app.GetCommerceCategory(),pSONYDLCInfo->chDLCKeyname,"EURO");
	
				// MGH -  keyname in the DLC file is 16 chars long, but there's no space for a nullptr terminating char
				memset(chKeyName, 0, sizeof(chKeyName));
				strncpy(chKeyName, pSONYDLCInfo->chDLCKeyname, 16);

#ifdef __ORBIS__
				strcpy(chName, chKeyName);
#else
				sprintf(chName,"%s-%s",app.GetCommerceCategory(),chKeyName);
#endif
				app.GetDLCSkuIDFromProductList(chName,chSkuID);

#if defined __ORBIS__ || defined __PSVITA__ || defined __PS3__
				if (app.CheckForEmptyStore(iPad) == false)
#endif
				{	
					if (app.DLCAlreadyPurchased(chSkuID))
					{
						app.DebugPrintf("Already purchased this DLC - DownloadAlreadyPurchased \n");
						app.DownloadAlreadyPurchased(chSkuID);
					}
					else
					{
						app.DebugPrintf("Not yet purchased this DLC - Checkout \n");
						app.Checkout(chSkuID);	
					}
				}
			}
			// need to re-enable input because the user can back out of the store purchase, and we'll be stuck
			pScene->m_bIgnoreInput = false;  // MGH - moved this to outside the pSONYDLCInfo, so we don't get stuck
#elif defined _XBOX_ONE
			StorageManager.InstallOffer(1,(WCHAR *)(pScene->m_currentPack->getPurchaseOfferId().c_str()), &RenableInput, pScene, nullptr);
#endif		
		}
		else // Is signed in, but not live.
		{
			pScene->showNotOnlineDialog(iPad);
			pScene->m_bIgnoreInput = false;
		}
	}
	else
	{
		pScene->m_bIgnoreInput = false;
	}

	return 0;
}

int UIScene_SkinSelectMenu::RenableInput(LPVOID lpVoid, int, int)
{
	static_cast<UIScene_SkinSelectMenu *>(lpVoid)->m_bIgnoreInput = false;
	return 0;
}

void UIScene_SkinSelectMenu::AddFavoriteSkin(int iPad,int iSkinID)
{
	unsigned int favCount = app.GetPlayerFavoriteSkinsCount(iPad);

	// DO NOT TOUCH THE RAM
	for(unsigned int i = 0; i < favCount; i++)
	{
		if(app.GetPlayerFavoriteSkin(iPad, i) == (unsigned int)iSkinID)
		{
			app.SetPlayerFavoriteSkinsPos(iPad, i);
			return;
		}
	}

	if(favCount < MAX_FAVORITE_SKINS)
	{
		// append at the end of the list
		app.SetPlayerFavoriteSkin(iPad, favCount, iSkinID);
		app.SetPlayerFavoriteSkinsPos(iPad, favCount);
	}
	else
	{
		// replace the next skin in the list
		unsigned char ucPos = app.GetPlayerFavoriteSkinsPos(iPad);
		ucPos = (ucPos + 1) % MAX_FAVORITE_SKINS;
		app.SetPlayerFavoriteSkin(iPad, ucPos, iSkinID);
		app.SetPlayerFavoriteSkinsPos(iPad, ucPos);
	}
}


void UIScene_SkinSelectMenu::handleReload()
{
	// Reinitialise a few values to prevent problems on reload
	m_bIgnoreInput=false;

	//m_currentNavigation = eSkinNavigation_Skin;
	m_currentPackCount = 0;

	m_labelSkinName.init(L"");
	m_labelSkinOrigin.init(L"");

	handlePackIndexChanged();
}

#ifdef _XBOX_ONE
void UIScene_SkinSelectMenu::HandleDLCLicenseChange()
{
	// update the lock flag
	handleSkinIndexChanged();
}
#endif




#ifdef __PSVITA__ 
int	UIScene_SkinSelectMenu::MustSignInReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	if(result==C4JStorage::EMessage_ResultAccept) 
	{
#ifdef __PS3__
		SQRNetworkManager_PS3::AttemptPSNSignIn(&UIScene_SkinSelectMenu::PSNSignInReturned, pParam,true);
#elif defined __PSVITA__
		SQRNetworkManager_Vita::AttemptPSNSignIn(&UIScene_SkinSelectMenu::PSNSignInReturned, pParam,true);
#elif defined __ORBIS__
		SQRNetworkManager_Orbis::AttemptPSNSignIn(&UIScene_SkinSelectMenu::PSNSignInReturned, pParam,true);
#endif
	}
	return 0;
}

int UIScene_SkinSelectMenu::PSNSignInReturned(void* pParam, bool bContinue, int iPad)
{
	if( bContinue )
	{
	}
	return 0;
}
#endif // __PSVITA__