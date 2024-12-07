#pragma once

#include "types.h"

// MKI$
VskString vsk_make_integer(VskInt value);
// MKS$
VskString vsk_make_single(VskSingle value);
// MKD$
VskString vsk_make_double(VskDouble value);
// CVI
VskInt vsk_convert_integer(const VskString& str);
// CVS
VskSingle vsk_convert_single(const VskString& str);
// CVD
VskDouble vsk_convert_double(const VskString& str);
// tests
void vsk_make_convert_tests(void);
