#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

using namespace std;

const int num = 10;     // 0~9的數字
const int pixel = 784; // 28x28 pixels
const int max_epo = 10;
const double lr = 0.01; // learning rate

// 分割 CSV 行字串
vector<string> split(const string& line, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(line);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// 讀取 MNIST CSV
void load_csv(const string& filename, vector<vector<double>>& images, vector<int>& labels) {
    ifstream file(filename);
    string line;
    int line_num = 0;

    while (getline(file, line)) {
        if (line_num == 0) { 
        line_num++;
        continue;
        }
        line_num++;
        vector<string> tokens = split(line, ',');

        if (tokens.size() != 781) {
            cerr << "Line " << line_num << ": Expected 781 columns, but got " << tokens.size() << endl;
            continue;
        }

        vector<double> image(pixel, 0.0);

        for (int i = 0; i < 780; ++i) {
            image[i] = stod(tokens[i]) / 255.0;
        }
        for (int i = 780; i < 784; ++i) {
            image[i] = 0.0; // padding
        }
        int label = stoi(tokens[780]); // 最後一欄是 label
        images.push_back(image);
        labels.push_back(label);
        
    }
}



// Softmax 計算
vector<double> softmax(const vector<double>& logits) {
    vector<double> probs(num);
    double maxLogit = *max_element(logits.begin(), logits.end());
    double sum = 0.0;
    for (int i = 0; i < num; ++i) {
        probs[i] = exp(logits[i] - maxLogit);
        sum += probs[i];
    }
    for (int i = 0; i < num; ++i) {
        probs[i] /= sum;
    }
    return probs;
}

// 推論：預測 label
int predict(const vector<double>& image, const vector<vector<double>>& weights, const vector<double>& biases) {
    vector<double> logits(num, 0.0);
    for (int j = 0; j < num; ++j) {
        for (int i = 0; i < pixel; ++i) {
            logits[j] += image[i] * weights[j][i];
        }
        logits[j] += biases[j];
    }
    return distance(logits.begin(), max_element(logits.begin(), logits.end()));
}

// 模型訓練
void train(
    const vector<vector<double>>& train_images,
    const vector<int>& train_labels,
    vector<vector<double>>& weights,
    vector<double>& biases
) {
    for (int epoch = 0; epoch < max_epo; ++epoch) {
        double total_loss = 0.0;
        int correct = 0;

        for (size_t n = 0; n < train_images.size(); ++n) {
            const vector<double>& x = train_images[n];
            int y = train_labels[n];

            // 計算 logits
            vector<double> logits(num, 0.0);
            for (int j = 0; j < num; ++j) {
                for (int i = 0; i < pixel; ++i) {
                    logits[j] += x[i] * weights[j][i];
                }
                logits[j] += biases[j];
            }

            // softmax & loss
            vector<double> probs = softmax(logits);
            total_loss -= log(probs[y]);

            // Accuracy
            int pred = distance(probs.begin(), max_element(probs.begin(), probs.end()));
            if (pred == y) correct++;

            // 梯度更新
            for (int j = 0; j < num; ++j) {
                double gradient = probs[j] - (j == y ? 1.0 : 0.0);
                for (int i = 0; i < pixel; ++i) {
                    weights[j][i] -= lr * gradient * x[i];
                }
                biases[j] -= lr * gradient;
            }
        }

        cout << "Epoch " << epoch + 1 << " - Loss: " << total_loss / train_images.size()
             << ", Accuracy: " << (double)correct / train_images.size() * 100 << "%" << endl;
    }
}

// 預測結果儲存到 CSV
void save_predictions(const string& filename, const vector<int>& predictions) {
    ofstream out(filename);
    for (int pred : predictions) {
        out << pred << '\n';
    }
}

// 計算 Macro F1-score
double compute_macro_f1(const vector<int>& true_labels, const vector<int>& pred_labels) {
    int m = num;
    double macro_f1 = 0.0;

    for (int c = 0; c < m; ++c) {
        int TP = 0, FP = 0, FN = 0;
        for (size_t i = 0; i < true_labels.size(); ++i) {
            if (pred_labels[i] == c && true_labels[i] == c) TP++;
            else if (pred_labels[i] == c && true_labels[i] != c) FP++;
            else if (pred_labels[i] != c && true_labels[i] == c) FN++;
        }

        double precision = (TP + FP == 0) ? 0 : (double)TP / (TP + FP);
        double recall = (TP + FN == 0) ? 0 : (double)TP / (TP + FN);
        double f1 = (precision + recall == 0) ? 0 : 2 * precision * recall / (precision + recall);

        macro_f1 += f1;
    }

    return macro_f1 / m;
}

// 主程式
int main() {
    vector<vector<double>> train_images, test_images;
    vector<int> train_labels, test_labels;

    load_csv("mnist_train.csv", train_images, train_labels);
    load_csv("mnist_test.csv", test_images, test_labels);

    // 初始化 weights & biases
    // 初始化 weights & biases with fixed seed
    unsigned seed = 42; // 設定隨機種子
    default_random_engine engine{seed};
    normal_distribution<double> dist(0.0, 0.01);

    vector<vector<double>> weights(num, vector<double>(pixel));
    vector<double> biases(num, 0.0);

    for (int j = 0; j < num; ++j)
        for (int i = 0; i < pixel; ++i)
            weights[j][i] = dist(engine);


    train(train_images, train_labels, weights, biases);

    // 預測
    vector<int> train_preds, test_preds;
    for (const auto& image : train_images)
        train_preds.push_back(predict(image, weights, biases));
    for (const auto& image : test_images)
        test_preds.push_back(predict(image, weights, biases));

    // 儲存預測結果
    save_predictions("result_train.csv", train_preds);
    save_predictions("result_test.csv", test_preds);

    // 計算 Macro F1-score
    double f1_train = compute_macro_f1(train_labels, train_preds);
    double f1_test = compute_macro_f1(test_labels, test_preds);

    cout << "Train Macro F1 Score: " << f1_train << endl;
    cout << "Test  Macro F1 Score: " << f1_test << endl;

    return 0;
}


//g++ -std=c++20 linear.cpp -o a