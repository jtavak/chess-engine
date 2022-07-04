#include <string>
#include <iostream>
#include <sstream>

#include "board.h"
#include "move.h"
#include "constants.h"

Board::Board(std::string fen){
    resetBoard();
    setBoardFEN(fen);
}

Board::Board(){
    resetBoard();
}

void Board::resetBoard(){
    BaseBoard::resetBoard();

    turn = WHITE;
    castling_rights = BB_CORNERS;
    ep_square = NO_SQUARE;
    fullmove_number = 1;
    halfmove_clock = 0;

    clearStack();
}

void Board::clearBoard(){
    BaseBoard::clearBoard();

    turn = WHITE;
    castling_rights = BB_EMPTY;
    ep_square = NO_SQUARE;
    fullmove_number = 1;
    halfmove_clock = 0;

    clearStack();
}

void Board::clearStack(){
    move_stack.clear();
    state_stack.clear();
}

void Board::push(Move move){
    // add current position to stack
    state_stack.push_back(BoardState(this));
    move_stack.push_back(move);

    Square prev_ep_square = ep_square;
    ep_square = NO_SQUARE;

    halfmove_clock++;
    if(turn == BLACK){
        fullmove_number++;
    }

    // reset halfmove clock if move is pawn advance or capture
    if(isZeroing(move)){
        halfmove_clock = 0;
    }

    BitBoard from_bb = BB_SQUARES[move.from_square];
    BitBoard to_bb = BB_SQUARES[move.to_square];

    PieceType piece_type = removePieceAt(move.from_square);
    Square capture_square = move.to_square;
    PieceType capture_piece_type = pieceTypeAt(capture_square);

    // update castling rights
    castling_rights &= (~from_bb & ~to_bb);
    if(piece_type == KING){
        if(turn == WHITE){
            castling_rights &= ~BB_RANK_1;
        } else {
            castling_rights &= ~BB_RANK_8;
        }
    }

    // handle special pawn moves
    if(piece_type == PAWN){
        int diff = move.to_square - move.from_square;

        if(diff == 16 && squareRank(move.from_square) == 1){
            ep_square = move.from_square + 8;
        } else if (diff == -16 && squareRank(move.from_square) == 6){
            ep_square = move.from_square - 8;
        } else if (move.to_square == ep_square && (abs(diff) == 7 || abs(diff) == 9) && capture_piece_type == NO_PIECE){
            int down = (turn == WHITE) ? -8 : 8;
            capture_square = ep_square + down;
            capture_piece_type = removePieceAt(capture_square);
        }
    }

    // handle pawn promotions
    bool promoted = false;
    if(move.promotion != NO_PIECE){
        promoted = true;
        piece_type = move.promotion;
    }

    // handle castling
    bool castling = piece_type == KING && (occupied_color[turn] & to_bb);
    if(castling){
        bool a_side = squareFile(move.to_square) < squareFile(move.from_square);

        removePieceAt(move.from_square);
        removePieceAt(move.to_square);

        if(a_side){
            setPieceAt((turn == WHITE) ? C1 : C8, KING, turn);
            setPieceAt((turn == WHITE) ? D1 : D8, ROOK, turn);
        } else {
            setPieceAt((turn == WHITE) ? G1 : G8, KING, turn);
            setPieceAt((turn == WHITE) ? F1 : F8, ROOK, turn);
        }
    } else {
        setPieceAt(move.to_square, piece_type, turn);
    }

    turn = (turn==WHITE) ? BLACK : WHITE;
}

Move Board::pop(){
    Move move = move_stack.back();
    move_stack.pop_back();

    BoardState bs = state_stack.back();
    state_stack.pop_back();

    bs.restore(this);

    return move;
}

bool Board::attackedForKing(BitBoard path, BitBoard occupied_squares){
    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;
    while(path){
        if(attackersMask(opp_turn, lsb(path), occupied_squares)){
            return true;
        }
        path &= (path - 1);
    }
    return false;
}

