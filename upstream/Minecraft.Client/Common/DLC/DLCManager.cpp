#include "stdafx.h"
#include <algorithm>
#include "DLCManager.h"
#include "DLCPack.h"
#include "DLCFile.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/File.h"
#include "../../Minecraft.h"
#include "../../TexturePackRepository.h"
#include "Common/UI/UI.h"
#include "lce_filesystem/FolderFile.h"

static bool isDigitW(wchar_t ch)
{
	return ch >= L'0' && ch <= L'9';
}

static bool isDataPackPckName(const wstring &baseNameLower)
{
	const wstring suffix = L"data.pck";
	if(baseNameLower.size() <= suffix.size() + 1)
	{
		return false;
	}
	if(baseNameLower[0] != L'x')
	{
		return false;
	}
	if(baseNameLower.compare(baseNameLower.size() - suffix.size(), suffix.size(), suffix) != 0)
	{
		return false;
	}
	const size_t digitsStart = 1;
	const size_t digitsEnd = baseNameLower.size() - suffix.size();
	if(digitsEnd <= digitsStart)
	{
		return false;
	}
	for(size_t i = digitsStart; i < digitsEnd; ++i)
	{
		if(!isDigitW(baseNameLower[i]))
		{
			return false;
		}
	}
	return true;
}

static bool hasPckFolderFallback(const wstring &path, wstring &folderPath)
{
	wstring lowerPath = toLower(path);
	const wstring pckSuffix = L".pck";
	if(lowerPath.size() <= pckSuffix.size())
	{
		return false;
	}
	if(lowerPath.compare(lowerPath.size() - pckSuffix.size(), pckSuffix.size(), pckSuffix) != 0)
	{
		return false;
	}

	const size_t nameStart = lowerPath.find_last_of(L"/\\");
	const size_t baseOffset = (nameStart == wstring::npos) ? 0 : (nameStart + 1);
	wstring baseNameLower = lowerPath.substr(baseOffset);
	if(!isDataPackPckName(baseNameLower))
	{
		return false;
	}

	folderPath = path.substr(0, path.size() - pckSuffix.size());
	return true;
}

static bool isValidDLCRange(const PBYTE pbData, DWORD dwLength, unsigned int offset, size_t size)
{
	return offset <= dwLength && size <= static_cast<size_t>(dwLength) - offset;
}

static DLCManager::EDLCType getFolderFileType(const wstring &path)
{
	wstring lowerPath = toLower(path);
	if(lowerPath == L"colours.col" || lowerPath.rfind(L"/colours.col") != wstring::npos)
	{
		return DLCManager::e_DLCType_ColourTable;
	}
	if(lowerPath.rfind(L"/languages.loc") != wstring::npos || lowerPath.rfind(L".loc") != wstring::npos)
	{
		return DLCManager::e_DLCType_LocalisationData;
	}
	if(lowerPath.rfind(L".xzp") != wstring::npos)
	{
		return DLCManager::e_DLCType_UIData;
	}
	if(lowerPath.rfind(L".grf") != wstring::npos)
	{
		return DLCManager::e_DLCType_GameRulesHeader;
	}
	return DLCManager::e_DLCType_Texture;
}

const WCHAR *DLCManager::wchTypeNamesA[]=
{
	L"XMLVERSION",
	L"DISPLAYNAME",
	L"THEMENAME",
	L"FREE", 
	L"CREDIT",
	L"CAPEPATH",
	L"BOX",
	L"ANIM",
	L"PACKID",
	L"NETHERPARTICLECOLOUR",
	L"ENCHANTTEXTCOLOUR",
	L"ENCHANTTEXTFOCUSCOLOUR",
	L"DATAPATH",
	L"PACKVERSION",
	L"OFFSET",
};

DLCManager::DLCManager()
{
	//m_bNeedsUpdated = true;
	m_bNeedsCorruptCheck = true;
}

DLCManager::~DLCManager()
{
	for ( DLCPack *pack : m_packs )
	{
		if ( pack )
			delete pack;
	}
}

