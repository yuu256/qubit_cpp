#pragma once
#include <vector>
#include <complex>

using Complex = std::complex<double>;
using StateVector = std::vector<Complex>;

// Phase 3: エラーモデルの一般化
enum class ErrorType {
    BitFlip,     // Xエラー
    PhaseFlip,   // Zエラー
    Depolarizing // 脱分極エラー（将来用）
};

class ErrorModel {
public:
    ErrorModel(double error_rate, ErrorType type = ErrorType::BitFlip)
        : p(error_rate), error_type(type) {}

    // データ量子ビットの範囲（Q0～Q2）に対して、確率 p に基づきランダムにエラーを注入する
    void inject_error(StateVector& state, int num_data_qubits);

private:
    double p;
    ErrorType error_type;
};