#ifndef _TEXTURESPACEARRANGER_H_
#define _TEXTURESPACEARRANGER_H_
#include <string>
#include <vector>

struct MyRect
{
    int x, y; // 这里的x, y是相对于Sprite的local offset
    int w, h;
};

struct CPoint {
    int x, y;
};

// Masks used to check the 4 corners of a sprite has a cutting line.
const static int MASK_TOP_LEFT = (1<<0);
const static int MASK_BOTTOM_LEFT = (1<<1);
const static int MASK_BOTTOM_RIGHT = (1<<2);
const static int MASK_TOP_RIGHT = (1<<3);

// This struct represents a Sprite in a packed texture.
struct SpriteInfo
{    
    int x, y;                   // The position of this sprite in the packed texture.
    int w, h;                   // Width and height of this sprite.
    void *userData;
    bool fitted;                // Flag to tell if this sprite has already got a position in the packed texture.
    std::vector<CPoint> vertex;
    int  shapeMask; // Mask to indicate if any of the 4 corners of this sprite has a cutting line.
};

inline bool InnerOn(int x0, int y0, int x1, int y1, int xp, int yp);
bool IsPointInside(const SpriteInfo& sprite, int x, int y);

class TextureSpaceArranger
{
public:

    TextureSpaceArranger(int width, int height);

    virtual void DoArrange(std::vector<SpriteInfo>& rects);

protected:

    void FindMorePossiblePositions(std::vector<std::pair<int,int> >& possiblePositions, const SpriteInfo &sprite);

    bool CanBePlacedAt(int x, int y, SpriteInfo &sprite);

    bool TryArrangeARect(SpriteInfo &rect);

    bool NotOverlap(const SpriteInfo& a, const SpriteInfo& b);
    
    std::vector<SpriteInfo> mOccupiedRects;

    std::vector<std::pair<int,int> > mPossibleLocations;

    int mWidth;
    int mHeight;    
};

#endif