/********************************************************************
Date:		2013/5/28    
Filename:	TextureSpaceArranger.cpp
Author:		maxtang

algorithm: Based on http://www.codeproject.com/Articles/210979/Fast-optimizing-rectangle-packing-algorithm-for-bu
           See also http://people.csail.mit.edu/rivest/pubs/BCR80.pdf

history:
            2014/2/14
            Added the feature to support that sprites can be shaped as polygons bellow. 
                   _____         ______
                  /     \       |      \
                 /       |      |       \
                |        |       \       |
                |       /         \______|
                 \_____/           

*********************************************************************/
#include "TexturePacker.h"
#include "GeoUtil.h"
#include <algorithm>


const int BOUNDING_PAD = 2;

struct Comp {
    bool operator()(const SpriteInfo& a, const SpriteInfo& b)
    {
        return a.h > b.h;
    }
};

TexturePacker::TexturePacker(int width, int height)
:mWidth(width)
,mHeight(height)
{    
    mPossibleLocations.push_back(std::make_pair(0, 0));
}

void TexturePacker::Pack(std::vector<SpriteInfo>& spriteList)
{
    int size = (int)spriteList.size();

    // Calculate sprites' width and height, then sort them.
    for (int i = 0; i < spriteList.size(); ++i)
    {
        int w = 0, h = 0;
        const SpriteInfo &sprite = spriteList[i];
        for (int j = 0; j <sprite.vertex.size(); ++j)
        {
            CPoint pt = sprite.vertex[j];
            w = std::max(pt.x, w);
            h = std::max(pt.y, h);
        }
        spriteList[i].w = w + BOUNDING_PAD;
        spriteList[i].h = h + BOUNDING_PAD;
    }

	// Sort sprites.
    std::sort(spriteList.begin(), spriteList.end(), Comp());

    mPossibleLocations.clear();
    mPossibleLocations.push_back(std::make_pair(0, 0));

    for (int i = 0; i < size; ++i)
    {
        TryArrangeARect(spriteList[i]);
    }
}

bool TexturePacker::CanBePlacedAt(int x, int y, SpriteInfo &sprite)
{
    bool result = true;    

    if (x + sprite.w >= mWidth || y + sprite.h >= mHeight)
        return false;

    sprite.x = x; 
	sprite.y = y;

    for (int j = 0; j < mOccupiedRects.size(); ++j)
    {
        const SpriteInfo &oth = mOccupiedRects[j];

        if ( !NotOverlap(sprite, oth) )
        {
            result = false;
            break;
        }
	}

    return result;
}

bool TexturePacker::TryArrangeARect(SpriteInfo &sprite)
{
    std::vector<std::pair<int,int> > possiblePositions = mPossibleLocations;

	// Find more possible positions in the corners of existing sprites.
    FindMorePossiblePositions(possiblePositions, sprite);

	// Sort the expanding list, from left to right, if two positions have the same x, sort them from top to bottom.
    std::sort(possiblePositions.begin(), possiblePositions.end());

    for (int i=0; i<possiblePositions.size(); ++i)
    {
        int x = possiblePositions[i].first;
        int y = possiblePositions[i].second;

        bool canBePlaced = CanBePlacedAt(x, y, sprite);

        if (canBePlaced)
        {
            sprite.fitted = true;
            sprite.x = x;
            sprite.y = y;

			// Add the top-right corner and bottom-left corner of the new sprite to expanding point list.
            mPossibleLocations.push_back(std::make_pair(sprite.x + sprite.w, sprite.y));
            mPossibleLocations.push_back(std::make_pair(sprite.x , sprite.y + sprite.h));

			// Draw a ray up from the right edge of the sprite, if the ray hit another sprite at point A,
			// add A to expanding point list.
			int maxY = -1;
            for (int k=0; k<mOccupiedRects.size(); ++k)
            {
                SpriteInfo &oth = mOccupiedRects[k];
                if (oth.x < x + sprite.w && x + sprite.w < oth.x + oth.w
                    && oth.y + oth.h < y)
                {
                    maxY = std::max(maxY, oth.y + oth.h);
                }
            }
            if( maxY >= 0)
            {
                mPossibleLocations.push_back(std::make_pair(x + sprite.w, maxY));
            }

            // Draw a line from the bottom-left of the sprite to left, if the ray hit another sprite at point B,
			// add B to expanding point list.
            int maxX = -1;
            for (int k=0; k<mOccupiedRects.size(); ++k)
            {
                SpriteInfo &oth = mOccupiedRects[k];
                if (oth.y < y + sprite.h && y + sprite.h < oth.y + oth.h
                    && x > oth.x + oth.w)
                {
                    maxX = std::max(maxX, oth.x + oth.w);
                }
            }
            if( maxX >= 0)
            {
                mPossibleLocations.push_back(std::make_pair(maxX, sprite.y + sprite.h));
            }

            // Draw a ray from any the bottom-left corner of any of the existing sprites to the left,
			// if the ray hit the newly placed sprite, add the cross point to expanding list.
            for (int k=0; k<mOccupiedRects.size(); ++k)
            {
                SpriteInfo &oth = mOccupiedRects[k];
                int underY = oth.y + oth.h;

                if (underY > sprite.y && underY < sprite.y + sprite.h
                    && oth.x > sprite.x + sprite.w)
                {
					mPossibleLocations.push_back(std::make_pair(sprite.x + sprite.w, underY));
                }
            }

			// Draw a ray along the right edge of any of the existing sprites up,
			// if the ray hit the newly placed sprite, add the cross point to expanding list.
            for (int k=0; k<mOccupiedRects.size(); ++k)
            {
                SpriteInfo &oth = mOccupiedRects[k];
                int rightX = oth.x + oth.w;

                // hit the sprite
                if (rightX > sprite.x && rightX < sprite.x + sprite.w
                    // and bellow
                    && oth.y > sprite.y + sprite.h)
                {
					mPossibleLocations.push_back(std::make_pair(rightX, sprite.y + sprite.h));
                }
            }

            mOccupiedRects.push_back(sprite);            
            return true;
        }
    }
    return false;
}

