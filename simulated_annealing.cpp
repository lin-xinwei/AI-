#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>

#include <algorithm>
#include <ctime>

using namespace std;

struct city {
    double x, y;
};

// 計算兩個城市之間的距離
double distance(const city &a, const city &b) {
    return hypot(a.x - b.x, a.y - b.y);
}


// 計算一個旅程的總長度
double cal_length(const vector<city> &cities, const vector<int> &tour) {
    double total = 0;
    int n = tour.size();
    for (int i = 0; i < n; ++i) {
        total += distance(cities[tour[i]], cities[tour[(i + 1) % n]]);
    }
    return total;
}

// 讀取城市(x, y)座標
void readCities(const string &filename, vector<city> &cities) {
    ifstream file(filename);
    city city;
    while (file >> city.x >> city.y) {
        cities.push_back(city);
    }
    file.close();
}


// 隨機生成最一開始的路徑
vector<int> rand_generate_tour(int n) {
    vector<int> tour(n);
    for (int i = 0; i < n; ++i) tour[i] = i;

    random_device rd;
    default_random_engine g(rd());  // 隨機數生成器
    shuffle(tour.begin(), tour.end(), g);

    return tour;
}

// 隨機改變路徑(交換兩城市位置)
vector<int> change_tour(const vector<int> &tour) {
    vector<int> neighbor = tour;
    int a = rand() % tour.size();
    int b = rand() % tour.size();
    swap(neighbor[a], neighbor[b]);
    return neighbor;
}

vector<int> simul_anneal(const vector<city> &cities) {
    int n = cities.size();
    int maxEvals = 1000 * n;    //限制條件，設定上限

    vector<int> current = rand_generate_tour(n);
    double currentCost = cal_length(cities, current);

    vector<int> best = current;
    double bestCost = currentCost;

    double T = 10000;         // 初始溫度
    double coolingRate = 0.995;

    int evals = 0;          // 評估次數
    while (evals < maxEvals && T > 1e-4) {
        vector<int> neighbor = change_tour(current);
        double lengh_cost = cal_length(cities, neighbor);
        double delta = lengh_cost - currentCost;

        if (delta < 0 || (exp(-delta / T) > ((double) rand() / RAND_MAX))) {
            current = neighbor;
            currentCost = lengh_cost;
            if (currentCost < bestCost) {
                best = current;
                bestCost = currentCost;
            }
        }

        T *= coolingRate;
        evals++;
    }

    return best;
}

void saveResult(const string &filename, const vector<int> &tour) {
    ofstream file(filename);
    for (int i : tour) file << i << " ";
    file << endl;
    file.close();
}

int main() {
    srand(time(0));

    //數字之間沒關聯，職介創一個vector儲存我要跑的維度
    vector<int> dims = {50, 100, 200, 500, 1000};
    
    for (int dim : dims) {
        string inputFile = "TSP_Dim=" + to_string(dim) + ".txt";
        string outputFile = "output_Dim=" + to_string(dim) + ".txt";

        vector<city> cities;
        readCities(inputFile, cities);

        vector<int> result = simul_anneal(cities);
        saveResult(outputFile, result);

        cout << "Dimension_" << dim << " done." << outputFile << endl;
    }
    return 0;
}


// g++ -std=c++20 simulated_annealing.cpp -o a