// ExampleUtil.cpp
#include "ExampleUtil.h"
#include <math.h>
#include <string.h> // memcpy

void utilIdentityMat4(float* mat)
{
	static const float c_identity[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	memcpy(mat, c_identity, sizeof(c_identity));
	return;
}

void utilRotateXMat4(float* mat, float rad)
{
	const float sv = sinf(rad);
	const float cv = cosf(rad);
	utilIdentityMat4(mat);
	mat[5] = cv;
	mat[6] = sv;
	mat[9] = -sv;
	mat[10] = cv;
	return;
}

void utilRotateYMat4(float* mat, float rad)
{
	const float sv = sinf(rad);
	const float cv = cosf(rad);
	utilIdentityMat4(mat);
	mat[0] = cv;
	mat[2] = -sv;
	mat[8] = sv;
	mat[10] = cv;
	return;
}

void utilRotateZMat4(float* mat, float rad)
{
	const float sv = sinf(rad);
	const float cv = cosf(rad);
	utilIdentityMat4(mat);
	mat[0] = cv;
	mat[1] = sv;
	mat[4] = -sv;
	mat[5] = cv;
	return;
}

void utilTranslateMat4(float* mat, float x, float y, float z)
{
	utilIdentityMat4(mat);
	mat[12] = x;
	mat[13] = y;
	mat[14] = z;
	return;
}

void utilScaleMat4(float* mat, float x, float y, float z)
{
	utilIdentityMat4(mat);
	mat[0] = x;
	mat[5] = y;
	mat[10] = z;
	return;
}

void utilMultiplyMat4(float* mat, const float* mat1, const float* mat2)
{
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			*mat++ = (mat1[j] * mat2[4 * i]) + (mat1[4 + j] * mat2[4 * i + 1])
				+ (mat1[8 + j] * mat2[4 * i + 2]) + (mat1[12 + j] * mat2[4 * i + 3]);
		}
	}
	return;
}

void utilOrthoMat4(float* mat, float left, float right, float bottom, float top, float znear, float zfar)
{
	utilIdentityMat4(mat);
	float tmp = 1.0f / (right - left);
	mat[0] = tmp * 2.0f;
	mat[12] = -tmp * (right + left);
	tmp = 1.0f / (top - bottom);
	mat[5] = tmp * 2.0f;
	mat[13] = -tmp * (top + bottom);
	tmp = 1.0f / (zfar - znear);
	mat[10] = tmp * -2.0f;
	mat[14] = -tmp * (zfar + znear);
	return;
}
