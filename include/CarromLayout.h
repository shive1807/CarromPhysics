#pragma once

#include "Circle2D.h"

#include <vector>

// Returns pieces in their standard carrom starting positions:
//   1 queen at center, 8 black and 8 white arranged in two concentric rings.
std::vector<Circle2D> makeCarromLayout();
