# ax Rendering Library

既存グラフィックスライブラリ上にOpenGLとして機能するレイヤーを構築するソフトウェアライブラリです。
現在はiOSのMetalバックエンドをサポートしています。

## iOSのMetalバックエンド

### 開発環境

ハードウェア: MacBook Air (M1,2020)
OS: macOS Monterey Version 12.6.2
Xcode: Xcode Version 13.3

### ビルド

Xcodeから下記のプロジェクトを開きます。
AXGLExample/AXGLExample.xcodeproj

ターゲットを選択してProduct->Buildにてビルドを行います。

### サンプルプログラム

Metal上でGLESを動作させることから、下記のような実装となっています。

UIViewを継承したクラスのlayerClassにおいてCAMetalLayerを返します。
```
+ (id)layerClass
{
  return [CAMetalLayer class];
}
```

初期化時にMetalのデフォルトデバイスを作成し、CAMetalLayerにデバイスとピクセルフォーマットを指定します。
```
- (void)initialize
{
	// デフォルトデバイスを作成して、MetalのLayerを設定
	_metalLayer = (CAMetalLayer*)[self layer];
	_device = MTLCreateSystemDefaultDevice();
	_metalLayer.device = _device;
	_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	_metalLayer.contentsScale = [UIScreen mainScreen].scale;
	// 以下、省略
}
```

MetalのDrawableはGLESと異なるため、CAMetalLayerをストレージに指定してRenderbufferを作成することによって、オンスクリーンのFramebufferを構築します。
※Apple GLESの場合は、CAEAGLLayerを指定したrenderbufferStorageメソッドで実行します。
```
- (void)setupOnscreenResources
{
	// コンテキストを設定
	BOOL res = [EAGLContext setCurrentContext:_context];
	if (res) {
		// CAMetalLayerをStorageに指定してRenderbufferを作成
		glGenRenderbuffers(1, &_colorRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
		[_context renderbufferStorageFromLayer:GL_RENDERBUFFER fromLayer:_metalLayer];
		// 深度のRenderbufferを作成
		GLsizei rb_width = (GLsizei)([_metalLayer drawableSize].width);
		GLsizei rb_height = (GLsizei)([_metalLayer drawableSize].height);
		glGenRenderbuffers(1, &_depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, rb_width, rb_height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		// Framebufferにアタッチする
		glGenFramebuffers(1, &_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// コンテキストを外しておく
		[EAGLContext setCurrentContext:nil];
	}
}
```

## 留意点

本ソフトウェアは glslang と SPIRV-Cross を使用しています。
ライセンスについては下記ディレクトリを参照してください。

glslang:
axgl/external/glslang

SPIRV-Cross:
axgl/external/SPIRV-Cross