std::vector<Move> Board::generatePseudoLegalMoves(BitBoard from_mask, BitBoard to_mask){
    std::vector<Move> moves;

    BitBoard our_pieces = occupied_color[turn];

    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;

    // generate piece moves
    BitBoard non_pawns = our_pieces & ~pawns & from_mask;
    while(non_pawns){
        Square from_square = lsb(non_pawns);
        
        BitBoard possible_moves = attacksMask(from_square) & ~our_pieces & to_mask;
        while(possible_moves){
            Square to_square = lsb(possible_moves);

            moves.push_back(Move(from_square, to_square));

            possible_moves &= (possible_moves - 1);
        }

        non_pawns &= (non_pawns - 1);
    }

    // generate castling moves
    BitBoard backrank = (turn == WHITE) ? BB_RANK_1 : BB_RANK_8;

    BitBoard bb_c = BB_FILE_C & backrank;
    BitBoard bb_d = BB_FILE_D & backrank;
    BitBoard bb_f = BB_FILE_F & backrank;
    BitBoard bb_g = BB_FILE_G & backrank;

    BitBoard king_mask = our_pieces & kings & backrank & from_mask; 

    BitBoard candidates = castling_rights & backrank & to_mask;
    while(candidates){
        BitBoard rook_mask = BB_SQUARES[lsb(candidates)];

        BitBoard king_to = (rook_mask < king_mask) ? bb_c : bb_g;
        BitBoard rook_to = (rook_mask < king_mask) ? bb_d : bb_f;

        BitBoard king_path = between(lsb(king_mask), lsb(king_to));
        BitBoard rook_path = between(lsb(candidates), lsb(rook_to));

        if(!(((occupied ^ king_mask ^ rook_mask) & (king_path | rook_path | king_to | rook_to)) || 
            attackedForKing(king_path | king_mask, occupied ^ king_mask) ||
            attackedForKing(king_to, occupied ^ king_mask ^ rook_mask ^ rook_to))){

                if(lsb(king_mask) == E1 && (kings & BB_E1)){
                    if(lsb(candidates) == H1){
                        moves.push_back(Move(E1, G1));
                    } else if (lsb(candidates) == A1){
                        moves.push_back(Move(E1, C1));
                    }
                } else if (lsb(king_mask) == E8 && (kings & BB_E8)){
                    if(lsb(candidates) == H8){
                        moves.push_back(Move(E8, G8));
                    } else if (lsb(candidates) == A8){
                        moves.push_back(Move(E8, C8));
                    }
                }
            
        }

        candidates &= (candidates - 1);
    }

    // generate pawn captures
    BitBoard capturers = pawns & occupied_color[turn] & from_mask;
    if(!capturers){
        return moves;
    }

    while(capturers){
        Square from_square = lsb(capturers);
        BitBoard targets = BB_PAWN_ATTACKS[turn][from_square] & occupied_color[opp_turn] & to_mask;

        while(targets){
            Square to_square = lsb(targets);
            if(squareRank(to_square) == 0 || squareRank(to_square) == 7){
                moves.push_back(Move(from_square, to_square, QUEEN));
                moves.push_back(Move(from_square, to_square, ROOK));
                moves.push_back(Move(from_square, to_square, BISHOP));
                moves.push_back(Move(from_square, to_square, KNIGHT));
            } else {
                moves.push_back(Move(from_square, to_square));
            }

            targets &= (targets - 1);
        }

        capturers &= (capturers - 1);
    }

    // prepare pawn advance generation
    BitBoard movable_pawns = pawns & occupied_color[turn] & from_mask;
    BitBoard single_moves, double_moves;
    if(turn == WHITE){
        single_moves = (movable_pawns << 8) & ~occupied;
        double_moves = (single_moves << 8) & ~occupied & (BB_RANK_3 | BB_RANK_4);
    } else {
        single_moves = (movable_pawns >> 8) & ~occupied;
        double_moves = (single_moves >> 8) & ~occupied & (BB_RANK_6 | BB_RANK_5);
    }

    single_moves &= to_mask;
    double_moves &= to_mask;

    // generate single pawn moves
    while(single_moves){
        Square to_square = lsb(single_moves);
        int delta = (turn == BLACK) ? 8 : -8;

        Square from_square = to_square + delta;

        if(squareRank(to_square) == 0 || squareRank(to_square) == 7){
            moves.push_back(Move(from_square, to_square, QUEEN));
            moves.push_back(Move(from_square, to_square, ROOK));
            moves.push_back(Move(from_square, to_square, BISHOP));
            moves.push_back(Move(from_square, to_square, KNIGHT));
        } else {
            moves.push_back(Move(from_square, to_square));
        }

        single_moves &= (single_moves - 1);
    }

    //generate double pawn moves
    while(double_moves){
        Square to_square = lsb(double_moves);
        int delta = (turn == BLACK) ? 16 : -16;

        Square from_square = to_square + delta;

        moves.push_back(Move(from_square, to_square));

        double_moves &= (double_moves - 1);
    }

    //generate en passant captures
    if(ep_square && (BB_SQUARES[ep_square] & to_mask) && !(BB_SQUARES[ep_square] & occupied)){
        BitBoard ep_rank = (turn == WHITE) ? BB_RANK_5 : BB_RANK_4;
        BitBoard capturers = pawns & occupied_color[turn] & from_mask &
                             BB_PAWN_ATTACKS[opp_turn][ep_square] & ep_rank;

        while(capturers){
            Square from_square = lsb(capturers);
            moves.push_back(Move(from_square, ep_square));

            capturers &= (capturers - 1);
        }
    }

    return moves;
}

