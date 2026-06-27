# 【C++×量子計算】学習ログ Vol.5：デモコードから本格シミュレータへ。リファクタリングとアーキテクチャ設計

C++を用いた量子計算シミュレータ実装ログ第5弾です。 

前回（Vol.4）までに「3量子ビット ビット反転符号」によるエラー訂正の一連の動作に成功しました 。今回は、コード内に散らばっていたマジックナンバーや重複ロジックを大掃除し、将来的に別の誤り訂正コード（Shorの符号やSurface Codeなど）へ拡張するための「設計の抽象化とシミュレータ基盤の分離」を行います 。

学習の軌跡（TODOコメントや思考プロセス）をコード内にすべて大切に残したまま、いかにして再利用性と拡張性の高い本格的なシミュレータ製品のアーキテクチャへと進化させるか、4つの改善ポイントに沿って解説します 。

---

## 1. アーキテクチャを洗練させる4つの改善ポイント

### ① マジックナンバー（32）の排除と「enum末尾トリック」

これまで、状態ベクトルの配列サイズやループの境界条件に `32` という即値（マジックナンバー）を直接書き込んでいました 。しかし、これでは量子ビットの数を増やしたり減らしたりした瞬間に、コード全体を力技で修正せざるを得なくなり崩壊します 。
そこで、量子ビットの管理に `enum` を導入し、**末尾に `NUM_QUBITS` を配置するトリック**を採用しました 。

```cpp
enum Qubit { Q0, Q1, Q2, A0, A1, NUM_QUBITS };
const int STATE_SIZE = 1 << NUM_QUBITS;

```

これにより、将来的にビットの名前（`Q3`, `Q4`, `A2` など）を `enum` の中に付け足すだけで、全体のビット総数（`NUM_QUBITS`）と状態ベクトルの配列サイズ（`STATE_SIZE`）がコンパイル時に自動追従される、安全な自動化機構が完成します 。

### ② 生々しいビット演算の隠蔽（ヘルパー関数化）

コード内のいたるところに登場していた `(index >> (NUM_QUBITS - 1 - target)) & 1` などの泥臭いビットシフト演算を、インライン関数としてカプセル化（隠蔽）しました 。

* 
`get_bit(index, qubit)`：指定したビットの値（0か1）を取得する 。


* 
`flip_bit(index, qubit)`：指定したビットを反転させたインデックスを生成する（XOR演算 `^` を利用） 。



これにより、各ゲート関数の内部が `if (get_bit(index, target) == 0)` のように英語の文章のごとく読めるようになり、可読性が劇的に向上しました 。

### ③ ゲート関数の共通パーツ化

XゲートとCNOTゲートは、「対象となる項を見つけ、ペアになる相手と中身（振幅）をスワップする」という本質的なロジックが酷似しています 。ビット操作関数を共通化した恩恵で、ゲート関数ごとの固有条件（`if` 文の中身）以外を綺麗に共通のパーツとして整理できました 。今後、ZゲートやHゲートといった他のゲートを追加する際も、この構造をそのまま流用できます 。

### ④ アルゴリズムとシミュレータ基盤の分離（関数分割）

`main()` 関数の中に直書きされていた量子回路のステップを、数理的な意味ごとに独立した関数（`encode`, `measure_syndrome`, `correct`, `decode`）に分割しました 。
これにより、高レベルの「量子誤り訂正アルゴリズムのフロー」と、低レベルの「量子ゲートの内部実装」が綺麗に分離され、ソフトウェアのレイヤー構造が洗練されました 。

---

## 2. リファクタリング＆シミュレータひな形を含む完全なC++コード

自分で後から見返したときにわからなくならないよう、**これまでのTODOコメントや下書きの思考プロセスを一切削除せずにすべて残した状態**で清書した、完全な実装コードです 。末尾には、次段階へ進化するための「シミュレータ化の枠組み（ひな形）」も追加しています 。

