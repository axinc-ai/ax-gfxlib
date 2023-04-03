// CoreShader.cpp
// Shaderクラスの実装
#include "CoreShader.h"
#include "CoreContext.h"
#include "../backend/BackendShader.h"
#include <string.h>

namespace axgl {

CoreShader::CoreShader(GLenum type) :
	m_type(type)
{
	m_objectType = TYPE_SHADER;
}

CoreShader::~CoreShader()
{
}

void CoreShader::compile(CoreContext* context)
{
	if ((context == nullptr) || (m_pBackendShader == nullptr)) {
		return;
	}
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	bool result = m_pBackendShader->compileSource(backend_context, m_source.c_str());
	if (!result) {
		m_compileStatus = GL_FALSE;
	} else {
		m_compileStatus = GL_TRUE;
	}
	return;
}

void CoreShader::setBinary(GLenum binaryFormat, const void* binary, GLsizei length, int index)
{
	// TODO: サポートするならバイナリデータのコピーが必要
	AXGL_UNUSED(binaryFormat);
	AXGL_UNUSED(binary);
	AXGL_UNUSED(length);
	AXGL_UNUSED(index);
	return;
}

void CoreShader::setSource(GLsizei count, const GLchar* const* string, const GLint* length)
{
	AXGL_ASSERT(count < 32);
	int len[32] = { 0 };
	// 文字列の長さを算出する
	size_t source_length = 0;
	if (length != nullptr) {
		// length指定あり
		for (int i = 0; i < count; i++) {
			if (length[i] != 0) {
				len[i] = length[i];
			} else {
				// length指定かつ 0 が指定
				len[i] = static_cast<int>(strlen(string[i]));
			}
			source_length += len[i];
		}
	} else {
		// length指定なし
		for (int i = 0; i < count; i++) {
			len[i] = static_cast<int>(strlen(string[i]));
			source_length += len[i];
		}
	}
	// ソース文字列のメモリを確保
	m_source.reserve(source_length + 1);
	// 各ソースを連結コピー
	for (int i = 0; i < count; i++) {
		m_source.append(string[i], len[i]);
	}
	return;
}

void CoreShader::getShaderiv(GLenum pname, GLint* params)
{
	AXGL_ASSERT(params != nullptr);
	switch (pname) {
	case GL_SHADER_TYPE:
		*params = m_type;
		break;
	case GL_DELETE_STATUS:
		*params = m_deleteStatus;
		break;
	case GL_COMPILE_STATUS:
		*params = m_compileStatus;
		break;
	case GL_INFO_LOG_LENGTH:
		*params = static_cast<GLint>(m_infoLog.length());
		break;
	case GL_SHADER_SOURCE_LENGTH:
		*params = static_cast<GLint>(m_source.length());
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreShader::getShaderInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
#if defined(AXGL_DEBUG)
	// デバッグ時は情報を出力
	GLsizei copy_len = static_cast<GLsizei>(m_infoLog.length());
	if ((bufSize - 1) < copy_len) {
		copy_len = bufSize - 1;
	}
	strncpy(infoLog, m_infoLog.c_str(), copy_len);

	if (length != nullptr) {
		*length = copy_len;
	}
#else
	AXGL_UNUSED(bufSize);
	AXGL_UNUSED(infoLog);
	// リリース時は情報出力なし
	if (length != nullptr) {
		*length = 0;
	}
#endif
	return;
}

void CoreShader::getShaderSource(GLsizei bufSize, GLsizei* length, GLchar* source)
{
	// NOTE: 通常は使用しない
	AXGL_UNUSED(bufSize);
	AXGL_UNUSED(length);
	AXGL_UNUSED(source);
	return;
}

void CoreShader::setDeleteStatus()
{
	m_deleteStatus = GL_TRUE;
	return;
}

bool CoreShader::initialize(CoreContext* context)
{
	AXGL_ASSERT(m_pBackendShader == nullptr);
	m_pBackendShader = BackendShader::create();
	if (m_pBackendShader == nullptr) {
		return false;
	}
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	return m_pBackendShader->initialize(backend_context, m_type);
}

void CoreShader::terminate(CoreContext* context)
{
	if (m_pBackendShader != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		m_pBackendShader->terminate(backend_context);

		BackendShader::destroy(m_pBackendShader);
		m_pBackendShader = nullptr;
	}
	return;
}

} // namespace axgl
