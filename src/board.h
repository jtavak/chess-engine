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

    // Board state
    Color turn;
    BitBoard castling_rights;
    Square ep_square;
    int fullmove_number, halfmove_clock;

    public:
        BoardState(Board* board);
        void restore(Board* board);

};

class Board: public BaseBoard{
    std::vector<Move> move_stack;
    std::vector<BoardState> state_stack;

    const std::vector<Move> generatePseudoLegalMoves(BitBoard from_mask, BitBoard to_mask) const;
    const std::vector<Move> generateLegalMoves(BitBoard from_mask, BitBoard to_mask) const;

    Move generatePseudoLegalEP(BitBoard from_mask, BitBoard to_mask) const;

    const std::vector<Move> generateEvasions(Square king_square, BitBoard checkers, BitBoard from_mask, BitBoard to_mask) const;

    bool EPSkewered(Square king_square, Square capturer_square) const;
    BitBoard sliderBlockers(Square king_square) const;
    bool isSafe(Square king_square, BitBoard blockers, Move move) const;

    bool isHalfmoves(int n) const;

    void clearStack();

    bool attackedForKing(BitBoard path, BitBoard occupied) const;

    public:
        Color turn;
        BitBoard castling_rights;
        Square ep_square;
        int fullmove_number, halfmove_clock;

        Board(std::string fen);
        Board();

        void resetBoard();
        void clearBoard();

        void push(const Move& move);
        Move pop();

        const std::vector<Move> generateLegalMoves() const;
        const std::vector<Move> generatePseudoLegalMoves() const;

        Move generatePseudoLegalEP() const;

        bool isPseudoLegal(const Move& move) const; 
        bool isCastling(const Move& move) const;
        bool isEnPassant(const Move& move) const;
        bool isIntoCheck(const Move& move) const;
        bool isZeroing(const Move& move) const;
        bool isCapture(const Move& move) const;
        bool isLegal(const Move& move) const;


        BitBoard checkersMask() const;
        bool isCheck() const;

        bool isCheckmate() const;
        bool isStalemate() const;
        bool hasInsufficientMaterial(Color color) const;
        bool isInsufficientMaterial() const;
        bool isFiftyMoves() const;

        Outcome gameOutcome() const;

        void setBoardFEN(std::string fen);

        void pushUCI(std::string uci);

        void print() const;

        bool operator == (const Board& b);
};