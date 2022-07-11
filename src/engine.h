#pragma once

#include "board.h"

struct ZobristTable{
    BitBoard pieces[64][6][2];
    BitBoard castling_rights[4];
    BitBoard ep_files[8];
    BitBoard black_to_move;
};

std::pair<int, Move> searchRoot(Board& b, int depth, const ZobristTable& table);
void initZobrist(ZobristTable& table);
uint64_t hashZobrist(const Board& b, const ZobristTable& table);