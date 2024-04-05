#include "TXLib.h"

#define VK_EXIT VK_ESCAPE

enum CheckKeyStateStatus
{
    EXIT     = 1,
    CONTINUE = 0,
};

struct Coordinates
{
    float x;
    float y;
};

const Coordinates initialShift = { .x = -1.325f,
                                   .y = 0 };

const int   windowWidth     = 800;
const int   windowHeight    = 600;
const float dx              = 1.f / windowWidth;
const float dy              = 1.f / windowHeight;
const float radiusMax_2     = 100;
const int   nCyclesMax      = 256;
const float minShift        = 10.f;
const int   multiplierShift = 10;
const float multiplierZoom  = 1.1f;

void
DrawMagicBeauty ();

int
FindNIteration  (Coordinates* initial);

RGBQUAD
DetermineColor  (int n);

CheckKeyStateStatus
CheckKeyState   (Coordinates* center, float* scale);

int main ()
{
    txCreateWindow (windowWidth, windowHeight);
    Win32::_fpreset ();
    txBegin ();

    DrawMagicBeauty ();

    return 0;
}

void
DrawMagicBeauty ()
{
    typedef RGBQUAD (&scr_t) [windowHeight][windowWidth];
    scr_t scr = (scr_t) *txVideoMemory();

    Coordinates center     = { .x = initialShift.x,
                               .y = initialShift.y };
    Coordinates initScale  = { .x = 1.0 * windowWidth  / windowWidth,
                               .y = 1.0 * windowHeight / windowWidth };
    float scale = 1;

    while (true)
    {
        if (CheckKeyState(&center, &scale)) return;

        for (int iy = 0; iy < windowHeight; iy++)
        {
            if (GetAsyncKeyState (VK_EXIT)) break;

            Coordinates initial = { .x = 0, .y = 0 };

            initial.x = (          - windowWidth  / 2) * dx * scale * initScale.x + center.x;
            initial.y = ((float)iy - windowHeight / 2) * dy * scale * initScale.y + center.y;

            for (int ix = 0; ix < windowWidth; ix++, initial.x += dx * scale)
            {
                int n = FindNIteration (&initial);

                scr[iy][ix] = DetermineColor (n);
            }
        }

        printf ("\t\r%.0lf", txGetFPS ());
        txSleep ();
    }
}

int
FindNIteration (Coordinates* initial)
{
    assert (initial);

    float x = initial->x;
    float y = initial->y;

    for (int i = 0; i < nCyclesMax; i++)
    {
        float x_2 = x * x;
        float y_2 = y * y;
        float xy  = x * y;

        float r_2 = x_2 + y_2;

        if (r_2 >= radiusMax_2) return i;

        x = x_2 - y_2 + initial->x;
        y = xy  + xy  + initial->y;
    }

    return nCyclesMax;
}

RGBQUAD DetermineColor (int n)
{
    float I = sqrtf (sqrtf ((float)n / (float)nCyclesMax)) * 255.f;

    BYTE    c     = (BYTE) I;
    RGBQUAD color = (n < nCyclesMax)
        ? RGBQUAD { (BYTE) (255 - c), (BYTE) (c % 2 * 64), c}
        : RGBQUAD { };

    return color;
}

CheckKeyStateStatus
CheckKeyState (Coordinates* center, float* scale)
{
    assert (center);
    assert (scale);

    if (GetAsyncKeyState (VK_EXIT)) return EXIT;

    float shift = minShift * (txGetAsyncKeyState (VK_SHIFT) ? multiplierShift : 1);

    if (txGetAsyncKeyState (VK_RIGHT)) center->x += shift * *scale * dx;
    if (txGetAsyncKeyState (VK_LEFT))  center->x -= shift * *scale * dx;
    if (txGetAsyncKeyState (VK_UP))    center->y += shift * *scale * dy;
    if (txGetAsyncKeyState (VK_DOWN))  center->y -= shift * *scale * dy;
    if (txGetAsyncKeyState ('A'))      *scale    *= multiplierZoom;
    if (txGetAsyncKeyState ('Z'))      *scale    /= multiplierZoom;

    return CONTINUE;
}