std::vector<Move> Board::generatePseudoLegalMoves(){
    return generatePseudoLegalMoves(BB_ALL, BB_ALL);
}

Move Board::generatePseudoLegalEP(BitBoard from_mask, BitBoard to_mask){
    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;

    BitBoard ep_rank = (turn == WHITE) ? BB_RANK_5 : BB_RANK_4;
    BitBoard capturers = pawns & occupied_color[turn] & from_mask &
                            BB_PAWN_ATTACKS[opp_turn][ep_square] & ep_rank;

    while(capturers){
        Square from_square = lsb(capturers);
        return(Move(from_square, ep_square));

        capturers &= (capturers - 1);
    }

    return NO_MOVE;
}

Move Board::generatePseudoLegalEP(){
    return generatePseudoLegalEP(BB_ALL, BB_ALL);
}

bool Board::isPseudoLegal(Move move){
    for(auto pseudo_legal_move: generatePseudoLegalMoves()){
        if(pseudo_legal_move == move){
            return true;
        }
    }
    return false;
}

bool Board::isCastling(Move move){
    if(kings & BB_SQUARES[move.from_square]){
        int diff = squareFile(move.from_square) - squareFile(move.to_square);
        return (abs(diff) > 1) || (bool)(rooks & occupied_color[turn] & BB_SQUARES[move.to_square]);
    }
    return false;
}

bool Board::isEnPassant(Move move){
    return (ep_square == move.to_square) &&
           (bool)(pawns & BB_SQUARES[move.from_square]) && 
           (abs(move.to_square - move.from_square) == 7 || abs(move.to_square - move.from_square) == 9) &&
           !(occupied & BB_SQUARES[move.to_square]);
}

bool Board::isIntoCheck(Move move){
    Square king_square = king(turn);

    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;

    BitBoard checkers = attackersMask(opp_turn, king_square);
    if(checkers){
        for(auto evasion: generateEvasions(king_square, checkers, BB_SQUARES[move.from_square], BB_SQUARES[move.to_square])){
            if(evasion == move){
                return !isSafe(king_square, sliderBlockers(king_square), move);
            }
        }
        return true;
    }
    return !isSafe(king_square, sliderBlockers(king_square), move);

}

bool Board::isZeroing(Move move){
    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;
    BitBoard touched = BB_SQUARES[move.from_square] ^ BB_SQUARES[move.to_square];
    return (touched & pawns) || (touched & occupied_color[opp_turn]);
}

