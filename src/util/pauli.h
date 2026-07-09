#pragma once
#include <vector>
#include <string>

// パウリ演算子の単一定義 (I: 何もしない, X: ビット反転, Y: 両方, Z: 位相)
enum class PauliType {
    I, X, Y, Z
};

// マルチ量子ビットパウリ演算子 (例: "ZZI" や "IXZ" をデータとして保持するクラス)
class PauliString {
public:
    // 文字列からパウリ演算子を構築する (例: "ZZI" -> [Z, Z, I])
    PauliString(const std::string& pauli_str) {
        for (char c : pauli_str) {
            if (c == 'I') op_list.push_back(PauliType::I);
            else if (c == 'X') op_list.push_back(PauliType::X);
            else if (c == 'Y') op_list.push_back(PauliType::Y);
            else if (c == 'Z') op_list.push_back(PauliType::Z);
        }
    }

    // 指定した量子ビットのパウリ型を取得する
    PauliType get_op(int qubit_idx) const {
        if (qubit_idx >= op_list.size()) return PauliType::I;
        return op_list[qubit_idx];
    }

    int size() const { return op_list.size(); }

private:
    std::vector<PauliType> op_list;
};