bool IsPointInside(const SpriteInfo& sprite, int x, int y)
{   
    bool inside = true;
    int n = sprite.vertex.size();
    for (int i = 0; i < n; ++i)
    {
        CPoint p0 = {sprite.vertex[i].x + sprite.x, sprite.vertex[i].y + sprite.y};
        int j = (i + 1) % n;
        CPoint p1 = {sprite.vertex[j].x + sprite.x, sprite.vertex[j].y + sprite.y};
        if (!LeftOn(p0.x, p0.y, p1.x, p1.y, x, y))
        {
            inside = false;
            break;
        }
    }
    return inside;
}

// Try to find positions in the corner of another sprite that our new sprite might be able to place at.
inline void FindPossiblePositionsInCorners(const SpriteInfo& sprite,const SpriteInfo& oth, std::vector<std::pair<int,int> >& possiblePositions)
{
    int idx = 0;
    int w = sprite.w, h = sprite.h;
    for (int corner = 0; corner < 4; ++corner, ++idx)
    {
        if (!(oth.shapeMask & (1 << corner)))
            continue;

        // Ignore top-left corner.
        if (corner > 0)
        {
            int j = (idx + 1) % oth.vertex.size();

            // p0-->p1 is the cutting line.
            CPoint p0 = {oth.vertex[idx].x + oth.x, oth.vertex[idx].y + oth.y};
            CPoint p1 = {oth.vertex[j].x + oth.x, oth.vertex[j].y + oth.y};

            if (corner == BOTTOMLEFT_CORNER)
            {
                if (w < p1.x - p0.x)
                {
                    int pos_y = (p1.y - p0.y) * w / (double)(p1.x - p0.x) + p0.y + oth.y + 1;
                    possiblePositions.push_back(std::make_pair<int, int>(p0.x + oth.x, pos_y));
                }
            }

            if (corner == BOTTOMRIGHT_CORNER)
            {
                if (h < p0.y - p1.y)
                {
                    int pos_x = h * (p1.x - p0.x) / (double)(-p1.y + p0.y) + p0.x + oth.x + 1;
                    possiblePositions.push_back(std::make_pair<int, int>(pos_x, p0.y - h));
                }
            }

            if (corner == TOPRIGHT_CORNER)
            {
                if (h < p0.y - p1.y)
                {
                    int pos_x = h * (p1.x - p0.x) / (double)(p1.y - p0.y) + p1.x + oth.x + 1;
                    possiblePositions.push_back(std::make_pair<int, int>(pos_x, p1.y));
                }
            }
        }

        ++idx;
    }
}

void TexturePacker::FindMorePossiblePositions(std::vector<std::pair<int,int> >& possiblePositions, const SpriteInfo &sprite)
{    
    for (int i = 0; i < mOccupiedRects.size(); ++i)
    {
        const SpriteInfo& oth = mOccupiedRects[i];

        if (oth.shapeMask & 14)
            FindPossiblePositionsInCorners(sprite, oth, possiblePositions);
    }
}

// Test if two sprites are not overlapped.
// Both sprites can be rectangles or truncated rectangles.
bool TexturePacker::NotOverlap(const SpriteInfo& a, const SpriteInfo& b)
{
    if (a.x + a.w <= b.x || a.y + a.h <= b.y
        || b.x + b.w <= a.x || b.y + b.h <= a.y)
        return true;

    int cnt_b = b.vertex.size();
    if (cnt_b > 4)
    {
        int idx = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (!(b.shapeMask & (1 << i)))
            {
                ++idx;
                continue;
            }

            const CPoint &p0 = b.vertex[idx], &p1 = b.vertex[(idx+1)%cnt_b];

            bool allOutside = true;
            for (int j = 0; j < a.vertex.size(); ++j)
            {
                const CPoint &p = a.vertex[j];

                if (LeftOn(p0.x + b.x, p0.y + b.y, p1.x + b.x, p1.y + b.y, p.x + a.x, p.y + a.y))
                {
                    allOutside = false;
                    break;
                }
            }

            if (allOutside)
                return true;

            if (b.shapeMask & (1 << i))
                ++idx;
            ++idx;
        }
    }    

    return false;
}
