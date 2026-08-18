#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "TextureAndGeometryPacket.h"



TextureAndGeometryPacket::TextureAndGeometryPacket()
{
	this->textureName = L"";
	this->dwTextureBytes = 0;
	this->pbData = nullptr;
	this->dwBoxC = 0;
	this->dwOffsetC = 0;
	this->BoxDataA = nullptr;
	this->OffsetDataA = nullptr;
	uiAnimOverrideBitmask=0;
}

TextureAndGeometryPacket::~TextureAndGeometryPacket()
{
	// can't free these - they're used elsewhere
// 	if(this->BoxDataA!=nullptr)
// 	{
// 		delete [] this->BoxDataA;
// 	}
//
// 	if(this->pbData!=nullptr)
// 	{
// 		delete [] this->pbData;
// 	}
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const wstring &textureName, PBYTE pbData, DWORD dwBytes)
{
	this->textureName = textureName;

	wstring skinValue = textureName.substr(7,textureName.size());
	skinValue = skinValue.substr(0,skinValue.find_first_of(L'.'));
	std::wstringstream ss;
	ss << std::dec << skinValue.c_str();
	ss >> this->dwSkinID;
	this->dwSkinID = MAKE_SKIN_BITMASK(true, this->dwSkinID);
	this->pbData = pbData;
	this->dwTextureBytes = dwBytes;
	this->dwBoxC = 0;
	this->dwOffsetC = 0;
	this->BoxDataA=nullptr;
	this->OffsetDataA=nullptr;
	this->uiAnimOverrideBitmask=0;
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const wstring &textureName, PBYTE pbData, DWORD dwBytes, DLCSkinFile *pDLCSkinFile)
{
	this->textureName = textureName;

	wstring skinValue = textureName.substr(7,textureName.size());
	skinValue = skinValue.substr(0,skinValue.find_first_of(L'.'));
	std::wstringstream ss;
	ss << std::dec << skinValue.c_str();
	ss >> this->dwSkinID;
	this->dwSkinID = MAKE_SKIN_BITMASK(true, this->dwSkinID);

	this->pbData = pbData;
	this->dwTextureBytes = dwBytes;
	this->uiAnimOverrideBitmask = pDLCSkinFile->getAnimOverrideBitmask();
	this->dwBoxC = pDLCSkinFile->getAdditionalBoxesCount();
	this->dwOffsetC = pDLCSkinFile->getOffsetsCount();
	if(this->dwBoxC!=0)
	{
		this->BoxDataA= new SKIN_BOX [this->dwBoxC];
		vector<SKIN_BOX *> *pSkinBoxes=pDLCSkinFile->getAdditionalBoxes();
		int iCount=0;

		for(auto& pSkinBox : *pSkinBoxes)
		{
			this->BoxDataA[iCount++]=*pSkinBox;
		}
	}
	else
	{
		this->BoxDataA=nullptr;
	}
	if(this->dwOffsetC!=0)
	{
		this->OffsetDataA= new SKIN_OFFSET [this->dwOffsetC];
		vector<SKIN_OFFSET *> *pSkinOffsets=pDLCSkinFile->getOffsets();
		int iCount=0;

		for(auto& pSkinOffset : *pSkinOffsets)
		{
			this->OffsetDataA[iCount++]=*pSkinOffset;
		}
	}
	else
	{
		this->OffsetDataA=nullptr;
	}
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const wstring &textureName, PBYTE pbData, DWORD dwBytes, vector<SKIN_BOX *> *pvSkinBoxes, vector<SKIN_OFFSET *> *pvSkinOffsets, unsigned int uiAnimOverrideBitmask)
{
	this->textureName = textureName;

	wstring skinValue = textureName.substr(7,textureName.size());
	skinValue = skinValue.substr(0,skinValue.find_first_of(L'.'));
	std::wstringstream ss;
	ss << std::dec << skinValue.c_str();
	ss >> this->dwSkinID;
	this->dwSkinID = MAKE_SKIN_BITMASK(true, this->dwSkinID);

	this->pbData = pbData;
	this->dwTextureBytes = dwBytes;
	this->uiAnimOverrideBitmask = uiAnimOverrideBitmask;
	if(pvSkinBoxes==nullptr)
	{
		this->dwBoxC=0;
		this->BoxDataA=nullptr;
	}
	else
	{
		this->dwBoxC = static_cast<DWORD>(pvSkinBoxes->size());
		this->BoxDataA= new SKIN_BOX [this->dwBoxC];
		int iCount=0;

		for(auto& pSkinBox : *pvSkinBoxes)
		{
			this->BoxDataA[iCount++]=*pSkinBox;
		}
	}
	if(pvSkinOffsets==nullptr)
	{
		this->dwOffsetC=0;
		this->OffsetDataA=nullptr;
	}
	else
	{
		this->dwOffsetC = static_cast<DWORD>(pvSkinOffsets->size());
		this->OffsetDataA= new SKIN_OFFSET [this->dwOffsetC];
		int iCount=0;

		for(auto& pSkinOffset : *pvSkinOffsets)
		{
			this->OffsetDataA[iCount++]=*pSkinOffset;
		}
	}

}

