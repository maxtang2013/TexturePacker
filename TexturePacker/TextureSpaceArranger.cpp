/********************************************************************
Date:		2013/5/28    
Filename:	TextureSpaceArranger.cpp
Author:		maxtang
history:
            2014/2/14
            支持（rectangle截角后形成的）polygon，如下图这样的
                   _____
                  /     \
                 /       |
                |        |
                |       /
                 \_____/
*********************************************************************/
#include "TextureSpaceArranger.h"
#include <algorithm>

struct Comp {
    bool operator()(const SpriteInfo& a, const SpriteInfo& b)
    {
        return a.h > b.h;
        //return max(a.w, a.h) > max(b.h, b.w);
    }
};

TextureSpaceArranger::TextureSpaceArranger(int width, int height)
:mWidth(width)
,mHeight(height)
{    
    mPossibleLocations.push_back(std::make_pair(0, 0));
}

void TextureSpaceArranger::DoArrange(std::vector<SpriteInfo>& spriteList)
{
    int size = (int)spriteList.size();

    // 先统计各部分的数据，获得总的大小，用来排序
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
        spriteList[i].w = w + 2;
        spriteList[i].h = h + 2;
    }

    std::sort(spriteList.begin(), spriteList.end(), Comp());

    mPossibleLocations.clear();
    mPossibleLocations.push_back(std::make_pair(0, 0));

    for (int i = 0; i < size; ++i)
    {
        TryArrangeARect(spriteList[i]);
    }
}

bool TextureSpaceArranger::CanBePlacedAt(int x, int y, SpriteInfo &sprite)
{
    bool canPutHere = true;    
    
    {
        if (x + sprite.w >= mWidth || y + sprite.h >= mHeight)
            return false;

        sprite.x = x; sprite.y = y;

        for (int j=0; j<mOccupiedRects.size(); ++j)
        {
            const SpriteInfo &oth = mOccupiedRects[j];

            if ( !NotOverlap(sprite, oth) )
            {
                canPutHere = false;
                break;
            }
        }
    }
    return canPutHere;
}

bool TextureSpaceArranger::TryArrangeARect(SpriteInfo &sprite)
{
    std::vector<std::pair<int,int> > possiblePositions = mPossibleLocations;
    FindMorePossiblePositions(possiblePositions, sprite);

    std::sort(possiblePositions.begin(), possiblePositions.end());

    for (int i=0; i<possiblePositions.size(); ++i)
    {
        int x = possiblePositions[i].first;
        int y = possiblePositions[i].second;

        bool canPutHere = CanBePlacedAt(x, y, sprite);

        if (canPutHere)
        {
            sprite.fitted = true;
            sprite.x = x;
            sprite.y = y;

            // 本次加入的矩形的右上角和左下角
            mPossibleLocations.push_back(std::make_pair(sprite.x + sprite.w, sprite.y));
            mPossibleLocations.push_back(std::make_pair(sprite.x , sprite.y + sprite.h));


            // 本次加入的矩形的右边（延长线），与可能的上方矩形的交点
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

            // 本次加入的矩形的下边（延长线），与可能的左方矩形的交点
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

            // 右方其它矩形的下边（延长线）与本矩形的右边的交点
            for (int k=0; k<mOccupiedRects.size(); ++k)
            {
                SpriteInfo &oth = mOccupiedRects[k];
                int underY = oth.y + oth.h;

                if (underY > sprite.y && underY < sprite.y + sprite.h
                    && oth.x > sprite.x + sprite.w)
                {
                    //bool shouldAdd = true;
                    //for (int l = 0; l < mOccupiedRects.size(); ++l)
                    //{
                    //    SpriteInfo& third = mOccupiedRects[l];
                    //    // 在本矩形右边，且也与oth下边线延长线相交
                    //    if (third.x > rect.x && underY > third.y && underY < third.y + third.h)
                    //    {
                    //        shouldAdd = false;
                    //        break;
                    //    }
                    //}

                    //if (shouldAdd)
                        mPossibleLocations.push_back(std::make_pair(sprite.x + sprite.w, underY));
                }
            }

            // 下方其它矩形的右边（延长线）与本矩形的下边的交点
            for (int k=0; k<mOccupiedRects.size(); ++k)
            {
                SpriteInfo &oth = mOccupiedRects[k];
                int rightX = oth.x + oth.w;

                // 延长线会与本矩形相交
                if (rightX > sprite.x && rightX < sprite.x + sprite.w
                    // 在下方
                    && oth.y > sprite.y + sprite.h)
                {
                    //bool shouldAdd = true;
                    //for (int l = 0; l < mOccupiedRects.size(); ++l)
                    //{
                    //    SpriteInfo& third = mOccupiedRects[l];
                    //    // 在本矩形下边，且也与oth右边延长线相交
                    //    if (third.y > rect.y && rightX > third.x && rightX < third.x + third.w)
                    //    {
                    //        shouldAdd = false;
                    //        break;
                    //    }
                    //}

                    //if (shouldAdd)
                        mPossibleLocations.push_back(std::make_pair(rightX, sprite.y + sprite.h));
                }
            }

            mOccupiedRects.push_back(sprite);            
            return true;
        }
    }
    return false;
}

