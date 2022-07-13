#include "engine.h"
#include "board.h"
#include "positiontables.h"

#include <cstdint>
#include <random>
#include <unordered_map>

#ifndef INT_MIN
#define INT_MIN -2147483648
#define INT_MAX 2147483647
#endif

inline int dotProduct(BitBoard bb, const int16_t weights[]){
    int accu = 0;
    while(bb){
        accu += weights[lsb(bb)];
        bb &= (bb - 1);
    }
    return accu;
}

inline int dotProductReverse(BitBoard bb, const int16_t weights[]){
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
    int mg_value = 0;
    int eg_value = 0;
    int game_phase = 0;

    Color turn = b.turn;

    // material values

    // pawns
    mg_value += popcount(b.pawns & b.occupied_color[turn]) * 82;
    mg_value -= popcount(b.pawns & b.occupied_color[turn ^ 1]) * 82;

    eg_value += popcount(b.pawns & b.occupied_color[turn]) * 94;
    eg_value -= popcount(b.pawns & b.occupied_color[turn ^ 1]) * 94;

    // knights
    mg_value += popcount(b.knights & b.occupied_color[turn]) * 337;
    mg_value -= popcount(b.knights & b.occupied_color[turn ^ 1]) * 337;

    eg_value += popcount(b.knights & b.occupied_color[turn]) * 281;
    eg_value -= popcount(b.knights & b.occupied_color[turn ^ 1]) * 281;

    game_phase += popcount(b.knights);

    // bishops
    mg_value += popcount(b.bishops & b.occupied_color[turn]) * 365;
    mg_value -= popcount(b.bishops & b.occupied_color[turn ^ 1]) * 365;

    eg_value += popcount(b.bishops & b.occupied_color[turn]) * 297;
    eg_value -= popcount(b.bishops & b.occupied_color[turn ^ 1]) * 297;

    game_phase += popcount(b.bishops);

    // rooks 
    mg_value += popcount(b.rooks & b.occupied_color[turn]) * 477;
    mg_value -= popcount(b.rooks & b.occupied_color[turn ^ 1]) * 477;

    eg_value += popcount(b.rooks & b.occupied_color[turn]) * 512;
    eg_value -= popcount(b.rooks & b.occupied_color[turn ^ 1]) * 512;

    game_phase += popcount(b.rooks) * 2;

    // queens
    mg_value += popcount(b.queens & b.occupied_color[turn]) * 1025;
    mg_value -= popcount(b.queens & b.occupied_color[turn ^ 1]) * 1025;

    eg_value += popcount(b.queens & b.occupied_color[turn]) * 936;
    eg_value -= popcount(b.queens & b.occupied_color[turn ^ 1]) * 936;

    game_phase += popcount(b.queens) * 4;

    // piece-position tables

    // pawns
    mg_value += dotProductReverse(b.pawns & b.occupied_color[turn], mg_pawn_table);
    mg_value -= dotProduct(b.pawns & b.occupied_color[turn ^ 1], mg_pawn_table);

    eg_value += dotProductReverse(b.pawns & b.occupied_color[turn], eg_pawn_table);
    eg_value -= dotProduct(b.pawns & b.occupied_color[turn ^ 1], eg_pawn_table);

    // knights
    mg_value += dotProductReverse(b.knights & b.occupied_color[turn], mg_knight_table);
    mg_value -= dotProduct(b.knights & b.occupied_color[turn ^ 1], mg_knight_table);

    eg_value += dotProductReverse(b.knights & b.occupied_color[turn], eg_knight_table);
    eg_value -= dotProduct(b.knights & b.occupied_color[turn ^ 1], eg_knight_table);

    // bishops
    mg_value += dotProductReverse(b.bishops & b.occupied_color[turn], mg_bishop_table);
    mg_value -= dotProduct(b.bishops & b.occupied_color[turn ^ 1], mg_bishop_table);

    eg_value += dotProductReverse(b.bishops & b.occupied_color[turn], eg_bishop_table);
    eg_value -= dotProduct(b.bishops & b.occupied_color[turn ^ 1], eg_bishop_table);
    
    // rooks
    mg_value += dotProductReverse(b.rooks & b.occupied_color[turn], mg_rook_table);
    mg_value -= dotProduct(b.rooks & b.occupied_color[turn ^ 1], mg_rook_table);

    eg_value += dotProductReverse(b.rooks & b.occupied_color[turn], eg_rook_table);
    eg_value -= dotProduct(b.rooks & b.occupied_color[turn ^ 1], eg_rook_table);

    // queens
    mg_value += dotProductReverse(b.queens & b.occupied_color[turn], mg_queen_table);
    mg_value -= dotProduct(b.queens & b.occupied_color[turn ^ 1], mg_queen_table);

    eg_value += dotProductReverse(b.queens & b.occupied_color[turn], eg_queen_table);
    eg_value -= dotProduct(b.queens & b.occupied_color[turn ^ 1], eg_queen_table);

    // kings
    mg_value += dotProductReverse(b.kings & b.occupied_color[turn], mg_king_table);
    mg_value -= dotProduct(b.kings & b.occupied_color[turn ^ 1], mg_king_table);

    eg_value += dotProductReverse(b.kings & b.occupied_color[turn], eg_king_table);
    eg_value -= dotProduct(b.kings & b.occupied_color[turn ^ 1], eg_king_table);

    int mg_phase = game_phase;
    if (mg_phase > 24) mg_phase = 24;

    int eg_phase = 24 - mg_phase;
    return (mg_value * mg_phase + eg_value * eg_phase) / 24;
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