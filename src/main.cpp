#include <iostream>
#include "code/quantum_code.h"
#include "error/error_model.h"
#include "simulator/simulator.h"

int main() {
    int trials = 10000;

    // 1. 使用する量子誤り訂正コードのインスタンス化
    BitFlip3Code code;

    std::cout << "--- Quantum Error Correction Statistical Simulator ---" << std::endl;
    std::cout << "Trials per probability: " << trials << "\n\n";
    std::cout << "Physical Error (p) | Logical Error Rate\n";
    std::cout << "-------------------|-------------------\n";

    // 2. 物理エラー率 p を変化させてモンテカルロ・シミュレーションを実行
    for (double p = 0.00; p <= 0.50; p += 0.05) {
        // 各ループステップで、その確率 p のエラーモデルを作成
        ErrorModel error_model(p, ErrorType::BitFlip);
        //ErrorModel error_model(p, ErrorType::PhaseFlip);
        //ErrorModel error_model(p, ErrorType::Depolarizing);
        
        // コードとエラーモデルをシミュレータにバインド
        Simulator simulator(code, error_model);
        
        // シミュレーション実行
        double logical_error_rate = simulator.run_simulation(trials);
        
        std::cout << "       " << p << "        |      " << logical_error_rate << "\n";
    }

    return 0;
}