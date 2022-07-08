#pragma once

#include <unordered_map>
#include <string>

#include "constants.h"

int lsb(BitBoard bb);
int msb(BitBoard bb);
int popcount(BitBoard bb);

int squareFile(Square square);
int squareRank(Square square);

void printBitBoard(BitBoard bb);

class BaseBoard{
    static bool initialized_attacks;
    static void generateAttacks();

    protected:
        static BitBoard BB_KNIGHT_ATTACKS[64], BB_KING_ATTACKS[64], BB_PAWN_ATTACKS[2][64];
        static BitBoard BB_DIAG_MASKS[64], BB_FILE_MASKS[64], BB_RANK_MASKS[64];
        static BitBoard BB_RAYS[64][64];
        static std::unordered_map<BitBoard, BitBoard> BB_DIAG_ATTACKS[64], BB_FILE_ATTACKS[64], BB_RANK_ATTACKS[64];

        static BitBoard ray(Square a, Square b);
        static BitBoard between(Square a, Square b);

    public:
        BitBoard occupied_color[2];
        BitBoard occupied;

        BitBoard pawns, knights, bishops, rooks, queens, kings;

        BaseBoard(std::string fen);
        BaseBoard();

        void resetBoard();
        void clearBoard();

        void setBoardFEN(std::string fen);

        PieceType removePieceAt(Square square);
        void setPieceAt(Square square, PieceType piecetype, Color color);

        BitBoard piecesMask(PieceType piecetype, Color color);

        PieceType pieceTypeAt(Square square);
        Color colorAt(Square square);

        Square king(Color color);

        BitBoard attacksMask(Square square);

        BitBoard attackersMask(Color color, Square square, BitBoard occupied_squares);
        BitBoard attackersMask(Color color, Square square);

        BitBoard pinMask(Color color, Square square);

        bool isAttackedBy(Color color, Square square);
        bool isPinned(Color color, Square square);

        void print();

        bool operator == (BaseBoard b);
};