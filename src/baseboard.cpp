#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <string>

#include "baseboard.h"
#include "constants.h"

int lsb(BitBoard bb){
    return __builtin_ffsll(bb)-1;
}

int msb(BitBoard bb){
    return 63-__builtin_clzll(bb);
}

int popcount(BitBoard bb){
    return __builtin_popcountll(bb);
}

int squareFile(Square square){
    return square & 7;
}

int squareRank(Square square){
    return square >> 3;
}

// Number of king steps from one square to another
int squareDistance(Square s1, Square s2){
    return std::max(std::abs(squareRank(s1) - squareRank(s2)), std::abs(squareFile(s1) - squareFile(s2)));
}

BitBoard slidingAttacks(Square square, BitBoard occupied, std::vector<int> deltas){

    BitBoard attacks = BB_EMPTY;

    for(int delta : deltas){
        Square sq = square;

        while(true){
            sq += delta;
            if(!(0 <= sq && sq < 64) || squareDistance(sq, sq-delta) > 2){
                break;
            }

            attacks |= BB_SQUARES[sq];

            if(occupied & BB_SQUARES[sq]){
                break;
            }

        }
    }

    return attacks;
}

BitBoard stepAttacks(Square square, std::vector<int> deltas){
    return slidingAttacks(square, BB_ALL, deltas);
}

BitBoard edges(Square square){
    BitBoard rank = BB_RANK_1 << (8*squareRank(square));
    BitBoard file = BB_FILE_A << squareFile(square);

    return ((BB_RANK_1 | BB_RANK_8) & ~rank) | ((BB_FILE_A | BB_FILE_H) & ~file);
}

void attackTable(std::vector<int> deltas, BitBoard* mask_table, std::map<BitBoard, BitBoard>* attack_table){

    for(int i = 0; i < 64; i++){
        std::map<BitBoard, BitBoard> attacks {};
        BitBoard mask = slidingAttacks(i, 0, deltas) & ~edges(i);

        // Carry rippler subset iteration
        BitBoard subset = BB_EMPTY;
        while(true){
            attacks.insert(std::pair<BitBoard, BitBoard>(subset, slidingAttacks(i, subset, deltas)));

            subset = (subset-mask) & mask;
            if(!subset){
                break;
            }
        }

        mask_table[i] = mask;
        attack_table[i] = attacks;
    }
}

void genRays(BitBoard rays[64][64], std::map<BitBoard, BitBoard> BB_DIAG_ATTACKS[64], std::map<BitBoard, BitBoard> BB_FILE_ATTACKS[64], std::map<BitBoard, BitBoard> BB_RANK_ATTACKS[64]){
    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 64; j++){
            BitBoard bb_i = BB_SQUARES[i];
            BitBoard bb_j = BB_SQUARES[j];

            if(BB_DIAG_ATTACKS[i][0] & bb_j){
                rays[i][j] = (BB_DIAG_ATTACKS[i][0] & BB_DIAG_ATTACKS[j][0]) | bb_i | bb_j;
            } else if (BB_RANK_ATTACKS[i][0] & bb_j){
                rays[i][j] = BB_RANK_ATTACKS[i][0] | bb_i;
            } else if (BB_FILE_ATTACKS[i][0] & bb_j){
                rays[i][j] = BB_FILE_ATTACKS[i][0] | bb_i;
            } else {
                rays[i][j] = BB_EMPTY;
            }
        }
    }
}

// Prints BitBoard with white on bottom.
void printBitBoard(BitBoard bb){
    for(int i = 7; i >= 0; i--){
        for(int j = 0; j < 8; j++){
            int square = 8*i+j;
            std::cout << ((bb & ( BB_ONE << square )) >> square) << ' ';
        }
        std::cout << std::endl;
    }
}

bool BaseBoard::initialized_attacks = false;

BitBoard BaseBoard::BB_KING_ATTACKS[64];
BitBoard BaseBoard::BB_KNIGHT_ATTACKS[64];
BitBoard BaseBoard::BB_PAWN_ATTACKS[2][64];

BitBoard BaseBoard::BB_DIAG_MASKS[64];
BitBoard BaseBoard::BB_FILE_MASKS[64];
BitBoard BaseBoard::BB_RANK_MASKS[64];

BitBoard BaseBoard::BB_RAYS[64][64];

std::map<BitBoard, BitBoard> BaseBoard::BB_DIAG_ATTACKS[64];
std::map<BitBoard, BitBoard> BaseBoard::BB_FILE_ATTACKS[64];
std::map<BitBoard, BitBoard> BaseBoard::BB_RANK_ATTACKS[64];

