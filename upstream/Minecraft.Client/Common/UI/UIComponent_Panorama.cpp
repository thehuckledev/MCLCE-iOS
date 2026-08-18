#include "stdafx.h"
#include "UI.h"
#include "UIComponent_Panorama.h"
#include "Minecraft.h"
#include "MultiPlayerLevel.h"
#include "../../../Minecraft.World/net.minecraft.world.level.dimension.h"
#include "../../../Minecraft.World/net.minecraft.world.level.storage.h"
#include "Tesselator.h"
#include "TexturePackRepository.h"
#include "TexturePack.h"
#include "Textures.h"
#include "BufferedImage.h"
#include "../GameRules/LevelGenerationOptions.h"
#include <fstream>
#include <direct.h>
#include <cmath>

#include "../../../Xbox/XML/ATGXmlParser.h"

static const wchar_t *PANORAMA_TEXTURE_RELPATH = L"Graphics/ControlType/Panorama/";

// default settings
static const float kDefaultPanoramaScrollSpeed = 0.0001f / 16.6667f;
static const float kDefaultPanoramaBlurSigma = 0.5f;

struct PanoramaConfig
{
	float scrollSpeed = kDefaultPanoramaScrollSpeed;
	float blurSigma = kDefaultPanoramaBlurSigma;

	// grid mode (classic panorama)
	bool gridEnabled = false;
	float gridSizeX = 1.0f;
	float gridSizeY = 1.0f;

	// nearest vs bilinear
	bool gridScalingNearest = false;
};

// atg xml parser my beloved
class PanoramaXmlCallback : public ATG::ISAXCallback
{
public:
	PanoramaConfig config;

	HRESULT StartDocument() override { return S_OK; }
	HRESULT EndDocument() override { return S_OK; }

	HRESULT ElementBegin(CONST WCHAR *strName, UINT NameLen, CONST ATG::XMLAttribute * /*pAttributes*/, UINT /*NumAttributes*/) override
	{
		wstring elementName(strName, NameLen);
		if(elementName == L"grid")
		{
			// grid switch
			config.gridEnabled = true;
		}
		m_elementStack.push_back(elementName);
		m_currentText.clear();
		return S_OK;
	}

	HRESULT ElementContent(CONST WCHAR *strData, UINT DataLen, BOOL More) override
	{
		m_currentText.append(strData, DataLen);
		if(!More)
		{
			ApplyCurrentElement();
			m_currentText.clear();
		}
		return S_OK;
	}

	HRESULT ElementEnd(CONST WCHAR * /*strName*/, UINT /*NameLen*/) override
	{
		ApplyCurrentElement();
		m_currentText.clear();
		if(!m_elementStack.empty())
		{
			m_elementStack.pop_back();
		}
		return S_OK;
	}

	HRESULT CDATABegin() override { return S_OK; }
	HRESULT CDATAData(CONST WCHAR * /*strCDATA*/, UINT /*CDATALen*/, BOOL /*bMore*/) override { return S_OK; }
	HRESULT CDATAEnd() override { return S_OK; }

	VOID Error(HRESULT /*hError*/, CONST CHAR * /*strMessage*/) override { }

private:
	void ApplyCurrentElement()
	{
		if(m_currentText.empty() || m_elementStack.empty()) return;

		const wstring &name = m_elementStack.back();
		const bool insideGrid = (m_elementStack.size() >= 2 &&
			m_elementStack[m_elementStack.size() - 2] == L"grid");

		if(name == L"panoramaSpeed")
		{
			config.scrollSpeed = static_cast<float>(wcstod(m_currentText.c_str(), nullptr));
		}
		else if(name == L"panoramaBlur")
		{
			config.blurSigma = static_cast<float>(wcstod(m_currentText.c_str(), nullptr));
		}
		else if(insideGrid && name == L"sizeX")
		{
			config.gridSizeX = static_cast<float>(wcstod(m_currentText.c_str(), nullptr));
		}
		else if(insideGrid && name == L"sizeY")
		{
			config.gridSizeY = static_cast<float>(wcstod(m_currentText.c_str(), nullptr));
		}
		else if(insideGrid && name == L"scaling")
		{
			config.gridScalingNearest = (_wcsicmp(m_currentText.c_str(), L"nearest") == 0);
		}
	}

