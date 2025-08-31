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

// each sudoku board is 81 16-bit ints
// a row/col can be obtanited using / and % operators

// first 4 bit in each cell is the number filled 1-9
const unsigned short int     CONTENT_MASK = 0b0000000000001111;
// bit 5 is the flag that the cell is available
const unsigned short int  NOTFILLED_MASK  = 0b0000000000010000;
// bit mask for possible numbers in the cell 1-9, bits 6-14
// a flag can be calculated using NOTFILLED_MASK << NUMBER_TO_FILL
const unsigned short int  ALLOPTIONS_MASK = 0b0011111111100000;


// Helper functions to convert cell indices 0-80 to row, column, and block
unsigned short int rowByCell(unsigned short int cell) {
    return cell / 9;
}

unsigned short int colByCell(unsigned short int cell) {
    return cell % 9;
}

unsigned short int blockByCell(unsigned short int cell) {
    return (rowByCell(cell) / 3) * 3 + colByCell(cell) / 3;
}

// Helper functions to list cells (0-80) by row, columns, or block
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

// A function to calculate all related cells (same row, column, or block) based on the cell index 0-80
const unordered_set<unsigned short int> & relatedCells(unsigned short int cell) {
    static vector<unordered_set<unsigned short int>> cache(81, unordered_set<unsigned short int>());
    unordered_set<unsigned short int> &result=cache[cell];
    if (result.size() == 0) {
       auto d1 = cellsByCol(colByCell(cell));
       auto d2 = cellsByRow(rowByCell(cell));
       auto d3 = cellsByBlock(blockByCell(cell));
       result.insert(d1.cbegin(), d1.cend());
       result.insert(d2.cbegin(), d2.cend());
       result.insert(d3.cbegin(), d3.cend());
    }
    return result;
}

// A function to remove the number from avaialble flags of all related cells
void updateRelCells(vector<unsigned short int>& board, unsigned short int cell, unsigned short int value) {
    for (auto relCell: relatedCells(cell)) {
        auto &relValue = board[relCell];
        if ((relValue & NOTFILLED_MASK) == NOTFILLED_MASK) {
            relValue &= ~(NOTFILLED_MASK << value);
        }
    }
}

// bit counte for 16-bit number
unsigned short int  bitCount16(unsigned short int value) {
    value = (value & 0b0101010101010101) + ((value & 0b1010101010101010) >> 1);
    value = (value & 0b0011001100110011) + ((value & 0b1100110011001100) >> 2);
    value = (value & 0b0000111100001111) + ((value & 0b1111000011110000) >> 4);
    value = (value & 0b0000000011111111) + ((value & 0b1111111100000000) >> 8);
    return value;
}


// a function to find a next not-filled cell with the least number of options
bool findTheBestEmptyCell(vector<unsigned short int>& board, stack<unsigned short int>& moves, unordered_set<unsigned short int> &filled, unordered_set<unsigned short int> &empty) {
    // set all not-filled cells flags to all options
    for (unsigned short cell: empty) {
        board[cell] |= ALLOPTIONS_MASK;
    }
    // for every filled cell, update non-filled related cells to remove the options already taken
    for (unsigned short cell: filled) {
        updateRelCells(board, cell, board[cell] & CONTENT_MASK);
    }
    
    // scan the board and find the cell with least available options 
    unsigned short int bestCell = 81;
    unsigned short int bestCellOptions = 10;
    for (unsigned short int cell: empty) {
        auto value = board[cell];
        auto newOptions = bitCount16(value >> 5);
        if (newOptions < bestCellOptions) {
            bestCell = cell;
            bestCellOptions = newOptions;
            if (bestCellOptions == 1) {
                board[bestCell] &= ~NOTFILLED_MASK;
                empty.erase(bestCell);
                filled.insert(bestCell);
                moves.push(bestCell);
                return true;
            }
        }
    }
    if (bestCellOptions == 10) {
        return false;
    }
    // if found, make the cell as temporarily filled, and push to the moves stack
    board[bestCell] &= ~NOTFILLED_MASK;
    empty.erase(bestCell);
    filled.insert(bestCell);
    moves.push(bestCell);
    return true;
    
}

// a function to output soluction
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

// a back-tracking solver for sudoku
void solveSudoku(vector<unsigned short int>& board, unordered_set<unsigned short int> &filled, unordered_set<unsigned short int> &empty) {
    stack<unsigned short int> moves; // moves stack
    if (findTheBestEmptyCell(board, moves, filled, empty)) { // find the cell with fewest options
        while (true) {
            // read the cell on the top of the stack, access is value
            auto nextCell = moves.top();
            auto &value = board[nextCell];
            auto options =  value >> 5;
            // exaine what options are avaiable for the cell 
            if (options == 0) {
                // if none-available, it is a dead-end, mark the cell as available and remove the cell from the stack
                value |= NOTFILLED_MASK;
                empty.insert(nextCell);
                filled.erase(nextCell);
                moves.pop();
                // if the stack is empty, then no solution for Sudoku
                if (moves.empty()) {
                    cout << "No solution exists!" << endl;
                    return;
                } else {
                    continue;
                }
            }
            // determine the next available choice for the cell, test bit by bit
            unsigned short int choice = 1;
            while ((options & 1) == 0) {
                options >>= 1;
                choice++;
            }
            // reset the content bits to 0
            value &= ~CONTENT_MASK;
            value &= ~(NOTFILLED_MASK << choice); // remove the choice from options list
            value |= choice; //  set them to the new choice,
     
            // find the next move and put to the stack 
            if (!findTheBestEmptyCell(board, moves, filled, empty)) {
                break;
            }
            
        }
    }
    // reached here, solution is ready
    outputSolution(board);
    
}

int main() {
    vector<unsigned short int> board(81, 0b11111111110000); // 0-80 cells for the board, mark everything empty  and available
    unordered_set<unsigned short int> filled, empty;
    for (unsigned short int i = 0; i < 81; i++) { // read 81 character
        char c;
        if (cin >> c) {
            if (c != '.') { // if not ".", mark as the filled and set the value
                if (c >= '1' && c <='9') {
                    board[i] = c - '0';
                    filled.insert(i);
                } else {
                    return 2;
                }
            } else {
               empty.insert(i);
            }
        } else {
            return 1;
        }
    }
    // solve sudoku and output solution
    solveSudoku(board, filled, empty);
    
    return 0;
}
