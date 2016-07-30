#ifndef _BOUNDINGGENERATOR_H_
#define _BOUNDINGGENERATOR_H_

#include "TextureSpaceArranger.h"
#include "MyPngWriter.h"

class BoundingGenerator {

public:

    BoundingGenerator();

    SpriteInfo GenerateOverlayRects(const std::string& spriteTextureFilePath);

private:

    void TryCutTopLeft();

    void TryCutBottomLeft();

    void TryCutBottomRight();

    void TryCutTopRight();

    void FindBoundingBox();

    bool HasValidPixelAt(int x, int y);

    bool Left(int x0, int y0, int x1, int y1, int xp, int yp);

    // ����faceUp������
    // true��ʾ�߶γ��ϣ���ʾ�������ص㶼�ڴ��߶��Ϸ������߶κϷ�
    // false��ʾ�߶γ���
    bool IsCutLineValide(int fx, int fy, int tx, int ty, bool faceUp);

private:

    const static int MIN_AREA_TO_CUT;

    SpriteInfo mSpriteInfo;

    int top, left, right, bottom;

    std::vector<int> leftmost_pix, rightmost_pix, topmost_pix, bottommost_pix;

    MyPngWriter* inFile;
};

#endif