	vector<wstring> m_elementStack;
	wstring m_currentText;
};

// load custom panorama data (if any) and return its settings
static PanoramaConfig LoadPanoramaConfig(const wstring &root)
{
	PanoramaConfig config;

	wstring xmlPath = root + L"panorama.xml";
	if(GetFileAttributesW(xmlPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return config;
	}

	const char *nativePath = wstringtofilename(xmlPath);

	PanoramaXmlCallback callback;
	ATG::XMLParser parser;
	parser.RegisterSAXCallbackInterface(&callback);

	HRESULT hr = parser.ParseXMLFile(nativePath);
	if(FAILED(hr))
	{
		printf("Failed to parse panorama.xml at %ls (hr=0x%08X)\n", xmlPath.c_str(), hr);
		return PanoramaConfig();
	}

	if(callback.config.gridSizeX <= 0.0f) callback.config.gridSizeX = 1.0f;
	if(callback.config.gridSizeY <= 0.0f) callback.config.gridSizeY = 1.0f;

	printf("Loaded panorama.xml overrides from %ls (speed=%f, blur=%f, grid=%s %fx%f, scaling=%s)\n",
		xmlPath.c_str(), callback.config.scrollSpeed, callback.config.blurSigma,
		callback.config.gridEnabled ? "on" : "off",
		callback.config.gridSizeX, callback.config.gridSizeY,
		callback.config.gridScalingNearest ? "nearest" : "linear");

	return callback.config;
}

static wstring GetPanoramaTexturePath()
{
	const wchar_t *envOverride = _wgetenv(L"PANORAMA_TEXTURE_PATH");
	if(envOverride && envOverride[0] != L'\0')
	{
		wstring path(envOverride);
		if(!path.empty() && path.back() != L'/' && path.back() != L'\\')
			path += L'/';
		return path;
	}

	if(app.getLevelGenerationOptions() != nullptr)
	{
		LevelGenerationOptions *levelGen = app.getLevelGenerationOptions();
		DLCPack *parentPack = levelGen->getParentDLCPack();
		if(parentPack != nullptr)
		{
			wstring packName = parentPack->getName();
			if(!packName.empty())
			{
				wchar_t buf[MAX_PATH];
				if(_wgetcwd(buf, MAX_PATH))
				{
					wstring path(buf);
					path += L"\\Windows64Media\\DLC\\";
					path += packName;
					path += L"\\Data\\ControlType\\Panorama\\";
					wstring sampleDay = path + L"Panorama_S.png";
					wstring sampleNight = path + L"Panorama_N.png";
					if(GetFileAttributesW(sampleDay.c_str()) != INVALID_FILE_ATTRIBUTES ||
					   GetFileAttributesW(sampleNight.c_str()) != INVALID_FILE_ATTRIBUTES)
					{
						return path;
					}
				}
			}
		}
	}

	const char *mountedPanoramaRoots[] =
	{
		"TPACK:/Data/ControlType/Panorama/",
		"WPACK:/Data/ControlType/Panorama/",
		"TPACK:Data/ControlType/Panorama/",
		"WPACK:Data/ControlType/Panorama/",
	};

	for(const char *mountedRoot : mountedPanoramaRoots)
	{
		wstring mountedPath = convStringToWstring(StorageManager.GetMountedPath(mountedRoot));
		if(mountedPath.empty()) continue;

		if(mountedPath.find(L"Data/ControlType/Panorama/") != wstring::npos)
		{
			// slash check cause for some reason its not always there
			size_t pos = mountedPath.find(L"Data/ControlType/Panorama/");
			if(pos != wstring::npos && pos > 0 && mountedPath[pos-1] != L'/' && mountedPath[pos-1] != L'\\')
			{
				mountedPath.insert(pos, 1, L'/');
			}
		}

		wstring sampleDay = mountedPath + L"Panorama_S.png";
		wstring sampleNight = mountedPath + L"Panorama_N.png";
		if(GetFileAttributesW(sampleDay.c_str()) != INVALID_FILE_ATTRIBUTES ||
		   GetFileAttributesW(sampleNight.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			if(!mountedPath.empty() && mountedPath.back() != L'/' && mountedPath.back() != L'\\')
				mountedPath += L'/';
			return mountedPath;
		}
	}

	wchar_t buf[MAX_PATH];
	if(_wgetcwd(buf, MAX_PATH))
	{
		wstring path(buf);
		path += L"\\Common\\Media\\MediaWindows64\\Graphics\\ControlType\\Panorama\\";
		return path;
	}

	// fallback
	return PANORAMA_TEXTURE_RELPATH;
}

static wstring GetPanoramaTexturePathKey()
{
	wstring key = GetPanoramaTexturePath();

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft != nullptr && pMinecraft->skins != nullptr)
	{
		TexturePack *selected = pMinecraft->skins->getSelected();
		if(selected != nullptr)
		{
			wchar_t buffer[32];
			swprintf(buffer, 32, L"|TPACKID=%u", static_cast<unsigned int>(selected->getId()));
			key += buffer;
		}
	}

	return key;
}

// opengl time :v
static int LoadPanoramaQuadTexture(const wstring &fileName, float blurSigma, int &outWidth, int &outHeight)
{
	wstring filePath = GetPanoramaTexturePath() + fileName;
	const char *nativePath = wstringtofilename(filePath);
	std::ifstream stream(nativePath, std::ios::binary);
	if(!stream)
	{
		printf("Failed to open panorama quad texture %ls\n", filePath.c_str());
		return -1;
	}

	stream.seekg(0, std::ios::end);
	std::streamoff size = stream.tellg();
	if(size <= 0)
	{
		printf("Empty panorama quad texture %ls\n", filePath.c_str());
		return -1;
	}
	stream.seekg(0, std::ios::beg);

	BYTE *data = new BYTE[static_cast<size_t>(size)];
	if(!stream.read(reinterpret_cast<char *>(data), size))
	{
		delete [] data;
		printf("Failed to read panorama quad texture %ls\n", filePath.c_str());
		return -1;
	}

	BufferedImage image(data, static_cast<DWORD>(size));
	delete [] data;

	if(image.getData() == nullptr)
	{
		printf("Failed to decode panorama quad texture %ls\n", filePath.c_str());
		return -1;
	}

	// gaussian blur
	const float sigma = blurSigma;
	const int width = image.getWidth();
	const int height = image.getHeight();

	if(sigma <= 0.0f)
	{
		outWidth = width;
		outHeight = height;

		int idNoBlur = Minecraft::GetInstance()->textures->getTexture(&image, C4JRender::TEXTURE_FORMAT_RxGyBzAw, false);
		printf("Loaded panorama background quad '%ls' (no blur) -> texture id %d\n", filePath.c_str(), idNoBlur);
		return idNoBlur;
	}

	const int radius = static_cast<int>(ceilf(3.0f * sigma));
	const int ksize = radius * 2 + 1;
	vector<float> kernel(ksize);
	float sum = 0.0f;
	for(int i = -radius; i <= radius; ++i)
	{
		float v = expf(-(i*i) / (2.0f * sigma * sigma));
		kernel[i + radius] = v;
		sum += v;
	}
	for(int i = 0; i < ksize; ++i) kernel[i] /= sum;

	float *tmpA = new float[width * height];
	float *tmpR = new float[width * height];
	float *tmpG = new float[width * height];
	float *tmpB = new float[width * height];

	int *sourcePixels = image.getData();

	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x)
		{
			float a = 0, r = 0, g = 0, b = 0;
			for(int k = -radius; k <= radius; ++k)
			{
				// horizontal wrap
				int sx = x + k;
				while(sx < 0) sx += width;
				while(sx >= width) sx -= width;
				int p = sourcePixels[sx + y * width];
				float w = kernel[k + radius];
				a += w * static_cast<float>((p >> 24) & 0xff);
				r += w * static_cast<float>((p >> 16) & 0xff);
				g += w * static_cast<float>((p >> 8) & 0xff);
				b += w * static_cast<float>(p & 0xff);
			}
			int idx = x + y * width;
			tmpA[idx] = a;
			tmpR[idx] = r;
			tmpG[idx] = g;
			tmpB[idx] = b;
		}
	}

	// vertical clamp
	BufferedImage blurredImage(width, height, BufferedImage::TYPE_INT_ARGB);
	int *targetPixels = blurredImage.getData();
	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x)
		{
			float a = 0, r = 0, g = 0, b = 0;
			for(int k = -radius; k <= radius; ++k)
			{
				int sy = y + k;
				if(sy < 0) sy = 0;
				if(sy >= height) sy = height - 1;
				int idx = x + sy * width;
				float w = kernel[k + radius];
				a += w * tmpA[idx];
				r += w * tmpR[idx];
				g += w * tmpG[idx];
				b += w * tmpB[idx];
			}
			int ia = static_cast<int>(a + 0.5f);
			int ir = static_cast<int>(r + 0.5f);
			int ig = static_cast<int>(g + 0.5f);
			int ib = static_cast<int>(b + 0.5f);
			if(ia < 0) ia = 0; if(ia > 255) ia = 255;
			if(ir < 0) ir = 0; if(ir > 255) ir = 255;
			if(ig < 0) ig = 0; if(ig > 255) ig = 255;
			if(ib < 0) ib = 0; if(ib > 255) ib = 255;
			targetPixels[x + y * width] = (ia << 24) | (ir << 16) | (ig << 8) | ib;
		}
	}

	delete [] tmpA;
	delete [] tmpR;
	delete [] tmpG;
	delete [] tmpB;

	outWidth = image.getWidth();
	outHeight = image.getHeight();

	int id = Minecraft::GetInstance()->textures->getTexture(&blurredImage, C4JRender::TEXTURE_FORMAT_RxGyBzAw, false);
	printf("Loaded panorama background quad '%ls' (blur sigma=%f) -> texture id %d\n", filePath.c_str(), sigma, id);
	return id;
}

