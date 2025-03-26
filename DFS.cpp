#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

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
            token.erase(remove(token.begin(), token.end(), ' '), token.end()); //Delete Space
            if (token[0] == '+') token = token.substr(1);  //Delete + char.
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

//看是否滿足 3-SAT
bool correspond(const vector<vector<int>> &clauses, const vector<int> &assignment) {
    for (const auto &clause : clauses) {
        bool clauseSatisfied = false;
        for (int literal : clause) {
            int varIndex = abs(literal) - 1;
            bool value = assignment[varIndex];  //賦值
            if (literal < 0) value = !value;    //若是負號則取反
            if (value) {
                clauseSatisfied = true;
                break;
            }
        }
        if (!clauseSatisfied) return false;  //有任何子句不滿足-->返回 false
    }
    return true;
}

//DFS搜尋
bool DeepFS(vector<vector<int>> &clauses, vector<int> &assignment, int varIndex) {
    if (varIndex == assignment.size()) {
        return correspond(clauses, assignment);
    }

    //try變數為 0
    assignment[varIndex] = 0;
    if (DeepFS(clauses, assignment, varIndex + 1)) return true;

    //try變數為 1
    assignment[varIndex] = 1;
    if (DeepFS(clauses, assignment, varIndex + 1)) return true;

    return false;
}


int main() {
    vector<int> dim = {10, 20, 30, 40, 50};  //要處理的 D 值
    ofstream outFile("result.txt");          //輸出結果到txt檔案

    for (int D : dim) {
        string filename = "3SAT_Dim=" + to_string(D) + ".csv";
        int D_num;
        vector<vector<int>> clauses = readCSV(filename, D_num);
        if (clauses.empty()) continue;  // 若檔案不存在則跳過

        vector<int> assignment(D_num, 0); // 初始變數賦值全為 0
        

        if (DeepFS(clauses, assignment, 0)) {
            for (int i = 0; i < D_num; i++) {
                cout << assignment[i];
                outFile << assignment[i] << " ";
            }
        } 

        else {
            outFile << "No solution"<< endl;;
            cout << "No solution for " << filename << endl;
        }
    }

    outFile.close();
    return 0;
}
