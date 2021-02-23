#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    unsigned char matrix[8];
    char num[10];
    while(1) {
        cout << "Írj be egy 8x8-as bináris mátrixot!" << endl;
        for (short i = 0; i < 8; i++) {
            cin >> num;
            matrix[i] = 0;
            for (short j = 0; j < 8; j++) {
                matrix[i] += (num[j]-48) << j;
            }
            matrix[i] = ~matrix[i];
        }
        for (short i = 0; i < 8; i++) {
            cout << "0x" << hex << setfill('0') << setw(2) << (int) matrix[i];
            if (i != 7) {
                cout << ", ";
            }
        }
        cout << endl;
        cout << "Folytatod a munkát? (i/n)" << endl;
        cin >> num;
        if (num[0] != 'i' && num[0] != 'I') {
            return 0;
        }
    }
    return 0;
}
