#pragma once
using namespace std;

#include "TileEntity.h"
#include <vector>

struct BannerPattern
{
	wstring pattern;
	int color;
};

class BannerTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_BANNERTILEENTITY; }
	static TileEntity *create() { return new BannerTileEntity(); }

private:
	int baseColor;
	bool m_isWall;
	vector<BannerPattern> patterns;

public:
	BannerTileEntity();
	virtual ~BannerTileEntity() {}

	int getBaseColor() const { return baseColor; }
	void setBaseColor(int color) { baseColor = color; }

	bool isWall() const { return m_isWall; }
	void setIsWall(bool wall) { m_isWall = wall; }

	const vector<BannerPattern> &getPatterns() const { return patterns; }
	void addPattern(const wstring &pattern, int color) { patterns.push_back({ pattern, color }); }
	void clearPatterns() { patterns.clear(); }

	virtual void save(CompoundTag *tag);
	virtual void load(CompoundTag *tag);
	virtual shared_ptr<Packet> getUpdatePacket();
	virtual shared_ptr<TileEntity> clone();
};
