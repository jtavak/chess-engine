#include "baseboard.h"
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
