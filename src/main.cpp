#include <iostream>
#include "Circle2D.h"

int main() {
    Circle2D piece;
    piece.position = {0.0f, 0.0f};
    piece.velocity = {1.0f, 0.5f};
    piece.radius   = 1.5f;
    piece.mass     = 1.0f;

    std::cout << "CarromPhysics\n";
    return 0;
}
