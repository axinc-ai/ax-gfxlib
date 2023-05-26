// ExampleUtil.h
#ifndef __ExampleUtil_h_
#define __ExampleUtil_h_

#ifdef __cplusplus
extern "C" {
#endif

#define EXAMPLE_UTIL_PI (3.14159265358979f)
void utilIdentityMat4(float* mat);
void utilRotateXMat4(float* mat, float rad);
void utilRotateYMat4(float* mat, float rad);
void utilRotateZMat4(float* mat, float rad);
void utilTranslateMat4(float* mat, float x, float y, float z);
void utilScaleMat4(float* mat, float x, float y, float z);
void utilMultiplyMat4(float* dst, const float* mat_l, const float* mat_r);
// NOTE: Ortho is the projection matrix of GL specification, including Z-axis flip
void utilOrthoMat4(float* mat, float left, float right, float bottom, float top, float znear, float zfar);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __ExampleUtil_h_
