#include "error_model.h"
#include "../util/random.h"
#include "../code/quantum_code.h" // BitFlip3CodeのQubit定義(Q0, Q1, Q2)を使うため

void ErrorModel::inject_error(StateVector& state, int num_data_qubits) {
    // シングルトンからサイコロ（乱数生成器）を取得
    Random& rand = Random::getInstance();

    if (error_type == ErrorType::BitFlip) {
        // データ量子ビット（Q0, Q1, Q2 の計3つ）に対して1つずつサイコロを振る
        // ※ 抽象的な設計にするため、本当は code クラスから apply_X を呼びたいですが、
        // 現フェーズでは簡易的に、3ビット符号のQ0, Q1, Q2(0, 1, 2番)に直接当てる、または関数を呼びます。
        // ここでは、1ビットずつ独立して確率 p の判定を行う
        
        // Q0へのエラー判定
        if (rand.next_double() < p) {
            // 将来的には code->apply_X を呼びますが、今はベタ書きのapply_X、
            // もしくはこの後実装する BitFlip3Code のメソッドを仲介させます。
        }
        
        // 【重要】独立した確率なので、if-else にせず、3つとも個別に if でサイコロを振る！
        // これにより「2つ同時にエラーが起きて訂正失敗する現実のノイズ」が再現できます。
    }
}