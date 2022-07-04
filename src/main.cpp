#include <iostream>
#include <chrono>

#include "baseboard.h"
#include "board.h"

using namespace std;

int main(){
    Board b = Board();

    std::vector<Move> moves = b.generateLegalMoves();

    for(auto move: moves){
        cout << move.to_uci() << endl;
    }


    return 0;
}