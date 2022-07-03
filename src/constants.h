#pragma once

#include <cstdint>
#include <vector>
#include <string>

typedef int Color;
const Color WHITE = 1;
const Color BLACK = 0;
const Color NO_COLOR = -1;

typedef int PieceType;
const PieceType NO_PIECE = 0;
const PieceType PAWN = 1;
const PieceType KNIGHT = 2;
const PieceType BISHOP = 3;
const PieceType ROOK = 4;
const PieceType QUEEN = 5;
const PieceType KING = 6;

typedef uint8_t Square;

const Square NO_SQUARE = -1;

const Square A1=0,  B1=1,  C1=2,  D1=3,  E1=4,  F1=5,  G1=6,  H1=7,
             A2=8,  B2=9,  C2=10, D2=11, E2=12, F2=13, G2=14, H2=15,
             A3=16, B3=17, C3=18, D3=19, E3=20, F3=21, G3=22, H3=23,
             A4=24, B4=25, C4=26, D4=27, E4=28, F4=29, G4=30, H4=31,
             A5=32, B5=33, C5=34, D5=35, E5=36, F5=37, G5=38, H5=39,
             A6=40, B6=41, C6=42, D6=43, E6=44, F6=45, G6=46, H6=47,
             A7=48, B7=49, C7=50, D7=51, E7=52, F7=53, G7=54, H7=55,
             A8=56, B8=57, C8=58, D8=59, E8=60, F8=61, G8=62, H8=63;

const Square SQUARES[64] = {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
};


typedef uint64_t BitBoard;

const BitBoard BB_ONE = 1ULL;

const BitBoard BB_A1 = BB_ONE << A1, BB_B1 = BB_ONE << B1, BB_C1 = BB_ONE << C1, BB_D1 = BB_ONE << D1, BB_E1 = BB_ONE << E1, BB_F1 = BB_ONE << F1, BB_G1 = BB_ONE << G1, BB_H1 = BB_ONE << H1, 
               BB_A2 = BB_ONE << A2, BB_B2 = BB_ONE << B2, BB_C2 = BB_ONE << C2, BB_D2 = BB_ONE << D2, BB_E2 = BB_ONE << E2, BB_F2 = BB_ONE << F2, BB_G2 = BB_ONE << G2, BB_H2 = BB_ONE << H2, 
               BB_A3 = BB_ONE << A3, BB_B3 = BB_ONE << B3, BB_C3 = BB_ONE << C3, BB_D3 = BB_ONE << D3, BB_E3 = BB_ONE << E3, BB_F3 = BB_ONE << F3, BB_G3 = BB_ONE << G3, BB_H3 = BB_ONE << H3, 
               BB_A4 = BB_ONE << A4, BB_B4 = BB_ONE << B4, BB_C4 = BB_ONE << C4, BB_D4 = BB_ONE << D4, BB_E4 = BB_ONE << E4, BB_F4 = BB_ONE << F4, BB_G4 = BB_ONE << G4, BB_H4 = BB_ONE << H4, 
               BB_A5 = BB_ONE << A5, BB_B5 = BB_ONE << B5, BB_C5 = BB_ONE << C5, BB_D5 = BB_ONE << D5, BB_E5 = BB_ONE << E5, BB_F5 = BB_ONE << F5, BB_G5 = BB_ONE << G5, BB_H5 = BB_ONE << H5, 
               BB_A6 = BB_ONE << A6, BB_B6 = BB_ONE << B6, BB_C6 = BB_ONE << C6, BB_D6 = BB_ONE << D6, BB_E6 = BB_ONE << E6, BB_F6 = BB_ONE << F6, BB_G6 = BB_ONE << G6, BB_H6 = BB_ONE << H6,
               BB_A7 = BB_ONE << A7, BB_B7 = BB_ONE << B7, BB_C7 = BB_ONE << C7, BB_D7 = BB_ONE << D7, BB_E7 = BB_ONE << E7, BB_F7 = BB_ONE << F7, BB_G7 = BB_ONE << G7, BB_H7 = BB_ONE << H7,
               BB_A8 = BB_ONE << A8, BB_B8 = BB_ONE << B8, BB_C8 = BB_ONE << C8, BB_D8 = BB_ONE << D8, BB_E8 = BB_ONE << E8, BB_F8 = BB_ONE << F8, BB_G8 = BB_ONE << G8, BB_H8 = BB_ONE << H8;

const BitBoard BB_SQUARES[64] = {
    BB_A1, BB_B1, BB_C1, BB_D1, BB_E1, BB_F1, BB_G1, BB_H1,
    BB_A2, BB_B2, BB_C2, BB_D2, BB_E2, BB_F2, BB_G2, BB_H2,
    BB_A3, BB_B3, BB_C3, BB_D3, BB_E3, BB_F3, BB_G3, BB_H3,
    BB_A4, BB_B4, BB_C4, BB_D4, BB_E4, BB_F4, BB_G4, BB_H4,
    BB_A5, BB_B5, BB_C5, BB_D5, BB_E5, BB_F5, BB_G5, BB_H5,
    BB_A6, BB_B6, BB_C6, BB_D6, BB_E6, BB_F6, BB_G6, BB_H6,
    BB_A7, BB_B7, BB_C7, BB_D7, BB_E7, BB_F7, BB_G7, BB_H7,
    BB_A8, BB_B8, BB_C8, BB_D8, BB_E8, BB_F8, BB_G8, BB_H8
};

const BitBoard BB_FILE_A = 0x0101010101010101, 
               BB_FILE_B = 0x0101010101010101 << 1, 
               BB_FILE_C = 0x0101010101010101 << 2, 
               BB_FILE_D = 0x0101010101010101 << 3, 
               BB_FILE_E = 0x0101010101010101 << 4, 
               BB_FILE_F = 0x0101010101010101 << 5, 
               BB_FILE_G = 0x0101010101010101 << 6, 
               BB_FILE_H = 0x0101010101010101 << 7;

const BitBoard BB_RANK_1 = 0xff,
               BB_RANK_2 = BB_RANK_1 << (8*1),
               BB_RANK_3 = BB_RANK_1 << (8*2),
               BB_RANK_4 = BB_RANK_1 << (8*3),
               BB_RANK_5 = BB_RANK_1 << (8*4),
               BB_RANK_6 = BB_RANK_1 << (8*5),
               BB_RANK_7 = BB_RANK_1 << (8*6),
               BB_RANK_8 = BB_RANK_1 << (8*7);

const BitBoard BB_EMPTY = 0x0;
const BitBoard BB_ALL = 0xffffffffffffffff;

const BitBoard BB_CORNERS = BB_A1 | BB_H1 | BB_A8 | BB_H8;
const BitBoard BB_CENTER = BB_D4 | BB_E4 | BB_D5 | BB_E5;

const BitBoard BB_LIGHT_SQUARES = 0x55aa55aa55aa55aa;
const BitBoard BB_DARK_SQUARES = 0xaa55aa55aa55aa55;

const BitBoard BB_BACKRANKS = BB_RANK_1 | BB_RANK_8;

const std::string SQUARE_NAMES[64] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", 
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", 
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", 
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", 
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", 
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", 
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", 
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};