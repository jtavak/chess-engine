#include <iostream>

#include "baseboard.h"
#include "board.h"
#include "engine.h"

using namespace std;

int main(){
    Board b = Board("rnbqkbnr/ppPppppp/8/8/8/8/PP1PPPPP/RNBQKBNR w KQkq - 0 1");

    b.pushUCI("c7b8n");
    b.print();
    cout << endl;

    return 0;
}