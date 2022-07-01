#pragma once

#include "chess.h"

void printBitBoard(BitBoard bb);

class BaseBoard{
    BitBoard occupiedColor[2];
    BitBoard occupied;

    BitBoard pawns;
    BitBoard knights;
    BitBoard bishops;
    BitBoard rooks;
    BitBoard queens;
    BitBoard kings;

    BitBoard promoted;

    public:
        BaseBoard();

        void resetBoard();
        void clearBoard();

        BitBoard piecesMask(PieceType piecetype, Color color);

        PieceType pieceTypeAt(Square square);
        Color colorAt(Square square);

        Square king(Color color);

        BitBoard attacksMask(Square square);

        void print();

};