DLCManager::EDLCParameterType DLCManager::getParameterType(const wstring &paramName)
{
	EDLCParameterType type = e_DLCParamType_Invalid;

	for(DWORD i = 0; i < e_DLCParamType_Max; ++i)
	{
		if(paramName.compare(wchTypeNamesA[i]) == 0)
		{
			type = static_cast<EDLCParameterType>(i);
			break;
		}
	}

	return type;
}

DWORD DLCManager::getPackCount(EDLCType type /*= e_DLCType_All*/)
{
	DWORD packCount = 0;
	if( type != e_DLCType_All )
	{
		for( DLCPack *pack : m_packs )
		{
			if( pack && pack->getDLCItemsCount(type) > 0 )
			{
				++packCount;
			}
		}
	}
	else
	{
		packCount = static_cast<DWORD>(m_packs.size());
	}
	return packCount;
}

void DLCManager::addPack(DLCPack *pack)
{
	m_packs.push_back(pack);
}

void DLCManager::removePack(DLCPack *pack)
{
	if(pack != nullptr)
	{
		auto it = find(m_packs.begin(), m_packs.end(), pack);
		if(it != m_packs.end() ) m_packs.erase(it);
		delete pack;
	}
}

void DLCManager::removeAllPacks(void)
{
	for( DLCPack *pack : m_packs )
	{
		if ( pack )
			delete pack;
	}

	m_packs.clear();
}

void DLCManager::LanguageChanged(void)
{
	for( DLCPack *pack : m_packs )
	{
		// update the language
		pack->UpdateLanguage();
	}
}

DLCPack *DLCManager::getPack(const wstring &name)
{
	DLCPack *pack = nullptr;
	//DWORD currentIndex = 0;
	for( DLCPack * currentPack : m_packs )
	{
		wstring wsName=currentPack->getName();

		if(wsName.compare(name) == 0)
		{
			pack = currentPack;
			break;
		}
	}
	return pack;
}

#ifdef _XBOX_ONE
DLCPack *DLCManager::getPackFromProductID(const wstring &productID)
{
	DLCPack *pack = nullptr;
	for( DLCPack *currentPack : m_packs )
	{
		wstring wsName=currentPack->getPurchaseOfferId();

		if(wsName.compare(productID) == 0)
		{
			pack = currentPack;
			break;
		}
	}
	return pack;
}
#endif

DLCPack *DLCManager::getPack(DWORD index, EDLCType type /*= e_DLCType_All*/)
{
	DLCPack *pack = nullptr;
	if( type != e_DLCType_All )
	{
		DWORD currentIndex = 0;
		for( DLCPack *currentPack : m_packs )
		{
			if(currentPack->getDLCItemsCount(type)>0)
			{
				if(currentIndex == index)
				{
					pack = currentPack;
					break;
				}
				++currentIndex;
			}
		}
	}
	else
	{
		if(index >= m_packs.size())
		{
			app.DebugPrintf("DLCManager: Trying to access a DLC pack beyond the range of valid packs\n");
			DEBUG_BREAK();
		}
		pack = m_packs[index];
	}

	return pack;
}

DWORD DLCManager::getPackIndex(DLCPack *pack, bool &found, EDLCType type /*= e_DLCType_All*/)
{
	DWORD foundIndex = 0;
	found = false;
	if(pack == nullptr)
	{
		app.DebugPrintf("DLCManager: Attempting to find the index for a nullptr pack\n");
		return foundIndex;
	}
	if( type != e_DLCType_All )
	{
		DWORD index = 0;
		for( DLCPack *thisPack : m_packs )
		{
			if(thisPack->getDLCItemsCount(type)>0)
			{
				if(thisPack == pack)
				{
					found = true;
					foundIndex = index;
					break;
				}
				++index;
			}
		}
	}
	else
	{
		DWORD index = 0;
		for( DLCPack *thisPack : m_packs )
		{
			if(thisPack == pack)
			{
				found = true;
				foundIndex = index;
				break;
			}
			++index;
		}
	}
	return foundIndex;
}

DWORD DLCManager::getPackIndexContainingSkin(const wstring &path, bool &found)
{
	DWORD foundIndex = 0;
	found = false;
	DWORD index = 0;
	for( DLCPack *pack : m_packs )
	{
		if(pack->getDLCItemsCount(e_DLCType_Skin)>0)
		{
			if(pack->doesPackContainSkin(path))
			{
				foundIndex = index;
				found = true;
				break;
			}
			++index;
		}
	}
	return foundIndex;
}

