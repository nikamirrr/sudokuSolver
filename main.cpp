//
//  main.cpp
//  sudokuSolver
//
//  Created by Nikolay Mirin on 3/23/20.
//  Copyright © 2020 Test. All rights reserved.
//

#include <iostream>
#include <vector>
#include <stack>
#include <unordered_set>
using namespace std;

const unsigned short int     CONTENT_MASK = 0b0000000000001111;
const unsigned short int  NOTFILLED_MASK  = 0b0000000000010000;
const unsigned short int  ALLOPTIONS_MASK = 0b0011111111100000;


unsigned short int rowByCell(unsigned short int cell) {
    return cell / 9;
}

unsigned short int colByCell(unsigned short int cell) {
    return cell % 9;
}

unsigned short int blockByCell(unsigned short int cell) {
    return (rowByCell(cell) / 3) * 3 + colByCell(cell) / 3;
}

vector<unsigned short int> cellsByRow(unsigned short int r) {
    vector<unsigned short int> result(9, r * 9);
    for (unsigned short int c = 0; c < 9; c++) {
        result[c] += c;
    }
    return result;
}

vector<unsigned short int> cellsByCol(unsigned short int c) {
    vector<unsigned short int> result(9, c);
    for (unsigned short int r = 0; r < 9; r++) {
        result[r] += 9 * r;
    }
    return result;
}

vector<unsigned short int> cellsByBlock(unsigned short int b) {
    vector<unsigned short int> result(9, 0);
    unsigned short int startR = (b / 3) * 3;
    unsigned short int startC = (b % 3) * 3;
    for (unsigned short int o = 0; o < 9; o++) {
        result[o] = (startR + o / 3) * 9 + startC + o % 3;
    }
    return result;
}

unordered_set<unsigned short int> relatedCells(unsigned short int cell) {
    unordered_set<unsigned short int> result;
    auto d1 = cellsByCol(colByCell(cell));
    auto d2 = cellsByRow(rowByCell(cell));
    auto d3 = cellsByBlock(blockByCell(cell));
    result.insert(d1.cbegin(), d1.cend());
    result.insert(d2.cbegin(), d2.cend());
    result.insert(d3.cbegin(), d3.cend());
    return result;
}

void updateRelCells(vector<unsigned short int>& board, unsigned short int cell, unsigned short int value) {
    for (auto relCell: relatedCells(cell)) {
        auto &relValue = board[relCell];
        if ((relValue & NOTFILLED_MASK) == NOTFILLED_MASK) {
            relValue &= ~(NOTFILLED_MASK << value);
        }
    }
}

unsigned short int  bitCount16(unsigned short int value) {
    value = (value & 0b0101010101010101) + ((value & 0b1010101010101010) >> 1);
    value = (value & 0b0011001100110011) + ((value & 0b1100110011001100) >> 2);
    value = (value & 0b0000111100001111) + ((value & 0b1111000011110000) >> 4);
    value = (value & 0b0000000011111111) + ((value & 0b1111111100000000) >> 8);
    return value;
}

bool findTheBestEmptyCell(vector<unsigned short int>& board, stack<unsigned short int>& moves) {
    for (unsigned short cell = 0; cell < 81; cell++) {
        auto &value = board[cell];
        if ((value & NOTFILLED_MASK) == NOTFILLED_MASK) {
            value |= ALLOPTIONS_MASK;
        }
    }
    for (unsigned short cell = 0; cell < 81; cell++) {
        auto value = board[cell];
        if ((value & NOTFILLED_MASK) != NOTFILLED_MASK) {
            updateRelCells(board, cell, value);
        }
    }
    
    
    unsigned short int bestCell = 81;
    unsigned short int bestCellOptions = 10;
    for (unsigned short int cell = 0; cell < 81; cell++) {
        auto value = board[cell];
        if ((value & NOTFILLED_MASK) == NOTFILLED_MASK) {
            auto newOptions = bitCount16(value >> 5);
            if (newOptions < bestCellOptions) {
                bestCell = cell;
                bestCellOptions = newOptions;
                if (bestCellOptions == 1) {
                    board[bestCell] &= ~NOTFILLED_MASK;
                    moves.push(bestCell);
                    return true;
                }
            }
        }
    }
    if (bestCellOptions == 10) {
        return false;
    }
    board[bestCell] &= ~NOTFILLED_MASK;
    moves.push(bestCell);
    return true;
    
}

void outputSolution(const vector<unsigned short int>& board) {
    for (unsigned short int cell = 0; cell < 81; cell++) {
        auto value = board[cell];
        if ((value & NOTFILLED_MASK) == NOTFILLED_MASK) {
            cout << ".";
        } else {
            cout << (value & CONTENT_MASK);
        }
        if (cell % 9 == 8) {
            cout << endl;
            if ((cell % 27) == 26) {
                cout << endl;
            }
        } else if ((cell % 3) == 2) {
            cout << " ";
        }

    }
}

void solveSudoku(vector<unsigned short int>& board) {
    stack<unsigned short int> moves;
    if (findTheBestEmptyCell(board, moves)) {
        while (true) {
            auto nextCell = moves.top();
            auto &value = board[nextCell];
            auto options =  value >> 5;
            
            if (options == 0) {
                value |= NOTFILLED_MASK;
                moves.pop();
                if (moves.empty()) {
                    cout << "No solution exists!" << endl;
                    return;
                } else {
                    continue;
                }
            }
            
            unsigned short int choice = 1;
            while ((options & 1) == 0) {
                options >>= 1;
                choice++;
            }
            value &= ~CONTENT_MASK;
            value &= ~(NOTFILLED_MASK << choice);
            value |= choice;
            //cout << endl;
            //outputSolution(board);
            //cout << endl;
            if (!findTheBestEmptyCell(board, moves)) {
                break;
            }
            
        }
    }
    outputSolution(board);
    
}

int main(int argc, const char * argv[]) {
    vector<unsigned short int> board(81, 0b11111111110000);
    for (unsigned short int i = 0; i < 81; i++) {
        char c;
        if (cin >> c) {
            if (c != '.') {
                if (c >= '1' && c <='9') {
                    board[i] = c - '0';
                } else {
                    return 2;
                }
            }
        } else {
            return 1;
        }
    }

    solveSudoku(board);
    
    return 0;
}
