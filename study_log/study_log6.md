# 【C++×量子計算】学習ログ Vol.6

将来の拡張性を意識したディレクトリ構成に
```
qubit_cpp/               （プロジェクトのルートフォルダ）
├── src/                 （すべてのソースコードをここに集約！）
│   ├── main.cpp
│   ├── code/
│   │   ├── quantum_code.h
│   │   └── quantum_code.cpp
│   ├── error/
│   │   ├── error_model.h
│   │   └── error_model.cpp
│   ├── simulator/
│   │   ├── simulator.h
│   │   └── simulator.cpp
│   └── util/
│       └── random.h
└── (将来的に README.md やビルド設定ファイルをここに置く)
```
## code/
量子回路についてのコードをまとめた

## error/
error注入機構

## simulator/
シミュレーション

-------


ビルドコマンドもきれいになった
```bash
g++ -O3 -Isrc src/main.cpp src/code/quantum_code.cpp src/error/error_model.cpp src/simulator/simulator.cpp -o qec_sim
```


## .h と .cpp の役割分担
複数ファイルに分ける場合、役割は以下のように分かれます。

### .h（ヘッダーファイル）：いわば「取扱説明書」

関数やクラスの「名前」と「形（引数や戻り値）」だけを宣言します。

他のファイルから呼び出すときは、この取扱説明書を #include します。

### .cpp（実装ファイル）：いわば「中身・中身の組み立て」

ヘッダーで宣言した関数の「実際の処理内容」を書きます。

### main.cpp：プログラムの入り口

全体の流れをコントロールし、必要に応じて他の .cpp で作った関数を呼び出します。

## 二重定義の防止

###  インクルードガード(古い)
```cpp
#ifndef MYFUNC_H
#define MYFUNC_H

// --- ここに中身を書く ---

#endif
```

### #pragra onceを使った方法（モダン）
```cpp
#pragma once

// --- ここに中身を書く ---
```
----

   


C++における `virtual`（バーチャル）は、**「オブジェクト指向プログラミング（OOP）」の核心である「多態性（ポリモーフィズム）」を実現するためのキーワード**です。

主に「仮想関数」**と**「仮想デストラクタ」という2つの重要な役割を持っています。初心者にとって最初の難所になりやすい部分なので、なぜ必要なのかを噛み砕いて解説します。

---

## 1. 仮想関数（Virtual Function）とは？

一言で言うと、「親クラスの形をした変数に子クラスを入れたときでも、ちゃんと子クラス側の関数を実行させるための仕組み」です。

### ❌ `virtual` を付けないとどうなるか？

例えば、「動物（親）」クラスを継承して、「犬（子）」クラスを作ったとします。

```cpp
#include <iostream>

class Animal {
public:
    void cry() { std::cout << "???" << std::endl; }
};

class Dog : public Animal {
public:
    void cry() { std::cout << "ワンワン！" << std::endl; } // 上書き（上書きしたつもり）
};

int main() {
    // 親クラスのポインタ（型）で、子クラスのインスタンスを指す
    Animal* myAnimal = new Dog();
    
    myAnimal->cry(); // 📢 結果は「???」になってしまう！
    
    delete myAnimal;
    return 0;
}

```

C++は静的な言語なので、`virtual` が付いていないと、コンパイラは「変数の型（`Animal*`）」だけを見て、親クラスの `cry()` を実行してしまいます。

### ⭕ `virtual` を付けると解決！

親クラスの関数の先頭に `virtual` を一文字足してみます。

```cpp
class Animal {
public:
    virtual void cry() { std::cout << "???" << std::endl; } // virtual を付けた！
};

```

これだけで、先ほどの `main` 関数の結果は **「ワンワン！」** に変わります。
`virtual` を付けることで、プログラムは実行時に「中身は実は `Dog` だな」と判断し、正しく子クラスの関数を呼び出してくれるようになります。（これを**オーバーライド**と呼びます）

---

## 2. 仮想デストラクタ（メモリリークの防止）

`virtual` を使う上で、**絶対に忘れてはいけない超重要ルール**があります。
それは、**「継承される前提の親クラスでは、デストラクタ（消滅時の関数）にも必ず `virtual` を付ける」** というルールです。

### ❌ `virtual` がない場合のメモリリーク

もし親クラスのデストラクタに `virtual` がないと、先ほどの `delete myAnimal;` をしたときに、**親クラスのデストラクタしか実行されません。**

もし `Dog` クラスの中で独自にメモリを確保（`new` など）していた場合、子クラスのデストラクタが呼ばれないため、そのメモリが解放されずに残る **「メモリリーク」** が発生します。

### ⭕ 正しい親クラスの定義

```cpp
class Animal {
public:
    virtual void cry() { std::cout << "???" << std::endl; }
    
    // 親クラスのデストラクタには必ず virtual を付ける！
    virtual ~Animal() { std::cout << "Animal消滅" << std::endl; }
};

```

こうしておけば、`delete myAnimal;` をしたときに「`Dog` のデストラクタ」⇒「`Animal` のデストラクタ」の順番で両方正しく実行され、安全にメモリが解放されます。

---

## 3. 純粋仮想関数（Pure Virtual Function）

さらに、「親クラスでは中身（処理）を一切書かない。子クラスで絶対に中身を実装してね！」と強制することもできます。これを**純粋仮想関数**と呼びます。

```cpp
class Animal {
public:
    // 後ろに「= 0」を付ける
    virtual void cry() = 0; 
};

```

こうすると、`Animal` クラスは「設計図（インターフェース）」の役割になり、`Animal` 単体でのインスタンス化（`new Animal()`）は禁止されます。そして、継承した `Dog` などの子クラスで `cry()` を書き忘れるとコンパイルエラーにしてくれるため、バグを未然に防げます。

---

##  まとめ

モダンなC++（C++11以降）では、子クラス側で上書きする際、末尾に `override` と書くことで、正しくオーバーライドできているかをコンパイラがチェックしてくれるため、セットで覚えるのがおすすめです！

```cpp
class Dog : public Animal {
public:
    void cry() override { std::cout << "ワンワン！" << std::endl; } // overrideを付ける
};

```

* **`virtual`**: 親クラスの関数につけて、「子クラスで上書きされるかもしれないよ」と伝える。
* **`override`**: 子クラスの関数につけて、「親クラスの仮想関数を上書きしたよ」と宣言する。