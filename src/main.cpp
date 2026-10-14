#include <iostream>
#include "Circle2D.h"
#include "Board.h"

int main() {
    Board board{{0.0f, 0.0f}, 600.0f};

    std::cout << "Board bounds:\n"
              << "  left="   << board.left()   << " right=" << board.right()  << "\n"
              << "  bottom=" << board.bottom() << " top="   << board.top()    << "\n";
    return 0;
}
