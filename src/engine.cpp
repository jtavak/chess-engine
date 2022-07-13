#include "engine.h"
#include "board.h"

#include <cstdint>
#include <random>
#include <unordered_map>

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

inline int dotProduct(BitBoard bb, const int8_t weights[]){
    int accu = 0;
    while(bb){
        accu += weights[lsb(bb)];
        bb &= (bb - 1);
    }
    return accu;
}

inline int dotProductReverse(BitBoard bb, const int8_t weights[]){
    int accu = 0;
    while(bb){
        accu += weights[63-lsb(bb)];
        bb &= (bb - 1);
    }
    return accu;
}

BitBoard randBitBoard(){
    std::random_device engine;
    BitBoard random_bb = engine() | (BitBoard)engine()<<32;

    return random_bb;
}

void initZobrist(ZobristTable& table){
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 6; j++){
            table.pieces[i][j][WHITE] = randBitBoard();
            table.pieces[i][j][BLACK] = randBitBoard();
        }
    }

    for(int i = 0; i < 8; i++){
        table.ep_files[i] = randBitBoard();
    }

    for(int i = 0; i < 4; i++){
        table.castling_rights[i] = randBitBoard();
    }

    table.black_to_move = randBitBoard();
}

uint64_t hashZobrist(const Board& b, const ZobristTable& table){
    uint64_t hash = 0;

    // hash pieces
    BitBoard pawns = b.pawns;
    while(pawns){
        hash ^= table.pieces[lsb(pawns)][PAWN-1][b.colorAt(lsb(pawns))];
        pawns &= (pawns - 1);
    }

    BitBoard knights = b.knights;
    while(knights){
        hash ^= table.pieces[lsb(knights)][KNIGHT-1][b.colorAt(lsb(knights))];
        knights &= (knights - 1);
    }

    BitBoard bishops = b.bishops;
    while(bishops){
        hash ^= table.pieces[lsb(bishops)][BISHOP-1][b.colorAt(lsb(bishops))];
        bishops &= (bishops - 1);
    }

    BitBoard rooks = b.rooks;
    while(rooks){
        hash ^= table.pieces[lsb(rooks)][ROOK-1][b.colorAt(lsb(rooks))];
        rooks &= (rooks - 1);
    }

    BitBoard queens = b.queens;
    while(queens){
        hash ^= table.pieces[lsb(queens)][QUEEN-1][b.colorAt(lsb(queens))];
        queens &= (queens - 1);
    }

    BitBoard kings = b.kings;
    while(kings){
        hash ^= table.pieces[lsb(kings)][KING-1][b.colorAt(lsb(kings))];
        kings &= (kings - 1);
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

// update zobrist hash BEFORE move is pushed to baord
uint64_t updateZobrist(uint64_t hash, const Board& board, const Move& move, const ZobristTable& table){
    PieceType from_piece = board.pieceTypeAt(move.from_square);
    Square capture_square = move.to_square;
    PieceType capture_piece_type = board.pieceTypeAt(capture_square);

    // clear ep_square if it exists
    Square prev_ep_square = board.ep_square;
    if(board.ep_square != NO_SQUARE){
        hash ^= table.ep_files[squareFile(prev_ep_square)];
    }

    // remove piece on from_square
    hash ^= table.pieces[move.from_square][from_piece-1][board.turn];

    // update castling rights
    BitBoard touched = BB_SQUARES[move.from_square] | BB_SQUARES[move.to_square];
    if(board.castling_rights & touched){
        if(BB_A1 & touched){
            hash ^= table.castling_rights[0];
        } else if (BB_A8 & touched){
            hash ^= table.castling_rights[1];
        } else if (BB_H1 & touched){
            hash ^= table.castling_rights[2];
        } else {
            hash ^= table.castling_rights[3];
        }
    }
    if(from_piece == KING){
        if(board.turn == WHITE){
            if(board.castling_rights & BB_A1){
                hash ^= table.castling_rights[0];
            }
            if(board.castling_rights & BB_H1){
                hash ^= table.castling_rights[2];
            }
        } else {
            if(board.castling_rights & BB_A8){
                hash ^= table.castling_rights[1];
            }
            if(board.castling_rights & BB_H8){
                hash ^= table.castling_rights[3];
            }
        }
    }

    // handle special pawn moves
    bool isEPCapture = false;
    if(from_piece == PAWN){
        int diff = move.to_square - move.from_square;

        if(diff == 16 || diff == -16){
            hash ^= table.ep_files[squareFile(move.from_square)];
        } else if (move.to_square == prev_ep_square && (abs(diff) == 7 || abs(diff) == 9) && capture_piece_type == NO_PIECE){
            int down = (board.turn == WHITE) ? -8 : 8;
            capture_square = prev_ep_square + down;
            hash ^= table.pieces[capture_square][PAWN-1][board.turn^1];

            isEPCapture = true;
        }
    }

    if(from_piece == KING && squareDistance(move.from_square, move.to_square) > 1){
        if(squareFile(move.to_square) < squareFile(move.from_square)){
            hash ^= table.pieces[(board.turn == WHITE) ? D1 : D8][ROOK-1][board.turn];
            hash ^= table.pieces[(board.turn == WHITE) ? A1 : A8][ROOK-1][board.turn];
        } else {
            hash ^= table.pieces[(board.turn == WHITE) ? F1 : F8][ROOK-1][board.turn];
            hash ^= table.pieces[(board.turn == WHITE) ? H1 : H8][ROOK-1][board.turn];
        }
    }

    // remove captured piece if not en passant
    if(capture_piece_type != NO_PIECE && !isEPCapture){
        hash ^= table.pieces[move.to_square][capture_piece_type-1][board.turn^1];
    }

    // place piece on to_square
    if(move.promotion == NO_PIECE){
        hash ^= table.pieces[move.to_square][from_piece-1][board.turn];
    } else {
        hash ^= table.pieces[move.to_square][move.promotion-1][board.turn];
    }

    // hash turn change
    hash ^= table.black_to_move;

    return hash;
}

// optimize for current color
int evaluation(const Board& b){
    int centipawn = 0;

    Color turn = b.turn;

    // material values
    centipawn += popcount(b.pawns & b.occupied_color[turn]) * 100;
    centipawn -= popcount(b.pawns & b.occupied_color[turn ^ 1]) * 100;

    centipawn += popcount(b.knights & b.occupied_color[turn]) * 320;
    centipawn -= popcount(b.knights & b.occupied_color[turn ^ 1]) * 320;

    centipawn += popcount(b.bishops & b.occupied_color[turn]) * 330;
    centipawn -= popcount(b.bishops & b.occupied_color[turn ^ 1]) * 330;

    centipawn += popcount(b.rooks & b.occupied_color[turn]) * 500;
    centipawn -= popcount(b.rooks & b.occupied_color[turn ^ 1]) * 500;

    centipawn += popcount(b.queens & b.occupied_color[turn]) * 900;
    centipawn -= popcount(b.queens & b.occupied_color[turn ^ 1]) * 900;

    // piece-position tables

    centipawn += dotProductReverse(b.pawns & b.occupied_color[turn], PAWN_TABLE);
    centipawn -= dotProduct(b.pawns & b.occupied_color[turn ^ 1], PAWN_TABLE);

    centipawn += dotProductReverse(b.knights & b.occupied_color[turn], KNIGHT_TABLE);
    centipawn -= dotProduct(b.knights & b.occupied_color[turn ^ 1], KNIGHT_TABLE);

    centipawn += dotProductReverse(b.bishops & b.occupied_color[turn], BISHOP_TABLE);
    centipawn -= dotProduct(b.bishops & b.occupied_color[turn ^ 1], BISHOP_TABLE);

    centipawn += dotProductReverse(b.rooks & b.occupied_color[turn], ROOK_TABLE);
    centipawn -= dotProduct(b.rooks & b.occupied_color[turn ^ 1], ROOK_TABLE);

    centipawn += dotProductReverse(b.queens & b.occupied_color[turn], QUEEN_TABLE);
    centipawn -= dotProduct(b.queens & b.occupied_color[turn ^ 1], QUEEN_TABLE);

    centipawn += dotProductReverse(b.kings & b.occupied_color[turn], KING_TABLE);
    centipawn -= dotProduct(b.kings & b.occupied_color[turn ^ 1], KING_TABLE);


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

int negamax(Board& b, int depth, int alpha, int beta, std::unordered_map<uint64_t, TTEntry>& transposition_table, const ZobristTable& z_table, const uint64_t prev_hash){
    int alpha_orig = alpha;

    auto tt_iter = transposition_table.find(prev_hash);
    if(tt_iter != transposition_table.end()){
        TTEntry t = tt_iter->second;
        if(t.depth >= depth){
            if(t.flag == EXACT){
                return t.value;
            } else if (t.flag == LOWER_BOUND){
                alpha = std::max(alpha, t.value);
            } else{
                beta = std::min(beta, t.value);
            }
            
            if(alpha >= beta){
                return t.value;
            }
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
            return (b.turn == WHITE) ? 30000+depth : -30000-depth;
        }
        return 0;
    }

    int value = INT_MIN + 1;

    for(int i = 0; i < moves.size(); i++){
        uint64_t hash = updateZobrist(prev_hash, b, moves.at(i), z_table);

        b.push(moves.at(i));

        value = std::max(value, -negamax(b, depth-1, -beta, -alpha, transposition_table, z_table, hash));
        alpha = std::max(alpha, value);
        if(alpha >= beta){
            b.pop();
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

    transposition_table[prev_hash] = tte;
 
    return value;
}

std::pair<int, Move> searchRoot(Board& b, int depth, const ZobristTable& z_table){
    std::vector<Move> moves = b.generateLegalMoves();

    if(moves.empty()){
        return std::pair<int, Move>(0, NO_MOVE);
    }

    int alpha = INT_MIN+1;
    int beta = INT_MAX-1;

    std::unordered_map<uint64_t, TTEntry> transposition_table;

    int best;

    uint64_t prev_hash = hashZobrist(b, z_table);

    for(int i = 0; i < moves.size(); i++){
        uint64_t hash = updateZobrist(prev_hash, b, moves.at(i), z_table);

        b.push(moves.at(i));

        int score = -negamax(b, depth-1, -beta, -alpha, transposition_table, z_table, hash);

        if(score >= beta){
            return std::pair<int, Move>(beta, moves.at(i));
        }

        if(score > alpha){
            alpha = score;
            best = i;
        }
        
        b.pop();
    }

    return std::pair<int, Move>(alpha, moves.at(best));
}