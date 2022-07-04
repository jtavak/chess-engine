#include <iostream>

#include "baseboard.h"
#include "board.h"

using namespace std;

int main(){
    Board b = Board();

    b.print();
    cout << endl;

    b.push(Move(E2, E4));
    b.print();
    cout << endl;

    b.push(Move(E7, E5));
    b.print();
    cout << endl;

    b.pop();
    b.print();
    cout << endl;

    return 0;
}