#include <iostream>

#include "baseboard.h"
#include "board.h"

using namespace std;

int main(){
    Board b = Board("r3k2r/8/2N5/8/8/8/8/4K3 b kq - 0 1");

    std::vector<Move> moves = b.generatePseudoLegalMoves();

    for(int i = 0; i < moves.size(); i++){
        
        cout << SQUARE_NAMES[(int)moves.at(i).from_square] << SQUARE_NAMES[(int)moves.at(i).to_square] << endl;
    }

    return 0;
}