#include "BoundingGenerator.h"
#include "MyPngWriter.h"
#include <cassert>
#include "GeoUtil.h"

const int BoundingGenerator::MIN_AREA_TO_CUT = 3500;

BoundingGenerator::BoundingGenerator()
{
}

BoundingGenerator::~BoundingGenerator()
{
	mPngFile->close();
}

SpriteInfo BoundingGenerator::GenerateMoreCompactBounding(const std::string& spriteTextureFilePath)
{
	int w, h;

	mPngFile = new MyPngWriter(1, 1, 0, spriteTextureFilePath.c_str());
    mPngFile->readfromfile(spriteTextureFilePath.c_str());

    w = mPngFile->getwidth();
	h = mPngFile->getheight();

	mTop = 0;
	mLeft = 0;
	mRight = w - 1; 
	mBottom = h - 1;
    
    FindBoundingPixels();
    
    mSpriteInfo.shapeMask = 0;

	for (int corner = 0; corner < 4; ++corner)
	{
		TryCutCorner(corner);
	}

    return mSpriteInfo;
}

inline void MakeSegmentDirectionRight(int &x0, int &y0, int &x1, int &y1, 
	int outer, int outerFixed, int inner, int innerFixed, int corner)
{
	switch(corner)
	{
	case TOPLEFT_CORNER:
	case BOTTOMRIGHT_CORNER:
		x0 = outer;
		y0 = outerFixed;
		x1 = innerFixed;
		y1 = inner;
		break;

	case BOTTOMLEFT_CORNER:
	case TOPRIGHT_CORNER:
		x0 = outerFixed;
		y0 = outer;
		x1 = inner;
		y1 = innerFixed;
		break;
	}
}

//  Try to cut off a corner of the sprite, by drawing a segment (x0, y0) --> (x1,y1),
//  so that there is no valid pixels on the left side of the segment.
//  We will keep the valid segment that the cut off triangle has the maximum area.
//  We achieve this goal by combining 'brute force' and 'binary search' algorithms.

