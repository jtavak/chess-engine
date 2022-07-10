#pragma once

#include <string>

#include "constants.h"

class Move{
    public:
        Square from_square;
        Square to_square;
        PieceType promotion;

        Move(Square from, Square to);
        Move(Square from, Square to, PieceType p);
        Move(std::string uci);

        bool operator == (Move move);
        bool operator != (Move move);

        std::string toUCI();
};

const Move NO_MOVE = Move(-1, -1, -1);