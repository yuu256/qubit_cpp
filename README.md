# Qubit C++ - 量子誤り訂正シミュレータ

## 概要

本プロジェクトは、C++による量子誤り訂正（Quantum Error Correction, QEC）の学習を目的として開発している状態ベクトルシミュレータです。

量子計算ライブラリ（QiskitやStimなど）を利用せず、量子ゲート・測定・シンドローム測定・誤り訂正を自作することで、量子誤り訂正アルゴリズムの理解を深めることを目的としています。

現在は3量子ビットビット反転符号（3-Qubit Bit Flip Code）の統計シミュレーションに対応しています。

---

## 主な機能

- 状態ベクトルシミュレーション
- Xゲート
- CNOTゲート
- 量子測定（状態収縮・規格化）
- 3量子ビットビット反転符号
- シンドローム測定
- 誤り訂正
- モンテカルロシミュレーション
- 物理エラー率と論理エラー率の計算

---

## ディレクトリ構成

```
src/
├── code/
│   ├── quantum_code.cpp
│   └── quantum_code.h
├── error/
│   ├── error_model.cpp
│   └── error_model.h
├── simulator/
│   ├── simulator.cpp
│   └── simulator.h
├── util/
│   ├── random.h
│   └── pauli.h
└── main.cpp
```

---

## 設計

本プロジェクトでは責務を分離した設計を採用しています。

- QuantumCode：量子誤り訂正コード
- ErrorModel：物理ノイズモデル
- Simulator：統計シミュレーション
- Random：乱数生成

今後は抽象クラス `QuantumCode` を利用して、複数の誤り訂正コードへ対応予定です。

---

## 今後の予定

- Phase Flip Code
- Depolarizing Error
- Shor Code
- Steane Code
- Surface Code
- CSV出力
- Rust版への移植
- SIMD・並列化による高速化

---

## 開発目的

本プロジェクトは量子誤り訂正アルゴリズムの理解、およびRust版シミュレータ開発のためのプロトタイプとして開発しています。