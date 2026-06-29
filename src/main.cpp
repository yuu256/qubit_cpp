/*#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <random>

using Complex = std::complex<double>;
using StateVector = std::vector<Complex>;

// 一番気になるところ：enumによる量子ビットの安全な管理
enum Qubit {
    Q0,
    Q1,
    Q2,
    A0,
    A1,
    NUM_QUBITS // 末尾に置くことで、ビットが増えても自動的に総数が更新されるトリック
};

// ① 32などのマジックナンバーを排除し、ビット数から自動追従
const int STATE_SIZE = 1 << NUM_QUBITS;

// ② ビット位置計算を何度も書かないためのインラインヘルパー関数
inline int get_bit(int index, int qubit) {
    return (index >> (NUM_QUBITS - 1 - qubit)) & 1;
}

inline int flip_bit(int index, int qubit) {
    return index ^ (1 << (NUM_QUBITS - 1 - qubit));
}

// --- 量子ゲート演算（共通ヘルパーを利用してリファクタリング） ---

// 1量子ビットXゲート：targetビットが0の項と1の項を入れ替える
// TODO: 0～31までループを回す -> ① STATE_SIZEに修正
// targetビットが0であるインデックスを見つけたら、
// それに対応する「targetビットが1であるインデックス」と中身(振幅)をスワップする。
// ※2重にスワップしないように注意！
void apply_X(StateVector& state, int target) {
    for (int index = 0; index < STATE_SIZE; ++index) {
        // targetビットの値（0か1か）を取り出す (② get_bitで共通化)
        int bit_val = get_bit(index, target);
        
        // targetビットが「0」のときだけ処理を行う（1のときは何もしない＝2重スワップ防止）
        if (bit_val == 0) {
            // targetビットを「1」に反転させたインデックスを計算 (② flip_bitで共通化)
            int pair_index = flip_bit(index, target);
            
            // 配列の「中身（振幅）」を入れ替える
            std::swap(state[index], state[pair_index]);
        }
    }
}

// CNOTゲート：controlビットが1の項だけ、targetビットの0と1を入れ替える
// TODO: 0～31までループを回す -> ① STATE_SIZEに修正
// 「controlビットが1」かつ「targetビットが0」であるインデックスを見つけたら、
// 「controlビットが1」かつ「targetビットが1」であるインデックスと中身をスワップする。
void apply_CNOT(StateVector& state, int control, int target) {
    // 0～31まで1重ループで回す -> ① STATE_SIZEに修正
    for (int index = 0; index < STATE_SIZE; ++index) {
        
        // ① controlビットの値（0か1）を取り出す (② get_bitで共通化)
        int control_val = get_bit(index, control);
        
        // ② targetビットの値（0か1）を取り出す (② get_bitで共通化)
        int target_val = get_bit(index, target);
        
        // ③ 「controlビットが1」かつ「targetビットが0」であるインデックスを見つけたら
        if (control_val == 1 && target_val == 0) {
            
            // ペアとなる「targetビットが1であるインデックス」を作る (② flip_bitで共通化)
            int pair_index = flip_bit(index, target);
            
            // 中身（振幅）をスワップする
            std::swap(state[index], state[pair_index]);
        }
    }
}

// 1量子ビット測定：確率に基づき0か1を返し、状態ベクトルを収縮・規格化する
int measure_qubit(StateVector& state, int target) {
    double prob_0 = 0.0;
    
    // ---- TODO: 1. targetビットが0である全ての「振幅の絶対値2乗」を足し合わせ、prob_0 を計算する ----
    for (int index = 0; index < STATE_SIZE; ++index) { // ① STATE_SIZEに修正
        int bit_val = get_bit(index, target); // ② get_bitで共通化
        
        if (bit_val == 0) {
            // std::norm は複素数の絶対値の2乗（|z|^2）を返すため、確率の計算に最適
            prob_0 += std::norm(state[index]);
        }
    }
    
    // ---- TODO: 2. 乱数（0.0～1.0）を生成し、prob_0 より小さければ結果は 0、大きければ 1 とする ----
    // 現代的なC++の乱数生成（メルセンヌ・ツイスタ）
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double rnd = dist(gen);
    
    int result = (rnd < prob_0) ? 0 : 1;
    
    // ---- TODO: 3. 選ばれなかった方の状態の振幅をすべて 0 にする ----
    for (int index = 0; index < STATE_SIZE; ++index) { // ① STATE_SIZEに修正
        int bit_val = get_bit(index, target); // ② get_bitで共通化
        
        // 測定結果（result）と一致しないビットを持つ項の振幅を消滅（0）させる
        if (bit_val != result) {
            state[index] = 0.0;
        }
    }
    
    // ---- TODO: 4. 残った状態ベクトルのノルムを計算し、合計が1になるよう全体を規格化する ----
    // 選ばれた世界の確率（resultが0ならprob_0、1なら1.0 - prob_0）
    double chosen_prob = (result == 0) ? prob_0 : (1.0 - prob_0);
    
    // ゼロ除算を防ぐためのチェック（確率が極めて低い世界が選ばれた場合の上限ガード）
    if (chosen_prob > 1e-15) {
        // 全体のノルム（ベクトルの長さ）は「選ばれた確率の平方根」になる
        double norm = std::sqrt(chosen_prob);
        
        for (int index = 0; index < STATE_SIZE; ++index) { // ① STATE_SIZEに修正
            state[index] /= norm;
        }
    }
    
    return result;
}

// --- さらに次の段階：アルゴリズムそのものとゲート実装を分離 (関数分割) ---

// 2. 符号化 (Encoding)
// Q0 の状態を Q1, Q2 にコピーして、大きな重ね合わせ状態を作る
void encode(StateVector& state) {
    apply_CNOT(state, Q0, Q1);
    apply_CNOT(state, Q0, Q2);
}


/*void inject_error(StateVector& state, double p = 0.0) {
    std::cout << "[Noise] Q1" << std::endl;
    apply_X(state, Q1); 
    /*確率pでランダムなエラー。エラーは別関数で*/