// original behavior
static void DrawScrollingPanoramaQuads(S32 width, S32 height, float panoramaAspect, float panoramaScroll, Tesselator *t)
{
	float sourceWidth = static_cast<float>(height) * panoramaAspect;
	float scroll = panoramaScroll - floorf(panoramaScroll);
	float scrollPx = scroll * sourceWidth;
	float startX = -scrollPx;
	const float overscanY = (static_cast<float>(height) * 0.03f > 8.0f) ? (static_cast<float>(height) * 0.03f) : 8.0f;
	const float drawTop = -overscanY;
	const float drawBottom = static_cast<float>(height) + overscanY;

	t->begin();
	for(float x = startX - sourceWidth; x < static_cast<float>(width) + sourceWidth; x += sourceWidth)
	{
		float left = x;
		float right = x + sourceWidth;
		t->vertexUV(left,  drawBottom, 0.0f, 0.0f, 1.0f);
		t->vertexUV(right, drawBottom, 0.0f, 1.0f, 1.0f);
		t->vertexUV(right, drawTop,    0.0f, 1.0f, 0.0f);
		t->vertexUV(left,  drawTop,    0.0f, 0.0f, 0.0f);
	}
	t->end();
}

// grid mode (classic panorama)
static void DrawGridPanoramaQuads(S32 width, S32 height, float tileWidth, float tileHeight, float panoramaScroll, Tesselator *t)
{
	if(tileWidth <= 0.0f) tileWidth = static_cast<float>(width);
	if(tileHeight <= 0.0f) tileHeight = static_cast<float>(height);

	const float scroll = panoramaScroll - floorf(panoramaScroll);
	const float scrollPx = scroll * tileWidth;
	const float startX = -scrollPx;

	t->begin();
	for(float y = -tileHeight; y < static_cast<float>(height) + tileHeight; y += tileHeight)
	{
		const float top = y;
		const float bottom = y + tileHeight;
		for(float x = startX - tileWidth; x < static_cast<float>(width) + tileWidth; x += tileWidth)
		{
			const float left = x;
			const float right = x + tileWidth;

			t->vertexUV(left,  bottom, 0.0f, 0.0f, 1.0f);
			t->vertexUV(right, bottom, 0.0f, 1.0f, 1.0f);
			t->vertexUV(right, top,    0.0f, 1.0f, 0.0f);
			t->vertexUV(left,  top,    0.0f, 0.0f, 0.0f);
		}
	}
	t->end();
}