BaseBoard::BaseBoard(){
    resetBoard();

    if(!initialized_attacks){
        generateAttacks();
        initialized_attacks = true;
    }
}

BaseBoard::BaseBoard(std::string fen){
    resetBoard();

    if(!initialized_attacks){
        generateAttacks();
        initialized_attacks = true;
    }

    setBoardFEN(fen);
}

void BaseBoard::generateAttacks(){
    // generate step attacks
    const std::vector<int> KNIGHT_MOVES {17, 15, 10, 6, -17, -15, -10, -6};
    const std::vector<int> KING_MOVES {9, 8, 7, 1, -9, -8, -7, -1};
    const std::vector<int> PAWN_MOVES_WHITE {7, 9};
    const std::vector<int> PAWN_MOVES_BLACK {-7, -9};
    for(int i = 0; i < 64; i++){
        BB_KNIGHT_ATTACKS[i] = stepAttacks(i, KNIGHT_MOVES);
        BB_KING_ATTACKS[i] = stepAttacks(i, KING_MOVES);
        BB_PAWN_ATTACKS[BLACK][i] = stepAttacks(i, PAWN_MOVES_BLACK);
        BB_PAWN_ATTACKS[WHITE][i] = stepAttacks(i, PAWN_MOVES_WHITE);
    }

    //generate slide attacks
    const std::vector<int> diag_moves {-9, -7, 7, 9};
    const std::vector<int> file_moves {-8, 8};
    const std::vector<int> rank_moves {-1, 1};

    attackTable(diag_moves, BB_DIAG_MASKS, BB_DIAG_ATTACKS);
    attackTable(file_moves, BB_FILE_MASKS, BB_FILE_ATTACKS);
    attackTable(rank_moves, BB_RANK_MASKS, BB_RANK_ATTACKS);

    //generate rays
    genRays(BB_RAYS, BB_DIAG_ATTACKS, BB_FILE_ATTACKS, BB_RANK_ATTACKS);
}

BitBoard BaseBoard::ray(Square a, Square b){
    return BB_RAYS[a][b];
}

BitBoard BaseBoard::between(Square a, Square b){
    BitBoard bb = BB_RAYS[a][b] & ((BB_ALL << a) ^ (BB_ALL << b));
    return bb & (bb - 1);
}

// Set board to standard chess starting position
void BaseBoard::resetBoard(){
    pawns = BB_RANK_2 | BB_RANK_7;
    knights = BB_B1 | BB_G1 | BB_B8 | BB_G8;
    bishops = BB_C1 | BB_F1 | BB_C8 | BB_F8;
    rooks = BB_CORNERS;
    queens = BB_D1 | BB_D8;
    kings = BB_E1 | BB_E8;

    promoted = BB_EMPTY;

    occupied_color[WHITE] = BB_RANK_1 | BB_RANK_2;
    occupied_color[BLACK] = BB_RANK_7 | BB_RANK_8;
    occupied = BB_RANK_1 | BB_RANK_2 | BB_RANK_7 | BB_RANK_8;
}

// Empty board
void BaseBoard::clearBoard(){
    pawns = BB_EMPTY;
    knights = BB_EMPTY;
    bishops = BB_EMPTY;
    rooks = BB_EMPTY;
    queens = BB_EMPTY;
    kings = BB_EMPTY;

    promoted = BB_EMPTY;

    occupied_color[WHITE] = BB_EMPTY;
    occupied_color[BLACK] = BB_EMPTY;
    occupied = BB_EMPTY;
}

void BaseBoard::setBoardFEN(std::string fen){
    clearBoard();

    int rank = 7;
    int file = 0;
    int i = 0;

    while(i < fen.length() && fen.at(i) != ' '){
        switch(fen.at(i)){
            case '/':
                rank--;
                file = 0;
                break;
            case 'P':
                setPieceAt(8*rank+file, PAWN, WHITE);
                file++;
                break;
            case 'p':
                setPieceAt(8*rank+file, PAWN, BLACK);
                file++;
                break;
            case 'N':
                setPieceAt(8*rank+file, KNIGHT, WHITE);
                file++;
                break;
            case 'n':
                setPieceAt(8*rank+file, KNIGHT, BLACK);
                file++;
                break;
            case 'B':
                setPieceAt(8*rank+file, BISHOP, WHITE);
                file++;
                break;
            case 'b':
                setPieceAt(8*rank+file, BISHOP, BLACK);
                file++;
                break;
            case 'R':
                setPieceAt(8*rank+file, ROOK, WHITE);
                file++;
                break;
            case 'r':
                setPieceAt(8*rank+file, ROOK, BLACK);
                file++;
                break;
            case 'Q':
                setPieceAt(8*rank+file, QUEEN, WHITE);
                file++;
                break;
            case 'q':
                setPieceAt(8*rank+file, QUEEN, BLACK);
                file++;
                break;
            case 'K':
                setPieceAt(8*rank+file, KING, WHITE);
                file++;
                break;
            case 'k':
                setPieceAt(8*rank+file, KING, BLACK);
                file++;
                break;
            default:
                file += fen.at(i)-'0';
        }
        i++;
    }
}

