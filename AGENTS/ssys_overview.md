# ssys overview

## ssys/ssys.h

### optional_t
- std::optional の簡易形

### hash32
- 文字列の 32bit hash を求めるクラス

### hash256
- 文字列の 256bit hash を求めるクラス

### Strehap
- 文字列変数を保存しそれがスコープをはずれても内容を保持するクラス

### Message
- ログ文字列を出力
  - public: なメンバ
	- output: ドライバ (デフォルトは default_output.h で定義）
	- is_esception: Message::exit() logic_error の例外を投げるか
	- level: 表示レベル Status::level 以下の level のログを出力
  - グローバル変数
	- g_message: Message のデフォルトインスタンス
  - マクロ
    - aux_print() / aux_exit() / aux_message / aux_error(): ラッパマクロ

### File

#### trivial funcs
~~~
     bool open(); // open
     void flush(); // flush
     void close(); // close
     void rewind(); // rewind
     size_t size() const; // retunr file size
     int64_t mtime() const; // return last modified time
     size_t tell(); // tell
     size_t read(void *buf, size_t count, bool is_abort = true); // read
     size_t write(const void *buf, size_t count, bool is_abort = true); // write
     int32_t scanf(const char *format, ...); // scanf
     void printf(const char *format, ...); // printf
     bool getchar(uint8_t &c); // getchar(). return false when EOF
     bool getline(std::string &line); // read line. return false when EOF
     bool getlist(...) // return list of lines. return false when EOF
     const std::filesystem::path &name() const noexcept; // file name
     FILE *fp() const noexcept { return m_fp; } // return raw FILE 
~~~

#### push/pop
- pushBase() で探索するパスのリストを追加
- popBase() で最後に pushBase() したパスのリストをpop。
- searchPath() でパスを探す
  - 相対パスのファイルにパスのリストを先頭に付加して現在のパスから上にたどりながら探索
~~~
       pushBase({"data/", "sample/" "."}
	   /*
       現在の path が /home/codex のとき "test.dat" は
         ./test.dat, ./data/test.dat, ./sample/test.dat, 
         codex/test.dat, codex/data/test.dat, codex/sample/test.dat,
         /home/codex/codex/test.dat, /home/codex/codex/data/test.dat, /home/codex/codex/sample/test.dat
       を探す
	   */
~~~
#### embed/unembed
- embed() は data() の内容を path に登録して searchPath() の検索に引っかかるようにする
- unembed() はこれを削除

### Timer
- uint64_t get_microsec(); // 現在の usec を返す。
- void set_microsec(uint64_t usec); //　usec をセット
- void sleep_microsec(uint64_t usec, uint64_t msec, uint64_t sec); // usec + msec + sec だけ sleep

### Seconds
- ゲーム内時間をフレームごとに更新するクラスです。
- ゲーム内時間は必ずしも実時間を反映はしません。
~~~
   Second second;	
   while (true) {
       second.update();
       some_process();
       swap_buffer();
    } 
	/*
    prev:  前回の経過時間 (sec)
    curr:  現在の経過時間 (sec)
    delta: 前回から差分 (sec) : 
    pase: 実時間に対して delta を pase 倍にする
    fixd_delta: 強制的に delta を一定にする
	*/
~~~	

### CharsetConverter
- SJIS/UTF8/UTF16-LE などの文字セットを変換
- puts() で入力し flush() であラインされた文字列のリストをかえします。
  - opengl_const()
	- OpenGL のマクロ文字列と enum 値をs相互変換します

### misc
~~~
   // sprintf() の std::string 版
   std::string string_printf()

   // std::string を delim でトークンに分割する
   //  - "" / '' で囲まれた部分は単一トークン (is_strip_quate = true 時)
   //   - cutoff はそこで分割されるがトークンに残る ('{', '}' など)
   std::vector<std::string> extract_from_string()

   // 前後の無駄な非表示文字を取り除く
   std::string peeloff_string();

   // 長過ぎる文字の後ろを省略して '...' に置換
   std::string pretty_string();

   // dmangle
   std::string demangle_string();

   // テキストファイルからテキストを読み出します。
   read_from_file() 

   // base_dir以下にもとと同じフォルダツリーをつくり suffix をつけたパスを返します。
   make_cache_path() 
~~~

## ssys/udp_socket.h
- UdpSocket は薄い udp socket / recvfrom / sendto などのラッパ

## ssys/murmur3.h
- constexpr な Murmur3 hash. hash32_t で使用

## ssys/simd.h
- SSE4.1 以上の SIMD ラッパ
  - normalize4() (x,y,z,w) のノルム
  - dnorm() (x,y,z,w) : div() と組み合わせて分母が 0 にならないようにガードする
  - cross()  ３次元の外積

## ssys/random_generator.h
- <random> の薄いラッパ

## ssys/half_float.h
- 16bit float 32bit float を相互変換

## ssys/serializer.h
- オブジェクトの seralize/deserialize
  - 対象は POD, POD を要素とする std::vector, std::pair, std::map および std::string。
  - is_dry=true で heap サイズを計算して is_tru=false で実際に serialize

## ssys/vector_view.h
- vector の連続したメモリを型の異なる他の vector にマップ

### ssys/shared_memory.h
- 共有メモリのラッパ

### ssys/cpu_prof.h
- cpu プロファイラ

### ssys/bmp.h
- BMP 画像の load/save

## ssys/attrs.h
- m_key, m_type, m_value を (private) メンバに待つ構造体 Attr の vector
~~~
    Attrs attrs = {
    	  {"int_value", 10},
    	  {"floatt_value", 10.},
    };
    //attrs.get() で value を get()
~~~

- attrs.trace() は attrs とそこから継承された attrs を追いかけて指定した key-value が正しく伝播しているか確認する
  - "attrts の key-value ごと使用されたかどうかの判定を行う
  - select() で切り出された key もできるだけもとの prefix を保存する
  - デフォルトではスコープを抜けると出力
  - trace()　が呼ばれたソースの位置を費用時。

