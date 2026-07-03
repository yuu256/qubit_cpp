#include "simulator.h"
#include <iostream>
#include <cmath>

// 1回の誤り訂正シミュレーションを実行し、成功(true)か失敗(false)を返す
bool Simulator::run_once() {
    // 毎回リセットされた「まっさらな状態ベクトル」を生成する
    StateVector state(quantum_code.get_state_size(), 0.0);
    
    // 1. 初期状態の設定 (例: alpha|00000> + beta|10000>)
    double alpha = 0.6;
    double beta = 0.8;
    state[0] = alpha;  // |00000> のインデックスは 0
    
    // 5量子ビットの場合、|10000> は 16番目 (STATE_SIZE / 2) になる
    int half_size = quantum_code.get_state_size() / 2;
    state[half_size] = beta;  
    
    // 2. 符号化 (Encoding)
    quantum_code.encode(state);
    
    // 3. エラー発生 (Noise)
    // 確率 p のエラーモデルを、データビット数（3つ）に対して適用する
    // simulator.cpp 内の修正箇所
    // error_model.inject_error(state, 3); から以下に変更：
    error_model.inject_error(state, quantum_code);
    //error_model.inject_error(state, 3); 
    
    // 4 & 5. シンドローム測定の仕込みと実行
    auto [m1, m2] = quantum_code.measure_syndrome(state);
    
    // 6. 復元 (Correction)
    quantum_code.correct(state, m1, m2);
    
    // 7. 検証 (Decoding)
    quantum_code.decode(state);

    // 【修正】測定結果（補助ビット A0, A1 の状態）に合わせて、確認するインデックスをコード側から取得
    auto [final_idx_0, final_idx_16] = quantum_code.get_verification_indices(m1, m2);

    // インデックスではなく、配列の「中身（実数部）」が元に戻ったかを判定する
    // 浮動小数点の誤差を考慮し、差が 1e-5 未満であれば「等しい（成功）」とみなす
    double epsilon = 1e-5;
    bool is_alpha_correct = std::abs(state[final_idx_0].real() - alpha) < epsilon;
    bool is_beta_correct  = std::abs(state[final_idx_16].real() - beta) < epsilon;

    return is_alpha_correct && is_beta_correct;
}

// 指定回数(trials)だけシミュレーションを繰り返し、論理エラー率を計算する
double Simulator::run_simulation(int trials) {
    int error_count = 0;
    
    // trials 回数だけループを回し、!run_once() の場合に error_count をインクリメントする
    for (int i = 0; i < trials; ++i) {
        if (!run_once()) {
            error_count++;
        }
    }
    
    // static_cast<double> を使って安全に浮動小数点数の割り算を行う
    return static_cast<double>(error_count) / trials;
}