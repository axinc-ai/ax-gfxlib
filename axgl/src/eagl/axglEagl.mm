// axglEagl.mm
#include <Foundation/Foundation.h>

// class prototype
@class CAMetalLayer;

// EAGLヘッダを除外するためのdefine宣言
#define _EAGL_H_ 1
#define _EAGL_DRAWABLE_H_ 1

#define USE_EAGL_MANGLE 1
#if defined(USE_EAGL_MANGLE)
#include <axgl/EAGL_mangle.h>
#endif

#ifdef __cplusplus
#define EAGL_EXTERN extern "C" __attribute__((visibility ("default")))
#else
#define EAGL_EXTERN extern __attribute__((visibility ("default")))
#endif
#define EAGL_EXTERN_CLASS __attribute__((visibility ("default")))

// version
#define EAGL_MAJOR_VERSION 1
#define EAGL_MINOR_VERSION 0

// EAGL enumerated values
typedef NS_ENUM(NSUInteger, EAGLRenderingAPI)
{
	kEAGLRenderingAPIOpenGLES1 = 1,
	kEAGLRenderingAPIOpenGLES2 = 2,
	kEAGLRenderingAPIOpenGLES3 = 3
};

// EAGLFunctions
EAGL_EXTERN void EAGLGetVersion(unsigned int* major, unsigned int* minor);

// EAGL Sharegroup --------
EAGL_EXTERN_CLASS
@interface EAGLSharegroup : NSObject
{
@package
	struct _EAGLSharegroupPrivate* _private;
}

@property (copy, nonatomic) NSString* debugLabel;
@end // EAGLSharegroup

// EAGL Context --------
EAGL_EXTERN_CLASS
@interface EAGLContext : NSObject
{
@package
	struct _EAGLContextPrivate* _private;
}

- (instancetype) init NS_UNAVAILABLE;
- (instancetype) initWithAPI:(EAGLRenderingAPI) api;
- (instancetype) initWithAPI:(EAGLRenderingAPI) api sharegroup:(EAGLSharegroup*) sharegroup NS_DESIGNATED_INITIALIZER;

+ (BOOL) setCurrentContext:(EAGLContext*) context;
+ (EAGLContext*) currentContext;

@property (readwrite) EAGLRenderingAPI API;
@property (readwrite) EAGLSharegroup* sharegroup;
@property (copy, nonatomic) NSString* debugLabel;
@property (getter=isMultiThreaded, nonatomic) BOOL multiThreaded;

- (struct _EAGLContextPrivate*) getContextPrivate;
- (void) dealloc;
@end // EAGLContext

//---------------------------------------------
// internal declaration of EAGLDrawable.h
// EAGLDrawable interface
@protocol EAGLDrawable
@property(copy) NSDictionary* drawableProperties;
@end

// extends EAGLContext interface
@interface EAGLContext (EAGLContextDrawableAdditions)
- (BOOL)renderbufferStorage:(NSUInteger)target fromDrawable:(id<EAGLDrawable>)drawable;
- (BOOL)presentRenderbuffer:(NSUInteger)target;
- (BOOL)renderbufferStorageFromLayer:(NSUInteger)target fromLayer:(CAMetalLayer*)layer;
@end
//---------------------------------------------
// implementations
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>
#include <QuartzCore/CAMetalLayer.h>

#include "../axglApi.h"
#include "../core/CoreContext.h"
#include "../core/CoreRenderbuffer.h"
#include "../core/CoreObjectsManager.h"
#include "../backend/metal/ContextMetal.h"
#include "../backend/metal/RenderbufferMetal.h"

enum {
	AXGL_EAGL_CONTEXT_KEY = 1
};

// functions --------
void EAGLGetVersion(unsigned int* major, unsigned int* minor)
{
	if (major != nullptr) {
		*major = EAGL_MAJOR_VERSION;
	}
	if (minor != nullptr) {
		*minor = EAGL_MINOR_VERSION;
	}
	return;
}

// EAGLSharegroup --------
@interface EAGLSharegroup()
- (axgl::AXGLObjectsManager)getPrivate;
- (void)setPrivate:(axgl::AXGLObjectsManager) shared;
@end

@implementation EAGLSharegroup
- (instancetype) init
{
	self = [super init];
	if (self != nil) {
		_private = nullptr;
	}
	return self;
}

- (void) dealloc
{
	if (_private != nullptr) {
		((axgl::AXGLObjectsManager)_private)->release(nullptr);
		_private = nullptr;
	}
	return;
}

- (axgl::AXGLObjectsManager)getPrivate
{
	return (axgl::AXGLObjectsManager)_private;
}

- (void)setPrivate:(axgl::AXGLObjectsManager) shared
{
	shared->addRef();
	if (_private != nullptr) {
		((axgl::AXGLObjectsManager)_private)->release(nullptr);
	}
	_private = (struct _EAGLSharegroupPrivate*)shared;
	return;
}
@end // EAGLSharegroup

// EAGLContext --------
@implementation EAGLContext

- (instancetype) init
{
	self = [self initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:nil];
	if (self != nil) {
		// NOTE: 必要なら追加の初期化を行う
	}
	return self;
}

- (instancetype) initWithAPI:(EAGLRenderingAPI) api
{
	if (api != kEAGLRenderingAPIOpenGLES3) {
		return nil; // not supported
	}
	self = [self initWithAPI:api sharegroup:nil];
	if (self != nil) {
		// NOTE: 必要なら追加の初期化を行う
	}
	return self;
}

