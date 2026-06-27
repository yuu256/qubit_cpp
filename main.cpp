#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <random>

using Complex = std::complex<double>;
using StateVector = std::vector<Complex>;

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
void apply_CNOT(StateVector& state, int control, int target) {
    // TODO: 0～31までループを回す
    // 「controlビットが1」かつ「targetビットが0」であるインデックスを見つけたら、
    // 「controlビットが1」かつ「targetビットが1」であるインデックスと中身をスワップする。
}

// 1量子ビット測定：確率に基づき0か1を返し、状態ベクトルを収縮・規格化する
int measure_qubit(StateVector& state, int target) {
    double prob_0 = 0.0;
    
    // TODO: 1. targetビットが0である全てのインデックスの「振幅の絶対値2乗」を足し合わせ、prob_0 を計算する
    
    // TODO: 2. 乱数（0.0～1.0）を生成し、prob_0 より小さければ結果は 0、大きければ 1 とする
    int result = 0; 
    
    // TODO: 3. 選ばれなかった方の状態の振幅をすべて 0 にする
    //（例：resultが0なら、targetビットが1の項の振幅を 0 にする）
    
    // TODO: 4. 残った状態ベクトルのノルム（全体の長さ）を計算し、合計が1になるよう全体を割り算（規格化）する
    
    return result;
}

// --- メイン処理：エラー訂正のシナリオ ---
int main() {
    // 状態ベクトルの初期化（全要素0）
    StateVector state(STATE_SIZE, 0.0);
    
    // 1. 初期状態の設定 (例: alpha|00000> + beta|10000>)
    double alpha = 0.6;
    double beta = 0.8;
    state[0] = alpha; // |00000> のインデックスは 0
    state[16] = beta; // |10000> のインデックスは 16 (1 << 4)
    
    std::cout << "--- 3-Qubit Bit-Flip Code Simulation ---" << std::endl;
    
    // 2. 符号化 (Encoding)
    // TODO: apply_CNOT を使って Q0 の状態を Q1, Q2 にコピー
    
    // 3. エラー発生 (Noise)
    // TODO: 任意のデータ量子ビット（例: Q1）に apply_X を適用
    
    // 4. シンドローム測定の仕込み
    // TODO: 補助ビット A0, A1 にパリティを書き込む (apply_CNOT)
    
    // 5. 測定の実行
    // TODO: measure_qubit を使って A0, A1 の測定結果(m1, m2)を取得
    int m1 = 0; // 仮
    int m2 = 0; // 仮
    
    // 6. 復元 (Correction)
    // TODO: m1, m2 の結果（古典ビット）に応じて if 文で分岐し、適切なデータビットに apply_X を適用
    
    // 7. 検証
    // TODO: 符号化の逆操作をして、最終的に state[0] と state[16] (または測定結果に応じた位置) に 
    // 元の alpha, beta が綺麗に戻っているか std::cout で表示して確認
    
    return 0;
}