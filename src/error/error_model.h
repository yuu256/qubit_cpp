#pragma once
#include "../code/quantum_code.h"
#include <vector>
#include <complex>

using Complex = std::complex<double>;
using StateVector = std::vector<Complex>;

enum class ErrorType { BitFlip, PhaseFlip, Depolarizing };

class ErrorModel {
public:
    ErrorModel(double error_rate, ErrorType type = ErrorType::BitFlip)
        : p(error_rate), error_type(type) {}

    // QuantumCode& code を受け取るように変更し、どんな量子符号にも対応できるようにする
    void inject_error(StateVector& state, QuantumCode& code);

private:
    double p;
    ErrorType error_type;
};