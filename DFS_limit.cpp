#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>  // 用於 pow()

using namespace std;

// 讀檔
vector<vector<int>> readCSV(const string &filename, int &D) {
    ifstream file(filename);
    vector<vector<int>> clauses;
    string line;

    while (getline(file, line)) {
        vector<int> clause;
        stringstream ss(line);
        string token;

        while (getline(ss, token, ',')) { 
            token.erase(remove(token.begin(), token.end(), ' '), token.end());  //Delete Space
            if (token[0] == '+') token = token.substr(1);       //Delete + char.
            clause.push_back(stoi(token));
        }

        clauses.push_back(clause);
    }

    file.close();
    D = 0;
    for (const auto &clause : clauses) {
        for (int lit : clause) {
            D = max(D, abs(lit)); 
        }
    }

    return clauses;
}

// 看是否滿足 3-SAT
bool correspond(const vector<vector<int>> &clauses, const vector<int> &assignment) {
    for (const auto &clause : clauses) {
        bool clauseSatisfied = false;
        for (int literal : clause) {
            int varIndex = abs(literal) - 1;
            bool value = assignment[varIndex];  // 賦值
            if (literal < 0) value = !value;    // 若是負號則取反
            if (value) {
                clauseSatisfied = true;
                break;
            }
        }
        if (!clauseSatisfied) return false;  // 有任何子句不滿足 --> 返回 false
    }
    return true;
}

// DFS 搜尋（加入節點上限）
bool DeepFS(vector<vector<int>> &clauses, vector<int> &assignment, int varIndex, int &expanded_nodes, int max_nodes) {
    if (expanded_nodes >= max_nodes) return false;  // 超過節點上限，返回 false

    expanded_nodes++;  // 記錄已展開的節點數

    if (varIndex == assignment.size()) {
        return correspond(clauses, assignment);
    }

    // try 變數為 0
    assignment[varIndex] = 0;
    if (DeepFS(clauses, assignment, varIndex + 1, expanded_nodes, max_nodes)) return true;

    // try 變數為 1
    assignment[varIndex] = 1;
    if (DeepFS(clauses, assignment, varIndex + 1, expanded_nodes, max_nodes)) return true;

    return false;
}


int main() {
    vector<int> dim = {10, 20, 30, 40, 50};  // 要處理的 D 值
    ofstream outFile("result.txt");          // 輸出結果到 txt 檔案

    for (int D : dim) {
        string filename = "3SAT_Dim=" + to_string(D) + ".csv";
        int var_counter;
        vector<vector<int>> clauses = readCSV(filename, var_counter);
        if (clauses.empty()) continue;  // 若檔案不存在則跳過

        vector<int> assignment(var_counter, 0); // 初始變數賦值全為 0
        int expanded_nodes = 0;                 // 記錄已展開的節點數
        int max_nodes = pow(D, 3);              // 設定最大節點數 D^3
        
        if (DeepFS(clauses, assignment, 0, expanded_nodes, max_nodes)) {
            for (int i = 0; i < var_counter; i++) {
                cout << assignment[i];
                outFile << assignment[i] << " ";
            }
            outFile << endl;
        } 

        else {
            outFile << "No solution" << endl;
            cout << "No solution for " << filename << endl;
        }
    }

    outFile.close();
    return 0;
}
