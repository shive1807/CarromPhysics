#pragma once

#include "Board.h"
#include "Circle2D.h"
#include "Pocket.h"

#include <array>
#include <vector>

struct CarromModel {
    Board                 board;
    std::vector<Circle2D> pieces;
    std::array<Pocket, 4> pockets;
};
