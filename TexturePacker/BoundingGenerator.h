#ifndef _BOUNDINGGENERATOR_H_
#define _BOUNDINGGENERATOR_H_

#include "TextureSpaceArranger.h"
#include "MyPngWriter.h"

class BoundingGenerator {

public:

	BoundingGenerator();
	
	~BoundingGenerator();
	
	SpriteInfo GenerateMoreCompactBounding(const std::string& spriteTextureFilePath);

private:

	void TryCutCorner(int cornerNo);

    void FindBoundingPixels();

    bool HasValidPixelAt(int x, int y);

    // bool Left(int x0, int y0, int x1, int y1, int xp, int yp);

	//   These points will make the corresponding cut invalid,
	//   with the first cutting-segment facing up, the second one facing down.
	//      ____       ______
	//    ./    |     |      |
	//    /     |      \     |
	//   |______|      .\____|
    bool IsCutLineValid(int fx, int fy, int tx, int ty, bool faceUp);

private:

    const static int MIN_AREA_TO_CUT;

    SpriteInfo mSpriteInfo;

    int mTop, mLeft, mRight, mBottom;

    std::vector<int> mTopMostInCol, mBottomMostInCol;

    MyPngWriter* mPngFile;
};

#endif