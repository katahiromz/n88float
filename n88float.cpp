// N88-BASICの浮動小数点数をエミュレートする
// MKI$ / MKS$ / MKD$ / CVI / CVS / CVD
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits>
#include "n88float.h"

#define IEEE752_SINGLE_BIAS     127
#define IEEE752_DOUBLE_BIAS     1023
#define N88_SINGLE_BIAS         128
#define N88_DOUBLE_BIAS         128
#define SINGLE_BIAS_OFFSET      (N88_SINGLE_BIAS - IEEE752_SINGLE_BIAS + 1)
#define DOUBLE_BIAS_OFFSET      (N88_DOUBLE_BIAS - IEEE752_DOUBLE_BIAS + 1)

typedef union tagIEEE754Single
{
    VskSingle f;
    uint32_t n;
} IEEE754Single;

typedef union tagIEEE754Double
{
    VskDouble f;
    uint64_t n;
} IEEE754Double;

typedef union tagN88Single
{
    uint8_t bytes[4];
    uint32_t n;
} N88Single;

typedef union tagN88Double
{
    uint8_t bytes[8];
    uint64_t n;
} N88Double;

// MKI$
VskString vsk_make_integer(VskInt value)
{
    return VskString(reinterpret_cast<char*>(&value), sizeof(value));
}

// MKS$
VskString vsk_make_single(VskSingle value)
{
    IEEE754Single ieee754;
    N88Single n88;

    ieee754.f = value;

    uint32_t s = (ieee754.n >> 31) & 0x00000001;
    uint32_t e = (ieee754.n >> 23) & 0x000000FF;
    uint32_t m = ieee754.n & 0x007FFFFF;

    if (std::isnan(ieee754.f))
    {
        n88.n = 0xFF000000;
    }
    else if (std::isinf(ieee754.f) || e + SINGLE_BIAS_OFFSET > 254)
    {
        n88.n = ieee754.f > 0 ? 0xFF7FFFFF : 0xFFFFFFFF;
    }
    else if (e == 0 && m == 0)
    {
        n88.n = s << 23;
    }
    else
    {
        n88.n = (s << 23) | (e + SINGLE_BIAS_OFFSET << 24) | m;
    }

    return VskString(reinterpret_cast<char*>(n88.bytes), sizeof(n88.bytes));
}

// MKD$
VskString vsk_make_double(VskDouble value)
{
    IEEE754Double ieee754;
    N88Double n88;

    ieee754.f = value;

    uint64_t s = (ieee754.n >> 63) & 0x0000000000000001;
    uint64_t e = (ieee754.n >> 52) & 0x00000000000007FF;
    uint64_t m = ieee754.n & 0x000FFFFFFFFFFFFF;

    if (std::isnan(ieee754.f))
    {
        n88.n = 0xFF00000000000000;
    }
    else if ((e == 0 && m == 0) || e <= -DOUBLE_BIAS_OFFSET)
    {
        n88.n = s << 55;
    }
    else if (std::isinf(ieee754.f) || e + DOUBLE_BIAS_OFFSET > 254)
    {
        n88.n = ieee754.f > 0 ? 0xFF7FFFFFFFFFFFFF : 0xFFFFFFFFFFFFFFFF;
    }
    else
    {
        n88.n = (s << 55) | ((e + DOUBLE_BIAS_OFFSET) << 56) | (m << 3);
    }

    return VskString(reinterpret_cast<char*>(n88.bytes), sizeof(n88.bytes));
}

// CVI
VskInt vsk_convert_integer(const VskString& str)
{
    assert(str.size() == sizeof(VskInt));
    VskInt ret;
    std::memcpy(&ret, str.c_str(), sizeof(ret));
    return ret;
}

