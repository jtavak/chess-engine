#include <iostream>
#include <cstdint>

#include <chrono>

#include "engine.h"
#include "board.h"

using namespace std;

int main(){
    Board b = Board();

    ZobristTable table;
    initZobrist(table);

    while(b.gameOutcome() == NO_OUTCOME){
        b.print();
        cout << "Enter a move: ";

        string uci;
        cin >> uci;
        Move m = Move(uci);

        while(!b.isLegal(m)){
            cout << "Move not legal.\nEnter a move: ";
            cin >> uci;
            m = Move(uci);
        }

        b.pushUCI(uci);

        if(b.gameOutcome() != NO_OUTCOME){
            break;
        }

        std::pair<int, Move> best_move = searchRoot(b, 6, table);
        cout << best_move.second.toUCI() << endl;
        b.push(best_move.second);

        cout << endl;
    }

    return 0;
}