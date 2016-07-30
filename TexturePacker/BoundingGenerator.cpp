#include "BoundingGenerator.h"
#include "MyPngWriter.h"
#include <cassert>

const int BoundingGenerator::MIN_AREA_TO_CUT = 3500;
enum {
	TOPLEFT_CORNER = 0, 
	BOTTOMLEFT_CORNER = 1,
	BOTTOMRIGHT_CORNER = 2,
	TOPRIGHT_CORNER = 3
};

BoundingGenerator::BoundingGenerator()
{
}

SpriteInfo BoundingGenerator::GenerateOverlayRects(const std::string& spriteTextureFilePath)
{
	int w, h;

	inFile = new MyPngWriter(1, 1, 0, spriteTextureFilePath.c_str());
    inFile->readfromfile(spriteTextureFilePath.c_str());

    w = inFile->getwidth();
	h = inFile->getheight();
    
    FindBoundingPixels();
    
    mSpriteInfo.shapeMask = 0;

    // Must call these 4 functions in that order.
	TryCutTopLeft();
    TryCutBottomLeft();
    TryCutBottomRight();
    TryCutTopRight();
    
    return mSpriteInfo;
}

void BoundingGenerator::TryCutCorner(int cornerNo)
{
	int dirX, dirY;
	int fromX, fromY, toX, toY;
	int bestCutArea, bestCutFromX, bestCutFromY, bestCutToX, bestCutToY;

	bestCutArea = -1;

	// NOTE:Our coordinate system looks this way.
	// --------->x                       _    
	// |                  ~/| TOPLEFT   |\  BOTTOMLEFT     /  dirX=-1          \   dirX=1
	// |                  /   dirX=1      \ dirX=-1       /   dirY=1            \  dirY=1
	// |                 /    dirY=-1      \ dirY=-1    |/_  BOTTOMRIGHT        _\| TOPRIGHT
	//\|/
	// y
	//
	// We draw the cutting line counter-clock-wise, on the paper. 
	switch (cornerNo) {
	case TOPLEFT_CORNER:
		dirX = 1;
		dirY = -1;
		break;

	case BOTTOMLEFT_CORNER:
		dirX = -1;
		dirY = -1;
		break;

	case BOTTOMRIGHT_CORNER:
		dirX = -1;
		dirY = 1;
		break;

	case TOPRIGHT_CORNER:
		dirX = 1;
		dirY = 1;
		break;
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

bool BoundingGenerator::Left(int x0, int y0, int x1, int y1, int xp, int yp)
{
    double area2 = (x1 - x0) * (double)(yp - y0) - (xp - x0) * (double)(y1 - y0);
    return area2 < 0;
}

// To test if a cutting line is valid or not, we don't need to test every pixels against the line.
// Instead, if the segment is facing up, we only check if the top-most pixel of every column
// is included by this cut.
bool BoundingGenerator::IsCutLineValid(int fromX, int fromY, int toX, int toY, bool faceUp)
{
    assert(faceUp == (fromX < toX));

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

    mTop = h;
    mLeft = w; mBottom = -1;
    mRight = -1;

    mTop = 0; mLeft = 0; mRight = w - 1; mBottom = h - 1;

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

