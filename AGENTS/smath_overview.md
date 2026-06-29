# smath functions
## smath/vec.h
- ssys/simd.h の高機能版
 - すべて 16byte アライン、余ったフィールドはゼロ詰め
 - 比較演算子の返り値が bool ではなくビットマスク
~~~
    Vec4f a(1,2,3,4), b(2,2,2,2);
    Vec4i det = (a == b);
    では 
    det == Vec4i(0, ~0, 0, 0);
    det.pack() == 0x00f0   // 8bit -> 1bit (32bit -> 4bit) pack
~~~
 - クラス 
 ~~~
   +- Vec4i (from vec4i_t)
   +- vec4f_t (from vec4f_t)
       +-- Vec4f
       +-- Vec3f
       +-- Vec2f
~~~
## smath/range.h
- メンバ p0, p1 が範囲の上限・下限を表すテンプレートクラス

## smath/mat4f.h
- 4x4 matrix クラス 
  - OpenGL と同じ座標系
  - world  ワールド座標系
  - view   カメラ（ビュー）座標系
  - screen スクリーン座標系  [-1.0, +1.0] 
  - texc   テクスチャ座標系（２次元） texc_[xy] = screen_[xy] * 0.5 + 0.5
  - frag   フレームバッファ（フラグメント）座標系. フレームバッファの解像度に依存
  - name convention
     - wordscreen : world -> screen 座標系の変換行列
       	   ex) Mat4f worldview, viewscreen, screen frag;
	       auto worldfrag = screenfrag * viewscreen * worldscreen;
  
  - 目的に応じた行列を作る
	- texcfrag(viewport)/fragtexc(viewport) : texc と frag 座標系の相互変換
	- screentexc/texcscreen: screen と texc 座標系の相互変換
	- projection: opengl に似た projection 行列生成
	- cross_product_matrix: 外積を行列のに変換（演算順序に注意）
	- tangentworld: 接平面への座標変換に使う
	- reprojection: z=0 平面で quad -> quad への変換

  - is_projectable(const std::vector<Vec3f> &points) 
     - いずれかの points を変換して w < 0 になれば false
  - shift(const std::vector<Vec3f> &points)
     - points が透視変換後に (-1,-1) - (+1,+1) にはいるように補正する
  - shift(const Rectf &from_viewport, const Rectf &to_viewport)
     - 透視変換後に from_viewport の領域が to_viewport になるように補正する
  - projection(const Vec3f &ray, const Plane3f &plane, bool is_perspective)
     - plane への射影変換 ( ray は is_perspective = true で点光源の中心、false で並行光源の方向）

## smath/quatf.h
 Quatf : quaternion class
 - stabilize()
   -- (0,0,1) の変換あとに up との外積 (roll 成分）が変わらないようにしてカメラを安定させる
 - from_target()
   -- effect が target と同じ向きになるような変換
 - from_target_and_axis()
   -- 回転軸 axis で effect が target と一番近い向きになるような変換。
 - from_eulerXYZ()/from_eulerZYX()
   -- eulear 角を XYZ/ZYX の順番で適用したときの quaternion

## smath/geometry.h
- struct Lin3f : p = t * dir + eye の直線
- struct Planef: dot(Vec4f(p, 1), eq) = 0 の平面
- struct Segmentf: ２点 p0, p1 を結ぶ線分
- struct Sphere3f: length(p - center) = radius の円

- unproject(): world　空間での視線を返す
- center_and_normal(): 点群 points から重心 center と法線 normal を簡易的に求める
- quantize_and_uniq(): 点群 points を量子化してまとめる

## smath/convex2f.h
- smath/convex2f.h は 3D 内に埋め込まれた 2D の凸面体クラス。m_points は頂点、m_plane は凸面が含まれる平面です。
- flip(): 頂点の並び（と法線）を反転
- cup(): ２つの convex を包絡する convex
- cap(): ２つの convex に含まれる convex
- walls() convex の各辺に対する法線  
- inside/intersect() ではプリミティブは同一平面にあると仮定

## smath/convex3f.h
- 3D の凸面体クラス。m_convexes 同士の頂点は一致しているものと仮定。
- center_of_gravity(): 重心
- moment(): 回転モーメント
- modulate(): 構造を保ったまま頂点だけ変形
- makeCyliner(): 円柱を作成する
- marge(): 重複している面をまとめる	
- auto_flip(): 面の向きをあわせる

## smath/dual_quatf.h
- dual quaternion のクラスです。

## smath/composition.h
- 3DCG のカメラの構図を扱うクラス
 - 複数の viewport, 複数の camera を制御する
   - メンバ：
 ~~~
std::vector<Rectf> m_viewports;	
std::vector<Rectf> m_scissors;	
std::deque<Mat4f> m_worldviews;	// カメラ行列 world -> view 
Mat4f m_viewscreen; //プロジェクション行列 (view -> screen)
uint32_t m_viewportIndex = 0;
~~~
   - 主なメンバ関数：
~~~
takeover() // worldview, viewscreen を引き継ぐ
shiftViewscreen() // points が透視変換後に -1 < x,y < +1 に収まるように viewscreen を調整
adjustDepth() //points が透視変換後に -1 < z < +1 に収まるように viewscreen を調整
adjustAspect(0) // viewport にあわせて viewscreen の aspect を調整
adjustCamera() // 向き dir, 上方向 up の条件で range が透視変換後に (-1 < x,y < +1) 収まるように worldview を調整
~~~

