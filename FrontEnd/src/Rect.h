#pragma once

struct Rect
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    bool operator==(const Rect& r) const
    {
        return
            x == r.x
            && y == r.y
            && w == r.w
            && h == r.h;
    }

    bool operator!=(const Rect& r) const
    {
        return !(*this == r);
    }
};