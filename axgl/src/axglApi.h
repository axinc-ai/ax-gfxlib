// axglApi.h
#ifndef __axglApi_h_
#define __axglApi_h_

namespace axgl {

// コンテキスト
using AXGLContext = class CoreContext*;
// オブジェクトマネージャ
using AXGLObjectsManager = class CoreObjectsManager*;

// コンテキストを作成
AXGLContext createContext(AXGLObjectsManager sharedObjectsManager);
// コンテキストを削除
void destroyContext(AXGLContext context);
// カレントコンテキストを設定
void setCurrentContext(AXGLContext context);
// カレントコンテキストを取得
AXGLContext getCurrentContext();
// オブジェクトマネージャをコンテキストから取得
AXGLObjectsManager getObjectsManager(AXGLContext context);

} // namespace axgl

#endif // __axglApi_h_