// CVS
VskSingle vsk_convert_single(const VskString& str)
{
    assert(str.size() == sizeof(VskSingle));

    N88Single n88;
    IEEE754Single ieee754;

    std::memcpy(&n88.bytes, str.c_str(), sizeof(N88Single));

    uint32_t s = (n88.n >> 23) & 0x00000001;
    uint32_t e = (n88.n >> 24) & 0x000000FF;
    uint32_t m = n88.n & 0x007FFFFF;

    if (e == 0x000000FF) 
    {
        if (m == 0)
        {
            ieee754.f = NAN;
        }
        else
        {
            ieee754.f = s == 0 ? INFINITY : -INFINITY;
        }
    }
    else if (n88.n == 0x00000000)
    {
        ieee754.f = 0.0f;
    }
    else if (n88.n == 0x00800000)
    {
        ieee754.f = -0.0f;
    }
    else
    {
        ieee754.n = (s << 31) | (e - SINGLE_BIAS_OFFSET) << 23 | m;
    }

    return ieee754.f;
}

// CVD
VskDouble vsk_convert_double(const VskString& str)
{
    assert(str.size() == sizeof(VskDouble));
    N88Double n88;
    IEEE754Double ieee754;

    std::memcpy(&n88.bytes, str.c_str(), sizeof(N88Double));

    uint64_t s = (n88.n >> 55) & 0x0000000000000001;
    uint64_t e = (n88.n >> 56) & 0x00000000000000FF;
    uint64_t m = n88.n & 0x007FFFFFFFFFFFFF;

    if (e == 0x00000000000000FF)
    {
        if (m == 0)
        {
            ieee754.f = NAN;
        }
        else
        {
            ieee754.f = s == 0 ? INFINITY : -INFINITY;
        }
    }
    else if (n88.n == 0x0000000000000000)
    {
        ieee754.f = 0.0;
    }
    else if (n88.n == 0x0080000000000000)
    {
        ieee754.f = -0.0;
    }
    else
    {
        ieee754.n = (s << 63) | (e - DOUBLE_BIAS_OFFSET) << 52 | (m >> 3);
    }

    return ieee754.f;
}

void vsk_make_integer_tests(void)
{
    VskString str;

    str = vsk_make_integer(0);
    assert(std::memcmp(str.c_str(), "\x00\x00", sizeof(VskInt)) == 0);

    str = vsk_make_integer(1);
    assert(std::memcmp(str.c_str(), "\x01\x00", sizeof(VskInt)) == 0);

    str = vsk_make_integer(2);
    assert(std::memcmp(str.c_str(), "\x02\x00", sizeof(VskInt)) == 0);

    str = vsk_make_integer(-2);
    assert(std::memcmp(str.c_str(), "\xFE\xFF", sizeof(VskInt)) == 0);
}

void vsk_convert_integer_tests(void)
{
    VskString str;

    str.assign("\x00\x00", sizeof(VskInt));
    assert(vsk_convert_integer(str) == 0);

    str.assign("\x01\x00", sizeof(VskInt));
    assert(vsk_convert_integer(str) == 1);

    str.assign("\x02\x00", sizeof(VskInt));
    assert(vsk_convert_integer(str) == 2);

    str.assign("\xFE\xFF", sizeof(VskInt));
    assert(vsk_convert_integer(str) == -2);
}

void vsk_make_single_tests(void)
{
    VskString str;

    str = vsk_make_single(0.0f);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00", sizeof(VskSingle)) == 0);

    str = vsk_make_single(1.0f);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x81", sizeof(VskSingle)) == 0);

    str = vsk_make_single(-1.0f);
    assert(std::memcmp(str.c_str(), "\x00\x00\x80\x81", sizeof(VskSingle)) == 0);

    str = vsk_make_single(2.0f);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x82", sizeof(VskSingle)) == 0);

    str = vsk_make_single(100.0f);
    assert(std::memcmp(str.c_str(), "\x00\x00\x48\x87", sizeof(VskSingle)) == 0);

    str = vsk_make_single(1.2f);
    assert(std::memcmp(str.c_str(), "\x9A\x99\x19\x81", sizeof(VskSingle)) == 0);

    str = vsk_make_single(-2.2f);
    assert(std::memcmp(str.c_str(), "\xCD\xCC\x8C\x82", sizeof(VskSingle)) == 0);

    str = vsk_make_single(INFINITY);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\x7F\xFF", sizeof(VskSingle)) == 0);

    str = vsk_make_single(-INFINITY);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\xFF\xFF", sizeof(VskSingle)) == 0);

    str = vsk_make_single(NAN);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\xFF", sizeof(VskSingle)) == 0);

    str = vsk_make_single(-0.0f);
    assert(std::memcmp(str.c_str(), "\x00\x00\x80\x00", sizeof(VskSingle)) == 0);

    str = vsk_make_single(8.50705867E+37f);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\x7F\xFE", sizeof(VskSingle)) == 0);

    str = vsk_make_single(-8.50705867E+37f);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\xFF\xFE", sizeof(VskSingle)) == 0);

    str = vsk_make_single(8.6E+37f);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\x7F\xFF", sizeof(VskSingle)) == 0);

    str = vsk_make_single(-8.6E+37f);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\xFF\xFF", sizeof(VskSingle)) == 0);
}

