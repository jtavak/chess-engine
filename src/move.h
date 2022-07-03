#pragma once

#include "constants.h"

class Move{
    public:
        Square from_square;
        Square to_square;
        PieceType promotion;

        Move(Square from, Square to);
        Move(Square from, Square to, PieceType p);
};