UIComponent_Panorama::UIComponent_Panorama(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	m_bShowingDay = true;
	tick();
}

wstring UIComponent_Panorama::getMoviePath()
{ 
	return L"";
}

void UIComponent_Panorama::tick()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	bool isDay = true;
	EnterCriticalSection(&pMinecraft->m_setLevelCS);
	if(pMinecraft->level!=nullptr)
	{
		int64_t i64TimeOfDay =0;
		// are we in the Nether? - Leave the time as 0 if we are, so we show daylight
		if(pMinecraft->level->dimension->id==0)
		{
			i64TimeOfDay = pMinecraft->level->getLevelData()->getGameTime() % 24000;
			isDay = (i64TimeOfDay <= 14000);
		}

		setPanorama(isDay);
	}
	else
	{
		setPanorama(true);
	}

	// fix: tick rate affected by user framerate
	const DWORD nowMs = GetTickCount();
	if(m_lastScrollTickMs != 0)
	{
		const DWORD kMaxElapsedMs = 250;
		DWORD elapsedMs = nowMs - m_lastScrollTickMs;

		if(elapsedMs > kMaxElapsedMs) elapsedMs = kMaxElapsedMs;
		m_panoramaScroll += m_panoramaScrollSpeed * static_cast<float>(elapsedMs);
	}
	m_lastScrollTickMs = nowMs;

	LeaveCriticalSection(&pMinecraft->m_setLevelCS);

	m_hasTickedOnce = true;
}

