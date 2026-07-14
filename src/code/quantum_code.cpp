#include "quantum_code.h"
#include "../util/random.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// ==========================================
// 量子ゲート・測定の内部実装
// ==========================================

// 1量子ビットXゲート：targetビットが0の項と1の項を入れ替える
// targetビットが0であるインデックスを見つけたら、
// それに対応する「targetビットが1であるインデックス」と中身(振幅)をスワップする。
void BitFlip3Code::apply_X(StateVector& state, int target) {
    int state_size = get_state_size();
    for (int index = 0; index < state_size; ++index) {
        // targetビットの値（0か1か）を取り出す (get_bitヘルパーでカプセル化)
        int bit_val = get_bit(index, target);
        
        // targetビットが「0」のときだけ処理を行う（1のときは何もしない＝2重スワップ防止）
        if (bit_val == 0) {
            // targetビットを「1」に反転させたインデックスを計算 (flip_bitヘルパーでカプセル化)
            int pair_index = flip_bit(index, target);
            
            // 配列の「中身（振幅）」を入れ替える
            std::swap(state[index], state[pair_index]);
        }
    }
}

// CNOTゲート：controlビットが1の項だけ、targetビットの0と1を入れ替える
// 「controlビットが1」かつ「targetビットが0」であるインデックスを見つけたら、
// 「controlビットが1」かつ「targetビットが1」であるインデックスと中身をスワップする。
void BitFlip3Code::apply_CNOT(StateVector& state, int control, int target) {
    int state_size = get_state_size();
    for (int index = 0; index < state_size; ++index) {
        // controlビットとtargetビットの値を取り出す (get_bitヘルパーを利用)
        int control_val = get_bit(index, control);
        int target_val = get_bit(index, target);
        
        // 「controlビットが1」かつ「targetビットが0」であるインデックスを見つけたら
        if (control_val == 1 && target_val == 0) {
            // ペアとなる「targetビットが1であるインデックス」を作る (flip_bitヘルパー)
            int pair_index = flip_bit(index, target);
            
            // 中身（振幅）をスワップする
            std::swap(state[index], state[pair_index]);
        }
    }
}

// 1量子ビットZゲート：targetビットが1の項の符号（位相）だけを反転させる
void BitFlip3Code::apply_Z(StateVector& state, int target) {
    int state_size = get_state_size();
    for (int index = 0; index < state_size; ++index) {
        // targetビットが「1」の項の符号（位相）だけを反転させる
        if (get_bit(index, target) == 1) {
            state[index] *= -1.0;
        }
    }
}

// 1量子ビット測定：確率に基づき0か1を返し、状態ベクトルを収縮・規格化する
int BitFlip3Code::measure_qubit(StateVector& state, int target) {
    double prob_0 = 0.0;
    int state_size = get_state_size();
    
    //targetビットが0の項の確率を計算する
    for (int index = 0; index < state_size; ++index) {
        int bit_val = get_bit(index, target);
        if (bit_val == 0) {
            // std::normで複素数絶対値の2乗（確率）を安全・高速に計算
            prob_0 += std::norm(state[index]);
        }
    }
    
    // 乱数（0.0～1.0）を生成し、prob_0 より小さければ結果は 0、大きければ 1 とする
    // 共通乱数クラス(シングルトン)からサイコロを借りる
    double rnd = Random::getInstance().next_double();
    int result = (rnd < prob_0) ? 0 : 1;
    
    // 選ばれなかった方の状態の振幅をすべて 0 にする（状態ベクトルの収縮）
    for (int index = 0; index < state_size; ++index) {
        int bit_val = get_bit(index, target);
        // 測定結果（result）と一致しないビットを持つ項の振幅を消滅（0）させる
        if (bit_val != result) {
            state[index] = 0.0;
        }
    }
    
    // 残った状態ベクトルのノルムを計算し、合計が1になるよう全体を規格化する
    double chosen_prob = (result == 0) ? prob_0 : (1.0 - prob_0);
    
    if (chosen_prob > 1e-15) {
        // 規格化のショートカット定理（選ばれた世界の確率の平方根が全体のノルムになる）
        double norm = std::sqrt(chosen_prob);
        for (int index = 0; index < state_size; ++index) {
            state[index] /= norm;
        }
    }
    
    return result;
}

// ==========================================
// 量子誤り訂正アルゴリズム（回路）
// ==========================================

// 符号化 (Encoding)
void BitFlip3Code::encode(StateVector& state) {
    apply_CNOT(state, Q0, Q1);
    apply_CNOT(state, Q0, Q2);
}

// シンドローム測定の仕込みと実行
std::pair<int, int> BitFlip3Code::measure_syndrome(StateVector& state) {
    apply_CNOT(state, Q0, A0);
    apply_CNOT(state, Q1, A0); // A0 = Q0 ⊕ Q1

    apply_CNOT(state, Q1, A1);
    apply_CNOT(state, Q2, A1); // A1 = Q1 ⊕ Q2
    
    int m1 = measure_qubit(state, A0);
    int m2 = measure_qubit(state, A1);
    
    return {m1, m2};
}

// 復元 (Correction)
void BitFlip3Code::correct(StateVector& state, int m1, int m2) {
    // (m1,m2) = (1,0)->Q0, (1,1)->Q1, (0,1)->Q2, (0,0)->ない
    if (m1 == 1 && m2 == 0) apply_X(state, Q0);
    else if (m1 == 1 && m2 == 1) apply_X(state, Q1);
    else if (m1 == 0 && m2 == 1) apply_X(state, Q2);
}

// 検証 (Decoding)
void BitFlip3Code::decode(StateVector& state) {
    apply_CNOT(state, Q0, Q1);
    apply_CNOT(state, Q0, Q2);
}

// シミュレータ側が検証用インデックスを自動計算するための関数
std::pair<int, int> BitFlip3Code::get_verification_indices(int m1, int m2) const {
    int syndrome_offset = (m1 << 1) | m2; 
    return {0 + syndrome_offset, 16 + syndrome_offset};
}