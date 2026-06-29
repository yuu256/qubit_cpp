#pragma once
#include "../code/quantum_code.h"
#include "../error/error_model.h"

class Simulator {
public:
    // 任意のコードとエラーモデルを受け取ってシミュレータを初期化
    Simulator(QuantumCode& code, ErrorModel& error_model)
        : quantum_code(code), error_model(error_model) {}

    // 1回の誤り訂正シミュレーションを実行し、成功(true)か失敗(false)を返す
    bool run_once();

    // 指定回数(trials)だけシミュレーションを繰り返し、論理エラー率を計算する
    double run_simulation(int trials);

private:
    QuantumCode& quantum_code;
    ErrorModel& error_model;
};