#include "BoundingGenerator.h"
#include "MyPngWriter.h"
#include <cassert>

const int BoundingGenerator::MIN_AREA_TO_CUT = 3500;

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
    
	// trim
    FindBoundingBox();
    
    mSpriteInfo.shapeMask = 0;

    // Must call these 4 functions in that order.
	TryCutTopLeft();
    TryCutBottomLeft();
    TryCutBottomRight();
    TryCutTopRight();
    
    return mSpriteInfo;
}

// It's an O( n*n*log(n) ) function.
void BoundingGenerator::TryCutTopLeft()
{
    // Try to cut the top left corner the sprite without losing any non-zero pixels.
    //   ______
    //  /     |
    // |      |
    // |______|

    int bestArea = -1, bestFx, bestTy; 

    for (int fx = left + 1; fx < right; ++fx)
    {
        int ty_max = bottom, ty_min = top;
        int ty_mid;

        // Binary search, to find the largest y that segment (fx, top) --> (left, y) is a valide cut.
        while (ty_max > ty_min + 1)
        {
            ty_mid = (ty_max + ty_min) / 2;
            if (IsCutLineValide(fx, top, left, ty_mid, false))
            {
                ty_min = ty_mid;
            }
            else
            {
                ty_max = ty_mid;
            }
        }

        int best = ty_min;

        if ( IsCutLineValide(fx, top, left, ty_max, false) )
            best = ty_max;

        int CutedArea = (fx - left) * (best - top);

        if (bestArea < CutedArea)
        {
            bestArea = CutedArea;
            bestFx = fx;
            bestTy = best;
        }
    }

    if (bestArea > MIN_AREA_TO_CUT)
    {
        CPoint pt0 = {bestFx, top}, pt1 = {left, bestTy};
        mSpriteInfo.vertex.push_back(pt0);
        mSpriteInfo.vertex.push_back(pt1);

        mSpriteInfo.shapeMask |= (1 << 0);
    }
    else
    {
        // No valid cutting found.
        CPoint pt ={left, top};
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

    for (int fy = fy_min; fy < bottom; ++fy)
    {
        int tx_max = right, tx_min = left;
        int tx_mid;

        // Binary search.
        while (tx_max > tx_min + 1)
        {
            tx_mid = (tx_max + tx_min) / 2;
            if (IsCutLineValide(left, fy, tx_mid, bottom, true))
            {
                tx_min = tx_mid;
            }
            else
            {
                tx_max = tx_mid;
            }
        }

        int best = tx_min;

        if ( IsCutLineValide(left, fy, tx_max, bottom, true) )
            best = tx_max;

        int CutedArea = (bottom - fy) * (best - left);

        if (bestArea < CutedArea)
        {
            bestArea = CutedArea;
            bestFy = fy;
            bestTx = best;
        }
    }

    if (bestArea > MIN_AREA_TO_CUT)
    {
        CPoint pt0 = {left, bestFy}, pt1 = {bestTx, bottom};
        mSpriteInfo.vertex.push_back(pt0);
        mSpriteInfo.vertex.push_back(pt1);

        mSpriteInfo.shapeMask |= (1 << 1);
    }
    else
    {
        // No valid cutting found.
        CPoint pt ={left, bottom};
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

    for (int fx = fx_min; fx < right; ++fx)
    {
        int ty_max = bottom, ty_min = top;
        int ty_mid;

        // Binary search.
        while (ty_max > ty_min + 1)
        {
            ty_mid = (ty_max + ty_min) / 2;
            if (IsCutLineValide(fx, bottom, right, ty_mid, true))
            {
                ty_max = ty_mid;
            }
            else
            {
                ty_min = ty_mid;
            }
        }

        int best = ty_max;

        if ( IsCutLineValide(fx, bottom, right, ty_min, true) )
            best = ty_min;

        int CutedArea = (right - fx) * (-best + bottom);

        if (bestArea < CutedArea)
        {
            bestArea = CutedArea;
            bestFx = fx;
            bestTy = best;
        }
    }

    if (bestArea > MIN_AREA_TO_CUT)
    {
        CPoint pt0 = {bestFx, bottom}, pt1 = {right, bestTy};
        mSpriteInfo.vertex.push_back(pt0);
        mSpriteInfo.vertex.push_back(pt1);

        mSpriteInfo.shapeMask |= (1 << 2);
    }
    else
    {
        // No valid cutting found.
        CPoint pt ={right, bottom};
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

    for (int fy = top + 1; fy < fy_max; ++fy)
    {
        int tx_max = right, tx_min = mSpriteInfo.vertex.front().x + 1;
        int tx_mid, best;

        // Binary search.
        while (tx_max > tx_min + 1)
        {
            tx_mid = (tx_max + tx_min) / 2;
            if (IsCutLineValide(right, fy, tx_mid, top, false))
            {
                tx_max = tx_mid;
            }
            else
            {
                tx_min = tx_mid;
            }
        }

        best = tx_max;

        if ( IsCutLineValide(right, fy, tx_min, top, false) )
            best = tx_min;

        int CutedArea = (fy - top) * (right - best);

        if (bestArea < CutedArea)
        {
            bestArea = CutedArea;
            bestFy = fy;
            bestTx = best;
        }
    }

    if (bestArea > MIN_AREA_TO_CUT)
    {
        CPoint pt0 = {right, bestFy}, pt1 = {bestTx, top};
        mSpriteInfo.vertex.push_back(pt0);
        mSpriteInfo.vertex.push_back(pt1);

        mSpriteInfo.shapeMask |= (1 << 3);
    }
    else
    {
        // No valid cutting line found.
        CPoint pt ={right, top};
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

bool BoundingGenerator::IsCutLineValide(int fx, int fy, int tx, int ty, bool faceUp)
{
    assert(faceUp == (fx < tx));

    int dir = faceUp ? 1 : -1;
    for (int x = fx; x != tx; x += dir)
    {
        int y = faceUp ? bottommost_pix[x] : topmost_pix[x];

		// If there is any valid pixel that is on the right side of this segment,
		// then this segment is invalid.
        if (!Left(fx, fy, tx, ty, x, y))
            return false;
    }

    return true;
}

void BoundingGenerator::FindBoundingBox()
{
    int w = inFile->getwidth(), h = inFile->getheight();

    top = h;
    left = w; bottom = -1;
    right = -1;

    top = 0; left = 0; right = w - 1; bottom = h - 1;

    for (int i = 0; i < w; ++i)
    {
        topmost_pix.push_back(h);
        bottommost_pix.push_back(-1);

        for (int j = 0; j < h; ++j)
        {
            if (HasValidPixelAt(i, j))
            {
                //left = std::min(left, i);
                //right = std::max(right, i);
                //top = std::min(top, j);
                //bottom = std::max(bottom, j);

                topmost_pix[i] = std::min(topmost_pix[i], j);
                bottommost_pix[i] = std::max(bottommost_pix[i], j);
            }
        }
    }

    for (int i = 0; i < h; ++i)
    {
        leftmost_pix.push_back(w);
        rightmost_pix.push_back(-1);

        for (int j = 0; j < w; ++j)
        {
            if (HasValidPixelAt(j, i))
            {
                leftmost_pix[i] = std::min(leftmost_pix[i], j);
                rightmost_pix[i] = std::max(rightmost_pix[i], j);
            }
        }
    }
}

