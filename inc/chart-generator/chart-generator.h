#pragma once

#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_ ## x
#endif

#if !defined(__bool_true_false_are_defined) || __bool_true_false_are_defined == 0
#define true  1
#define false 0
typedef char bool;
#endif
