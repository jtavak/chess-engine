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

    std::vector<Move> generatePseudoLegalMoves(BitBoard from_mask, BitBoard to_mask);
    std::vector<Move> generateLegalMoves(BitBoard from_mask, BitBoard to_mask);

    Move generatePseudoLegalEP(BitBoard from_mask, BitBoard to_mask);

    std::vector<Move> generateEvasions(Square king_square, BitBoard checkers, BitBoard from_mask, BitBoard to_mask);

    bool EPSkewered(Square king_square, Square capturer_square);
    BitBoard sliderBlockers(Square king_square);
    bool isSafe(Square king_square, BitBoard blockers, Move move);

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

        std::vector<Move> generateLegalMoves();
        std::vector<Move> generatePseudoLegalMoves();

        Move generatePseudoLegalEP();

        bool isPseudoLegal(Move move);
        bool isCastling(Move move);
        bool isEnPassant(Move move);

        BitBoard checkersMask();
        bool isCheck();

        bool isCheckmate();

        void setBoardFEN(std::string fen);

        void print();
};