- (instancetype) initWithAPI:(EAGLRenderingAPI) api sharegroup:(EAGLSharegroup *)sharegroup
{
	if ((api != kEAGLRenderingAPIOpenGLES3) && (api != kEAGLRenderingAPIOpenGLES2)) {
		return nil; // not supported
	}
	self = [super init];
	if (self != nil) {
		_private = nullptr;
		// ObjectsManagerを取得
		axgl::AXGLObjectsManager shared_objects_manager = nullptr;
		if (sharegroup != nil) {
			shared_objects_manager = [sharegroup getPrivate];
		}
		// Contextを作成
		axgl::AXGLContext axgl_context = axgl::createContext(shared_objects_manager);
		if (axgl_context != nullptr) {
			_private = reinterpret_cast<struct _EAGLContextPrivate*>(axgl_context);
		}
		self.API = api;
		if (sharegroup == nil) {
			// ContextからObjectsManagerを取得
			axgl::AXGLObjectsManager objects_manager = axgl::getObjectsManager(axgl_context);
			// Sharegroupを作成してObjectsManagerを設定
			self.sharegroup = [[EAGLSharegroup alloc] init];
			[self.sharegroup setPrivate:objects_manager];
		} else {
			// Sharegroupを保持
			self.sharegroup = sharegroup;
		}
		self.multiThreaded = NO;
	}
	return self;
}

- (struct _EAGLContextPrivate*) getContextPrivate
{
	return _private;
}

+ (BOOL) setCurrentContext:(EAGLContext *)context
{
	// AXGLのカレントコンテキストを設定
	axgl::AXGLContext axgl_context = nullptr;
	if (context != nil) {
		struct _EAGLContextPrivate* ctx = [context getContextPrivate];
		axgl_context = reinterpret_cast<axgl::AXGLContext>(ctx);
	}
	axgl::setCurrentContext(axgl_context);
	// EAGLContextをThreadDictionaryに保持
	NSMutableDictionary* local_storage = [[NSThread currentThread] threadDictionary];
	if (local_storage != nil) {
		if (context != nil) {
			// set object
			[local_storage setObject:context forKey:@(AXGL_EAGL_CONTEXT_KEY)];
		} else {
			// remove
			[local_storage removeObjectForKey:@(AXGL_EAGL_CONTEXT_KEY)];
		}
	} else {
		return NO;
	}
	return YES;
}

+ (EAGLContext*) currentContext
{
	NSMutableDictionary* local_storage = [[NSThread currentThread] threadDictionary];
	EAGLContext* eagl_context = nil;
	if (local_storage != nil) {
		eagl_context = [local_storage objectForKey:@(AXGL_EAGL_CONTEXT_KEY)];
	}
	return eagl_context;
}

- (void) dealloc
{
	if (_private != nullptr) {
		self.sharegroup = nil;
		axgl::AXGLContext axgl_context = reinterpret_cast<axgl::AXGLContext>(_private);
		axgl::destroyContext(axgl_context);
		_private = nullptr;
	}
	return;
}

- (BOOL)renderbufferStorage:(NSUInteger)target fromDrawable:(id<EAGLDrawable>)drawable
{
	AXGL_DBGOUT("renderbufferStorage is not supported.¥n");
	return YES;
}

- (BOOL)presentRenderbuffer:(NSUInteger)target
{
	// カレントコンテキストを取得
	axgl::AXGLContext axgl_context = axgl::getCurrentContext();
	if (axgl_context == nullptr) {
		return NO;
	}
	// RBOを取得
	axgl::CoreRenderbuffer* core_renderbuffer = axgl_context->getCurrentRenderbuffer();
	if (core_renderbuffer == nullptr) {
		return NO;
	}
	// バックエンドのRenderbufferを取得
	axgl::ContextMetal* context_metal = static_cast<axgl::ContextMetal*>(axgl_context->getBackendContext());
	axgl::RenderbufferMetal* renderbuffer_metal = static_cast<axgl::RenderbufferMetal*>(core_renderbuffer->getBackendRenderbuffer());
	if ((context_metal == nullptr) || (renderbuffer_metal == nullptr)) {
		return NO;
	}
	// Renderbufferの表示を実行
	bool result = context_metal->presentRenderbuffer(renderbuffer_metal);
	if (!result) {
		AXGL_DBGOUT("presentRenderbuffer failed¥n");
	}
	return (result) ? YES : NO;
}

- (BOOL)renderbufferStorageFromLayer:(NSUInteger)target fromLayer:(CAMetalLayer *)layer
{
	// カレントコンテキストを取得
	axgl::AXGLContext axgl_context = axgl::getCurrentContext();
	if (axgl_context == nullptr) {
		return NO;
	}
	// RBOを取得
	axgl::CoreRenderbuffer* core_renderbuffer = axgl_context->getCurrentRenderbuffer();
	if (core_renderbuffer == nullptr) {
		return NO;
	}
	// バックエンドのRenderbufferを取得
	axgl::ContextMetal* context_metal = static_cast<axgl::ContextMetal*>(axgl_context->getBackendContext());
	axgl::RenderbufferMetal* renderbuffer_metal = static_cast<axgl::RenderbufferMetal*>(core_renderbuffer->getBackendRenderbuffer());
	if ((context_metal == nullptr) || (renderbuffer_metal == nullptr)) {
		return NO;
	}
	// CAMetalLayerからRenderbufferのストレージを設定
	bool result = renderbuffer_metal->setStorageFromLayer(context_metal, layer);
	if (result) {
		// BackendからRenderbufferの情報を取得させて更新する
		core_renderbuffer->updateStorageInformationFromBackend();
		axgl_context->setupInitialViewport(core_renderbuffer);
	} else {
		AXGL_DBGOUT("setStorageFromLayer failed¥n");
	}
	return (result) ? YES : NO;
}

@end // EAGLContext
