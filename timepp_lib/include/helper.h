#pragma once

#include "auto_release.h"

#define elem_of(x) (sizeof(x)/sizeof(x[0]))

#define STR_(x) #x
#define STR(x) STR_(x)

#define __LOC__ __FILE__ "(" STR(__LINE__) "): "