void BaseBoard::removePieceAt(Square square){
    PieceType piecetype = pieceTypeAt(square);
    BitBoard mask = BB_SQUARES[square];

    switch(piecetype){
        case PAWN:
            pawns ^= mask;
            break;

        case KNIGHT:
            knights ^= mask;
            break;

        case BISHOP:
            bishops ^= mask;
            break;

        case ROOK:
            rooks ^= mask;
            break;

        case QUEEN:
            queens ^= mask;
            break;

        case KING:
            kings ^= mask;
            break;

        default:
            return;
    }

    occupied ^= mask;
    occupied_color[WHITE] &= ~mask;
    occupied_color[BLACK] &= ~mask;

    promoted &= ~mask;
}

void BaseBoard::setPieceAt(Square square, PieceType piecetype, Color color, bool promoted){
    removePieceAt(square);

    BitBoard mask = BB_SQUARES[square];

    switch(piecetype){
        case PAWN:
            pawns |= mask;
            break;

        case KNIGHT:
            knights |= mask;
            break;

        case BISHOP:
            bishops |= mask;
            break;

        case ROOK:
            rooks |= mask;
            break;

        case QUEEN:
            queens |= mask;
            break;

        case KING:
            kings |= mask;
            break;

        default:
            return;
    }

    occupied ^= mask;
    occupied_color[color] ^= mask;

    if(promoted){
        promoted ^= mask;
    }
}

void BaseBoard::setPieceAt(Square square, PieceType piecetype, Color color){
    setPieceAt(square, piecetype, color, false);
}

// Returns BitBoard mask of squares with PieceType and Color
BitBoard BaseBoard::piecesMask(PieceType piecetype, Color color){
    BitBoard bb;
    switch(piecetype){
        case PAWN:
            bb = pawns;
            break;

        case KNIGHT:
            bb = knights;
            break;

        case BISHOP:
            bb = bishops;
            break;

        case ROOK:
            bb = rooks;
            break;

        case QUEEN:
            bb = queens;
            break;

        case KING:
            bb = kings;
            break;

        default:
            bb = BB_EMPTY;
    }

    return bb & occupied_color[color];
}

// Returns pieceType at a square or NO_PIECE
PieceType BaseBoard::pieceTypeAt(Square square){
    BitBoard mask = BB_SQUARES[square];

    if(!(occupied & mask)){
        return NO_PIECE;
    } else if (pawns & mask){
        return PAWN;
    } else if (knights & mask){
        return KNIGHT;
    } else if (bishops & mask){
        return BISHOP;
    } else if (rooks & mask){
        return ROOK;
    } else if (queens & mask){
        return QUEEN;
    } else if (kings & mask){
        return KING;
    }
    return NO_PIECE;
}

// Returns the Color at a square or NO_COLOR
Color BaseBoard::colorAt(Square square){
    BitBoard mask = BB_SQUARES[square];

    if(occupied_color[WHITE] & mask){
        return WHITE;
    } else if (occupied_color[BLACK] & mask){
        return BLACK;
    }

    return NO_COLOR;
}

// Returns Square containing the king of one color
Square BaseBoard::king(Color color){
    
    return msb(occupied_color[color] & kings);
}

// Returns a BitBoard mask of all possible attacks from a square
BitBoard BaseBoard::attacksMask(Square square){
    BitBoard bb_square = BB_SQUARES[square];
    
    if(bb_square & pawns){
        if(bb_square & occupied_color[WHITE]){
            return BB_PAWN_ATTACKS[WHITE][square];
        }
        return BB_PAWN_ATTACKS[BLACK][square];
    } else if (bb_square & knights){
        return BB_KNIGHT_ATTACKS[square];
    } else if (bb_square & kings){
        return BB_KING_ATTACKS[square];
    } else {
        BitBoard attacks = BB_EMPTY;
        if((bb_square & bishops) || (bb_square & queens)){
            attacks = BB_DIAG_ATTACKS[square][BB_DIAG_MASKS[square] & occupied];
        }
        if((bb_square & rooks) || (bb_square & queens)){
            attacks |= (BB_RANK_ATTACKS[square][BB_RANK_MASKS[square] & occupied] | BB_FILE_ATTACKS[square][BB_FILE_MASKS[square] & occupied]);
        }
        return attacks;
    }
}