void UIComponent_Panorama::EnsurePanoramaTexturesLoaded()
{
	wstring currentRoot = GetPanoramaTexturePath();
	wstring currentRootKey = GetPanoramaTexturePathKey();
	if(m_bPanoramaTexturesLoaded && currentRoot == m_panoramaTextureRoot && currentRootKey == m_panoramaTextureRootKey) return;

	if(m_texPanoramaDay >= 0)
	{
		Minecraft::GetInstance()->textures->releaseTexture(m_texPanoramaDay);
		m_texPanoramaDay = -1;
	}
	if(m_texPanoramaNight >= 0)
	{
		Minecraft::GetInstance()->textures->releaseTexture(m_texPanoramaNight);
		m_texPanoramaNight = -1;
	}

	m_panoramaTextureRoot = currentRoot;
	m_panoramaTextureRootKey = currentRootKey;
	m_bPanoramaTexturesLoaded = false;

	// search for panorama.xml
	PanoramaConfig config = LoadPanoramaConfig(currentRoot);
	m_panoramaScrollSpeed = config.scrollSpeed;
	m_panoramaBlurSigma = config.blurSigma;
	m_panoramaGridEnabled = config.gridEnabled;
	m_panoramaGridSizeX = config.gridSizeX;
	m_panoramaGridSizeY = config.gridSizeY;
	m_panoramaGridScalingNearest = config.gridScalingNearest;

	int dayWidth = 0;
	int dayHeight = 0;
	m_texPanoramaDay   = LoadPanoramaQuadTexture(L"Panorama_S.png", m_panoramaBlurSigma, dayWidth, dayHeight);
	int nightWidth = 0;
	int nightHeight = 0;
	m_texPanoramaNight = LoadPanoramaQuadTexture(L"Panorama_N.png", m_panoramaBlurSigma, nightWidth, nightHeight);
	if(dayWidth > 0 && dayHeight > 0)
	{
		m_panoramaAspect = static_cast<float>(dayWidth) / static_cast<float>(dayHeight);
		m_panoramaNativeWidth = dayWidth;
		m_panoramaNativeHeight = dayHeight;
	}
	else if(nightWidth > 0 && nightHeight > 0)
	{
		m_panoramaAspect = static_cast<float>(nightWidth) / static_cast<float>(nightHeight);
		m_panoramaNativeWidth = nightWidth;
		m_panoramaNativeHeight = nightHeight;
	}
	m_bPanoramaTexturesLoaded = (m_texPanoramaDay >= 0 || m_texPanoramaNight >= 0);
}

