#include "constants.h"
#include "move.h"

#include <string>

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

bool Move::operator != (Move move){
    if(move.from_square == from_square && move.to_square == to_square && move.promotion == promotion){
        return false;
    }
    return true;
}

std::string Move::toUCI(){
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

Move::Move(std::string uci){
    if(uci.length() == 4){
        for(int i = 0; i < 64; i++){
            if(uci.substr(0, 2) == SQUARE_NAMES[i]){
                from_square = i;
            }

            if(uci.substr(2, 2) == SQUARE_NAMES[i]){
                to_square = i;
            }
        }
        promotion = NO_PIECE;
    } else if(uci.length() == 5){
        for(int i = 0; i < 64; i++){
            if(uci.substr(0, 2) == SQUARE_NAMES[i]){
                from_square = i;
            }

            if(uci.substr(2, 2) == SQUARE_NAMES[i]){
                to_square = i;
            }
        }

        char piece_codes[4] = {'n', 'b', 'r', 'q'};
        for(int i = 0; i < 6; i++){
            if(uci.at(4) == piece_codes[i]){
                promotion = i+2;
            }
        }
    }
}
