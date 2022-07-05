#include "engine.h"
#include "board.h"

#include <cstdint>

const int8_t PAWN_TABLE[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

const int8_t KNIGHT_TABLE[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int8_t BISHOP_TABLE[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int8_t ROOK_TABLE[64] = {
    0,  0,  0,  0,  0,  0,  0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0,  0,  0,  0,  0,  0,-5,
    -5, 0,  0,  0,  0,  0,  0,-5,
    -5, 0,  0,  0,  0,  0,  0,-5,
    -5, 0,  0,  0,  0,  0,  0,-5,
    -5, 0,  0,  0,  0,  0,  0,-5,
    0,  0,  0,  5,  5,  0,  0, 0
};

const int8_t QUEEN_TABLE[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int8_t KING_TABLE[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

int dotProduct(BitBoard bb, const int8_t weights[]){
    BitBoard bit = 1;
    int accu = 0;
    for(int sq = 0; sq < 64; sq++, bit <<= 1){
        if(bb & bit){
            accu += weights[sq];
        }
    }
    return accu;
}

int dotProductReverse(BitBoard bb, const int8_t weights[]){
    BitBoard bit = BB_H8;
    int accu = 0;
    for(int sq = 0; sq < 64; sq++, bit >>= 1){
        if(bb & bit){
            accu += weights[sq];
        }
    }
    return accu;
}

// optimize for current color
int evaluation(Board b){
    int centipawn = 0;

    Color turn = b.turn;
    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;

    // material values
    centipawn += popcount(b.pawns & b.occupied_color[turn]) * 100;
    centipawn -= popcount(b.pawns & b.occupied_color[opp_turn]) * 100;

    centipawn += popcount(b.knights & b.occupied_color[turn]) * 320;
    centipawn -= popcount(b.knights & b.occupied_color[opp_turn]) * 320;

    centipawn += popcount(b.bishops & b.occupied_color[turn]) * 330;
    centipawn -= popcount(b.bishops & b.occupied_color[opp_turn]) * 330;

    centipawn += popcount(b.rooks & b.occupied_color[turn]) * 500;
    centipawn -= popcount(b.rooks & b.occupied_color[opp_turn]) * 500;

    centipawn += popcount(b.queens & b.occupied_color[turn]) * 900;
    centipawn -= popcount(b.queens & b.occupied_color[opp_turn]) * 900;

    // piece-position tables

    centipawn += dotProductReverse(b.pawns & b.occupied_color[turn], PAWN_TABLE);
    centipawn -= dotProduct(b.pawns & b.occupied_color[opp_turn], PAWN_TABLE);

    centipawn += dotProductReverse(b.knights & b.occupied_color[turn], KNIGHT_TABLE);
    centipawn -= dotProduct(b.knights & b.occupied_color[opp_turn], KNIGHT_TABLE);

    centipawn += dotProductReverse(b.bishops & b.occupied_color[turn], BISHOP_TABLE);
    centipawn -= dotProduct(b.bishops & b.occupied_color[opp_turn], BISHOP_TABLE);

    centipawn += dotProductReverse(b.rooks & b.occupied_color[turn], ROOK_TABLE);
    centipawn -= dotProduct(b.rooks & b.occupied_color[opp_turn], ROOK_TABLE);

    centipawn += dotProductReverse(b.queens & b.occupied_color[turn], QUEEN_TABLE);
    centipawn -= dotProduct(b.queens & b.occupied_color[opp_turn], QUEEN_TABLE);

    centipawn += dotProductReverse(b.kings & b.occupied_color[turn], KING_TABLE);
    centipawn -= dotProduct(b.kings & b.occupied_color[opp_turn], KING_TABLE);


    return centipawn;
}

int negamax(Board b, int depth, int alpha, int beta){
    if(b.isInsufficientMaterial() || b.isFiftyMoves()){
        return 0;
    }
    
    if(depth == 0){
        return evaluation(b);
    }

    std::vector<Move> moves = b.generateLegalMoves();

    if(moves.empty()){
        if(b.isCheck()){
            return (b.turn == WHITE) ? 30000 : -30000;
        }
        return 0;
    }

    for(int i = 0; i < moves.size(); i++){
        b.push(moves.at(i));

        int score = -negamax(b, depth-1, -beta, -alpha);

        if(score >= beta){
            return beta;
        }

        if(score > alpha){
            alpha = score;
        }
        
        b.pop();
    }

    return alpha;
}

std::pair<int, Move> searchRoot(Board b, int depth){
    std::vector<Move> moves = b.generateLegalMoves();

    if(moves.empty()){
        return std::pair(0, NO_MOVE);
    }

    int alpha = INT_MIN+1;
    int beta = INT_MAX-1;

    int best;

    for(int i = 0; i < moves.size(); i++){
        b.push(moves.at(i));

        int score = -negamax(b, depth-1, -beta, -alpha);

        if(score >= beta){
            return std::pair(beta, moves.at(i));
        }

        if(score > alpha){
            alpha = score;
            best = i;
        }
        
        b.pop();
    }

    return std::pair(alpha, moves.at(best));
}