void UIComponent_Panorama::DrawPanoramaBackgroundQuad(S32 width, S32 height)
{
	EnsurePanoramaTexturesLoaded();

	int texId = m_bShowingDay ? m_texPanoramaDay : m_texPanoramaNight;
	if(texId < 0) return; // failed

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	Minecraft::GetInstance()->textures->bind(texId);

	// nearest should only be used in grid mode, i dont necessarily see a purpose for it otherwise
	const int panoramaFilter = (m_panoramaGridEnabled && m_panoramaGridScalingNearest) ? GL_NEAREST : GL_LINEAR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, panoramaFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, panoramaFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	Tesselator *t = Tesselator::getInstance();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if(m_panoramaGridEnabled)
	{
		const float tileWidth = static_cast<float>(m_panoramaNativeWidth) * m_panoramaGridSizeX;
		const float tileHeight = static_cast<float>(m_panoramaNativeHeight) * m_panoramaGridSizeY;
		DrawGridPanoramaQuads(width, height, tileWidth, tileHeight, m_panoramaScroll, t);
	}
	else
	{
		DrawScrollingPanoramaQuads(width, height, m_panoramaAspect, m_panoramaScroll, t);
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void UIComponent_Panorama::render(S32 width, S32 height, C4JRender::eViewportType viewport)
{
	bool specialViewport =	(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_TOP) ||
		(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM) ||
		(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_LEFT) ||
		(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT);
	if(specialViewport)
	{
		S32 xPos = 0;
		S32 yPos = 0;
		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			yPos = static_cast<S32>(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			xPos = static_cast<S32>(ui.getScreenWidth() / 2);
			break;
		}
		ui.setupRenderPosition(xPos, yPos);

		S32 tileXStart = 0;
		S32 tileYStart = 0;
		S32 tileWidth = width;
		S32 tileHeight = height;

		if((viewport == C4JRender::VIEWPORT_TYPE_SPLIT_LEFT) || (viewport == C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT))
		{
			tileHeight = static_cast<S32>(ui.getScreenHeight());
		}
		else
		{
			tileWidth = static_cast<S32>(ui.getScreenWidth());
			tileYStart = static_cast<S32>(ui.getScreenHeight() / 2);
		}

		m_renderWidth = tileWidth;
		m_renderHeight = tileHeight;

		DrawPanoramaBackgroundQuad(tileWidth, tileHeight);
	}
	else
	{
		ui.setupRenderPosition(0, 0);
		m_renderWidth = static_cast<S32>(ui.getScreenWidth());
		m_renderHeight = static_cast<S32>(ui.getScreenHeight());
		DrawPanoramaBackgroundQuad(static_cast<S32>(ui.getScreenWidth()), static_cast<S32>(ui.getScreenHeight()));
	}
}

void UIComponent_Panorama::setPanorama(bool isDay)
{
	if(isDay != m_bShowingDay)
	{
		m_bShowingDay = isDay;
	}
}