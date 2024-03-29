#ifndef _EAGL_DRAWABLE_H_
#define _EAGL_DRAWABLE_H_

#include <axgl/EAGL.h>

@class CAMetalLayer;

/************************************************************************/
/* Keys for EAGLDrawable drawableProperties dictionary                  */
/*                                                                      */
/* kEAGLDrawablePropertyRetainedBacking:                                */
/*  Type: NSNumber (boolean)                                            */
/*  Legal Values: True/False                                            */
/*  Default Value: False                                                */
/*  Description: True if EAGLDrawable contents are retained after a     */
/*               call to presentRenderbuffer.  False, if they are not   */
/*                                                                      */
/* kEAGLDrawablePropertyColorFormat:                                    */
/*  Type: NSString                                                      */
/*  Legal Values: kEAGLColorFormat*                                     */
/*  Default Value: kEAGLColorFormatRGBA8                                */
/*  Description: Format of pixels in renderbuffer                       */
/************************************************************************/
EAGL_EXTERN NSString * const kEAGLDrawablePropertyRetainedBacking;
EAGL_EXTERN NSString * const kEAGLDrawablePropertyColorFormat;

/************************************************************************/
/* Values for kEAGLDrawablePropertyColorFormat key                      */
/************************************************************************/
EAGL_EXTERN NSString * const kEAGLColorFormatRGBA8;
EAGL_EXTERN NSString * const kEAGLColorFormatRGB565;
EAGL_EXTERN NSString * const kEAGLColorFormatSRGBA8 NS_AVAILABLE_IOS(7_0);

/************************************************************************/
/* EAGLDrawable Interface                                               */
/************************************************************************/
@protocol EAGLDrawable

/* Contains keys from kEAGLDrawableProperty* above */
@property(copy) NSDictionary* drawableProperties;

@end

/* Extends EAGLContext interface */
@interface EAGLContext (EAGLContextDrawableAdditions)

/* Attaches an EAGLDrawable as storage for the OpenGL ES renderbuffer object bound to <target> */
- (BOOL)renderbufferStorage:(NSUInteger)target fromDrawable:(id<EAGLDrawable>)drawable;

/* Request the native window system display the OpenGL ES renderbuffer bound to <target> */
- (BOOL)presentRenderbuffer:(NSUInteger)target;

/* TODO: description */
- (BOOL)renderbufferStorageFromLayer:(NSUInteger)target fromLayer:(CAMetalLayer*)layer;

@end /* EAGLDrawable protocol */


#endif /* _EAGL_DRAWABLE_H_ */
