// TestInterface.h
#ifndef TestInterface_h
#define TestInterface_h

#ifdef __cplusplus
extern "C" {
#endif

// 初期化
void* exampleInitialize(void);
// 終了
void exampleTerminate(void* instance);
// リソース作成
void exampleCreateResources(void* instance);
// リソース破棄
void exampleDestroyResources(void* instance);
// 描画
void exampleRender(void* instance, uint32_t framebuffer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TestInterface_h
