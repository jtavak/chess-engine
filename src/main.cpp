#include <iostream>

#include "engine.h"
#include "board.h"

using namespace std;

int main(){
    Board b = Board("r1bq1rk1/p1p2ppp/5n2/3p4/1b6/2NB4/PPP2PPP/R1BQ1RK1 w - - 2 10");

    while(b.gameOutcome() == NO_OUTCOME){
        b.print();
        cout << "Enter a move: ";

        string uci;
        cin >> uci;
        Move m = Move(uci);

        while(!b.isLegal(m)){
            cout << "Move not legal.\nEnter a move: ";
        }

        b.pushUCI(uci);

        std::pair<int, Move> best_move = searchRoot(b, 6);
        cout << best_move.second.toUCI() << endl;
        b.push(best_move.second);

        cout << endl;
    }

    return 0;
}