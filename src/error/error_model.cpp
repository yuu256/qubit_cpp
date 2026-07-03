#include "error_model.h"
#include "../util/random.h"
#include "../code/quantum_code.h"

void ErrorModel::inject_error(StateVector& state, QuantumCode& code) {
    Random& rand = Random::getInstance();

    // データ量子ビット（0, 1, 2番）に対して1つずつサイコロを振る
    for (int qubit = 0; qubit < 3; ++qubit) {
        
        // --- モード 1: ビット反転ノイズ (X) ---
        if (error_type == ErrorType::BitFlip) {
            if (rand.next_double() < p) {
                code.apply_X(state, qubit);
            }
        }
        
        // --- モード 2: 位相反転ノイズ (Z) ---
        else if (error_type == ErrorType::PhaseFlip) {
            if (rand.next_double() < p) {
                code.apply_Z(state, qubit); // 新設したZエラーを注入！
            }
        }
        
        // --- モード 3: 脱分極ノイズ (X, Z, または Y が均等に起きる) ---
        else if (error_type == ErrorType::Depolarizing) {
            // まず、そもそもこのビットにエラーが起きるかどうかを確率 p で判定
            if (rand.next_double() < p) {
                // エラーが起きる場合、3種類のエラー（X, Z, Y）に1/3の確率で均等に分配する
                double error_choice = rand.next_double();
                
                if (error_choice < 1.0 / 3.0) {
                    code.apply_X(state, qubit); // 1/3の確率で X エラー
                } 
                else if (error_choice < 2.0 / 3.0) {
                    code.apply_Z(state, qubit); // 1/3の確率で Z エラー
                } 
                else {
                    // 1/3の確率で Y エラー（XゲートとZゲートの両方が作用した状態）
                    code.apply_X(state, qubit);
                    code.apply_Z(state, qubit);
                }
            }
        }
    }
}