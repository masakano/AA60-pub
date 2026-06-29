# gsys class

## コンストラクタの標準形式

- GsCanvas (lib/gsys/canvas.h) は GsObject (lib/gsys/object.h) を継承してフレームバッファを使う処理を行う
-- GsCanvas を継承したクラスの置き場: lib/gsys/canvas (lib/gsys/src/canvas)
- GsNode (lib/gsys/node.h) は GsObject を継承してモデルの描画を行う
-- GsNode を継承したクラスの置き場: lib/gsys/node (lib/gsys/src/node)
- GsPainter (lib/gsys/painter.h) は GsObject を継承してモデルの描画を行う
-- GsNode を継承したクラスの置き場: lib/gsys/node (lib/gsys/src/node)

これらはテンプレートから統一的に使えるように、以下の標準のコンストラクタ形式を持つことが推奨される
例：
~~~
class Tonemap : public GsCanvas {
public:
	explicit Tonemap(const char *name = nullptr) : GsCanvas(name) {}
	explicit Tonemap(const Attrs &attrs) : Tonemap() { init(attrs); }
	void init(const Attrs &attrs) override;
	...
};
~~~
- 実態の初期は init(const Attrs &attrs) が受持ちコンストラクタは init() を呼ぶだけ
- 派生クラスは init() の最小に親クラスの init() を呼ぶ
- パラメータは attrs (lib/ssys/attrs.h) 経由で行う

例：
~~~
void Tonemap::init(const Attrs &attrs)
{
	// parent
	Attrs def_attrs = {
	       {"path", "canvas/tonemap/gather.us"}, // additiona option
	};
	GsCanvas::init(def_attrs + attrs);
	...
}
~~~
## シェーダプログラム
### ファイル形式
- シェーダーは GLSL で vert, frag, geom, tess, 各ステージのシェーダがひとつにまとまっていてプロプロセッサで
- 各ステージは vert:, frag:, geom:, tess: のラベルでわけられます。ファイルの拡張子は '.us'
   ex) 
~~~
   % cat simplest.us
   vert:
       in vec4 a_position;
       uniform mat4 u_nodescreen; 
       main() {       
       	       gl_Position = u_nodescreen * a_position;
       }
   frag:
       out vec4 final_color;
       main() {
		final_color = vec4(1.0);
	}
~~~
### 変数名のつけかた
~~~
    vertex shader の入力 (attribute): a_xxx
    buffer: 	  	 	      b_xxxx
    uniform 変数:  		      u_xxxx
    uniform block: 		      ub_xxxx
    fragment shader 入力: 	      f_xxxx
    fragment shader 出力: 	      final_colorX
~~~

## shader ファイルのパス
~~~
   lib/gsys/shaders
	+-- default	# 共通の変数
	+-- canvas	# GsCanvas 系
	+-- deocrator	# GsDecorator 系
	+-- painter	# GsDecorator 系
	+-- compute	# SpuComputeArray 系
~~~
- shader をロードする preprocessor (cpp) は暗黙で "lib/gsys/shaders/" に include path を張っているので
  例えば lib/gsys/shader/painter/mcube/mcube.us に置かれたファイルは、
~~~
		Attrs attrs = {
		        {"path",  "painter/mcube/mcube.us"}, 
		};
		GsPainter::init(attrs);
~~~		
    のように読み込める

				













