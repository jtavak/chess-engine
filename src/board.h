#pragma once

#include <string>
#include <vector>

#include "move.h"
#include "baseboard.h"
#include "constants.h"

class Board;

class BoardState{
    // BaseBoard state
    BitBoard pawns, knights, bishops, rooks, queens, kings;

    BitBoard occupied;
    BitBoard occupied_color[2];

    BitBoard promoted;

    // Board state
    Color turn;
    BitBoard castling_rights;
    Square ep_square;
    int fullmove_number, halfmove_clock;

    public:
        BoardState(Board board);
        void restore(Board* board);

};

class Board: public BaseBoard{
    std::vector<Move> move_stack;
    std::vector<BoardState> state_stack;

    void clearStack();

    bool attackedForKing(BitBoard path, BitBoard occupied);

    public:
        Color turn;
        BitBoard castling_rights;
        Square ep_square;
        int fullmove_number, halfmove_clock;

        Board(std::string fen);
        Board();

        void resetBoard();
        void clearBoard();

        std::vector<Move> generatePseudoLegalMoves(BitBoard from_mask, BitBoard to_mask);
        std::vector<Move> generatePseudoLegalMoves();

        void setBoardFEN(std::string fen);

        void print();
};