inline bool InnerOn(int x0, int y0, int x1, int y1, int xp, int yp)
{
    double area2 = (x1 - x0) * (double)(yp - y0) - (xp - x0) * (double)(y1 - y0);
    return area2 <= 0;
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
        if (!InnerOn(p0.x, p0.y, p1.x, p1.y, x, y))
        {
            inside = false;
            break;
        }
    }
    return inside;
}

inline void FindPossiblePositions(const SpriteInfo& sprite,const SpriteInfo& oth, std::vector<std::pair<int,int> >& possiblePositions)
{
    // 在被切掉的角这里找到可能的放置点    
    int idx = 0;
    int w = sprite.w, h = sprite.h;
    for (int i = 0; i < 4; ++i, ++idx)
    {
        if (!(oth.shapeMask & (1 << i)))
            continue;

        // 左上角无需处理
        if (i > 0)
        {
            int j = (idx + 1) % oth.vertex.size();

            // p0-->p1是某个角的截断线
            CPoint p0 = {oth.vertex[idx].x + oth.x, oth.vertex[idx].y + oth.y};
            CPoint p1 = {oth.vertex[j].x + oth.x, oth.vertex[j].y + oth.y};

            // 左下角
            if (i == 1)
            {
                if (w < p1.x - p0.x)
                {
                    int pos_y = (p1.y - p0.y) * w / (double)(p1.x - p0.x) + p0.y + oth.y + 1;
                    possiblePositions.push_back(std::make_pair<int, int>(p0.x + oth.x, pos_y));
                }
            }

            // 右下角
            if (i == 2)
            {
                if (h < p0.y - p1.y)
                {
                    int pos_x = h * (p1.x - p0.x) / (double)(-p1.y + p0.y) + p0.x + oth.x + 1;
                    possiblePositions.push_back(std::make_pair<int, int>(pos_x, p0.y - h));
                }
            }
            // 右上角
            if (i == 3)
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

void TextureSpaceArranger::FindMorePossiblePositions(std::vector<std::pair<int,int> >& possiblePositions, const SpriteInfo &sprite)
{    
    for (int i = 0; i < mOccupiedRects.size(); ++i)
    {
        const SpriteInfo& oth = mOccupiedRects[i];

        if (oth.shapeMask & 14)
            FindPossiblePositions(sprite, oth, possiblePositions);
    }
}



bool TextureSpaceArranger::NotOverlap(const SpriteInfo& a, const SpriteInfo& b)
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

                if (InnerOn(p0.x + b.x, p0.y + b.y, p1.x + b.x, p1.y + b.y, p.x + a.x, p.y + a.y))
                {
                    // 点在此边的内侧
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