DLCPack *DLCManager::getPackContainingSkin(const wstring &path)
{
	DLCPack *foundPack = nullptr;
	for( DLCPack *pack : m_packs )
	{
		if(pack->getDLCItemsCount(e_DLCType_Skin)>0)
		{
			if(pack->doesPackContainSkin(path))
			{
				foundPack = pack;
				break;
			}
		}
	}
	return foundPack;
}

DLCSkinFile *DLCManager::getSkinFile(const wstring &path)
{
	DLCSkinFile *foundSkinfile = nullptr;
	for( DLCPack *pack : m_packs )
	{
		foundSkinfile=pack->getSkinFile(path);
		if(foundSkinfile!=nullptr)
		{
			break;
		}
	}
	return foundSkinfile;
}

DWORD DLCManager::checkForCorruptDLCAndAlert(bool showMessage /*= true*/)
{
	DWORD corruptDLCCount = m_dwUnnamedCorruptDLCCount;	
	DLCPack *firstCorruptPack = nullptr;

	for( DLCPack *pack : m_packs )
	{
		if( pack->IsCorrupt() )
		{
			++corruptDLCCount;
			if(firstCorruptPack == nullptr) firstCorruptPack = pack;
		}
	}

	if(corruptDLCCount > 0 && showMessage)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		if(corruptDLCCount == 1 && firstCorruptPack != nullptr)
		{
			// pass in the pack format string
			WCHAR wchFormat[132];
			swprintf(wchFormat, 132, L"%ls\n\n%%ls", firstCorruptPack->getName().c_str());

			C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_CORRUPT_DLC_TITLE, IDS_CORRUPT_DLC, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr,wchFormat);

		}
		else
		{
			C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_CORRUPT_DLC_TITLE, IDS_CORRUPT_DLC_MULTIPLE, uiIDA,1,ProfileManager.GetPrimaryPad());
		}
	}

	SetNeedsCorruptCheck(false);

	return corruptDLCCount;
}

bool DLCManager::readDLCDataFile(DWORD &dwFilesProcessed, const wstring &path, DLCPack *pack, bool fromArchive)
{
	return readDLCDataFile( dwFilesProcessed, wstringtofilename(path), pack, fromArchive);
}


