//Jacob Buhler
//solver.cpp
//Algorithm X implementation
#include <iostream>
#include <vector>
#include <array>
#include <climits>
#include <cstddef>

struct DLX {//dancing links implementation
    struct Column;
    struct Node {
        Node *L, *R, *U, *D;
        Column *C;
        int rowID;
        Node() : L(this), R(this), U(this), D(this), C(nullptr), rowID(-1) {}
    };
    struct Column : Node {
        int size;
        Column() : Node(), size(0) { C = this; }
    };

    Column* header;
    std::vector<Column*> colList;
    std::vector<Node*> nodePool;
    std::vector<Node*> solution;

    DLX(int ncols) {
        header = new Column();
        header->L = header->R = header;
        // create each column header and link into header’s list
        for (int i = 0; i < ncols; ++i) {
            Column* c = new Column();
            colList.push_back(c);
            c->R = header;
            c->L = header->L;
            header->L->R = c;
            header->L = c;
            c->U = c->D = c;
        }
    }

    //Add one row corresponding to a choice, with 1’s in the given columns
    void addRow(int rowID, const std::vector<int>& cols) {
        Node* rowHead = nullptr;
        for (int j : cols) {
            Column* c = colList[j];
            Node* nd = new Node();
            nodePool.push_back(nd);
            nd->C = c;
            nd->rowID = rowID;
            // link into column vertically
            nd->D = c;
            nd->U = c->U;
            c->U->D = nd;
            c->U = nd;
            c->size++;
            // link into row horizontally
            if (!rowHead) {
                rowHead = nd;
                nd->L = nd->R = nd;
            } else {
                nd->R = rowHead;
                nd->L = rowHead->L;
                rowHead->L->R = nd;
                rowHead->L = nd;
            }
        }
    }

    // Cover a column
    void cover(Column* c) {
        c->R->L = c->L;
        c->L->R = c->R;
        for (Node* i = c->D; i != c; i = i->D) {
            for (Node* j = i->R; j != i; j = j->R) {
                j->D->U = j->U;
                j->U->D = j->D;
                j->C->size--;
            }
        }
    }

    // Uncover a column
    void uncover(Column* c) {
        for (Node* i = c->U; i != c; i = i->U) {
            for (Node* j = i->L; j != i; j = j->L) {
                j->C->size++;
                j->D->U = j;
                j->U->D = j;
            }
        }
        c->R->L = c;
        c->L->R = c;
    }

    //recursive search
    bool search(int k = 0) {
        if (header->R == header) {
            // all columns covered = solution found
            return true;
        }
        //choose column with fewest nodes
        Column* c = nullptr;
        int best = INT_MAX;
        for (Column* j = static_cast<Column*>(header->R); j != header;
             j = static_cast<Column*>(j->R)) {
            if (j->size < best) {
                best = j->size;
                c = j;
            }
        }
        cover(c);
        for (Node* r = c->D; r != c; r = r->D) {
            solution.push_back(r);
            for (Node* j = r->R; j != r; j = j->R) cover(j->C);
            if (search(k+1)) return true;
            // backtrack
            solution.pop_back();
            for (Node* j = r->L; j != r; j = j->L) uncover(j->C);
        }
        uncover(c);
        return false;
    }
};

//Wrapper to translate a 9×9 Sudoku into an exact-cover problem
struct SudokuSolver {
    static constexpr int N = 9;
    DLX dlx;
    std::array<std::array<int, N>, N> grid;

    SudokuSolver(const std::array<std::array<int, N>, N>& g)
      : dlx(324), grid(g)
    {
        //for each cell, if fixed add one row; else add all 9 possibilities
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (grid[r][c] != 0) {
                    int d = grid[r][c];
                    dlx.addRow(r*81 + c*9 + (d-1), cols(r,c,d));
                } else {
                    for (int d = 1; d <= N; ++d)
                        dlx.addRow(r*81 + c*9 + (d-1), cols(r,c,d));
                }
            }
        }
    }

    // compute the four constraint-column indices for (r,c,d)
    std::vector<int> cols(int r, int c, int d) {
        int box = (r/3)*3 + (c/3);
        return {
            r*9 + c, // cell constraint
            81 + r*9 + (d-1), // row constraint
            162 + c*9 + (d-1), // column constraint
            243 + box*9 + (d-1) // box constraint
        };
    }

    bool solve() { return dlx.search(); }

    //reconstruct a solved grid
    auto getSolution() {
        std::array<std::array<int, N>, N> sol{};
        for (auto node : dlx.solution) {
            int id = node->rowID;
            int r = id / 81;
            int c = (id / 9) % 9;
            int d = (id % 9) + 1;
            sol[r][c] = d;
        }
        return sol;
    }
};

int main() {
    using Grid = std::array<std::array<int,9>,9>;
    std::string line;
    Grid puzzle;

    //read until eof
    while (true) {
        // try to read 9 valid lines
        int row = 0;
        while (row < 9 && std::getline(std::cin, line)) {
            if (line.size() < 9) continue;
            //parse exactly first 9 chars
            for (int c = 0; c < 9; ++c) {
                char ch = line[c];
                if (ch >= '1' && ch <= '9') puzzle[row][c] = ch - '0';
                else                      puzzle[row][c] = 0;
            }
            ++row;
        }
        if (row < 9) break;

        // solve
        SudokuSolver solver(puzzle);
        if (!solver.solve()) {
            std::cout << "No solution found\n";
        } else {
            auto sol = solver.getSolution();
            for (auto &r : sol) {
                for (int x : r) std::cout << x;
                std::cout << "\n";
            }
        }
        std::cout << "\n";
    }
    return 0;
}
