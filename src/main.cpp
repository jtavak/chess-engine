#include <iostream>

#include "board.h"

using namespace std;

int main(){
    BaseBoard b = BaseBoard();

    printBitBoard(b.attacksMask(A8));

    return 0;
}