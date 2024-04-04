# ����������� ������ ��������� �� ����� C � ������� ��������� �������������� ���������� SIMD / AVX (Advanced Vector eXtensions)

### _������������ ����������� �� ������� ��������� ������� �������� "��������� ������������"_

## ���������
� ������ ���������� ��������� ������� ���������� ���� ��� ������������� SIMD ����������. ��������� ���������� �� ���������� � 4 ������� �������� ���������, � ����� � �������������� ������ ����������� ����������� -O2 � -O3.

## ����������� ����������
| �������           |     ��������                         |
|-------------------|--------------------------------------|
| ������� �����      |����� �����                           |
| ������� ����       |����� ����                            |
| ������� �����      |����� �����                           |
| ������� ������     |����� ������                          |
|   A               |���������� �����������                |
|   Z               |���������� �����������                |
|  shift            |���������� �������� ����������     |

## ������������� �������
### ��������� ������������
��������� ������������ - �������, ��������� ����� �� ����������� ���������, ��� ������� ������ ������������ ����������� z_(n+1) = z_n 2 + z_0. ������������ ������ ��������� ������������: ��������� �����, ��� ������� ���������� �������������� R, ����� ��� ��� ����� ����������� n ����������� |z_n|< R.
� ������ ���������� ��������� (x, y) �������������� �� ��������� �������. x = x 2 + y 2 + x_0, y = 2xy + y_0. ��� ��������� ���������� x 2, y 2 �������������� ���� ���.

### AVX ����������
���������� AVX (Advanced Vector eXtensions � ������������������� ��������� ����������) - �������������� ����������, ��������������� �� ������������ ���������� ���������� �������� � ������ SIMD (Single-Instruction, Multiple-Data - ��������� ����� ������, ������������� ����� ������) � �� ������ �� ���������� ������� � ��������� ������. ���������� avx ���������� ���� ������� ��� ��������� ������� � �����������. ������ � �� ������� ������������ ��������� �������� � ��������� ������ � ����������� x86-64.

## ������� �� ��������� ��������� ������������ (���������)
���� �������� ������� �� ���������� �������� �������� ��������� ��� ������ ����� z_0. ���� �������� � ������ RGB. ����������� ����� �� ���������� �������� ��������� � �������:

## �������� ���������
������� ���������� �������� ��� ������ ���������� ������������ 10 ���, ����� ���� ������� ������� ����� ������ ���������. ����� ��������� ���������.

## ������������ ������������:
���������: 12th Gen Intel(R) Core(TM) i7-1255U  1.70 GHz
����������� ������: 16,0 ��
��: Windows 10
����������: g++.exe (GCC) 4.8.1
������ �����������: O0, O1, O2, O3
������ ������ ������: -msse3 -mmmx -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

## ����������
### ������ ������ ��������� (�����������)
������ ���������� ��������� ��������.
```
float x_2 = x * x;
float y_2 = y * y;
float xy  = x * y;
float r_2 = x_2 + y_2;

if (r_2 >= radiusMax_2) return i;
```
### ������ ������ ��������� (�� �������)
��� �������� ���������� ��� ������� �� 4 ����� � ������� for
```
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
```
### ������ ������ ��������� (���� ����������)
����� ���� ������, � �������� � ��� ����������� � ������� inline �������
```
inline void mm_add_ps  (float mm[4], const float mm1[4], const float mm2[4])
{
    for (int i = 0; i < 4; i++) mm[i] = mm1[i] + mm2[i];
}
inline void mm_mul_ps  (float mm[4], const float mm1[4], const float mm2[4])
{
    for (int i = 0; i < 4; i++) mm[i] = mm1[i] * mm2[i];
}
...
mm_mul_ps (x_2, x, x);
mm_mul_ps (y_2, y, y);
mm_mul_ps (xy,  x, y);
mm_add_ps (r_2, x_2, y_2);

float cmp[4] = { 0 };
mm_cmple_ps (cmp, r_2, r_2Max);
mask = mm_movemask_ps (cmp);
if (mask == 0) return nIterations;
```

### ��������� ������ ��������� (SIMD ����������)
������������� SIMD ����������
```
__m128 x_2 = _mm_mul_ps (x, x);
__m128 y_2 = _mm_mul_ps (y, y);
__m128 xy  = _mm_mul_ps (x, y);
__m128 r_2 = _mm_add_ps (x_2, y_2);

__m128 cmp = _mm_cmple_ps (r_2, _mm_set_ps1 (radiusMax_2));
nIterations = _mm_sub_epi32 (nIterations, _mm_castps_si128 (cmp));

int mask = _mm_movemask_ps (cmp);
if (mask == 0) return nIterations;
```

## ���������� ��������� � ��������� ������
��������� ������������� � ������� ������� __rdtsc. ������ ��������� ������������ � �������:
������ ����������:
| | -O0 | -O1 | -O2 | -O3 |
| --- | --- | --- | --- | --- |
| v1, 1e4 cycles |43185 +- 529 | 21489 +- 583 | 21399 +- 26 | 21166 +- 64 |
| v2, 1e4 cycles |68054 +- 131 | 32754 +- 124 | 21709 +- 168 | 8986 +- 23 |
| v3, 1e4 cycles |144593 +- 1013 | 46325 +- 158 | 14637 +- 27 | 9456 +- 25 |
| v4, 1e4 cycles |23204 +- 225 | 5804 +- 17 | 5990 +- 18 | 5988 +- 16 |
<img src="img/plot.png">
������ ���� ��������� ����� ���������� � ����� tests

## ������ ������������
� ��������� �������� �� ���������� ���������:
```
unsigned long long* data          = ReadData (nTest);
unsigned long long  meanData      = SUM (data[i]) / nCycle;
unsigned long long* deviation     = (data[i] - meanData) ^ 2;
unsigned long long  meanDeviation = SUM (deviation[i]) / nCycle;
unsigned long long  meanSigma     = sqrt (meanDeviation);
```
���������:
| | -O0 | -O1 | -O2 | -O3 |
| --- | --- | --- | --- | --- |
| v1, 1e4 cycles |529| 583 | 26 | 64 |
| v2, 1e4 cycles |131| 124 | 168 | 23 |
| v3, 1e4 cycles |1013| 158 | 27 | 25 |
| v4, 1e4 cycles |225| 17 | 18 | 16 |


## ������
��������� ���������� � ~3.5 ���. ���������� ���!!!