// ほぼ等しい
inline bool nearly_equal_single(float x0, float x1)
{
    return fabs(x1 - x0) < 0.0001;
}

void vsk_convert_single_tests(void)
{
    VskString str;

    str.assign("\x00\x00\x00\x00", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), 0.0f));

    str.assign("\x00\x00\x00\x81", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), 1.0f));

    str.assign("\x00\x00\x80\x81", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), -1.0f));

    str.assign("\x00\x00\x00\x82", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), 2.0f));

    str.assign("\x00\x00\x48\x87", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), 100.0f));

    str.assign("\x9A\x99\x19\x81", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), 1.2f));

    str.assign("\xCD\xCC\x8C\x82", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), -2.2f));

    VskSingle value;

    str.assign("\xFF\xFF\x7F\xFF", sizeof(VskSingle));
    value = vsk_convert_single(str);
    assert(std::isinf(value) && value > 0);

    str.assign("\xFF\xFF\xFF\xFF", sizeof(VskSingle));
    value = vsk_convert_single(str);
    assert(std::isinf(value) && value < 0);

    str.assign("\x00\x00\x00\xFF", sizeof(VskSingle));
    value = vsk_convert_single(str);
    assert(std::isnan(value));

    str.assign("\x00\x00\x80\x00", sizeof(VskSingle));
    value = vsk_convert_single(str);
    assert(value == -0.0f);

    str.assign("\xFF\xFF\x7F\xFE", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), 8.50705867E+37f));

    str.assign("\xFF\xFF\xFF\xFE", sizeof(VskSingle));
    assert(nearly_equal_single(vsk_convert_single(str), -8.50705867E+37f));
}