```cpp
#include <iostream>
#include <vector>
#include <complex>
#include <random>
#include <cmath>

using Complex = std::complex<double>;
using StateVector = std::vector<Complex>;

// ① 32を直書きしないための enum 末尾トリック
enum Qubit {
    Q0, // データ量子ビット0
    Q1, // データ量子ビット1
    Q2, // データ量子ビット2
    A0, // 補助量子ビット0
    A1, // 補助量子ビット1
    NUM_QUBITS // 末尾に置くことで、自動的に量子ビットの総数になる
};

// 安全のために 1 << NUM_QUBITS で自動計算
const int STATE_SIZE = 1 << NUM_QUBITS; 

// ② ビット位置計算を何度も書かないためのヘルパー関数（インライン）
inline int get_bit(int index, int qubit) {
    // index のなかの「target番目の量子ビット」の値（0か1）を取り出す式を共通化
    return (index >> (NUM_QUBITS - 1 - qubit)) & 1;
}

inline int flip_bit(int index, int qubit) {
    // targetビットを1に反転（あるいは0に反転）させたペアのインデックスを安全に生成
    return index ^ (1 << (NUM_QUBITS - 1 - qubit));
}

// --- 量子ゲート演算（共通ヘルパーを使ってスッキリ共通化） ---

// 1量子ビットXゲート：targetビットが0の項と1の項を入れ替える
// TODO: 0～31までループを回す
// targetビットが0であるインデックスを見つけたら、
// それに対応する「targetビットが1であるインデックス」と中身(振幅)をスワップする。
// ※2重にスワップしないように注意！
void apply_X(StateVector& state, int target) {
    // マジックナンバーを STATE_SIZE に置き換え
    for (int index = 0; index < STATE_SIZE; ++index) {
        // targetビットの値（0か1か）を取り出す
        int bit_val = get_bit(index, target);
        
        // 2重スワップを防ぐため、0のときだけ処理
        if (bit_val == 0) {
            int pair_index = flip_bit(index, target);
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
    for (int index = 0; index < STATE_SIZE; ++index) {
        int control_val = get_bit(index, control);
        int target_val  = get_bit(index, target);
        
        // CNOTの条件：「controlが1」かつ「targetが0」の項だけを処理（2重スワップ自動ガード）
        if (control_val == 1 && target_val == 0) {
            int pair_index = flip_bit(index, target);
            std::swap(state[index], state[pair_index]);
        }
    }
}

// 1量子ビット測定：確率に基づき0か1を返し、状態ベクトルを収縮・規格化する
int measure_qubit(StateVector& state, int target) {
    double prob_0 = 0.0;
    
    // TODO: 1. targetビットが0である全てのインデックスの「振幅の絶対値2乗」を足し合わせ、prob_0 を計算する
    for (int index = 0; index < STATE_SIZE; ++index) {
        // targetビットの値（0か1か）を取り出す
        int bit_val = get_bit(index, target);
        
        // targetビットが「0」のときだけ処理を行う
        if (bit_val == 0) {
            // prob_0 += pow(state[index],2.0); -> std::complex から double への変換
            prob_0 += std::norm(state[index]);
        }
    }
    
    // TODO: 2. 乱数（0.0～1.0）を生成し、prob_0 より小さければ結果は 0、大きければ 1 とする
    // double rnd 乱数生成（メルセンヌ・ツイスタを使用）
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double rnd = dist(gen);
    
    int result = (rnd < prob_0) ? 0 : 1;
    
    // 規格化（TODO 4）のためのショートカット用ノルムを計算
    double chosen_prob = (result == 0) ? prob_0 : (1.0 - prob_0);
    double new_norm = std::sqrt(chosen_prob);
    
    // TODO: 3. 選ばれなかった方の状態の振幅をすべて 0 にする
    //（例：resultが0なら、targetビットが1の項の振幅を 0 にする）
    // TODO: 4. 残った状態ベクトルのノルム（全体の長さ）を計算し、合計が1になるよう全体を割り算（規格化）する
    // stateの中身を足して(=sum)、sum/=sum -> ショートカット定理により、1回のループで収縮と規格化を同時に完了させる
    for (int index = 0; index < STATE_SIZE; ++index) {
        int bit_val = get_bit(index, target);
        
        if (bit_val != result) {
            state[index] = 0.0; // 状態の収縮
        } else {
            state[index] /= new_norm; // 規格化
        }
    }
    
    return result;
}

// --- ⑤ さらに次の段階：アルゴリズムそのものとゲート実装の分離 ---

// 2. 符号化 (Encoding)
void encode(StateVector& state) {
    apply_CNOT(state, Q0, Q1);
    apply_CNOT(state, Q0, Q2);
}

// 3. エラー発生 (Noise)
void inject_error(StateVector& state) {
    apply_X(state, Q1); // 今回はQ1エラーの固定シナリオ
}

// 4 & 5. シンドローム測定の仕込みと実行
void measure_syndrome(StateVector& state, int& m1, int& m2) {
    apply_CNOT(state, Q0, A0);
    apply_CNOT(state, Q1, A0);
    apply_CNOT(state, Q1, A1);
    apply_CNOT(state, Q2, A1);
    
    m1 = measure_qubit(state, A0);
    m2 = measure_qubit(state, A1);
}

// 6. 復元 (Correction)
void correct(StateVector& state, int m1, int m2) {
    // (m1,m2) = (1,0)->Q0, (1,1)->Q1, (0,1)->Q2, (0,0)->エラーなし
    if (m1 == 1 && m2 == 0) {
        apply_X(state, Q0);
    }
    else if (m1 == 1 && m2 == 1) {
        apply_X(state, Q1);
    }
    else if (m1 == 0 && m2 == 1) {
        apply_X(state, Q2);
    }
}

// 7. 検証 (Decoding)
void decode(StateVector& state) {
    apply_CNOT(state, Q0, Q1);
    apply_CNOT(state, Q0, Q2);
}

// --- 次に実装すべきもの（単発のデモから「シミュレータ」への進化のための枠組み・ひな形） ---

// 1. run_once() を作り、1回の誤り訂正を実行して成功・失敗を返す。
// 3. ランダムなビット反転エラー（確率 p）を導入する。
bool run_once(double p) {
    // TODO: ここにランダムノイズ混じりの1回のエラー訂正フローを記述する
    // 成功（元の状態を維持できた）なら true, 失敗なら false を返すように肉付けする
    return true;
}

// 2. run_simulation(int trials, double p) を作り、指定回数繰り返して論理エラー率を計算する。
// 4. 物理エラー率 p を変化させて、論理エラー率との関係を出力する。
double run_simulation(int trials, double p) {
    // TODO: trials 回数だけ run_once(p) を繰り返し、システム全体の「論理エラー率」を算出して返す
    return 0.0;
}

// --- メイン処理：エラー訂正のシナリオ ---
int main() {
    // 状態ベクトルの初期化（全要素0）
    StateVector state(STATE_SIZE, 0.0);
    
    // 1. 初期状態の設定 (例: alpha|00000> + beta|10000>)
    double alpha = 0.6;
    double beta = 0.8;
    state[0] = alpha;  // |00000> のインデックスは 0
    state[16] = beta; // |10000> のインデックスは 16 (1 << 4)
    
    std::cout << "--- 3-Qubit Bit-Flip Code Simulation (Refactored) ---" << std::endl;
    
    // レイヤー構造が分離された美しい数理フローの実行
    encode(state);
    inject_error(state);
    
    int m1 = 0, m2 = 0;
    measure_syndrome(state, m1, m2);
    std::cout << "Syndrome Measured: (m1, m2) = (" << m1 << ", " << m2 << ")" << std::endl;
    
    correct(state, m1, m2);
    decode(state);
    
    // 8. 結果の出力：状態収縮のズレ（測定結果に応じた位置）を計算して確認
    int final_base_index = (m1 << 1) | m2; 
    int idx_alpha = 0 + final_base_index;  
    int idx_beta  = 16 + final_base_index; 
    
    std::cout << "--- Final Result Verification ---" << std::endl;
    std::cout << "state[" << idx_alpha << "] (alpha): " << state[idx_alpha] << " (Expected: 0.6)" << std::endl;
    std::cout << "state[" << idx_beta << "] (beta) : " << state[idx_beta] << " (Expected: 0.8)" << std::endl;
    
    return 0;
}

```

---

## 3. 今後の拡張・実装メモ（随時追記）

* [ ] 確率 $p$ で動くエラー挿入を `inject_error` や `run_once` の枠組み内に実装し、ランダムノイズモデルを完成させる 


* [ ] `run_simulation` 関数の中身を記述し、物理エラー率 $p$ を $0.01 \sim 0.30$ まで変化させたときの「論理エラー率」の統計データを集集する 


* [ ] 集計した統計データをCSV形式などで出力し、Python（matplotlib）などでエラー訂正の「閾値（しきい値）」をプロットする実験を行う 


* [ ] C++で構築したこのクリーンなアーキテクチャの構造をキープしたまま、Rust（ndarrayやnum-complexクレート）への移植に挑戦する