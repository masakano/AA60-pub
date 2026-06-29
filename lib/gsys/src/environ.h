//
//
//
#pragma once

#include <spu/spu.h>

#define prt_f3(s, x) (aux_printf("\t%-30s : %g %g %g\n", s, x.f[0], x.f[1], x.f[2]))
#define prt_f4(s, x) (aux_printf("\t%-30s : %g %g %g %g\n", s, x.f[0], x.f[1], x.f[2], x.f[3]))
#define prt_f(s, x) (aux_printf("\t%-30s : %g\n", s, x))
#define prt_i(s, x) (aux_printf("\t%-30s : %d\n", s, x))
#define prt_h(s, x) (aux_printf("\t%-30s : 0x%08x\n", s, x))
#define prt_s(s, x) (aux_printf("\t%-30s : %s\n", s, x))
