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
	inFile->close();
}

SpriteInfo BoundingGenerator::GenerateOverlayRects(const std::string& spriteTextureFilePath)
{
	int w, h;

	inFile = new MyPngWriter(1, 1, 0, spriteTextureFilePath.c_str());
    inFile->readfromfile(spriteTextureFilePath.c_str());

    w = inFile->getwidth();
	h = inFile->getheight();

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

	//printf("GOOD....\n");
    // Must call these 4 functions in that order.
	//TryCutTopLeft();
    //TryCutBottomLeft();
    //TryCutBottomRight();
    //TryCutTopRight();
    
    return mSpriteInfo;
}

inline void AssignValue(int &x0, int &y0, int &x1, int &y1, 
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
//
// (x0,y0)_____            ________          ________               _____(x1, y1)      
//    /       |           /       |         /       |              /     \  
//   /        |          /        |        /        |             /       \ (x0, y0)
//  / (x1,y1) |   ==>   /         | ==>   /         |     ==>    /         |
//  |         |         |         |       |         |£¨x1,y1)    |         |
//  |         |         |(x0,y0)  |       |        /             |        /
//  |_________|          \________|        \______/ (x0,y0£©      \______/
//                      (x1,y1)
void BoundingGenerator::TryCutCorner(int cornerNo)
{
	int outerFrom, outerTo, innerFrom, innerTo;
	int outerFixed, innerFixed;
	int bestCutArea, bestOutter, bestInner;
	bool faceUp;
	int x0, y0, x1, y1;

	switch (cornerNo) {
	case TOPLEFT_CORNER:
		outerFrom = mLeft + 1;
		outerTo = mRight;
		outerFixed = mTop;

		innerFrom = mTop;
		innerTo = mBottom + 1;
		innerFixed = mLeft;

		faceUp = false;
		break;

	case BOTTOMLEFT_CORNER:
		outerFrom = mSpriteInfo.vertex.back().y + 1;
		outerTo = mBottom;
		outerFixed = mLeft;

		innerFixed = mBottom;
		innerFrom = mLeft;
		innerTo = mRight + 1;

		faceUp = true;
		break;

	case BOTTOMRIGHT_CORNER:
		outerFrom = mSpriteInfo.vertex.back().x + 1;
		outerTo = mRight;
		outerFixed = mBottom;

		innerFixed = mRight;
		innerFrom = mBottom;
		innerTo = mTop - 1;
		faceUp = true;
		break;

	case TOPRIGHT_CORNER:

		outerFrom = mTop;
		outerTo = mSpriteInfo.vertex.back().y - 1;
		outerFixed = mRight;

		innerFrom = mRight;
		innerTo = mSpriteInfo.vertex.front().x;
		innerFixed = mTop;

		faceUp = false;
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

			AssignValue(x0, y0, x1, y1, outer, outerFixed, mid, innerFixed, cornerNo);

			if (IsCutLineValid(x0, y0, x1, y1, faceUp))
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
		AssignValue(x0, y0, x1, y1, bestOutter, outerFixed, bestInner, innerFixed, cornerNo);

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

// It's an O( n*n*log(n) ) function.
void BoundingGenerator::TryCutTopLeft()
{
    // Try to cut the top left corner of the sprite without losing any valid pixels.
    //   ______
    //  /     |
    // |      |
    // |______|

    int bestArea = -1, bestFx, bestTy; 

    for (int fx = mLeft + 1; fx < mRight; ++fx)
    {
        int ty_max = mBottom, ty_min = mTop;
        int ty_mid;

        // Binary search, to find the largest y that segment (fx, top) --> (left, y) is a valide cut.
        while (ty_max > ty_min + 1)
        {
            ty_mid = (ty_max + ty_min) / 2;
            if (IsCutLineValid(fx, mTop, mLeft, ty_mid, false))
            {
                ty_min = ty_mid;
            }
            else
            {
                ty_max = ty_mid;
            }
        }

        int best = ty_min;

        if ( IsCutLineValid(fx, mTop, mLeft, ty_max, false) )
            best = ty_max;

        int CutedArea = (fx - mLeft) * (best - mTop);

        if (bestArea < CutedArea)
        {
            bestArea = CutedArea;
            bestFx = fx;
            bestTy = best;
        }
    }

    if (bestArea > MIN_AREA_TO_CUT)
    {
        CPoint pt0 = {bestFx, mTop}, pt1 = {mLeft, bestTy};
        mSpriteInfo.vertex.push_back(pt0);
        mSpriteInfo.vertex.push_back(pt1);

        mSpriteInfo.shapeMask |= (1 << 0);
    }
    else
    {
        // No valid cutting found.
        CPoint pt ={mLeft, mTop};
        mSpriteInfo.vertex.push_back(pt);
    }
}

void BoundingGenerator::TryCutBottomLeft()
{
    // Try to cut the bottom-left corner, given that the top-left corner might have been cut.
    //   _____
    //  /     |
    // |      |
    //  \_____|

    int bestArea = -1, bestFy, bestTx;

    // Start below the top left cutting line.
    int fy_min = mSpriteInfo.vertex.back().y + 1;    

    for (int fy = fy_min; fy < mBottom; ++fy)
    {
        int tx_max = mRight, tx_min = mLeft;
        int tx_mid;

        // Binary search.
        while (tx_max > tx_min + 1)
        {
            tx_mid = (tx_max + tx_min) / 2;
            if (IsCutLineValid(mLeft, fy, tx_mid, mBottom, true))
            {
                tx_min = tx_mid;
            }
            else
            {
                tx_max = tx_mid;
            }
        }

        int best = tx_min;

        if ( IsCutLineValid(mLeft, fy, tx_max, mBottom, true) )
            best = tx_max;

        int CutedArea = (mBottom - fy) * (best - mLeft);

        if (bestArea < CutedArea)
        {
            bestArea = CutedArea;
            bestFy = fy;
            bestTx = best;
        }
    }

    if (bestArea > MIN_AREA_TO_CUT)
    {
        CPoint pt0 = {mLeft, bestFy}, pt1 = {bestTx, mBottom};
        mSpriteInfo.vertex.push_back(pt0);
        mSpriteInfo.vertex.push_back(pt1);

        mSpriteInfo.shapeMask |= (1 << 1);
    }
    else
    {
        // No valid cutting found.
        CPoint pt ={mLeft, mBottom};
        mSpriteInfo.vertex.push_back(pt);
    }

}

void BoundingGenerator::TryCutBottomRight()
{
    // Bottom-right corner.
    //   _____
    //  /     |
    // |      |
    //  \____/ 

    int bestArea = -1, bestFx, bestTy;

    // Start to the right of the bottom-left cutting line.
    int fx_min = mSpriteInfo.vertex.back().x + 1;

    for (int fx = fx_min; fx < mRight; ++fx)
    {
        int ty_max = mBottom, ty_min = mTop;
        int ty_mid;

        // Binary search.
        while (ty_max > ty_min + 1)
        {
            ty_mid = (ty_max + ty_min) / 2;
            if (IsCutLineValid(fx, mBottom, mRight, ty_mid, true))
            {
                ty_max = ty_mid;
            }
            else
            {
                ty_min = ty_mid;
            }
        }

        int best = ty_max;

        if ( IsCutLineValid(fx, mBottom, mRight, ty_min, true) )
            best = ty_min;

        int CutedArea = (mRight - fx) * (-best + mBottom);

        if (bestArea < CutedArea)
        {
            bestArea = CutedArea;
            bestFx = fx;
            bestTy = best;
        }
    }

    if (bestArea > MIN_AREA_TO_CUT)
    {
        CPoint pt0 = {bestFx, mBottom}, pt1 = {mRight, bestTy};
        mSpriteInfo.vertex.push_back(pt0);
        mSpriteInfo.vertex.push_back(pt1);

        mSpriteInfo.shapeMask |= (1 << 2);
    }
    else
    {
        // No valid cutting found.
        CPoint pt ={mRight, mBottom};
        mSpriteInfo.vertex.push_back(pt);
    }
}

void BoundingGenerator::TryCutTopRight()
{
    // Top-right corner.
    //   ____
    //  /    \
    // |      |
    //  \____/ 
    int bestArea = -1, bestFy, bestTx;

    // Start from above the bottom-right cutting line.
    int fy_max = mSpriteInfo.vertex.back().y;

    for (int fy = mTop + 1; fy < fy_max; ++fy)
    {
        int tx_max = mRight, tx_min = mSpriteInfo.vertex.front().x + 1;
        int tx_mid, best;

        // Binary search.
        while (tx_max > tx_min + 1)
        {
            tx_mid = (tx_max + tx_min) / 2;
            if (IsCutLineValid(mRight, fy, tx_mid, mTop, false))
            {
                tx_max = tx_mid;
            }
            else
            {
                tx_min = tx_mid;
            }
        }

        best = tx_max;

        if ( IsCutLineValid(mRight, fy, tx_min, mTop, false) )
            best = tx_min;

        int CutedArea = (fy - mTop) * (mRight - best);

        if (bestArea < CutedArea)
        {
            bestArea = CutedArea;
            bestFy = fy;
            bestTx = best;
        }
    }

    if (bestArea > MIN_AREA_TO_CUT)
    {
        CPoint pt0 = {mRight, bestFy}, pt1 = {bestTx, mTop};
        mSpriteInfo.vertex.push_back(pt0);
        mSpriteInfo.vertex.push_back(pt1);

        mSpriteInfo.shapeMask |= (1 << 3);
    }
    else
    {
        // No valid cutting line found.
        CPoint pt ={mRight, mTop};
        mSpriteInfo.vertex.push_back(pt);
    }
}

bool BoundingGenerator::HasValidPixelAt(int x, int y)
{
    return inFile->getAlpha(x, y) > 0 || inFile->getRed(x, y) > 0
        || inFile->getGreen(x, y) > 0 || inFile->getBlue(x, y) > 0;
}



// To test if a cutting line is valid or not, we don't need to test every pixels against the line.
// Instead, if the segment is facing up, we only check if the top-most pixel of every column
// is included by this cut.
bool BoundingGenerator::IsCutLineValid(int fromX, int fromY, int toX, int toY, bool faceUp)
{
	//   These points will make the corresponding cut invalid,
	//   with the first cutting-segment facing up, the second one facing down.
	//      ____       ______
	//    ./    |     |      |
	//    /     |      \     |
	//   |______|      .\____|

    int dir = faceUp ? 1 : -1;
    for (int x = fromX; x != toX; x += dir)
    {
        int y = faceUp ? bottommost_pix[x] : topmost_pix[x];

		// If there is any valid pixel that is on the right side of this segment,
		// then this segment is invalid.
        if (!Left(fromX, fromY, toX, toY, x, y))
            return false;
    }

    return true;
}

void BoundingGenerator::FindBoundingPixels()
{
    int w = inFile->getwidth(), h = inFile->getheight();

    for (int i = 0; i < w; ++i)
    {
        topmost_pix.push_back(h);
        bottommost_pix.push_back(-1);

        for (int j = 0; j < h; ++j)
        {
            if (HasValidPixelAt(i, j))
            {
                topmost_pix[i] = std::min(topmost_pix[i], j);
                bottommost_pix[i] = std::max(bottommost_pix[i], j);
            }
        }
    }
}

