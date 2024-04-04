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

const Coordinates initialShift = { .x = -1.325f, .y = 0 };

const int   windowWidth     = 800;
const int   windowHeight    = 600;
const float dx              = 1.f / windowWidth;
const float dy              = 1.f / windowHeight;
const float radiusMax_2     = 100;
const int   nCyclesMax      = 256;
const float minShift        = 10.f;
const int   multiplierShift = 10;

const int nCounts = 4;

void
DrawMagicBeauty ();

CheckKeyStateStatus
CheckKeyState   (Coordinates* center, float* scale);

int
FindNIteration  (Coordinates* initial, int* n);

RGBQUAD
DetermineColor  (int n);

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

    Coordinates center     = { .x = 0, .y = 0 };
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

            initial.x = (          - windowWidth  / 2 + center.x) *
                dx * scale * initScale.x + initialShift.x;

            initial.y = ((float)iy - windowHeight / 2 + center.y) *
                dy * scale * initScale.y + initialShift.y;

            for (int ix = 0; ix < windowWidth;
                ix += nCounts, initial.x += nCounts * dx * scale)
            {
                int iteration[nCounts] = { 0 };

                FindNIteration (&initial, &(iteration[0]));

                for (int i = 0; i < nCounts; i++)
                {
                    scr[iy][ix + i] = DetermineColor (iteration[i]);
                }
            }
        }

        printf ("\t\r%.0lf", txGetFPS ());
        txSleep ();
    }
}

int
FindNIteration (Coordinates* initial, int* n)
{
    assert (initial);

    Coordinates initialArray[nCounts] = { 0 };

    for (int i = 0; i < nCounts; i++)
    {
        initialArray[i].x = initial->x + (float)i * dx;
        initialArray[i].y = initial->y;
    }

    float x[nCounts] = { 0 };
    float y[nCounts] = { 0 };

    for (int i = 0; i < nCounts; i++) x[i] = initialArray[i].x;
    for (int i = 0; i < nCounts; i++) y[i] = initialArray[i].y;

    int mask = 0;
    for (int i = 0; i < nCounts; i++) mask |= (1 << i);

    for (int i = 0; i < nCyclesMax; i++)
    {
        float x_2[nCounts] = { 0 };
        float y_2[nCounts] = { 0 };
        float xy [nCounts] = { 0 };
        float r_2[nCounts] = { 0 };

        for (int j = 0; j < nCounts; j++) x_2[j] = x[j] * x[j];
        for (int j = 0; j < nCounts; j++) y_2[j] = y[j] * y[j];
        for (int j = 0; j < nCounts; j++) xy [j] = x[j] * y[j];
        for (int j = 0; j < nCounts; j++) r_2[j] = x_2[j] + y_2[j];

        for (int j = 0; j < nCounts; j++)
        {
            if (mask & (1 << j) && r_2[j] >= radiusMax_2)
            {
                n[j]  = i;
                mask &= ~(1 << j);
            }
        }

        if (mask == 0) return 0;

        for (int j = 0; j < nCounts; j++) x[j] = x_2[j] - y_2[j] + initialArray[j].x;
        for (int j = 0; j < nCounts; j++) y[j] = xy [j] + xy [j] + initialArray[j].y;
    }

    for (int j = 0; j < nCounts; j++)
    {
        if (mask & (1 << j))
        {
            n[j] = nCyclesMax;
        }
    }

    return 0;
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

    if (txGetAsyncKeyState (VK_RIGHT)) center->x += shift;
    if (txGetAsyncKeyState (VK_LEFT))  center->x -= shift;
    if (txGetAsyncKeyState (VK_UP))    center->y += shift;
    if (txGetAsyncKeyState (VK_DOWN))  center->y -= shift;
    if (txGetAsyncKeyState ('A'))      *scale    += shift * dy;
    if (txGetAsyncKeyState ('Z'))      *scale    -= shift * dy;

    return CONTINUE;
}