bool Board::isLegal(Move move){
    return isPseudoLegal(move) && !isIntoCheck(move);
}

BitBoard Board::checkersMask(){
    Square our_king = king(turn);
    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;

    return attackersMask(opp_turn, our_king);
}

bool Board::isCheck(){
    return (bool)checkersMask();
}

bool Board::isCheckmate(){
    if(!isCheck()){
        return false;
    }

    return generateLegalMoves().empty();
}

bool Board::isStalemate(){
    if(isCheck()){
        return false;
    }
    return generateLegalMoves().empty();
}

bool Board::EPSkewered(Square king_square, Square capturer_square){
    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;
    int delta = (turn == WHITE) ? -8 : 8;
    Square last_double = ep_square + delta;

    BitBoard occupancy = occupied & ~BB_SQUARES[last_double] & ~BB_SQUARES[capturer_square] | BB_SQUARES[ep_square];

    BitBoard horizontal_attackers = occupied_color[opp_turn] & (rooks | queens);
    if(BB_RANK_ATTACKS[king_square][BB_RANK_MASKS[king_square] & occupancy] & horizontal_attackers){
        return true;
    }

    BitBoard diagonal_attackers = occupied_color[opp_turn] & (bishops | queens);
    if(BB_DIAG_ATTACKS[king_square][BB_DIAG_MASKS[king_square] & occupancy] & diagonal_attackers){
        return true;
    }

    return false;
}

BitBoard Board::sliderBlockers(Square king_square){
    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;

    BitBoard rooks_and_queens = rooks | queens;
    BitBoard bishops_and_queens = bishops | queens;

    BitBoard snipers = (BB_RANK_ATTACKS[king_square][0] & rooks_and_queens) |
                       (BB_FILE_ATTACKS[king_square][0] & rooks_and_queens) |
                       (BB_DIAG_ATTACKS[king_square][0] & bishops_and_queens);

    BitBoard blockers = BB_EMPTY;

    BitBoard opp_snipers = snipers & occupied_color[opp_turn];

    while(opp_snipers){
        Square sniper = lsb(opp_snipers);

        BitBoard b = between(king_square, sniper) & occupied;

        if(b && (BB_SQUARES[lsb(b)] == b)){
            blockers |= b;
        }

        opp_snipers &= (opp_snipers - 1);
    }

    return blockers & occupied_color[turn];
}

bool Board::isSafe(Square king_square, BitBoard blockers, Move move){
    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;

    if(move.from_square == king_square){
        if(isCastling(move)){
            return true;
        } else {
            return !isAttackedBy(opp_turn, move.to_square);
        }
    }

    if(isEnPassant(move)){
        return (pinMask(turn, move.from_square) & BB_SQUARES[move.to_square]) &&
               !EPSkewered(king_square, move.from_square);
    }

    return !(blockers & BB_SQUARES[move.from_square]) ||
           (ray(move.from_square, move.to_square) & BB_SQUARES[king_square]);
}

std::vector<Move> Board::generateEvasions(Square king_square, BitBoard checkers, BitBoard from_mask, BitBoard to_mask){
    std::vector<Move> moves;

    BitBoard sliders = checkers & (bishops | rooks | queens);

    BitBoard attacked = BB_EMPTY;
    while(sliders){
        Square checker = lsb(sliders);

        attacked |= ray(king_square, checker) & ~BB_SQUARES[checker];

        sliders &= (sliders - 1);
    }

    if(BB_SQUARES[king_square] & from_mask){
        BitBoard to_squares = BB_KING_ATTACKS[king_square] & ~occupied_color[turn] & ~attacked & to_mask;
        while(to_squares){
            moves.push_back(Move(king_square, lsb(to_squares)));
            to_squares &= (to_squares - 1);
        }
    }

    Square checker = msb(checkers);
    if(BB_SQUARES[checker] == checkers){
        BitBoard target = between(king_square, checker) | checkers;

        std::vector<Move> blocks_and_captures = generatePseudoLegalMoves(~kings & from_mask, target & to_mask);
        moves.insert(moves.end(), blocks_and_captures.begin(), blocks_and_captures.end());

        if(ep_square != NO_SQUARE && !(BB_SQUARES[ep_square] & target)){
            int delta = (turn == WHITE) ? -8 : 8;
            Square last_double = ep_square + delta;
            if(last_double == checker){
                moves.push_back(generatePseudoLegalEP(from_mask, to_mask));
            }
        }
    }

    return moves;
}

