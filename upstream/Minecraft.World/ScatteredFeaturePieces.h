#pragma once

#include "StructurePiece.h"

class ScatteredFeaturePieces
{
public:
	static void loadStatic();

private:
	class ScatteredFeaturePiece : public StructurePiece
	{
	protected:
		int width;
		int height;
		int depth;

		int heightPosition;

		ScatteredFeaturePiece();
		ScatteredFeaturePiece(Random *random, int west, int floor, int north, int width, int height, int depth);

		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);
		bool updateAverageGroundHeight(Level *level, BoundingBox *chunkBB, int offset);
	};

public:
	class DesertPyramidPiece : public ScatteredFeaturePiece
	{
	public:
		static StructurePiece *Create() { return new DesertPyramidPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_DesertPyramidPiece; }

	private:
		bool hasPlacedChest[4];

	public:
		DesertPyramidPiece();
		DesertPyramidPiece(Random *random, int west, int north);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);

		bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	};

	class JunglePyramidPiece : public ScatteredFeaturePiece
	{
	public:
		static StructurePiece *Create() { return new JunglePyramidPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_JunglePyramidPiece; }

	private:
		bool placedMainChest;
		bool placedHiddenChest;
		bool placedTrap1;
		bool placedTrap2;

	public:
		JunglePyramidPiece();
		JunglePyramidPiece(Random *random, int west, int north);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);

	public:
		bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	private:
		class MossStoneSelector : public BlockSelector
		{
		public:
			void next(Random *random, int worldX, int worldY, int worldZ, bool isEdge);
		};

		static MossStoneSelector stoneSelector;

	};

	class SwamplandHut : public ScatteredFeaturePiece
	{
	public:
		static StructurePiece *Create() { return new SwamplandHut(); }
		virtual EStructurePiece GetType() { return eStructurePiece_SwamplandHut; }

	private:
		bool spawnedWitch;

	public:
		SwamplandHut();
		SwamplandHut(Random *random, int west, int north);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);

	public:
		bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};
};