//  Algorithm details, for the top most case.
//  1) If (x0, y0) is a fixed point, then the best cut would be the one with the maximum y1.
//     We find such a 'y1' by applying a binary search.
//  2) To find the best position for (x0, y0), we use an exhaustive algorithm.
//        
// (x0,y0)_____            ________          ________               _____(x1, y1)      
//    /       |           /       |         /       |              /     \  
//   /        |          /        |        /        |             /       \ (x0, y0)
//  / (x1,y1) |   ==>   /         | ==>   /         |     ==>    /         |
//  |         |         |         |       |         |(x1,y1)     |         |
//  |         |         |(x0,y0)  |       |        /             |        /
//  |_________|          \________|        \______/ (x0,y0)       \______/
//                      (x1,y1)
void BoundingGenerator::TryCutCorner(int cornerNo)
{
	int outerFrom, outerTo, innerFrom, innerTo;
	int outerFixed, innerFixed;
	int bestCutArea, bestOutter, bestInner;
	bool checkBottomMostPixels;
	int x0, y0, x1, y1;

	switch (cornerNo) {
	case TOPLEFT_CORNER:
		outerFrom = mLeft + 1;
		outerTo = mRight;
		outerFixed = mTop;

		innerFrom = mTop;
		innerTo = mBottom + 1;
		innerFixed = mLeft;

		checkBottomMostPixels = false;
		break;

	case BOTTOMLEFT_CORNER:
		outerFrom = mSpriteInfo.vertex.back().y + 1;
		outerTo = mBottom;
		outerFixed = mLeft;

		innerFixed = mBottom;
		innerFrom = mLeft;
		innerTo = mRight + 1;

		checkBottomMostPixels = true;
		break;

	case BOTTOMRIGHT_CORNER:
		outerFrom = mSpriteInfo.vertex.back().x + 1;
		outerTo = mRight;
		outerFixed = mBottom;

		innerFixed = mRight;
		innerFrom = mBottom;
		innerTo = mTop - 1;
		checkBottomMostPixels = true;
		break;

	case TOPRIGHT_CORNER:

		outerFrom = mTop;
		outerTo = mSpriteInfo.vertex.back().y - 1;
		outerFixed = mRight;

		innerFrom = mRight;
		innerTo = mSpriteInfo.vertex.front().x;
		innerFixed = mTop;

		checkBottomMostPixels = false;
		break;
	}

	bestCutArea = -1;

	for (int outer = outerFrom; outer < outerTo; outer += 1)
	{
		int mid;
		int inner = innerFrom;
		int innerEnd = innerTo;
		
		// Binary search, to find the best value for inner that segment(x0, y0) --> (x1, y1) is a valid cutting line.
		while (abs(inner - innerEnd) > 1)
		{
			mid = (innerEnd + inner) / 2;

			MakeSegmentDirectionRight(x0, y0, x1, y1, outer, outerFixed, mid, innerFixed, cornerNo);

			if (IsCutLineValid(x0, y0, x1, y1, checkBottomMostPixels))
			{
				inner = mid;
			}
			else
			{
				innerEnd = mid;
			}
		}

		int area = 0;
		if (cornerNo == BOTTOMRIGHT_CORNER || cornerNo == BOTTOMLEFT_CORNER)
		{
			area = abs((outer - outerTo) * (inner - innerFrom));
		}
		else
		{
			area = abs((outer - outerFrom) * (inner - innerFrom));
		}

		if (area > bestCutArea)
		{
			bestOutter = outer;
			bestCutArea = area;
			bestInner = inner;
		}
	}

	if (bestCutArea > MIN_AREA_TO_CUT)
	{
		MakeSegmentDirectionRight(x0, y0, x1, y1, bestOutter, outerFixed, bestInner, innerFixed, cornerNo);

		CPoint pt0 = {x0, y0}, pt1 = {x1, y1};
		mSpriteInfo.vertex.push_back(pt0);
		mSpriteInfo.vertex.push_back(pt1);

		mSpriteInfo.shapeMask |= (1 << cornerNo);
	}
	else
	{
		switch(cornerNo)
		{
		case TOPLEFT_CORNER:
			x0 = mLeft;
			y0 = mTop;
			break;

		case BOTTOMLEFT_CORNER:
			x0 = mLeft;
			y0 = mBottom;
			break;

		case BOTTOMRIGHT_CORNER:
			x0 = mRight;
			y0 = mBottom;
			break;

		case TOPRIGHT_CORNER:
			x0 = mRight;
			y0 = mTop;
			break;
		}

		CPoint pt ={x0, y0};
		mSpriteInfo.vertex.push_back(pt);
	}
}


bool BoundingGenerator::HasValidPixelAt(int x, int y)
{
    return mPngFile->getAlpha(x, y) > 0 || mPngFile->getRed(x, y) > 0
        || mPngFile->getGreen(x, y) > 0 || mPngFile->getBlue(x, y) > 0;
}

// To test if a cutting line is valid or not, we don't need to test every pixels against the line.
// Instead, if the segment is facing up, we only check if the top most pixel (or bottom most) of every column
// is included by this cut.
bool BoundingGenerator::IsCutLineValid(int fromX, int fromY, int toX, int toY, bool checkBottomMost)
{
	//   These points will make the corresponding cut invalid,
	//   with the first cutting-segment facing up, the second one facing down.
	//      ____       ______
	//    ./    |     |      |
	//    /     |      \     |
	//   |______|      .\____|

    int dir = toX > fromX ? 1 : -1;
	std::vector<int>& checkArr = checkBottomMost ? mBottomMostInCol : mTopMostInCol;

    for (int x = fromX; x != toX; x += dir)
    {
        int y = checkArr[x];

		if (y < 0 || y > mBottom) continue;

		// If there is any valid pixel that is on the right side of this segment,
		// then this segment is invalid.
        if (!Left(fromX, fromY, toX, toY, x, y))
            return false;
    }

    return true;
}

void BoundingGenerator::FindBoundingPixels()
{
    int w = mPngFile->getwidth(), h = mPngFile->getheight();
	int i, j;

 	mTopMostInCol.resize(w);
 	mBottomMostInCol.resize(w);

    for (i = 0; i < w; ++i)
    {
		j = 0;
		while (j < h && !HasValidPixelAt(i, j)) ++j;
		mTopMostInCol[i] = j;
		
		j = h - 1;
		while (j >= 0 && !HasValidPixelAt(i, j)) --j;
		mBottomMostInCol[i] = j;
    }
}

