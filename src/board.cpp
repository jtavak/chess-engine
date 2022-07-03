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

    return moves;
}

std::vector<Move> Board::generatePseudoLegalMoves(){
    return generatePseudoLegalMoves(BB_ALL, BB_ALL);
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

BoardState::BoardState(Board board){
    pawns = board.pawns;
    bishops = board.bishops;
    knights = board.knights;
    rooks = board.rooks;
    queens = board.queens;
    kings = board.kings;

    occupied = board.occupied;
    occupied_color[WHITE] = board.occupied_color[WHITE];
    occupied_color[BLACK] = board.occupied_color[BLACK];

    promoted = board.promoted;

    turn = board.turn;
    castling_rights = board.castling_rights;
    ep_square = board.ep_square;
    fullmove_number = board.fullmove_number;
    halfmove_clock = board.halfmove_clock;
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

    board->promoted = promoted;

    board->turn = turn;
    board->castling_rights = castling_rights;
    board->ep_square = ep_square;
    board->fullmove_number = fullmove_number;
    board->halfmove_clock = halfmove_clock;
    
}