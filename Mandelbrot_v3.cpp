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

inline void mm_set_ps (float mm[4], float val0, float val1,
                       float val2, float val3)
{
    mm[0] = val0;
    mm[1] = val1;
    mm[2] = val2;
    mm[3] = val3;
}

inline void mm_set_ps1 (float mm[4], float val)
{
    for (int i = 0; i < 4; i++) mm[i] = val;
}

inline void mm_cpy_ps  (float mm[4], const float mm1[4])
{
    for (int i = 0; i < 4; i++) mm[i] = mm1[i];
}

inline void mm_add_ps  (float mm[4], const float mm1[4], const float mm2[4])
{
    for (int i = 0; i < 4; i++) mm[i] = mm1[i] + mm2[i];
}

inline void mm_sub_ps  (float mm[4], const float mm1[4], const float mm2[4])
{
    for (int i = 0; i < 4; i++) mm[i] = mm1[i] - mm2[i];
}

inline void mm_mul_ps  (float mm[4], const float mm1[4], const float mm2[4])
{
    for (int i = 0; i < 4; i++) mm[i] = mm1[i] * mm2[i];
}

inline void mm_add_epi32 (int mm[4], const int   mm1[4], const float mm2[4])
{
    for (int i = 0; i < 4; i++) mm[i] = mm1[i] + (int)mm2[i];
}

inline void mm_cmple_ps (float mm[4], const float mm1[4], const float mm2[4])
{
    for (int i = 0; i < 4; i++)
        if (mm1[i] < mm2[i]) mm[i] = 1;
}

inline int  mm_movemask_ps (const float cmp[4])
{
    int mask = 0;
    for (int i = 0; i < 4; i++) mask |= (!!cmp[i] << i);
    return mask;
}

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

                FindNIteration (&initial, &iteration[0]);

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

    float r_2Max [nCounts] = { 0 };
    float numbers[nCounts] = { 0 };

    mm_set_ps1 (r_2Max, radiusMax_2);
    mm_set_ps  (numbers, 0, 1, 2, 3);


    float dxArray[nCounts] = { 0 };

    mm_set_ps1 (dxArray, dx);
    mm_mul_ps  (dxArray, dxArray, numbers);


    float x0[nCounts] = { 0 };
    float y0[nCounts] = { 0 };

    mm_set_ps1 (x0, initial->x);
    mm_add_ps  (x0, x0, dxArray);
    mm_set_ps1 (y0, initial->y);


    float x[nCounts] = { 0 };
    float y[nCounts] = { 0 };

    mm_cpy_ps (x, x0);
    mm_cpy_ps (y, y0);


    int mask = 0;
    for (int i = 0; i < nCounts; i++) mask |= (1 << i);

    for (int i = 0; i < nCyclesMax; i++)
    {
        float x_2[nCounts] = { 0 };
        float y_2[nCounts] = { 0 };
        float xy [nCounts] = { 0 };
        float r_2[nCounts] = { 0 };

        mm_mul_ps (x_2, x, x);
        mm_mul_ps (y_2, y, y);
        mm_mul_ps (xy,  x, y);
        mm_add_ps (r_2, x_2, y_2);

        float cmp[4] = { 0 };
        mm_cmple_ps (cmp, r_2, r_2Max);
        mask = mm_movemask_ps (cmp);

        if (mask == 0) return 0;
        mm_add_epi32 (n, n, cmp);

        mm_sub_ps (x, x_2, y_2);
        mm_add_ps (x, x,   x0);
        mm_add_ps (y, xy,  xy);
        mm_add_ps (y, y,   y0);
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