std::vector<Move> Board::generateLegalMoves(BitBoard from_mask, BitBoard to_mask){
    std::vector<Move> moves;

    Color opp_turn = (turn==WHITE) ? BLACK : WHITE;

    BitBoard king_mask = kings & occupied_color[turn];
    Square king_square = lsb(king_mask);

    BitBoard blockers = sliderBlockers(king_square);
    BitBoard checkers = attackersMask(opp_turn, king_square);

    if(checkers){
        for(auto move: generateEvasions(king_square, checkers, from_mask, to_mask)){
            if(isSafe(king_square, blockers, move)){
                moves.push_back(move);
            }
        }
    } else {
        for(auto move: generatePseudoLegalMoves(from_mask, to_mask)){
            if(isSafe(king_square, blockers, move)){
                moves.push_back(move);
            }
        }
    }

    return moves;
}

std::vector<Move> Board::generateLegalMoves(){
    return generateLegalMoves(BB_ALL, BB_ALL);
}

void Board::setBoardFEN(std::string fen){
    Board::clearBoard();

    std::istringstream iss(fen);
    std::string s;

    // position 
    iss >> s;
    BaseBoard::setBoardFEN(s);

    // turn
    iss >> s;
    if(s == "b"){
        turn = BLACK;
    } else {
        turn = WHITE;
    }

    // castling rights
    iss >> s;
    if(s == "-"){
        castling_rights = BB_EMPTY;
    } else {
        for(auto &flag: s){
            if(flag == 'K'){
                castling_rights |= BB_RANK_1 & BB_FILE_H;
            } else if (flag == 'Q'){
                castling_rights |= BB_RANK_1 & BB_FILE_A;
            } else if (flag == 'k'){
                castling_rights |= BB_RANK_8 & BB_FILE_H;
            } else if (flag == 'q'){
                castling_rights |= BB_RANK_8 & BB_FILE_A;
            }
        }
    }

    iss >> s;
    if(s == "-"){
        ep_square = NO_SQUARE;
    } else {
        int i = 0;
        while(i < 64){
            if(s == SQUARE_NAMES[i]){
                ep_square = i;
                break;
            }
            i++;
        }
    }

    // halfmove clock
    iss >> s;
    halfmove_clock = std::stoi(s);

    // fullmove
    iss >> s;
    fullmove_number = std::stoi(s);
}

void Board::print(){
    BaseBoard::print();
}

BoardState::BoardState(Board* board){
    pawns = board->pawns;
    bishops = board->bishops;
    knights = board->knights;
    rooks = board->rooks;
    queens = board->queens;
    kings = board->kings;

    occupied = board->occupied;
    occupied_color[WHITE] = board->occupied_color[WHITE];
    occupied_color[BLACK] = board->occupied_color[BLACK];

    turn = board->turn;
    castling_rights = board->castling_rights;
    ep_square = board->ep_square;
    fullmove_number = board->fullmove_number;
    halfmove_clock = board->halfmove_clock;
}

void BoardState::restore(Board* board){
    board->pawns = pawns;
    board->bishops = bishops;
    board->knights = knights;
    board->rooks = rooks;
    board->queens = queens;
    board->kings = kings;

    board->occupied = occupied;
    board->occupied_color[WHITE] = occupied_color[WHITE];
    board->occupied_color[BLACK] = occupied_color[BLACK];

    board->turn = turn;
    board->castling_rights = castling_rights;
    board->ep_square = ep_square;
    board->fullmove_number = fullmove_number;
    board->halfmove_clock = halfmove_clock;
    
}