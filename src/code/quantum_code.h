#pragma once
#include <vector>
#include <complex>
#include <utility>

using Complex = std::complex<double>;
using StateVector = std::vector<Complex>;

// Phase 5: 任意コード対応のための抽象基底クラス
class QuantumCode {
public:
    virtual ~QuantumCode() = default;

    // 純粋仮想関数：派生クラス（BitFlip3など）で必ず実装する
    virtual void encode(StateVector& state) = 0;
    virtual std::pair<int, int> measure_syndrome(StateVector& state) = 0;
    virtual void correct(StateVector& state, int m1, int m2) = 0;
    virtual void decode(StateVector& state) = 0;
    
    // 量子ビット数などのメタ情報を取得する共通インターフェース
    virtual int get_num_qubits() const = 0;
    virtual int get_state_size() const = 0;
    virtual std::pair<int, int> get_verification_indices(int m1, int m2) const = 0;
};

// 従来の「3量子ビット ビット反転符号」は、この基底クラスを継承して作ります
class BitFlip3Code : public QuantumCode {
public:
    // 量子ビットのインデックス定義（enumによる安全な管理）
    enum Qubit { Q0, Q1, Q2, A0, A1, NUM_QUBITS };

    void encode(StateVector& state) override;
    std::pair<int, int> measure_syndrome(StateVector& state) override;
    void correct(StateVector& state, int m1, int m2) override;
    void decode(StateVector& state) override;

    int get_num_qubits() const override { return NUM_QUBITS; }
    int get_state_size() const override { return 1 << NUM_QUBITS; }
    std::pair<int, int> get_verification_indices(int m1, int m2) const override;

private:
    // ビット操作のヘルパー関数（インライン関数としてカプセル化）
    inline int get_bit(int index, int qubit) {
        return (index >> (NUM_QUBITS - 1 - qubit)) & 1;
    }
    inline int flip_bit(int index, int qubit) {
        return index ^ (1 << (NUM_QUBITS - 1 - qubit));
    }

    // 内部で利用する個別ゲートや測定関数
    void apply_X(StateVector& state, int target);
    void apply_CNOT(StateVector& state, int control, int target);
    int measure_qubit(StateVector& state, int target);
};