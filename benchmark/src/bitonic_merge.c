#include <inttypes.h>
#include <emmintrin.h>

int32_t data1[4096];
int32_t data2[4096];

void bitonic_merge_setup()
{
    int i;

    for (i=0; i<4096; i++) {
        data1[i] = i;
        data2[i] = 4095-i;
    }
}

int bitonic_merge_check()
{
    int i;
    int count = 0;

    for (i=0; i<4096; i++) {
        if (data1[i] > data2[i])
            count++;
        if (data1[i] != i && data2[i] != i)
            count++;
        if (data1[i] != 4095-i && data2[i] != 4095-i)
            count++;
    }
    return count;
}

void bitonic_merge_test1()
{
    int i;

    for (i=0; i<4096; i++) {
        if (data1[i] > data2[i]) {
            int32_t temporary;
            temporary = data1[i];
            data1[i] = data2[i];
            data2[i] = temporary;
        }
    }
}

void bitonic_merge_test2()
{
    int i;
    int32_t * __restrict rdata1 = data1;
    int32_t * __restrict rdata2 = data2;

    for (i=0; i<4096; i++) {
        if (data1[i] > data2[i]) {
            int32_t temporary;
            temporary = data1[i];
            rdata1[i] = data2[i];
            rdata2[i] = temporary;
        }
    }
}

void bitonic_merge_test3()
{
    int i;

    for(i = 0; i < 4096; i += 4) {
        __m128i v0, v1, v2, v3;
        __m128i w0, w1, w2, w3;
        __m128i c0;

        v0 = _mm_load_si128((__m128i *)(data1 + i));
        w0 = _mm_load_si128((__m128i *)(data2 + i));

        c0 = _mm_cmpgt_epi32(v0, w0);

        v1 = _mm_andnot_si128(c0, v0);
        w1 = _mm_and_si128(c0, w0);
        v3 = _mm_add_epi32(v1, w1);

        v2 = _mm_and_si128(c0, v0);
        w2 = _mm_andnot_si128(c0, w0);
        w3 = _mm_add_epi32(v2, w2);

        _mm_store_si128((__m128i *)(data1 + i), v3);
        _mm_store_si128((__m128i *)(data2 + i), w3);
    }
}

void bitonic_merge_test4()
{
    int i;

    for(i = 0; i < 4096; i += 8) {
        __m128i v0, v1, v2, v3;
        __m128i x0, x1, x2, x3;
        __m128i w0, w1, w2, w3;
        __m128i y0, y1, y2, y3;
        __m128i c0, c1;

        v0 = _mm_load_si128((__m128i *)(data1 + i));
        x0 = _mm_load_si128((__m128i *)(data1 + i + 4));
        w0 = _mm_load_si128((__m128i *)(data2 + i));
        y0 = _mm_load_si128((__m128i *)(data2 + i + 4));

        c0 = _mm_cmpgt_epi32(v0, w0);
        c1 = _mm_cmpgt_epi32(x0, y0);

        v1 = _mm_andnot_si128(c0, v0);
        w1 = _mm_and_si128(c0, w0);
        v3 = _mm_add_epi32(v1, w1);

        v2 = _mm_and_si128(c0, v0);
        w2 = _mm_andnot_si128(c0, w0);
        w3 = _mm_add_epi32(v2, w2);

        x1 = _mm_andnot_si128(c1, x0);
        y1 = _mm_and_si128(c1, y0);
        x3 = _mm_add_epi32(x1, y1);

        x2 = _mm_and_si128(c1, x0);
        y2 = _mm_andnot_si128(c1, y0);
        y3 = _mm_add_epi32(x2, y2);

        _mm_store_si128((__m128i *)(data1 + i), v3);
        _mm_store_si128((__m128i *)(data1 + i + 4), x3);
        _mm_store_si128((__m128i *)(data2 + i), w3);
        _mm_store_si128((__m128i *)(data2 + i + 4), y3);
    }
}

void bitonic_merge_copy()
{
    int i;

    for(i = 0; i < 4096; i += 8) {
        __m128i v0, v1;

        v0 = _mm_load_si128((__m128i *)(data1 + i));
        v1 = _mm_load_si128((__m128i *)(data1 + i + 4));

        _mm_store_si128((__m128i *)(data2 + i), v0);
        _mm_store_si128((__m128i *)(data2 + i + 4), v1);
    }
}

void bitonic_empty_function()
{
}
