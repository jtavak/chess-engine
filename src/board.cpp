#include "board.h"
#include <iostream>
#include <algorithm>
#include <vector>

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

// Prints BitBoard with white on bottom.
void printBitBoard(BitBoard bb){
    for(int i = 7; i >= 0; i--){
        for(int j = 0; j < 8; j++){
            int bbIndex = 8*i+j;
            std::cout << ((bb & ( BB_ONE << bbIndex )) >> bbIndex) << ' ';
        }
        std::cout << std::endl;
    }
}


BaseBoard::BaseBoard(){
    resetBoard();
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

    occupiedColor[WHITE] = BB_RANK_1 | BB_RANK_2;
    occupiedColor[BLACK] = BB_RANK_7 | BB_RANK_8;
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

    occupiedColor[WHITE] = BB_EMPTY;
    occupiedColor[BLACK] = BB_EMPTY;
    occupied = BB_EMPTY;
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

    return bb & occupiedColor[color];
}

// Returns pieceType at a square or NO_PIECE
PieceType BaseBoard::pieceTypeAt(Square square){
    BitBoard mask = BB_ONE << square;

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
    BitBoard mask = BB_ONE << square;

    if(occupiedColor[WHITE] & mask){
        return WHITE;
    } else if (occupiedColor[BLACK] & mask){
        return BLACK;
    }

    return NO_COLOR;
}

// Returns Square containing the king of one color
Square BaseBoard::king(Color color){
    
    return msb(occupiedColor[color] & kings);
}

// Returns a BitBoard mask of all possible attacks from a square
BitBoard BaseBoard::attacksMask(Square square){
    BitBoard bb_square = BB_ONE << square;
    return BB_EMPTY;
}

// Prints the chess board with white on bottom
void BaseBoard::print(){

    for(int i = 7; i >= 0; i--){
        for(int j = 0; j < 8; j++){
            Color color = colorAt(8*i+j);
            PieceType pieceType = pieceTypeAt(8*i+j);
            char pieceChar;

            switch(pieceType){
                case PAWN:
                    pieceChar='P';
                    break;

                case KNIGHT:
                    pieceChar='N';
                    break;

                case BISHOP:
                    pieceChar='B';
                    break;

                case ROOK:
                    pieceChar='R';
                    break;

                case QUEEN:
                    pieceChar='Q';
                    break;

                case KING:
                    pieceChar='K';
                    break;

                default:
                    pieceChar='.';
            }

            if(color == BLACK && pieceChar != '.'){
                std::cout << (char)(pieceChar + ('a'- 'A')) << ' ';
            } else {
                std::cout << pieceChar << ' ';
            }
        }
        std::cout << std::endl;
    }
}