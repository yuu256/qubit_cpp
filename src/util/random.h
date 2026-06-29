#pragma once
#include <random>

class Random {
public:
    // システム全体で一意の乱数生成器にアクセスする（シングルトンパターン）
    static Random& getInstance() {
        static Random instance;
        return instance;
    }

    // 0.0 ～ 1.0 の実数乱数を取得
    double next_double() {
        return dist(gen);
    }

    // 指定した範囲 [min, max] の整数乱数を取得（ランダムなエラービットの選択用）
    int next_int(int min, int max) {
        std::uniform_int_distribution<int> int_dist(min, max);
        return int_dist(gen);
    }

private:
    Random() : gen(rd()), dist(0.0, 1.0) {}
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;
};