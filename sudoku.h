#ifndef SUDOKU_H
#define SUDOKU_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <set>

using namespace std;

const unsigned int p_l_max = 4;

struct sudoku_grid
{
    int value;
    set<int> p_values;
};

class Sudoku
{

private:
    unsigned int unsolved;
    sudoku_grid grid[9][9];
    set<int> p_square[3][3];
    set<int> p_line[2][9];

public:
    Sudoku();
    Sudoku(int v[9][9]);
    Sudoku(Sudoku &other);

    virtual ~Sudoku(){}

    void setGrid(const int v[9][9]);
    void getGrid(int v[9][9]) const;
    bool checkGrid() const;

    bool isSolved() const;
    bool isSolved(int x, int y) const;

    int  getValue(int x, int y) const;

private:
    void initGrid();
    void setValue(int x, int y, int v);

    void itemSolved();

    bool isPossibility(int x, int y, int v) const;
    int  getPossibility(int x, int y, unsigned int it) const;
    void insertPossibility(int x, int y, int v);
    bool removePossibility(int x, int y, int v);
    bool numberPossibilities(int x, int y, unsigned int n) const;

    bool checkInSquare(int x, int y, int v);
    bool checkInLines(int x, int y, int v);
    void removeInSquare(int x, int y, int v);
    void removeInLines(int x, int y, int v);

    bool emptyPossibilities() const;
    void printPossibilities() const;

    void fillPossibilities();
    void fillSharedPossibilities();

    bool solveValue(unsigned int p_nb, unsigned int p_it);

    bool recursiveSolve(unsigned int p_l);

    friend ostream& operator<<(ostream& os, const Sudoku& su);

    friend bool sudokuSolve(Sudoku& su);
};

#endif // SUDOKU_H
