
# configuration

~~~
- project-dir
   +-- AGENTS.md
   +-- AGENTS
   |    +-- ssys_overview.md
   |    +-- smath_overview.md
   |    +-- ...
   |
   +-- lib
   |    +-- ssys/
   |    |     +-- CMakeLists.txt
   |	|     +-- ...
   |    +-- smath/
   |    |     +-- CMakeLists.txt
   |	|     +-- ...
   |    +-- spu/
   |    |     +-- CMakeLists.txt
   |	|     +-- ...
   |    +-- spu++/
   |    |     +-- CMakeLists.txt
   |	|     +-- ...
   |    +-- gsys/
   |    |     +-- CMakeLists.txt
   |    |     +-- ...
   |	+-- ...
   |    |
   |    +-- libssys.so
   |    +-- libsmath.so
   |    +-- libspu.so
   |    +-- libspu++.so
   |    +-- libgsys.so
   |
   +-- basic
   |    +-- CMakeLists.txt
   |	+-- ...
   +-- nvsamples
   |    +-- CMakeLists.txt
   |	+-- ...
   +-- ....
~~~

## lib

- ssys	OS 機能のラッパライブラリ (libssys.so)
- smath	3DCG SIMD ライブラリ (libsmath.so)
- spu	グラフィクスライブラリ (OpenGL ラッパ）
- spu++	libspu の C++ ラッパ
- gsys	高レベルライブラリ
- library dependency:
  - ssys
  - smath  <- ssys (spu に依存なし）
  - spu    <- ssys (smath に依存なし）
  - spu++  <- ssys, smath, spu
  - gsys   <- ssys, smath, spu, spu++


# environment / files

- python がない場合は python3 を使う
- include gurad は #pragma once を使う
- RCS/ '*,v' はバックアップなので無視
- codex.txt は codex との会話ログなので無視
- lib/external は検索や編集からはずす

# convention

## 命名規則

- クラス・構造体              UpperCamelCase
- クラス・構造体の関数         lowerCamelCase	
- クラスのメンバ              m_lowerCamelCase
- 構造体のメンバ              snake_case
- static/local 変数          snake_case
- static/const constant 変数 c_snake_case
- static 関数                snake_case
- マクロ / enum              e_snake_case		
- shader uniform            u_snake_case
- shader uniform block      ub_snake_case
- shader macro              def_snake_case

## 型と変数名

- 'textv' や 'lenv' のように末尾に 'v' がついたポインタ配列は nullptr 終端とします。
      例 const void *ptrv = {p0, p1, nullptr};
- bool         m_isXXX, is_XXX
- vector, map  複数形にする (ex: datas, values)
- 要素の個数     m_xxxCount, xxx_count
- 行列の変数名
  - 変換の順番で命名します。
~~~	
 Mat4f worldview;  // world to view 座標変換
 Mat4f viewscreen; // view to screen 座標変換
 Mat4f worldscreen = screenview * worldview;
~~~

## cast
- なるべく auto の型推論を使う。
- for 文のインデックスは auto を使って範囲指定の条件が、size_t などの場合はに初期化の値で unsigned に誘導
~~~
int n: 
for (auto i = 0; i < n; i++) {...}
	
auto size = array.size();
for (auto i = 0u; i < size; i++) {...}
		  
/* これは避ける */
for (auto i = 0; i < int(size); i++) {...}
~~~		

- 基本型への cast は static_cast ではなくコンストラクタを使う。

~~~
// preferred: 
auto float_value = float(int_value) 

// not preferred:
auto float_value = static_cast<float>(int_value);
~~~

- 特例として (void *) への cast は許す。
~~~
Attrs attrs = { 
 	 {"array.data", (void *)array.data()},
};
~~~	 
## class (struct) 定義
### 定義順
- public: -> protect: -> private の順番。その中では 
  - メンバ定義
  - 関数定義
  - inline static メンバ定義
  - inline static 関数定義
の順番で定義

### アクセス権
- struct メンバは public, 
- class メンバは private/protectd, メソッド経由でアクセス
- getXXX() は reference を返す getter 
  - ex: std::vector<GsDrawcall> &getDrawcalls() { return m_drawcalls; }
  - const/non-const 版のふたつをもつ
  - xxxx() は const 修飾の getter (ex: const Range3f &range() const { return m_range; })

# ビルド
- ソースファイルおよび追加の include path は sources.cmake に書かれる
- ビルド後 clang-format をかける
  
# library 概要
- AGENTS/*.md

# unit test
- unit_tests フォルダに一部 smoke test があるので使い方は必要に応じて参照
	
	
