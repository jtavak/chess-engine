#include "constants.h"
#include "move.h"

Move::Move(Square from, Square to, PieceType p){
    from_square = from;
    to_square = to;
    promotion = p;
}

Move::Move(Square from, Square to){
    from_square = from;
    to_square = to;
    promotion = NO_PIECE;
}

bool Move::operator == (Move move){
    if(move.from_square == from_square && move.to_square == to_square && move.promotion == promotion){
        return true;
    }
    return false;
}

std::string Move::to_uci(){
    std::string promotion_letter;

    switch(promotion){
        case QUEEN:
            promotion_letter = "q";
            break;
        case ROOK:
            promotion_letter = "r";
            break;
        case BISHOP:
            promotion_letter = "b";
            break;
        case KNIGHT:
            promotion_letter = "n";
            break;
        default:
            promotion_letter = "";
    }

    return SQUARE_NAMES[(int)from_square] + SQUARE_NAMES[(int)to_square] + promotion_letter;
}
