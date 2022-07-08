#include "engine.h"
#include "board.h"

#include <cstdint>
#include <random>

#ifndef INT_MIN
#define INT_MIN -2147483648
#define INT_MAX 2147483647
#endif

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

BitBoard randBitBoard(){
    std::random_device engine;
    BitBoard random_bb = engine() | (BitBoard)engine()<<32;

    return random_bb;
}

void init_zobrist(ZobristTable* table){
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 6; j++){
            table->pieces[i][j][WHITE] = randBitBoard();
            table->pieces[i][j][BLACK] = randBitBoard();
        }
    }

    for(int i = 0; i < 8; i++){
        table->ep_files[i] = randBitBoard();
    }

    for(int i = 0; i < 4; i++){
        table->castling_rights[i] = randBitBoard();
    }

    table->black_to_move = randBitBoard();
}

uint64_t hash_zobrist(Board b, const ZobristTable table){
    uint64_t hash;

    BitBoard pieces;
    pieces = b.kings;

    // hash pieces
    while(pieces){
        Square sq = lsb(pieces);
        hash ^= table.pieces[sq][PAWN-1][b.colorAt(sq)];
        pieces &= (pieces - 1);
    }

    pieces = b.knights;

    while(pieces){
        Square sq = lsb(pieces);
        hash ^= table.pieces[sq][KNIGHT-1][b.colorAt(sq)];
        pieces &= (pieces - 1);
    }

    pieces = b.bishops;

    while(pieces){
        Square sq = lsb(pieces);
        hash ^= table.pieces[sq][BISHOP-1][b.colorAt(sq)];
        pieces &= (pieces - 1);
    }

    pieces = b.rooks;

    while(pieces){
        Square sq = lsb(pieces);
        hash ^= table.pieces[sq][ROOK-1][b.colorAt(sq)];
        pieces &= (pieces - 1);
    }

    pieces = b.queens;

    while(pieces){
        Square sq = lsb(pieces);
        hash ^= table.pieces[sq][QUEEN-1][b.colorAt(sq)];
        pieces &= (pieces - 1);
    }

    pieces = b.kings;

    while(pieces){
        Square sq = lsb(pieces);
        hash ^= table.pieces[sq][KING-1][b.colorAt(sq)];
        pieces &= (pieces - 1);
    }

    // hash castling rights
    if(b.castling_rights & BB_A1){
        hash ^= table.castling_rights[0];
    }
    if(b.castling_rights & BB_A8){
        hash ^= table.castling_rights[1];
    }
    if(b.castling_rights & BB_H1){
        hash ^= table.castling_rights[2];
    }
    if(b.castling_rights & BB_H8){
        hash ^= table.castling_rights[3];
    }
    
    // hash ep square
    if(b.ep_square != NO_SQUARE){
        hash ^= table.ep_files[squareFile(b.ep_square)];
    }

    // hash color to move
    if(b.turn == BLACK){
        hash ^= table.black_to_move;
    }

    return hash;
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

const uint8_t LOWER_BOUND = 0;
const uint8_t EXACT = 1;
const uint8_t UPPER_BOUND = 2;

struct TTEntry {
    int value;
    uint8_t flag;
    uint8_t depth;
};

int negamax(Board b, int depth, int alpha, int beta, std::unordered_map<uint64_t, TTEntry>* transposition_table, const ZobristTable zobrist_table){
    int alpha_orig = alpha;

    uint64_t hash = hash_zobrist(b, zobrist_table);
    auto map_node = transposition_table->find(hash);
    if(map_node != transposition_table->end() && map_node->second.depth >= depth){
        if(map_node->second.flag == EXACT){
            return map_node->second.value;
        } else if (map_node->second.flag == LOWER_BOUND){
            alpha = std::max(alpha, map_node->second.value);
        } else if (map_node->second.flag == UPPER_BOUND){
            beta = std::min(beta, map_node->second.value);
        }

        if(alpha >= beta){
            return map_node->second.value;
        }
    }

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

    int value = INT_MIN + 1;

    for(int i = 0; i < moves.size(); i++){
        b.push(moves.at(i));

        value = std::max(value, -negamax(b, depth-1, -beta, -alpha, transposition_table, zobrist_table));
        alpha = std::max(alpha, value);
        if(alpha >= beta){
            break;
        }
        
        b.pop();
    }

    TTEntry tte;
    tte.value = value;
    if(value <= alpha_orig){
        tte.flag = UPPER_BOUND;
    } else if (value >= beta){
        tte.flag = LOWER_BOUND;
    } else {
        tte.flag = EXACT;
    }
    tte.depth = depth;
    transposition_table->insert(std::pair(hash, tte));

    return value;
}

std::pair<int, Move> searchRoot(Board b, int depth, const ZobristTable zobrist_table){
    std::vector<Move> moves = b.generateLegalMoves();

    if(moves.empty()){
        return std::pair(0, NO_MOVE);
    }

    int alpha = INT_MIN+1;
    int beta = INT_MAX-1;

    std::unordered_map<uint64_t, TTEntry> transposition_table;

    int best;

    for(int i = 0; i < moves.size(); i++){
        b.push(moves.at(i));

        int score = -negamax(b, depth-1, -beta, -alpha, &transposition_table, zobrist_table);

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