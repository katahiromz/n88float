#pragma once

#include <cstdint>
#include <string>

typedef int16_t VskInt;
typedef float VskSingle;
typedef double VskDouble;
typedef std::string VskString;

static_assert(sizeof(VskInt) == 2, "");
static_assert(sizeof(VskSingle) == 4, "");
static_assert(sizeof(VskDouble) == 8, "");