void vsk_make_double_tests(void)
{
    VskString str;

    str = vsk_make_double(0.0);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(VskDouble)) == 0);

    str = vsk_make_double(1.0);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x00\x81", sizeof(VskDouble)) == 0);

    str = vsk_make_double(-1.0);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x80\x81", sizeof(VskDouble)) == 0);

    str = vsk_make_double(2.0);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x00\x82", sizeof(VskDouble)) == 0);

    str = vsk_make_double(100.0);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x48\x87", sizeof(VskDouble)) == 0);

    //str = vsk_make_double(1.2);
    //assert(std::memcmp(str.c_str(), "\x9A\x99\x99\x99\x99\x99\x19\x81", sizeof(VskDouble)) == 0);

    str = vsk_make_double(1.2);
    assert(std::memcmp(str.c_str(), "\x98\x99\x99\x99\x99\x99\x19\x81", sizeof(VskDouble)) == 0);

    str = vsk_make_double(1.25);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x20\x81", sizeof(VskDouble)) == 0);

    //str = vsk_make_double(-2.2);
    //assert(std::memcmp(str.c_str(), "\xCD\xCC\xCC\xCC\xCC\xCC\x8C\x82", sizeof(VskDouble)) == 0);

    str = vsk_make_double(-2.2);
    assert(std::memcmp(str.c_str(), "\xD0\xCC\xCC\xCC\xCC\xCC\x8C\x82", sizeof(VskDouble)) == 0);

    str = vsk_make_double(-2.5);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\xA0\x82", sizeof(VskDouble)) == 0);

    str = vsk_make_double(INFINITY);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\xFF\xFF\xFF\xFF\x7F\xFF", sizeof(VskDouble)) == 0);

    str = vsk_make_double(-INFINITY);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", sizeof(VskDouble)) == 0);

    str = vsk_make_double(NAN);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x00\xFF", sizeof(VskDouble)) == 0);

    str = vsk_make_double(-0.0);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x80\x00", sizeof(VskDouble)) == 0);

    str = vsk_make_double(8.5070591730234606E+37);
    assert(std::memcmp(str.c_str(), "\xF8\xFF\xFF\xFF\xFF\xFF\x7F\xFE", sizeof(VskSingle)) == 0);

    str = vsk_make_double(-8.5070591730234606E+37);
    assert(std::memcmp(str.c_str(), "\xF8\xFF\xFF\xFF\xFF\xFF\xFF\xFE", sizeof(VskSingle)) == 0);

    str = vsk_make_double(8.6E+37);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\xFF\xFF\xFF\xFF\x7F\xFF", sizeof(VskSingle)) == 0);

    str = vsk_make_double(-8.6E+37);
    assert(std::memcmp(str.c_str(), "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", sizeof(VskSingle)) == 0);

    str = vsk_make_double(2.9387358770557188e-39);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x00\x01", sizeof(VskSingle)) == 0);

    str = vsk_make_double(2.9387358770557000e-39);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(VskSingle)) == 0);

    str = vsk_make_double(-2.9387358770557188e-39);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x80\x01", sizeof(VskSingle)) == 0);

    str = vsk_make_double(-2.9387358770557000e-39);
    assert(std::memcmp(str.c_str(), "\x00\x00\x00\x00\x00\x00\x80\x00", sizeof(VskSingle)) == 0);
}

// ほぼ等しい
inline bool nearly_equal_double(double x0, double x1)
{
    return fabs(x1 - x0) < 0.0001;
}

void vsk_convert_double_tests(void)
{
    VskString str;

    str.assign("\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), 0));

    str.assign("\x00\x00\x00\x00\x00\x00\x00\x81", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), 1.0));

    str.assign("\x00\x00\x00\x00\x00\x00\x80\x81", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), -1.0));

    str.assign("\x00\x00\x00\x00\x00\x00\x00\x82", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), 2.0));

    str.assign("\x00\x00\x00\x00\x00\x00\x48\x87", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), 100.0));

    str.assign("\x9A\x99\x99\x99\x99\x99\x19\x81", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), 1.2));

    str.assign("\xCD\xCC\xCC\xCC\xCC\xCC\x8C\x82", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), -2.2));

    VskDouble value;

    str.assign("\xFF\xFF\xFF\xFF\xFF\xFF\x7F\xFF", sizeof(VskDouble));
    value = vsk_convert_double(str);
    assert(std::isinf(value) && value > 0);

    str.assign("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", sizeof(VskDouble));
    value = vsk_convert_double(str);
    assert(std::isinf(value) && value < 0);

    str.assign("\x00\x00\x00\x00\x00\x00\x00\xFF", sizeof(VskDouble));
    value = vsk_convert_double(str);
    assert(std::isnan(value));

    str.assign("\x00\x00\x00\x00\x00\x00\x80\x00", sizeof(VskDouble));
    value = vsk_convert_double(str);
    assert(value == -0.0f);

    str.assign("\xF8\xFF\xFF\xFF\xFF\xFF\x7F\xFE", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), 8.5070591730234606E+37));

    str.assign("\xF8\xFF\xFF\xFF\xFF\xFF\xFF\xFE", sizeof(VskDouble));
    assert(nearly_equal_double(vsk_convert_double(str), -8.5070591730234606E+37));
}

void vsk_make_convert_tests(void)
{
    vsk_make_integer_tests();
    vsk_convert_integer_tests();

    vsk_make_single_tests();
    vsk_convert_single_tests();

    vsk_make_double_tests();
    vsk_convert_double_tests();
}

#ifdef N88FLOAT_EXE
int main(void)
{
    vsk_make_convert_tests();
    return 0;
}
#endif