//}*/
/*
// 3. エラー発生 (Noise)
// データ量子ビット（Q0, Q1, Q2）それぞれに対して、独立して確率 p でXエラーを発生させる
void inject_error(StateVector& state, double p) {
    // 高精度な乱数生成器の準備
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // 乱数が確率 p より小さければ、そのビットにエラー（Xゲート）を適用する
    if (dist(gen) < p) apply_X(state, Q0);
    if (dist(gen) < p) apply_X(state, Q1);
    if (dist(gen) < p) apply_X(state, Q2);
}

// 4 & 5. シンドローム測定の仕込みと実行
// 補助ビット A0, A1 に隣り合うデータビットのパリティ（異同）を書き込み、測定結果(m1, m2)を取得
std::pair<int, int> measure_syndrome(StateVector& state) {
    // 補助ビット A0, A1 に隣り合うデータビットのパリティ（異同）を書き込む
    apply_CNOT(state, Q0, A0);
    apply_CNOT(state, Q1, A0); // A0 = Q0 ⊕ Q1

    apply_CNOT(state, Q1, A1);
    apply_CNOT(state, Q2, A1); // A1 = Q1 ⊕ Q2
    
    // 測定の実行
    int m1 = measure_qubit(state, A0); // =1 (今回のerrorはQ1)
    int m2 = measure_qubit(state, A1); // =1
    //std::cout << "Measurement Syndrome: (m1, m2) = (" << m1 << ", " << m2 << ")" << std::endl;
    
    return {m1, m2};
}

// 6. 復元 (Correction)
// m1, m2 の結果（古典ビット）に応じて条件分岐し、適切なデータビットのエラーを反転させて直す
void correct(StateVector& state, int m1, int m2) {
    // (m1,m2) = (1,0)->Q0, (1,1)->Q1, (0,1)->Q2, (0,0)->ない
    if (m1 == 1 && m2 == 0) {
        //std::cout << "-> Correct Q0 error " << std::endl;
        apply_X(state, Q0);
    }
    else if (m1 == 1 && m2 == 1) {
        //std::cout << "-> Correct Q1 error" << std::endl;
        apply_X(state, Q1); // 今回はここを通る！
    }
    else if (m1 == 0 && m2 == 1) {
        //std::cout << "-> Correct Q2 error" << std::endl;
        apply_X(state, Q2);
    }
    else {
        //std::cout << "-> No errors were detected." << std::endl;
    }
}

// 7. 検証 (Decoding)
// 符号化の逆操作をして、Q0に状態を集約する
void decode(StateVector& state) {
    apply_CNOT(state, Q0, Q1);
    apply_CNOT(state, Q0, Q2);
}


// --- 次に実装すべきもの（枠組み・ひな形のみ） ---

// 1. run_once() を作り、1回の誤り訂正を実行して成功・失敗を返す。
// 3. ランダムなビット反転エラー（確率 p）をここに導入する予定。
// TODO: ここに1回分のシミュレーションの流れを記述する


// 1回の誤り訂正シミュレーションを実行し、成功(true)か失敗(false)を返す
bool run_once(double p) {
    StateVector state(STATE_SIZE, 0.0);
    
    // 初期状態の設定
    double alpha = 0.6;
    double beta = 0.8;
    state[0] = alpha;  
    state[16] = beta;  
    
    // 一連の量子回路フロー
    encode(state);
    inject_error(state, p); // 確率 p に基づいてランダムエラーを注入
    auto [m1, m2] = measure_syndrome(state);
    correct(state, m1, m2);
    decode(state);

    // 検証するインデックスの計算
    int syndrome_offset = (m1 << 1) | m2; 
    int final_idx_0 = 0 + syndrome_offset;   
    int final_idx_16 = 16 + syndrome_offset;

    // 【修正】インデックスではなく、配列の「中身（実数部）」が元に戻ったかを判定する
    // 浮動小数点の誤差を考慮し、差が 1e-5 未満であれば「等しい（成功）」とみなす
    double epsilon = 1e-5;
    bool is_alpha_correct = std::abs(state[final_idx_0].real() - alpha) < epsilon;
    bool is_beta_correct  = std::abs(state[final_idx_16].real() - beta) < epsilon;

    return is_alpha_correct && is_beta_correct;
}

// 2. run_simulation(int trials, double p) を作り、指定回数繰り返して論理エラー率を計算する。
// 4. 物理エラー率 p を変化させて、論理エラー率との関係を出力する基盤。
double run_simulation(int trials, double p) {
    int error_count = 0;
    // TODO: trials 回数だけループを回し、!run_once(p) の場合に error_count をインクリメントする
    for (int i = 0;i < trials;i++) {
        if(!run_once(p)) {error_count++;}
    }
    return static_cast<double>(error_count) / trials;
}*/


// --- メイン処理（きわめてシンプルに洗練） ---


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
        
        // コードとエラーモデルをシミュレータにバインド
        Simulator simulator(code, error_model);
        
        // シミュレーション実行
        double logical_error_rate = simulator.run_simulation(trials);
        
        std::cout << "       " << p << "        |      " << logical_error_rate << "\n";
    }

    return 0;
}