void TextureAndGeometryPacket::handle(PacketListener *listener)
{
	listener->handleTextureAndGeometry(shared_from_this());
}

void TextureAndGeometryPacket::read(DataInputStream *dis) //throws IOException
{
	textureName = dis->readUTF();
	dwSkinID = static_cast<DWORD>(dis->readInt());

    short rawTextureBytes = dis->readShort();
    if (rawTextureBytes <= 0)
    {
        dwTextureBytes = 0;
    }
    else
    {
        dwTextureBytes = (DWORD)(unsigned short)rawTextureBytes;
        if (dwTextureBytes > 65536)
        {
            dwTextureBytes = 0;
        }
    }

	if(dwTextureBytes>0)
	{
		this->pbData= new BYTE [dwTextureBytes];

		for(DWORD i=0;i<dwTextureBytes;i++)
		{
			this->pbData[i] = dis->readByte();
		}
	}
	uiAnimOverrideBitmask = dis->readInt();

	short rawBoxC = dis->readShort();
    if (rawBoxC <= 0)
    {
        dwBoxC = 0;
    }
    else
    {
        dwBoxC = (DWORD)(unsigned short)rawBoxC;
        if (dwBoxC > 256)
        {
            dwBoxC = 0; // sane limit for skin boxes
        }
    }
	

	if(dwBoxC>0)
	{
		this->BoxDataA= new SKIN_BOX [dwBoxC];
	}

	for(DWORD i=0;i<dwBoxC;i++)
	{
		this->BoxDataA[i].ePart = static_cast<eBodyPart>(dis->readShort());
		this->BoxDataA[i].fX = dis->readFloat();
		this->BoxDataA[i].fY = dis->readFloat();
		this->BoxDataA[i].fZ = dis->readFloat();
		this->BoxDataA[i].fH = dis->readFloat();
		this->BoxDataA[i].fW = dis->readFloat();
		this->BoxDataA[i].fD = dis->readFloat();
		this->BoxDataA[i].fU = dis->readFloat();
		this->BoxDataA[i].fV = dis->readFloat();
		this->BoxDataA[i].fA = dis->readFloat();
		this->BoxDataA[i].fM = dis->readFloat();
		this->BoxDataA[i].fS = dis->readFloat();
	}

	short rawOffsetC = dis->readShort();

	if (rawOffsetC <= 0)
	{
		dwOffsetC = 0;
	}
	else
	{
		dwOffsetC = (DWORD)(unsigned short)rawOffsetC;
		if (dwOffsetC > 256)
		{
			dwOffsetC = 0; // sane limit for skin offsets
		}
	}

	if (dwOffsetC > 0)
	{
		this->OffsetDataA = new SKIN_OFFSET[dwOffsetC];
	}

	for(DWORD i=0;i<dwOffsetC;i++)
	{
		this->OffsetDataA[i].ePart = static_cast<eBodyOffset>(dis->readShort());
		this->OffsetDataA[i].fD = dis->readFloat();
		this->OffsetDataA[i].fO = dis->readFloat();
	}
}

void __fastcall TextureAndGeometryPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeUTF(textureName);
	dos->writeInt(dwSkinID);
	dos->writeShort(static_cast<short>(dwTextureBytes));
	for(DWORD i=0;i<dwTextureBytes;i++)
	{
		dos->writeByte(this->pbData[i]);
	}
	dos->writeInt(uiAnimOverrideBitmask);

	dos->writeShort(static_cast<short>(dwBoxC));
	for(DWORD i=0;i<dwBoxC;i++)
	{
		dos->writeShort(static_cast<short>(this->BoxDataA[i].ePart));
		dos->writeFloat(this->BoxDataA[i].fX);
		dos->writeFloat(this->BoxDataA[i].fY);
		dos->writeFloat(this->BoxDataA[i].fZ);
		dos->writeFloat(this->BoxDataA[i].fH);
		dos->writeFloat(this->BoxDataA[i].fW);
		dos->writeFloat(this->BoxDataA[i].fD);
		dos->writeFloat(this->BoxDataA[i].fU);
		dos->writeFloat(this->BoxDataA[i].fV);
		dos->writeFloat(this->BoxDataA[i].fA);
		dos->writeFloat(this->BoxDataA[i].fM);
		dos->writeFloat(this->BoxDataA[i].fS);
	}

	dos->writeShort(static_cast<short>(dwOffsetC));
	for(DWORD i=0;i<dwOffsetC;i++)
	{
		dos->writeShort(static_cast<short>(this->OffsetDataA[i].ePart));
		dos->writeFloat(this->OffsetDataA[i].fD);
		dos->writeFloat(this->OffsetDataA[i].fO);
	}
}

int TextureAndGeometryPacket::getEstimatedSize()
{
	return 4096+ +sizeof(int) + sizeof(float)*8*4;
}