bool DLCManager::readDLCDataFile(DWORD &dwFilesProcessed, const string &path, DLCPack *pack, bool fromArchive)
{
	wstring wPath = convStringToWstring(path);
	if (fromArchive && app.getArchiveFileSize(wPath) >= 0)
	{
		byteArray bytes = app.getArchiveFile(wPath);
		return processDLCDataFile(dwFilesProcessed, bytes.data, bytes.length, pack);
	}
	else if (fromArchive) return false;

	wstring finalPathW = wPath;
#ifdef _WINDOWS64
	string finalPath = StorageManager.GetMountedPath(path.c_str());
	if(finalPath.size() == 0) finalPath = path;
	finalPathW = convStringToWstring(finalPath);
#elif defined(_DURANGO)
	wstring finalPath = StorageManager.GetMountedPath(wPath.c_str());
	if(finalPath.size() == 0) finalPath = wPath;
	finalPathW = finalPath;
#endif

	if(!fromArchive)
	{
		wstring folderPath;
		if(hasPckFolderFallback(finalPathW, folderPath))
		{
			wstring resolvedFolderPath = folderPath;
#ifdef _WINDOWS64
			const char *folderPathA = wstringtofilename(folderPath);
			string mountedFolderPath = StorageManager.GetMountedPath(folderPathA);
			if(mountedFolderPath.size() > 0)
			{
				resolvedFolderPath = convStringToWstring(mountedFolderPath);
			}
#elif defined(_DURANGO)
			wstring mountedFolderPath = StorageManager.GetMountedPath(folderPath.c_str());
			if(mountedFolderPath.size() > 0)
			{
				resolvedFolderPath = mountedFolderPath;
			}
#endif
			if(readDLCDataFolder(dwFilesProcessed, resolvedFolderPath, pack))
			{
				return true;
			}
		}
	}

#ifdef _WINDOWS64
	HANDLE file = CreateFile(finalPath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#elif defined(_DURANGO)
	HANDLE file = CreateFile(finalPath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		DWORD error = GetLastError();
		app.DebugPrintf("Failed to open DLC data file with error code %d (%x)\n", error, error);
		if( dwFilesProcessed == 0 ) removePack(pack);
		assert(false);
		return false;
	}

	DWORD bytesRead,dwFileSize = GetFileSize(file,nullptr);
	PBYTE pbData =  (PBYTE) new BYTE[dwFileSize];
	BOOL bSuccess = ReadFile(file,pbData,dwFileSize,&bytesRead,nullptr);
	if(bSuccess==FALSE)
	{
		// need to treat the file as corrupt, and flag it, so can't call fatal error
		//app.FatalLoadError();
	}
	else
	{
		CloseHandle(file);
	}
	if(bSuccess==FALSE)
	{
		// Corrupt or some other error. In any case treat as corrupt
		app.DebugPrintf("Failed to read %s from DLC content package\n", path.c_str());
		pack->SetIsCorrupt( true );
		SetNeedsCorruptCheck(true);
		return false;
	}
	return processDLCDataFile(dwFilesProcessed, pbData, bytesRead, pack);
}

bool DLCManager::readDLCDataFolder(DWORD &dwFilesProcessed, const wstring &path, DLCPack *pack)
{
	File folder(path);
	if(!folder.exists() || !folder.isDirectory())
	{
		return false;
	}

	FolderFile folderFile(path);
	vector<wstring> *fileList = folderFile.getFileList();
	if(fileList == nullptr || fileList->empty())
	{
		delete fileList;
		return false;
	}

	struct FolderEntry
	{
		wstring rawPath;
		wstring normalizedPath;
		int size;
	};
	vector<FolderEntry> entries;
	entries.reserve(fileList->size());

	unsigned int totalBytes = 0;
	for(const auto &rawPath : *fileList)
	{
		wstring normalized = replaceAll(rawPath, L"\\", L"/");
		if(normalized.empty() || normalized == L"0")
		{
			continue;
		}
		int size = folderFile.getFileSize(rawPath);
		if(size <= 0)
		{
			continue;
		}
		entries.push_back({ rawPath, normalized, size });
		totalBytes += static_cast<unsigned int>(size);
	}
	delete fileList;

	if(entries.empty() || totalBytes == 0)
	{
		return false;
	}

	PBYTE dataBuffer = new BYTE[totalBytes];
	pack->SetDataPointer(dataBuffer);

	unsigned int offset = 0;
	for(const auto &entry : entries)
	{
		byteArray data = folderFile.getFile(entry.rawPath);
		if(data.data == nullptr || data.length == 0)
		{
			continue;
		}

		DLCManager::EDLCType type = getFolderFileType(entry.normalizedPath);
		DLCFile *dlcFile = pack->addFile(type, entry.normalizedPath);
		if(dlcFile != nullptr)
		{
			memcpy(dataBuffer + offset, data.data, data.length);
			dlcFile->addData(dataBuffer + offset, data.length);
			offset += data.length;
			++dwFilesProcessed;
		}
		delete [] data.data;
	}

	return dwFilesProcessed > 0;
}

bool DLCManager::processDLCDataFile(DWORD &dwFilesProcessed, PBYTE pbData, DWORD dwLength, DLCPack *pack)
{
	if(pbData == nullptr || pack == nullptr || dwLength < sizeof(unsigned int) * 2)
	{
		return false;
	}

	const PBYTE pbEnd = pbData + dwLength;
	unordered_map<int, EDLCParameterType> parameterMapping;
	unsigned int uiCurrentByte = 0;

	// File format defined in the DLC_Creator
	// File format: Version 2
	// unsigned long, version number
	// unsigned long, t = number of parameter types
	// t * DLC_FILE_PARAM structs mapping strings to id's
	// unsigned long, n = number of files
	// n * DLC_FILE_DETAILS describing each file in the pack
	// n * files of the form
	// // unsigned long, p = number of parameters
	// // p * DLC_FILE_PARAM describing each parameter for this file
	// // ulFileSize bytes of data blob of the file added
	if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, sizeof(unsigned int)))
	{
		return false;
	}
	unsigned int uiVersion = readUInt32(pbData, false);
	uiCurrentByte += sizeof(unsigned int);

	bool bSwapEndian = false;
	unsigned int uiVersionSwapped = SwapInt32(uiVersion);
	if(uiVersion <= CURRENT_DLC_VERSION_NUM)
	{
		bSwapEndian = false;
	}
	else if(uiVersionSwapped <= CURRENT_DLC_VERSION_NUM)
	{
		bSwapEndian = true;
	}
	else
	{
		app.DebugPrintf("Unknown DLC version of %u\n", uiVersion);
		return false;
	}
	pack->SetDataPointer(pbData);

	if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, sizeof(unsigned int)))
	{
		return false;
	}
	unsigned int uiParameterCount = readUInt32(&pbData[uiCurrentByte], bSwapEndian);
	uiCurrentByte += sizeof(unsigned int);
	bool bXMLVersion = false;

	for(unsigned int i = 0; i < uiParameterCount; ++i)
	{
		if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, sizeof(C4JStorage::DLC_FILE_PARAM)))
		{
			return false;
		}

		C4JStorage::DLC_FILE_PARAM *pParams = reinterpret_cast<C4JStorage::DLC_FILE_PARAM *>(&pbData[uiCurrentByte]);
		unsigned int dwWchCount = bSwapEndian ? SwapInt32(pParams->dwWchCount) : pParams->dwWchCount;
		size_t paramSize = sizeof(C4JStorage::DLC_FILE_PARAM) + (dwWchCount * sizeof(WCHAR));
		if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, paramSize))
		{
			return false;
		}

		pParams->dwType = bSwapEndian ? SwapInt32(pParams->dwType) : pParams->dwType;
		pParams->dwWchCount = dwWchCount;
		char16_t* wchData = reinterpret_cast<char16_t*>(pParams->wchData);
		if (bSwapEndian)
		{
			SwapUTF16Bytes(wchData, dwWchCount);
		}

		wstring parameterName(reinterpret_cast<WCHAR *>(pParams->wchData), pParams->dwWchCount);
		EDLCParameterType type = getParameterType(parameterName);
		if(type != e_DLCParamType_Invalid)
		{
			parameterMapping[pParams->dwType] = type;
			if(type == e_DLCParamType_XMLVersion)
			{
				bXMLVersion = true;
			}
		}

		uiCurrentByte += static_cast<unsigned int>(paramSize);
	}

	if (bXMLVersion)
	{
		if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, sizeof(unsigned int)))
		{
			return false;
		}
		uiCurrentByte += sizeof(unsigned int);
	}

	if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, sizeof(unsigned int)))
	{
		return false;
	}
	unsigned int uiFileCount = readUInt32(&pbData[uiCurrentByte], bSwapEndian);
	uiCurrentByte += sizeof(unsigned int);

	if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, sizeof(C4JStorage::DLC_FILE_DETAILS)))
	{
		return false;
	}
	C4JStorage::DLC_FILE_DETAILS *pFile = reinterpret_cast<C4JStorage::DLC_FILE_DETAILS *>(&pbData[uiCurrentByte]);

	DWORD dwTemp = uiCurrentByte;
	for(unsigned int i = 0; i < uiFileCount; ++i)
	{
		if(!isValidDLCRange(pbData, dwLength, dwTemp, sizeof(C4JStorage::DLC_FILE_DETAILS)))
		{
			return false;
		}

		pFile = reinterpret_cast<C4JStorage::DLC_FILE_DETAILS *>(&pbData[dwTemp]);
		unsigned int dwWchCount = bSwapEndian ? SwapInt32(pFile->dwWchCount) : pFile->dwWchCount;
		size_t fileDetailsSize = sizeof(C4JStorage::DLC_FILE_DETAILS) + (dwWchCount * sizeof(WCHAR));
		if(!isValidDLCRange(pbData, dwLength, dwTemp, fileDetailsSize))
		{
			return false;
		}

		dwTemp += static_cast<DWORD>(fileDetailsSize);
	}

	if(!isValidDLCRange(pbData, dwLength, dwTemp, 0))
	{
		return false;
	}

	PBYTE pbTemp = pbData + dwTemp;
	pFile = reinterpret_cast<C4JStorage::DLC_FILE_DETAILS *>(&pbData[uiCurrentByte]);

	for(unsigned int i = 0; i < uiFileCount; ++i)
	{
		if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, sizeof(C4JStorage::DLC_FILE_DETAILS)))
		{
			return false;
		}

		pFile = reinterpret_cast<C4JStorage::DLC_FILE_DETAILS *>(&pbData[uiCurrentByte]);
		unsigned int dwWchCount = bSwapEndian ? SwapInt32(pFile->dwWchCount) : pFile->dwWchCount;
		unsigned int uiFileSize = bSwapEndian ? SwapInt32(pFile->uiFileSize) : pFile->uiFileSize;
		size_t fileHeaderSize = sizeof(C4JStorage::DLC_FILE_DETAILS) + (dwWchCount * sizeof(WCHAR));
		if(!isValidDLCRange(pbData, dwLength, uiCurrentByte, fileHeaderSize))
		{
			return false;
		}

		pFile->dwType = bSwapEndian ? SwapInt32(pFile->dwType) : pFile->dwType;
		pFile->uiFileSize = uiFileSize;
		char16_t* wchFile = reinterpret_cast<char16_t*>(pFile->wchFile);
		if (bSwapEndian)
		{
			SwapUTF16Bytes(wchFile, dwWchCount);
		}

		EDLCType type = static_cast<EDLCType>(pFile->dwType);

		DLCFile *dlcFile = nullptr;
		DLCPack *dlcTexturePack = nullptr;

		if(type == e_DLCType_TexturePack)
		{
			dlcTexturePack = new DLCPack(pack->getName(), pack->getLicenseMask());
		}
		else if(type != e_DLCType_PackConfig)
		{
			dlcFile = pack->addFile(type,(WCHAR *)pFile->wchFile);
		}

		if(!isValidDLCRange(pbData, dwLength, static_cast<unsigned int>(pbTemp - pbData), sizeof(unsigned int)))
		{
			return false;
		}
		uiParameterCount = readUInt32(pbTemp, bSwapEndian);
		pbTemp += sizeof(unsigned int);
		C4JStorage::DLC_FILE_PARAM *pParams = reinterpret_cast<C4JStorage::DLC_FILE_PARAM *>(pbTemp);
		for(unsigned int j = 0; j < uiParameterCount; ++j)
		{
			if(!isValidDLCRange(pbData, dwLength, static_cast<unsigned int>(pbTemp - pbData), sizeof(C4JStorage::DLC_FILE_PARAM)))
			{
				return false;
			}

			pParams = reinterpret_cast<C4JStorage::DLC_FILE_PARAM *>(pbTemp);
			unsigned int dwParamWchCount = bSwapEndian ? SwapInt32(pParams->dwWchCount) : pParams->dwWchCount;
			size_t paramSize = sizeof(C4JStorage::DLC_FILE_PARAM) + (dwParamWchCount * sizeof(WCHAR));
			if(!isValidDLCRange(pbData, dwLength, static_cast<unsigned int>(pbTemp - pbData), paramSize))
			{
				return false;
			}

			pParams->dwType = bSwapEndian ? SwapInt32(pParams->dwType) : pParams->dwType;
			pParams->dwWchCount = dwParamWchCount;
			char16_t* wchData = reinterpret_cast<char16_t*>(pParams->wchData);
			if (bSwapEndian)
			{
				SwapUTF16Bytes(wchData, dwParamWchCount);
			}

			auto it = parameterMapping.find(pParams->dwType);
			if(it != parameterMapping.end())
			{
				if(type == e_DLCType_PackConfig)
				{
					pack->addParameter(it->second,(WCHAR *)pParams->wchData);
				}
				else if(dlcFile != nullptr)
				{
					dlcFile->addParameter(it->second,(WCHAR *)pParams->wchData);
				}
				else if(dlcTexturePack != nullptr)
				{
					dlcTexturePack->addParameter(it->second, (WCHAR *)pParams->wchData);
				}
			}

			pbTemp += static_cast<unsigned int>(paramSize);
		}

		if(!isValidDLCRange(pbData, dwLength, static_cast<unsigned int>(pbTemp - pbData), uiFileSize))
		{
			return false;
		}

		if(dlcTexturePack != nullptr)
		{
			DWORD texturePackFilesProcessed = 0;
			bool validPack = processDLCDataFile(texturePackFilesProcessed, pbTemp, uiFileSize, dlcTexturePack);
			pack->SetDataPointer(nullptr); // If it's a child pack, it doesn't own the data
			if(!validPack || texturePackFilesProcessed == 0)
			{
				delete dlcTexturePack;
				dlcTexturePack = nullptr;
			}
			else
			{
				pack->addChildPack(dlcTexturePack);

				if(dlcTexturePack->getDLCItemsCount(e_DLCType_Texture) > 0)
				{
					Minecraft::GetInstance()->skins->addTexturePackFromDLC(dlcTexturePack, dlcTexturePack->GetPackId() );
				}
			}
			++dwFilesProcessed;
		}
		else if(dlcFile != nullptr)
		{
			dlcFile->addData(pbTemp, uiFileSize);

			// TODO - 4J Stu Remove the need for this vSkinNames vector, or manage it differently
			switch(pFile->dwType)
			{
			case e_DLCType_Skin:
				app.vSkinNames.push_back((WCHAR *)pFile->wchFile);
				break;
			}

			++dwFilesProcessed;
		}

		pbTemp += uiFileSize;
		uiCurrentByte += static_cast<unsigned int>(fileHeaderSize);
	}

	if(pack->getDLCItemsCount(e_DLCType_GameRules) > 0
		|| pack->getDLCItemsCount(e_DLCType_GameRulesHeader) > 0)
	{
		app.m_gameRules.loadGameRules(pack);
	}

	if(pack->getDLCItemsCount(e_DLCType_Audio) > 0)
	{
		//app.m_Audio.loadAudioDetails(pack);
	}
	// TODO Should be able to delete this data, but we can't yet due to how it is added to the Memory textures (MEM_file)

	return true;
}

