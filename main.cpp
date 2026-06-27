#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <random>

using Complex = std::complex<double>;
using StateVector = std::vector<Complex>;

// 量子ビットのインデックス定義
const int Q0 = 0;
const int Q1 = 1;
const int Q2 = 2;
const int A0 = 3;
const int A1 = 4;
const int NUM_QUBITS = 5;
const int STATE_SIZE = 1 << NUM_QUBITS;

// --- 量子ゲート演算（ここを自分で実装する） ---


// 1量子ビットXゲート：targetビットが0の項と1の項を入れ替える
    // TODO: 0～31までループを回す
    // targetビットが0であるインデックスを見つけたら、
    // それに対応する「targetビットが1であるインデックス」と中身(振幅)をスワップする。
    // ※2重にスワップしないように注意！
void apply_X(StateVector& state, int target) {
    for (int index = 0; index < 32; ++index) {
        // targetビットの値（0か1か）を取り出す
        int bit_val = (index >> (NUM_QUBITS - 1 - target)) & 1;
        
        // targetビットが「0」のときだけ処理を行う（1のときは何もしない＝2重スワップ防止）
        if (bit_val == 0) {
            // targetビットを「1」に反転させたインデックスを計算
            int pair_index = index | (1 << (NUM_QUBITS - 1 - target));
            
            // 配列の「中身（振幅）」を入れ替える
            std::swap(state[index], state[pair_index]);
        }
    }
}

// CNOTゲート：controlビットが1の項だけ、targetビットの0と1を入れ替える
// TODO: 0～31までループを回す
// 「controlビットが1」かつ「targetビットが0」であるインデックスを見つけたら、
// 「controlビットが1」かつ「targetビットが1」であるインデックスと中身をスワップする。
// CNOTゲート：controlビットが1の項だけ、targetビットの0と1を入れ替える
void apply_CNOT(StateVector& state, int control, int target) {
    // 0～31まで1重ループで回す
    for (int index = 0; index < 32; ++index) {
        
        // ① controlビットの値（0か1）を取り出す
        int control_val = (index >> (NUM_QUBITS - 1 - control)) & 1;
        
        // ② targetビットの値（0か1）を取り出す
        int target_val = (index >> (NUM_QUBITS - 1 - target)) & 1;
        
        // ③ 「controlビットが1」かつ「targetビットが0」であるインデックスを見つけたら
        if (control_val == 1 && target_val == 0) {
            
            // ペアとなる「targetビットが1であるインデックス」を作る
            int pair_index = index | (1 << (NUM_QUBITS - 1 - target));
            
            // 中身（振幅）をスワップする
            std::swap(state[index], state[pair_index]);
        }
    }
}

// 1量子ビット測定：確率に基づき0か1を返し、状態ベクトルを収縮・規格化する
int measure_qubit(StateVector& state, int target) {
    double prob_0 = 0.0;
    
    // ---- TODO: 1. targetビットが0である全ての「振幅の絶対値2乗」を足し合わせ、prob_0 を計算する ----
    for (int index = 0; index < 32; ++index) {
        int bit_val = (index >> (NUM_QUBITS - 1 - target)) & 1;
        
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
    for (int index = 0; index < 32; ++index) {
        int bit_val = (index >> (NUM_QUBITS - 1 - target)) & 1;
        
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
        
        for (int index = 0; index < 32; ++index) {
            state[index] /= norm;
        }
    }
    
    return result;
}

// --- メイン処理：エラー訂正のシナリオ ---

int main() {
    // 状態ベクトルの初期化（全要素0）
    StateVector state(STATE_SIZE, 0.0);
    
    // 1. 初期状態の設定 (例: alpha|00000> + beta|10000>)
    double alpha = 0.6;
    double beta = 0.8;
    state[0] = alpha;  // |00000> のインデックスは 0
    state[16] = beta;  // |10000> のインデックスは 16 (1 << 4)
    
    std::cout << "--- 3-Qubit Bit-Flip Code Simulation ---" << std::endl;
    std::cout << "初期状態の確率振幅: alpha = " << state[0] << ", beta = " << state[16] << "\n\n";
    
    // 2. 符号化 (Encoding)
    // Q0 の状態を Q1, Q2 にコピーして、大きな重ね合わせ状態を作る
    apply_CNOT(state, Q0, Q1);
    apply_CNOT(state, Q0, Q2);
    
    // 3. エラー発生 (Noise)
    // 今回は「Q1」にビット反転エラーが発生したというシナリオにします
    std::cout << "[Noise] Q1の量子ビットに反転エラーが発生！" << std::endl;
    apply_X(state, Q1); 
    
    // 4. シンドローム測定の仕込み
    // 補助ビット A0, A1 に隣り合うデータビットのパリティ（異同）を書き込む
    apply_CNOT(state, Q0, A0);
    apply_CNOT(state, Q1, A0); // A0 = Q0 ⊕ Q1

    apply_CNOT(state, Q1, A1);
    apply_CNOT(state, Q2, A1); // A1 = Q1 ⊕ Q2
    
    // 5. 測定の実行
    // 補助ビット A0, A1 を測定し、どこでエラーが起きたかの手がかり（シンドローム）を得る
    int m1 = measure_qubit(state, A0);
    int m2 = measure_qubit(state, A1);
    std::cout << "測定シンドローム: (m1, m2) = (" << m1 << ", " << m2 << ")" << std::endl;
    
    // 6. 復元 (Correction)
    // m1, m2 の結果（古典ビット）に応じて条件分岐し、適切なデータビットのエラーを反転させて直す
    // (m1,m2) = (1,0)->Q0, (1,1)->Q1, (0,1)->Q2, (0,0)->ない
    if (m1 == 1 && m2 == 0) {
        std::cout << "-> Q0のエラーを訂正します。" << std::endl;
        apply_X(state, Q0);
    }
    else if (m1 == 1 && m2 == 1) {
        std::cout << "-> Q1のエラーを訂正します。" << std::endl;
        apply_X(state, Q1); // 今回はここを通る！
    }
    else if (m1 == 0 && m2 == 1) {
        std::cout << "-> Q2のエラーを訂正します。" << std::endl;
        apply_X(state, Q2);
    }
    else {
        std::cout << "-> エラーは検出されませんでした。" << std::endl;
    }
    
    // 7. 検証 (Decoding)
    // 符号化の逆操作をして、Q0に状態を集約する
    // 【重要バグ修正】コントロールとターゲットが逆になっていたのを修正
    apply_CNOT(state, Q0, Q1);
    apply_CNOT(state, Q0, Q2);
    
    std::cout << "\n--- 最終結果の確認 ---" << std::endl;
    std::cout << "state[0]  (|00000>): " << state[0] << " (元データ: alpha = " << alpha << ")" << std::endl;
    std::cout << "state[16] (|10000>): " << state[16] << " (元データ: beta  = " << beta << ")" << std::endl;
    
    return 0;
}