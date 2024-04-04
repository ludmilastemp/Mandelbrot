#include "TXLib.h"
#include <emmintrin.h>

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
const int   multiplierShift = 3;
const float multiplierZoom  = 1.1f;

const int nCounts = 128 / (sizeof (float) * CHAR_BIT);

void
DrawMagicBeauty ();

CheckKeyStateStatus
CheckKeyState  (Coordinates* center, float* scale);

__m128i
FindNIteration (Coordinates* initial, float scale);

__m128
CountIterationToColor (__m128i nIterations);

RGBQUAD
DetermineColor (int n, float f);

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
    scr_t screen =  (scr_t) *txVideoMemory();

    Coordinates center     = { .x = 0,
                               .y = 0 };
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

            float dxMulScale = dx * scale;

            initial.x = (          - windowWidth  / 2) * dx * scale * initScale.x + center.x;
            initial.y = ((float)iy - windowHeight / 2) * dy * scale * initScale.y + center.y;

            for (int ix = 0; ix < windowWidth;
                ix += nCounts, initial.x += nCounts * dx * scale)
            {
                union
                {
                    __m128i m128i;
                    int32_t arr[4];
                } iters;

                iters.m128i = FindNIteration (&initial, scale);
                __m128 I    = CountIterationToColor (iters.m128i);

                int   a = 0;
                float b = 0;
                RGBQUAD color = { 0 };

                for (int i = 0; i < 4; i++)
                {
                    a = iters.arr[i];
                    b = (((float*)&I)[i]);

                    color = DetermineColor (a, b);

                    screen[iy][ix + i] = color;
                }
            }
        }

        printf ("\t\rfps  = %.0lf", txGetFPS ());
        txSleep ();
    }
}

__m128i
FindNIteration (Coordinates* initial, float scale)
{
    assert (initial);

    __m128i nIterations = { 0 };

    __m128 dxArray = _mm_set_ps1 (dx);
    dxArray = _mm_mul_ps  (dxArray, _mm_set_ps  (3, 2, 1, 0));
    dxArray = _mm_mul_ps  (dxArray, _mm_set_ps1 (scale));

    __m128 x0 = { 0 };
    __m128 y0 = { 0 };

    x0 = _mm_set_ps1 (initial->x);
    x0 = _mm_add_ps  (_mm_set_ps1 (initial->x), dxArray);
    y0 = _mm_set_ps1 (initial->y);

    __m128 x = x0;
    __m128 y = y0;

    for (int i = 0; i <= nCyclesMax; i++)
    {
        __m128 x_2 = _mm_mul_ps (x, x);
        __m128 y_2 = _mm_mul_ps (y, y);
        __m128 xy  = _mm_mul_ps (x, y);
        __m128 r_2 = _mm_add_ps (x_2, y_2);

        __m128 cmp = _mm_cmple_ps (r_2, _mm_set_ps1 (radiusMax_2));
        nIterations = _mm_sub_epi32 (nIterations, _mm_castps_si128 (cmp));

        int mask = _mm_movemask_ps (cmp);
        if (mask == 0) return nIterations;

        x = _mm_sub_ps (x_2, y_2);
        x = _mm_add_ps (x,   x0);
        y = _mm_add_ps (xy,  xy);
        y = _mm_add_ps (y,   y0);
    }

    return nIterations;
}

__m128
CountIterationToColor (__m128i nIterations)
{
    __m128 I = _mm_mul_ps (_mm_sqrt_ps (_mm_sqrt_ps (_mm_div_ps (_mm_cvtepi32_ps (nIterations),
                                                                 _mm_set_ps1 ((float)nCyclesMax)
                                                                )
                                                    )
                                       ),
                           _mm_set_ps1 (255.f));
    return I;
}

RGBQUAD
DetermineColor (int n, float f)
{
    BYTE    c     = (BYTE) f;
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
