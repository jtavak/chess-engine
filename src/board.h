#pragma once

#include <map>
#include "chess.h"

void printBitBoard(BitBoard bb);

class BaseBoard{
    BitBoard occupied_color[2];
    BitBoard occupied;

    BitBoard pawns, knights, bishops, rooks, queens, kings;
    BitBoard promoted;

    static bool initialized_attacks;
    static void generateAttacks();

    static BitBoard BB_KNIGHT_ATTACKS[64], BB_KING_ATTACKS[64], BB_PAWN_ATTACKS[2][64];
    static BitBoard BB_DIAG_MASKS[64], BB_FILE_MASKS[64], BB_RANK_MASKS[64];
    static std::map<BitBoard, BitBoard> BB_DIAG_ATTACKS[64], BB_FILE_ATTACKS[64], BB_RANK_ATTACKS[64];

    public:
        BaseBoard();

        void resetBoard();
        void clearBoard();

        BitBoard piecesMask(PieceType piecetype, Color color);

        PieceType pieceTypeAt(Square square);
        Color colorAt(Square square);

        Square king(Color color);

        BitBoard attacksMask(Square square);
        BitBoard attackersMask(Color color, Square square);

        bool isAttackedBy(Color color, Square square);

        void print();
};