DWORD DLCManager::retrievePackIDFromDLCDataFile(const string &path, DLCPack *pack)
{
	DWORD packId = 0;
	wstring wPath = convStringToWstring(path);

#ifdef _WINDOWS64
	string finalPath = StorageManager.GetMountedPath(path.c_str());
	if(finalPath.size() == 0) finalPath = path;
	HANDLE file = CreateFile(finalPath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#elif defined(_DURANGO)
	wstring finalPath = StorageManager.GetMountedPath(wPath.c_str());
	if(finalPath.size() == 0) finalPath = wPath;
	HANDLE file = CreateFile(finalPath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	DWORD bytesRead,dwFileSize = GetFileSize(file,nullptr);
	PBYTE pbData =  (PBYTE) new BYTE[dwFileSize];
	BOOL bSuccess = ReadFile(file,pbData,dwFileSize,&bytesRead,nullptr);
	if(bSuccess==FALSE)
	{
		// need to treat the file as corrupt, and flag it, so can't call fatal error
		//app.FatalLoadError();
	}
	else
	{
		CloseHandle(file);
	}
	if(bSuccess==FALSE)
	{
		// Corrupt or some other error. In any case treat as corrupt
		app.DebugPrintf("Failed to read %s from DLC content package\n", path.c_str());
		delete [] pbData;
		return 0;
	}
	packId=retrievePackID(pbData, bytesRead, pack);
	delete [] pbData;

	return packId;
}

DWORD DLCManager::retrievePackID(PBYTE pbData, DWORD dwLength, DLCPack *pack)
{
	DWORD packId=0;
	bool bPackIDSet=false;
	unordered_map<int, EDLCParameterType> parameterMapping;
	unsigned int uiCurrentByte=0;

	// File format defined in the DLC_Creator
	// File format: Version 2
	// unsigned long, version number
	// unsigned long, t = number of parameter types
	// t * DLC_FILE_PARAM structs mapping strings to id's
	// unsigned long, n = number of files
	// n * DLC_FILE_DETAILS describing each file in the pack
	// n * files of the form
	// // unsigned long, p = number of parameters
	// // p * DLC_FILE_PARAM describing each parameter for this file
	// // ulFileSize bytes of data blob of the file added
	unsigned int uiVersion=*(unsigned int *)pbData;
	uiCurrentByte+=sizeof(int);

	if(uiVersion < CURRENT_DLC_VERSION_NUM)
	{
		app.DebugPrintf("DLC version of %d is too old to be read\n", uiVersion);
		return 0;
	}
	pack->SetDataPointer(pbData);
	unsigned int uiParameterCount=*(unsigned int *)&pbData[uiCurrentByte];
	uiCurrentByte+=sizeof(int);
	C4JStorage::DLC_FILE_PARAM *pParams = (C4JStorage::DLC_FILE_PARAM *)&pbData[uiCurrentByte];
	for(unsigned int i=0;i<uiParameterCount;i++)
	{
		// Map DLC strings to application strings, then store the DLC index mapping to application index
		wstring parameterName(static_cast<WCHAR *>(pParams->wchData));
		EDLCParameterType type = getParameterType(parameterName);
		if( type != e_DLCParamType_Invalid )
		{
			parameterMapping[pParams->dwType] = type;
		}
		uiCurrentByte+= sizeof(C4JStorage::DLC_FILE_PARAM)+(pParams->dwWchCount*sizeof(WCHAR));
		pParams = (C4JStorage::DLC_FILE_PARAM *)&pbData[uiCurrentByte];
	}

	unsigned int uiFileCount=*(unsigned int *)&pbData[uiCurrentByte];
	uiCurrentByte+=sizeof(int);
	C4JStorage::DLC_FILE_DETAILS *pFile = (C4JStorage::DLC_FILE_DETAILS *)&pbData[uiCurrentByte];

	DWORD dwTemp=uiCurrentByte;
	for(unsigned int i=0;i<uiFileCount;i++)
	{
		dwTemp+=sizeof(C4JStorage::DLC_FILE_DETAILS)+pFile->dwWchCount*sizeof(WCHAR);
		pFile = (C4JStorage::DLC_FILE_DETAILS *)&pbData[dwTemp];
	}
	PBYTE pbTemp=((PBYTE )pFile);
	pFile = (C4JStorage::DLC_FILE_DETAILS *)&pbData[uiCurrentByte];

	for(unsigned int i=0;i<uiFileCount;i++)
	{
		EDLCType type = static_cast<EDLCType>(pFile->dwType);

		// Params
		uiParameterCount=*(unsigned int *)pbTemp;
		pbTemp+=sizeof(int);
		pParams = (C4JStorage::DLC_FILE_PARAM *)pbTemp;
		for(unsigned int j=0;j<uiParameterCount;j++)
		{
			auto it = parameterMapping.find(pParams->dwType);

			if(it != parameterMapping.end() )
			{
				if(type==e_DLCType_PackConfig)
				{
					if(it->second==e_DLCParamType_PackId)
					{				
						wstring wsTemp=static_cast<WCHAR *>(pParams->wchData);
						std::wstringstream ss;
						// 4J Stu - numbered using decimal to make it easier for artists/people to number manually
						ss << std::dec << wsTemp.c_str();
						ss >> packId;
						bPackIDSet=true;
						break;
					}
				}
			}
			pbTemp+=sizeof(C4JStorage::DLC_FILE_PARAM)+(sizeof(WCHAR)*pParams->dwWchCount);
			pParams = (C4JStorage::DLC_FILE_PARAM *)pbTemp;
		}

		if(bPackIDSet) break;
		// Move the pointer to the start of the next files data;
		pbTemp+=pFile->uiFileSize;
		uiCurrentByte+=sizeof(C4JStorage::DLC_FILE_DETAILS)+pFile->dwWchCount*sizeof(WCHAR);

		pFile=(C4JStorage::DLC_FILE_DETAILS *)&pbData[uiCurrentByte];
	}

	parameterMapping.clear();
	return packId;
}