BitBoard BaseBoard::attackersMask(Color color, Square square, BitBoard occupied_squares){
    BitBoard rank_pieces = BB_RANK_MASKS[square] & occupied_squares;
    BitBoard file_pieces = BB_FILE_MASKS[square] & occupied_squares;
    BitBoard diag_pieces = BB_DIAG_MASKS[square] & occupied_squares;

    BitBoard queens_and_rooks = queens | rooks;
    BitBoard queens_and_bishops = queens | bishops;

    Color pawn_color = (color==WHITE) ? BLACK : WHITE;

    BitBoard attackers = (BB_KING_ATTACKS[square] & kings) |
                         (BB_KNIGHT_ATTACKS[square] & knights) |
                         (BB_RANK_ATTACKS[square][rank_pieces] & queens_and_rooks) |
                         (BB_FILE_ATTACKS[square][file_pieces] & queens_and_rooks) |
                         (BB_DIAG_ATTACKS[square][diag_pieces] & queens_and_bishops) |
                         (BB_PAWN_ATTACKS[pawn_color][square] & pawns);

    return attackers & occupied_color[color];
}

BitBoard BaseBoard::attackersMask(Color color, Square square){
    return attackersMask(color, square, occupied);
}

bool BaseBoard::isAttackedBy(Color color, Square square){
    return (bool) attackersMask(color, square);
}

BitBoard BaseBoard::pinMask(Color color, Square square){
    BitBoard king_square = king(color);
    BitBoard square_mask = BB_SQUARES[square];

    Color opposite_color = (color==WHITE) ? BLACK : WHITE;

    BitBoard rays;
    BitBoard sliders;
    BitBoard snipers;
    
    // File pins
    rays = BB_FILE_ATTACKS[king_square][0];
    sliders = rooks | queens;

    if(rays & square_mask){
        snipers = rays & sliders & occupied_color[opposite_color];
        while(snipers){
            if((between(lsb(snipers), king_square) & (occupied | square_mask)) == square_mask){
                return ray(lsb(snipers), king_square);
            }
            snipers &= (snipers-1);
        }
    }

    // Rank pins

    rays = BB_RANK_ATTACKS[king_square][0];
    sliders = rooks | queens;

    if(rays & square_mask){
        snipers = rays & sliders & occupied_color[opposite_color];
        while(snipers){
            if((between(lsb(snipers), king_square) & (occupied | square_mask)) == square_mask){
                return ray(lsb(snipers), king_square);
            }
            snipers &= (snipers-1);
        }
    }

    // Diag pins

    rays = BB_DIAG_ATTACKS[king_square][0];
    sliders = bishops | queens;

    if(rays & square_mask){
        snipers = rays & sliders & occupied_color[opposite_color];
        while(snipers){
            if((between(lsb(snipers), king_square) & (occupied | square_mask)) == square_mask){
                return ray(lsb(snipers), king_square);
            }
            snipers &= (snipers-1);
        }
    }

    return BB_ALL;
}

bool BaseBoard::isPinned(Color color, Square square){
    return pinMask(color, square) != BB_ALL;
}

// Prints the chess board with white on bottom
void BaseBoard::print(){

    for(int i = 7; i >= 0; i--){
        for(int j = 0; j < 8; j++){
            Color color = colorAt(8*i+j);
            PieceType piece_type = pieceTypeAt(8*i+j);
            char piece_char;

            switch(piece_type){
                case PAWN:
                    piece_char='P';
                    break;

                case KNIGHT:
                    piece_char='N';
                    break;

                case BISHOP:
                    piece_char='B';
                    break;

                case ROOK:
                    piece_char='R';
                    break;

                case QUEEN:
                    piece_char='Q';
                    break;

                case KING:
                    piece_char='K';
                    break;

                default:
                    piece_char='.';
            }

            if(color == BLACK && piece_char != '.'){
                std::cout << (char)(piece_char + ('a'- 'A')) << ' ';
            } else {
                std::cout << piece_char << ' ';
            }
        }
        std::cout << std::endl;
    }
}