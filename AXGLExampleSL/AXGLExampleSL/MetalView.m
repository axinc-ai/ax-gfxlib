// MetalView.m
// NOTE: Include first to avoid multiple include with Apple's <EAGL.h>
#include <axgl/EAGL.h>
#include <axgl/EAGLDrawable.h>
#include <axgl/ES3/gl.h>
#import "MetalView.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include "ExampleInterface.h"

@interface MetalView()
{
	void* _exampleInstance;
	GLuint _colorRenderbuffer;
	GLuint _depthRenderbuffer;
	GLuint _framebuffer;
}
@property (nonatomic, strong) CADisplayLink *displayLink;
@property (nonatomic, weak) CAMetalLayer *metalLayer;
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) EAGLContext* context;
@end // MetalView()

@implementation MetalView

+ (id)layerClass
{
	return [CAMetalLayer class];
}

- (void)createInitialResources
{
	// Create EAGLContext (AXGL version)
	EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES3;
	_context = [[EAGLContext alloc] initWithAPI:api];
	if (_context == NULL) {
		NSLog(@"initWithAPI FAILED");
		exit(1);
	}
	if (![EAGLContext setCurrentContext:_context]) {
		_context = nil;
		NSLog(@"setCurrentContext FAILED");
		exit(1);
	}
	
	BOOL res = [EAGLContext setCurrentContext:_context];
	if (res) {
		exampleCreateResources(_exampleInstance);
		[EAGLContext setCurrentContext:nil];
	}
}

- (void)initialize
{
	// Create a default device and set to CAMetalLayer
	_metalLayer = (CAMetalLayer*)[self layer];
	_device = MTLCreateSystemDefaultDevice();
	_metalLayer.device = _device;
	_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	_metalLayer.contentsScale = [UIScreen mainScreen].scale;

	_exampleInstance = exampleInitialize();
	[self createInitialResources];
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder
{
	if ((self = [super initWithCoder:aDecoder]))
	{
		[self initialize];
	}
	
	return self;
}

- (void)setupOnscreenResources
{
	// Set the current context
	BOOL res = [EAGLContext setCurrentContext:_context];
	if (res) {
		// Create a renderbuffer by specifying CAMetalLayer as a storage
		glGenRenderbuffers(1, &_colorRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
		[_context renderbufferStorageFromLayer:GL_RENDERBUFFER fromLayer:_metalLayer];
		// Create a depth renderbuffer
		GLsizei rb_width = (GLsizei)([_metalLayer drawableSize].width);
		GLsizei rb_height = (GLsizei)([_metalLayer drawableSize].height);
		glGenRenderbuffers(1, &_depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, rb_width, rb_height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		// Attach renderbuffers to the framebuffer
		glGenFramebuffers(1, &_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// Release the current context
		[EAGLContext setCurrentContext:nil];
	}
}

- (void)destroyOnscreenResources
{
	BOOL res = [EAGLContext setCurrentContext:_context];
	if (res) {
		if (_colorRenderbuffer != 0) {
			glDeleteRenderbuffers(1, &_colorRenderbuffer);
			_colorRenderbuffer = 0;
		}
		if (_depthRenderbuffer != 0) {
			glDeleteRenderbuffers(1, &_depthRenderbuffer);
			_depthRenderbuffer = 0;
		}
		if (_framebuffer != 0) {
			glDeleteFramebuffers(1, &_framebuffer);
			_framebuffer = 0;
		}
		[EAGLContext setCurrentContext:nil];
	}
}

- (void)didMoveToSuperview
{
	[super didMoveToSuperview];
	if (self.superview != nil) {
		self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayLinkDidFire:)];
		[self.displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
		[self setupOnscreenResources];
	} else {
		[self destroyOnscreenResources];
		[self.displayLink invalidate];
		self.displayLink = nil;
	}
}

- (void)render
{
	BOOL res = [EAGLContext setCurrentContext:_context];
	if (res) {
		exampleRender(_exampleInstance, _framebuffer);
		// NOTE: Call glBindRenderbuffer() if CAMetalLayer is not the current renderbuffer
		glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
		BOOL present_res = [_context presentRenderbuffer:GL_RENDERBUFFER];
		if (!present_res) {
			NSLog(@"presentRenderbuffer FAILED");
		}
	}
}

- (void)displayLinkDidFire:(CADisplayLink *)displayLink
{
	[self render];
}

@end
