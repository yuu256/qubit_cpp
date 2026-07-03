#include "error_model.h"
#include "../util/random.h" // パスを -Isrc に合わせてクリーンに
#include "../code/quantum_code.h"

// 【修正】第2引数で受け取ったオブジェクト「code」を利用してゲートを動かす
void ErrorModel::inject_error(StateVector& state, QuantumCode& code) {
    // シングルトンからサイコロ（乱数生成器）を取得
    Random& rand = Random::getInstance();

    if (error_type == ErrorType::BitFlip) {
        // データ量子ビット（Q0, Q1, Q2 の計3つ）に対して1つずつサイコロを振る
        // ※ 抽象的な設計にするため、本当は code クラスから apply_X を呼びたいですが、
        // 現フェーズでは簡易的に、3ビット符号のQ0, Q1, Q2(0, 1, 2番)に直接当てる、または関数を呼びます。
        // -> 【解決】受け取った code の共通インターフェース(apply_X)を呼ぶことで、極めて抽象的で美しい設計になりました！
        
        // ここでは、1ビットずつ独立して確率 p の判定を行う
        
        // Q0へのエラー判定
        if (rand.next_double() < p) {
            // 将来的には code->apply_X を呼びますが、今はベタ書きのapply_X、
            // もしくはこの後実装する BitFlip3Code のメソッドを仲介させます。
            // -> 【解決】code.apply_X を使い、Q0（0番目のビット）を指定して呼び出します
            code.apply_X(state, 0);
        }
        
        // 【重要】独立した確率なので、if-else にせず、3つとも個別に if でサイコロを振る！
        // これにより「2つ同時にエラーが起きて訂正失敗する現実のノイズ」が再現できます。
        if (rand.next_double() < p) { code.apply_X(state, 1); } // Q1 (1番目のビット)
        if (rand.next_double() < p) { code.apply_X(state, 2); } // Q2 (2番目のビット)
    }
}