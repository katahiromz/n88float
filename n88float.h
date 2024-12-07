#pragma once

#include "types.h"

VskString vsk_make_integer(VskInt value);       // MKI$
VskString vsk_make_single(VskSingle value);     // MKS$
VskString vsk_make_double(VskDouble value);     // MKD$

VskInt vsk_convert_integer(const VskString& str);   // CVI
VskSingle vsk_convert_single(const VskString& str); // CVS
VskDouble vsk_convert_double(const VskString& str); // CVD

void vsk_make_convert_tests(void); // tests
