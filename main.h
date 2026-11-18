#pragma once

#include "types.h"

Solution dplem(const Problem *problem);
Solution bdplem(const Problem *problem, int threads);
Solution pdplem(const Problem *problem, int threads);