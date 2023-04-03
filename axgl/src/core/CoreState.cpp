// CoreState.cpp
// Stateクラスの実装
#include "CoreState.h"
#include "CoreBuffer.h"
#include "CoreTexture.h"
#include "CoreFramebuffer.h"
#include "CoreRenderbuffer.h"
#include "CoreVertexArray.h"
#include "CoreSampler.h"
#include "CoreTransformFeedback.h"
#include "CoreProgram.h"
#include "CoreQuery.h"

namespace axgl {

CoreState::CoreState()
{
	// GL_NONE : 0
	memset(m_drawBuffer, 0, sizeof(m_drawBuffer));
}

CoreState::~CoreState()
{
}

void CoreState::releaseResources(CoreContext* context)
{
	if (m_pArrayBuffer != nullptr) {
		m_pArrayBuffer->release(context);
		m_pArrayBuffer = nullptr;
	}
	if (m_pCopyReadBuffer != nullptr) {
		m_pCopyReadBuffer->release(context);
		m_pCopyReadBuffer = nullptr;
	}
	if (m_pCopyWriteBuffer != nullptr) {
		m_pCopyWriteBuffer->release(context);
		m_pCopyWriteBuffer = nullptr;
	}
	if (m_drawParameters.indexBuffer != nullptr) {
		m_drawParameters.indexBuffer->release(context);
		m_drawParameters.indexBuffer = nullptr;
	}
	if (m_pPixelPackBuffer != nullptr) {
		m_pPixelPackBuffer->release(context);
		m_pPixelPackBuffer = nullptr;
	}
	if (m_pPixelUnpackBuffer != nullptr) {
		m_pPixelUnpackBuffer->release(context);
		m_pPixelUnpackBuffer = nullptr;
	}
	if (m_pTransformFeedbackBuffer != nullptr) {
		m_pTransformFeedbackBuffer->release(context);
		m_pTransformFeedbackBuffer = nullptr;
	}
	if (m_pUniformBuffer != nullptr) {
		m_pUniformBuffer->release(context);
		m_pUniformBuffer = nullptr;
	}
	for (int32_t i = 0; i < AXGL_MAX_VERTEX_ATTRIBS; i++) {
		if (m_drawParameters.vertexBuffer[i] != nullptr) {
			m_drawParameters.vertexBuffer[i]->release(context);
			m_drawParameters.vertexBuffer[i] = nullptr;
		}
	}
	for (int32_t i = 0; i < AXGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS; i++) {
		if (m_indexedTransformFeedbackBuffer[i].buffer != nullptr) {
			m_indexedTransformFeedbackBuffer[i].buffer->release(context);
			m_indexedTransformFeedbackBuffer[i].buffer = nullptr;
		}
	}
	for (int32_t i = 0; i < AXGL_MAX_UNIFORM_BUFFER_BINDINGS; i++) {
		if (m_drawParameters.uniformBuffer[i].buffer != nullptr) {
			m_drawParameters.uniformBuffer[i].buffer->release(context);
			m_drawParameters.uniformBuffer[i].buffer = nullptr;
		}
	}
	// release binding textures
	for (int32_t i = 0; i < AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++) {
		if (m_drawParameters.texture2d[i] != nullptr) {
			m_drawParameters.texture2d[i]->release(context);
			m_drawParameters.texture2d[i] = nullptr;
		}
		if (m_drawParameters.texture3d[i] != nullptr) {
			m_drawParameters.texture3d[i]->release(context);
			m_drawParameters.texture3d[i] = nullptr;
		}
		if (m_drawParameters.textureCube[i] != nullptr) {
			m_drawParameters.textureCube[i]->release(context);
			m_drawParameters.textureCube[i] = nullptr;
		}
		if (m_drawParameters.texture2dArray[i] != nullptr) {
			m_drawParameters.texture2dArray[i]->release(context);
			m_drawParameters.texture2dArray[i] = nullptr;
		}
	}

	return;
}

void CoreState::setBuffer(CoreContext* context, GLenum target, CoreBuffer* buffer)
{
	switch (target) {
	case GL_ARRAY_BUFFER:
		setCoreBuffer(context, &m_pArrayBuffer, buffer);
		break;
	case GL_COPY_READ_BUFFER:
		setCoreBuffer(context, &m_pCopyReadBuffer, buffer);
		break;
	case GL_COPY_WRITE_BUFFER:
		setCoreBuffer(context, &m_pCopyReadBuffer, buffer);
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		setCoreBuffer(context, &(m_drawParameters.indexBuffer), buffer);
		break;
	case GL_PIXEL_PACK_BUFFER:
		setCoreBuffer(context, &m_pPixelPackBuffer, buffer);
		break;
	case GL_PIXEL_UNPACK_BUFFER:
		setCoreBuffer(context, &m_pPixelUnpackBuffer, buffer);
		break;
	case GL_TRANSFORM_FEEDBACK_BUFFER:
		setCoreBuffer(context, &m_pTransformFeedbackBuffer, buffer);
		break;
	case GL_UNIFORM_BUFFER:
		setCoreBuffer(context, &m_pUniformBuffer, buffer);
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

CoreBuffer* CoreState::getBuffer(GLenum target) const
{
	CoreBuffer* buffer_object = nullptr;
	switch (target) {
	case GL_ARRAY_BUFFER:
		buffer_object = m_pArrayBuffer;
		break;
	case GL_COPY_READ_BUFFER:
		buffer_object = m_pCopyReadBuffer;
		break;
	case GL_COPY_WRITE_BUFFER:
		buffer_object = m_pCopyWriteBuffer;
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		buffer_object = m_drawParameters.indexBuffer;
		break;
	case GL_PIXEL_PACK_BUFFER:
		buffer_object = m_pPixelPackBuffer;
		break;
	case GL_PIXEL_UNPACK_BUFFER:
		buffer_object = m_pPixelUnpackBuffer;
		break;
	case GL_TRANSFORM_FEEDBACK_BUFFER:
		buffer_object = m_pTransformFeedbackBuffer;
		break;
	case GL_UNIFORM_BUFFER:
		buffer_object = m_pUniformBuffer;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return buffer_object;
}

void CoreState::setTexture(CoreContext* context, GLenum target, CoreTexture* texture)
{
	int tex_index = m_activeTexture - GL_TEXTURE0;
	if (tex_index < AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
		CoreTexture** dst = nullptr;
		switch (target) {
		case GL_TEXTURE_2D:
			dst = &m_drawParameters.texture2d[tex_index];
			break;
		case GL_TEXTURE_3D:
			dst = &m_drawParameters.texture3d[tex_index];
			break;
		case GL_TEXTURE_2D_ARRAY:
			dst = &m_drawParameters.texture2dArray[tex_index];
			break;
		case GL_TEXTURE_CUBE_MAP:
			dst = &m_drawParameters.textureCube[tex_index];
			break;
		default:
			setErrorCode(GL_INVALID_ENUM);
			break;
		}
		if (dst != nullptr) {
			if (texture != nullptr) {
				setCoreTexture(context, dst, texture);
				GLenum cur_target = texture->getTarget();
				if (cur_target == 0) {
					texture->setTarget(target);
				}
			} else {
				// unset texture
				setCoreTexture(context, dst, nullptr);
			}
			m_drawParameters.dirtyFlags |= TEXTURE_BINDING_DIRTY_BIT;
		}
	}
	return;
}

CoreTexture* CoreState::getTexture(GLenum target) const
{
	CoreTexture* texture_object = nullptr;
	int tex_index = m_activeTexture - GL_TEXTURE0;
	if (tex_index < AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
		switch (target) {
		case GL_TEXTURE_2D:
			texture_object = m_drawParameters.texture2d[tex_index];
			break;
		case GL_TEXTURE_3D:
			texture_object = m_drawParameters.texture3d[tex_index];
			break;
		case GL_TEXTURE_2D_ARRAY:
			texture_object = m_drawParameters.texture2dArray[tex_index];
			break;
		case GL_TEXTURE_CUBE_MAP:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			texture_object = m_drawParameters.textureCube[tex_index];
			break;
		default:
			setErrorCode(GL_INVALID_ENUM);
			break;
		}
	}
	return texture_object;
}

void CoreState::setFramebuffer(CoreContext* context, GLenum target, CoreFramebuffer* framebuffer)
{
	switch (target) {
	case GL_FRAMEBUFFER:
		setCoreFramebuffer(context, &m_pReadFramebuffer, framebuffer);
		setCoreFramebuffer(context, &m_pDrawFramebuffer, framebuffer);
		m_drawParameters.framebufferDraw = framebuffer;
		updatePipelineStateAttachments();
		break;
	case GL_READ_FRAMEBUFFER:
		setCoreFramebuffer(context, &m_pReadFramebuffer, framebuffer);
		break;
	case GL_DRAW_FRAMEBUFFER:
		setCoreFramebuffer(context, &m_pDrawFramebuffer, framebuffer);
		m_drawParameters.framebufferDraw = framebuffer;
		updatePipelineStateAttachments();
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

CoreFramebuffer* CoreState::getFramebuffer(GLenum target) const
{
	CoreFramebuffer* core_framebuffer = nullptr;
	switch (target) {
	case GL_FRAMEBUFFER:
		core_framebuffer = m_pDrawFramebuffer;
		break;
	case GL_READ_FRAMEBUFFER:
		core_framebuffer = m_pReadFramebuffer;
		break;
	case GL_DRAW_FRAMEBUFFER:
		core_framebuffer = m_pDrawFramebuffer;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return core_framebuffer;
}

void CoreState::setRenderbuffer(CoreContext* context, GLenum target, CoreRenderbuffer* renderbuffer)
{
	switch (target) {
	case GL_RENDERBUFFER:
		if (renderbuffer != nullptr) {
			renderbuffer->addRef();
		}
		if (m_pRenderbuffer != nullptr) {
			m_pRenderbuffer->release(context);
		}
		m_pRenderbuffer = renderbuffer;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

CoreRenderbuffer* CoreState::getRenderbuffer(GLenum target) const
{
	CoreRenderbuffer* core_renderbuffer = nullptr;
	switch (target) {
	case GL_RENDERBUFFER:
		core_renderbuffer = m_pRenderbuffer;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return core_renderbuffer;
}

void CoreState::setVertexArray(CoreContext* context, CoreVertexArray* vertexArray)
{
	if (vertexArray != nullptr) {
		vertexArray->addRef();
	}
	if (m_drawParameters.vertexArray != nullptr) {
		m_drawParameters.vertexArray->release(context);
	}
	m_drawParameters.vertexArray = vertexArray;
	BackendVertexArray* backend_vertex_array = nullptr;
	if (vertexArray != nullptr) {
		backend_vertex_array = vertexArray->getBackendVertexArray();
	}
	m_drawParameters.renderPipelineState.vertexArray = backend_vertex_array;
	m_drawParameters.dirtyFlags |= VERTEX_ARRAY_BINDING_DIRTY_BIT;
	return;
}

CoreVertexArray* CoreState::getVertexArray() const
{
	return m_drawParameters.vertexArray;
}

void CoreState::setSampler(CoreContext* context, GLuint unit, CoreSampler* sampler)
{
	if (unit >= AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
		setErrorCode(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		return;
	}
	setCoreSampler(context, &m_drawParameters.samplers[unit], sampler);
	m_drawParameters.dirtyFlags |= SAMPLER_BINDING_DIRTY_BIT;
	return;
}

CoreSampler* CoreState::getSampler(GLuint unit) const
{
	if (unit >= AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
		setErrorCode(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		return nullptr;
	}
	return m_drawParameters.samplers[unit];
}

void CoreState::setTransformFeedback(CoreContext* context, GLenum target, CoreTransformFeedback* transformFeedback)
{
	switch (target) {
	case GL_TRANSFORM_FEEDBACK:
		if (transformFeedback != nullptr) {
			transformFeedback->addRef();
		}
		if (m_pTransformFeedback != nullptr) {
			m_pTransformFeedback->release(context);
		}
		m_pTransformFeedback = transformFeedback;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

CoreTransformFeedback* CoreState::getTransformFeedback(GLenum target) const
{
	CoreTransformFeedback* core_transform_feedback = nullptr;
	switch (target) {
	case GL_TRANSFORM_FEEDBACK:
		core_transform_feedback = m_pTransformFeedback;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return core_transform_feedback;
}

void CoreState::setProgram(CoreContext* context, CoreProgram* program)
{
	if (program != nullptr) {
		program->addRef();
	}
	if (m_drawParameters.program != nullptr) {
		m_drawParameters.program->release(context);
	}
	m_drawParameters.program = program;
	BackendProgram* backend_program = nullptr;
	if (program != nullptr) {
		backend_program = program->getBackendProgram();
	}
	m_drawParameters.renderPipelineState.program = backend_program;
	m_drawParameters.dirtyFlags |= CURRENT_PROGRAM_DIRTY_BIT;
	return;
}

CoreProgram* CoreState::getProgram() const
{
	return m_drawParameters.program;
}

void CoreState::setCurrentQuery(CoreContext* context, GLenum target, CoreQuery* query)
{
	switch (target) {
	case GL_ANY_SAMPLES_PASSED:
		setCoreQuery(context, &m_drawParameters.anySamplesPassedQuery, query);
		m_drawParameters.dirtyFlags |= ANY_SAMPLES_PASSED_QUERY_DIRTY_BIT;
		break;
	case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
		setCoreQuery(context, &m_drawParameters.anySamplesPassedConservativeQuery, query);
		m_drawParameters.dirtyFlags |= ANY_SAMPLES_PASSED_QUERY_DIRTY_BIT;
		break;
	case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
		setCoreQuery(context, &m_pTransformFeedbackPrimitivesWritten, query);
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

CoreQuery* CoreState::getCurrentQuery(GLenum target) const
{
	CoreQuery* core_query = nullptr;
	switch (target) {
	case GL_ANY_SAMPLES_PASSED:
		core_query = m_drawParameters.anySamplesPassedQuery;
		break;
	case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
		core_query = m_drawParameters.anySamplesPassedConservativeQuery;
		break;
	case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
		core_query = m_pTransformFeedbackPrimitivesWritten;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return core_query;
}

void CoreState::setEnableVertexAttrib(GLuint index, GLboolean enable)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->enable = enable;
	return;
}

void CoreState::setVertexAttrib1f(GLuint index, GLfloat x)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = x;
	dst->currentValue[1] = 0.0f;
	dst->currentValue[2] = 0.0f;
	dst->currentValue[3] = 1.0f;
	return;
}

void CoreState::setVertexAttrib1fv(GLuint index, const GLfloat *v)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = v[0];
	dst->currentValue[1] = 0.0f;
	dst->currentValue[2] = 0.0f;
	dst->currentValue[3] = 1.0f;
	return;
}

void CoreState::setVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = x;
	dst->currentValue[1] = y;
	dst->currentValue[2] = 0.0f;
	dst->currentValue[3] = 1.0f;
	return;
}

void CoreState::setVertexAttrib2fv(GLuint index, const GLfloat *v)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = v[0];
	dst->currentValue[1] = v[1];
	dst->currentValue[2] = 0.0f;
	dst->currentValue[3] = 1.0f;
	return;
}

void CoreState::setVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = x;
	dst->currentValue[1] = y;
	dst->currentValue[2] = z;
	dst->currentValue[3] = 1.0f;
	return;
}

void CoreState::setVertexAttrib3fv(GLuint index, const GLfloat *v)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = v[0];
	dst->currentValue[1] = v[1];
	dst->currentValue[2] = v[2];
	dst->currentValue[3] = 1.0f;
	return;
}

void CoreState::setVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = x;
	dst->currentValue[1] = y;
	dst->currentValue[2] = z;
	dst->currentValue[3] = w;
	return;
}

void CoreState::setVertexAttrib4fv(GLuint index, const GLfloat *v)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = v[0];
	dst->currentValue[1] = v[1];
	dst->currentValue[2] = v[2];
	dst->currentValue[3] = v[3];
	return;
}

void CoreState::setVertexAttribPointer(CoreContext* context, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer, CoreBuffer* buffer)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	// vertex attributes parameters
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->size = size;
	dst->type = type;
	dst->normalized = normalized;
	dst->stride = stride;
	dst->pointer = pointer;
	// array buffer
	if (m_drawParameters.vertexBuffer[index] != nullptr) {
		m_drawParameters.vertexBuffer[index]->release(context);
	}
	m_drawParameters.vertexBuffer[index] = buffer;
	if (m_drawParameters.vertexBuffer[index] != nullptr) {
		m_drawParameters.vertexBuffer[index]->addRef();
	}
	return;
}

void CoreState::setVertexAttribIPointer(CoreContext* context, GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer, CoreBuffer* buffer)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	// vertex attributes parameters
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->size = size;
	dst->type = type;
	dst->normalized = GL_FALSE;
	dst->stride = stride;
	dst->pointer = pointer;
	// array buffer
	if (m_drawParameters.vertexBuffer[index] != nullptr) {
		m_drawParameters.vertexBuffer[index]->release(context);
	}
	m_drawParameters.vertexBuffer[index] = buffer;
	if (m_drawParameters.vertexBuffer[index] != nullptr) {
		m_drawParameters.vertexBuffer[index]->addRef();
	}
	return;
}

void CoreState::setVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = static_cast<float>(x);
	dst->currentValue[1] = static_cast<float>(y);
	dst->currentValue[2] = static_cast<float>(z);
	dst->currentValue[3] = static_cast<float>(w);
	return;
}

void CoreState::setVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->currentValue[0] = static_cast<float>(x);
	dst->currentValue[1] = static_cast<float>(y);
	dst->currentValue[2] = static_cast<float>(z);
	dst->currentValue[3] = static_cast<float>(w);
	return;
}

void CoreState::setVertexAttribDivisor(GLuint index, GLuint divisor)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	VertexAttrib* dst = &m_drawParameters.renderPipelineState.vertexAttribs[index];
	dst->divisor = divisor;
	return;
}

void CoreState::getVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const VertexAttrib& src = m_drawParameters.renderPipelineState.vertexAttribs[index];
	AXGL_ASSERT(params != nullptr);
	switch (pname) {
	case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
		{
			const CoreBuffer* buf = m_drawParameters.vertexBuffer[index];
			if (buf != nullptr) {
				*params = static_cast<GLfloat>(buf->getId());
			} else {
				*params = 0.0f;
			}
		}
		break;
	case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
		*params = static_cast<GLfloat>(src.enable);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_SIZE:
		*params = static_cast<GLfloat>(src.size);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
		*params = static_cast<GLfloat>(src.stride);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_TYPE:
		*params = static_cast<GLfloat>(src.type);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
		*params = static_cast<GLfloat>(src.normalized);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
		*params = isIntegerType(src.type) ? static_cast<GLfloat>(GL_TRUE) : static_cast<GLfloat>(GL_FALSE);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
		*params = static_cast<GLfloat>(src.divisor);
		break;
	case GL_CURRENT_VERTEX_ATTRIB:
		params[0] = src.currentValue[0];
		params[1] = src.currentValue[1];
		params[2] = src.currentValue[2];
		params[3] = src.currentValue[3];
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreState::getVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
	getVertexAttribIiv(index, pname, params);
	return;
}

void CoreState::getVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const VertexAttrib& src = m_drawParameters.renderPipelineState.vertexAttribs[index];
	switch (pname) {
	case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
		{
			const CoreBuffer* buf = m_drawParameters.vertexBuffer[index];
			if (buf != nullptr) {
				*params = static_cast<GLint>(buf->getId());
			} else {
				*params = 0;
			}
		}
		break;
	case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
		*params = src.enable;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_SIZE:
		*params = src.size;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
		*params = src.stride;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_TYPE:
		*params = src.type;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
		*params = src.normalized;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
		*params = isIntegerType(src.type) ? GL_TRUE : GL_FALSE;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
		*params = src.divisor;
		break;
	case GL_CURRENT_VERTEX_ATTRIB:
		params[0] = static_cast<GLint>(src.currentValue[0]);
		params[1] = static_cast<GLint>(src.currentValue[1]);
		params[2] = static_cast<GLint>(src.currentValue[2]);
		params[3] = static_cast<GLint>(src.currentValue[3]);
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreState::getVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const VertexAttrib& src = m_drawParameters.renderPipelineState.vertexAttribs[index];
	switch (pname) {
	case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
		{
			const CoreBuffer* buf = m_drawParameters.vertexBuffer[index];
			if (buf != nullptr) {
				*params = buf->getId();
			} else {
				*params = 0;
			}
		}
		break;
	case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
		*params = src.enable;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_SIZE:
		*params = static_cast<GLuint>(src.size);
		break;
	case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
		*params = src.stride;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_TYPE:
		*params = src.type;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
		*params = src.normalized;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
		*params = isIntegerType(src.type) ? GL_TRUE : GL_FALSE;
		break;
	case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
		*params = src.divisor;
		break;
	case GL_CURRENT_VERTEX_ATTRIB:
		params[0] = static_cast<GLuint>(src.currentValue[0]);
		params[1] = static_cast<GLuint>(src.currentValue[1]);
		params[2] = static_cast<GLuint>(src.currentValue[2]);
		params[3] = static_cast<GLuint>(src.currentValue[3]);
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreState::getVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const VertexAttrib& src = m_drawParameters.renderPipelineState.vertexAttribs[index];

	switch (pname) {
	case GL_VERTEX_ATTRIB_ARRAY_POINTER:
		AXGL_ASSERT(pointer != nullptr);
		*pointer = const_cast<void*>(src.pointer);
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreState::setEnable(GLenum cap, GLboolean enable)
{
	switch (cap) {
	case GL_BLEND:
		m_drawParameters.renderPipelineState.blendParams.blendEnable = enable;
		break;
	case GL_CULL_FACE:
		m_drawParameters.cullFaceParams.cullFaceEnable = enable;
		m_drawParameters.dirtyFlags |= CULL_FACE_DIRTY_BIT;
		break;
	case GL_DEPTH_TEST:
		m_drawParameters.depthStencilState.depthTestParams.depthTestEnable = enable;
		break;
	case GL_DITHER:
		m_dither = enable;
		break;
	case GL_POLYGON_OFFSET_FILL:
		m_drawParameters.polygonOffsetParams.polygonOffsetFillEnable = enable;
		m_drawParameters.dirtyFlags |= POLYGON_OFFSET_DIRTY_BIT;
		break;
	case GL_PRIMITIVE_RESTART_FIXED_INDEX:
		m_primitiveRestartFixedIndex = enable;
		break;
	case GL_RASTERIZER_DISCARD:
		m_rasterizerDiscard = enable;
		break;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		m_drawParameters.renderPipelineState.sampleCoverageParams.sampleAlphaToCoverageEnable = enable;
		break;
	case GL_SAMPLE_COVERAGE:
		m_drawParameters.renderPipelineState.sampleCoverageParams.sampleCoverageEnable = enable;
		break;
	case GL_SCISSOR_TEST:
		m_drawParameters.scissorParams.scissorTestEnable = enable;
		m_drawParameters.dirtyFlags |= SCISSOR_DIRTY_BIT;
		break;
	case GL_STENCIL_TEST:
		m_drawParameters.depthStencilState.stencilTestParams.stencilTestEnable = enable;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

GLboolean CoreState::getEnable(GLenum cap)
{
	GLboolean rval = GL_FALSE;
	switch (cap) {
	case GL_BLEND:
		rval = m_drawParameters.renderPipelineState.blendParams.blendEnable;
		break;
	case GL_CULL_FACE:
		rval = m_drawParameters.cullFaceParams.cullFaceEnable;
		break;
	case GL_DEPTH_TEST:
		rval = m_drawParameters.depthStencilState.depthTestParams.depthTestEnable;
		break;
	case GL_DITHER:
		rval = m_dither;
		break;
	case GL_POLYGON_OFFSET_FILL:
		rval = m_drawParameters.polygonOffsetParams.polygonOffsetFillEnable;
		break;
	case GL_PRIMITIVE_RESTART_FIXED_INDEX:
		rval = m_primitiveRestartFixedIndex;
		break;
	case GL_RASTERIZER_DISCARD:
		rval = m_rasterizerDiscard;
		break;
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		rval = m_drawParameters.renderPipelineState.sampleCoverageParams.sampleAlphaToCoverageEnable;
		break;
	case GL_SAMPLE_COVERAGE:
		rval = m_drawParameters.renderPipelineState.sampleCoverageParams.sampleCoverageEnable;
		break;
	case GL_SCISSOR_TEST:
		rval = m_drawParameters.scissorParams.scissorTestEnable;
		break;
	case GL_STENCIL_TEST:
		rval = m_drawParameters.depthStencilState.stencilTestParams.stencilTestEnable;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return rval;
}

void CoreState::setActiveTexture(GLenum texture)
{
	if (texture >= (GL_TEXTURE0 + AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_activeTexture = texture;
	return;
}

GLenum CoreState::getActiveTexture() const
{
	return m_activeTexture;
}

void CoreState::setBlendColor(float red, float green, float blue, float alpha)
{
	float* dst = m_drawParameters.blendColor;
	dst[0] = red;
	dst[1] = green;
	dst[2] = blue;
	dst[3] = alpha;
	return;
}

static inline bool is_acceptable_blend_equation_mode(GLenum mode)
{
	bool result = false;
	switch (mode) {
	case GL_FUNC_ADD:
	case GL_FUNC_SUBTRACT:
	case GL_FUNC_REVERSE_SUBTRACT:
	case GL_MIN:
	case GL_MAX:
		result = true;
		break;
	default:
		break;
	}
	return result;
}

void CoreState::setBlendEquation(GLenum modeRgb, GLenum modeAlpha)
{
	if (!is_acceptable_blend_equation_mode(modeRgb)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_blend_equation_mode(modeAlpha)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_drawParameters.renderPipelineState.blendParams.blendEquation[0] = modeRgb;
	m_drawParameters.renderPipelineState.blendParams.blendEquation[1] = modeAlpha;
	return;
}

static inline bool is_acceptable_blend_func(GLenum func)
{
	bool result = false;
	switch (func) {
	case GL_ZERO:
	case GL_ONE:
	case GL_SRC_COLOR:
	case GL_ONE_MINUS_SRC_COLOR:
	case GL_DST_COLOR:
	case GL_ONE_MINUS_DST_COLOR:
	case GL_SRC_ALPHA:
	case GL_ONE_MINUS_SRC_ALPHA:
	case GL_DST_ALPHA:
	case GL_ONE_MINUS_DST_ALPHA:
	case GL_CONSTANT_COLOR:
	case GL_ONE_MINUS_CONSTANT_COLOR:
	case GL_CONSTANT_ALPHA:
	case GL_ONE_MINUS_CONSTANT_ALPHA:
	case GL_SRC_ALPHA_SATURATE:
		result = true;
		break;
	default:
		break;
	}
	return result;
}

void CoreState::setBlendFunc(GLenum srcRgb, GLenum dstRgb, GLenum srcAlpha, GLenum dstAlpha)
{
	if (!is_acceptable_blend_func(srcRgb)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_blend_func(dstRgb)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_blend_func(srcAlpha)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_blend_func(dstAlpha)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_drawParameters.renderPipelineState.blendParams.blendSrc[0] = srcRgb;
	m_drawParameters.renderPipelineState.blendParams.blendSrc[1] = srcAlpha;
	m_drawParameters.renderPipelineState.blendParams.blendDst[0] = dstRgb;
	m_drawParameters.renderPipelineState.blendParams.blendDst[1] = dstAlpha;
	return;
}

void CoreState::setColorClearValue(float red, float green, float blue, float alpha)
{
	for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
		m_clearParams.colorClearValue[i][0] = red;
		m_clearParams.colorClearValue[i][1] = green;
		m_clearParams.colorClearValue[i][2] = blue;
		m_clearParams.colorClearValue[i][3] = alpha;
	}
	return;
}

void CoreState::setDepthClearValue(float d)
{
	m_clearParams.depthClearValue = d;
	return;
}

void CoreState::setStencilClearValue(GLint s)
{
	m_clearParams.stencilClearValue = s;
	return;
}

void CoreState::setColorWriteMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	GLboolean* dst = m_drawParameters.renderPipelineState.writemaskParams.colorWritemask;
	dst[0] = red;
	dst[1] = green;
	dst[2] = blue;
	dst[3] = alpha;
	return;
}

void CoreState::setCullFaceMode(GLenum mode)
{
	m_drawParameters.cullFaceParams.cullFaceMode = mode;
	m_drawParameters.dirtyFlags |= CULL_FACE_DIRTY_BIT;
	return;
}

static inline bool is_acceptable_depth_func(GLenum func)
{
	bool result = false;
	switch (func) {
	case GL_NEVER:
	case GL_LESS:
	case GL_LEQUAL:
	case GL_GREATER:
	case GL_NOTEQUAL:
	case GL_GEQUAL:
	case GL_ALWAYS:
		result = true;
		break;
	default:
		break;
	}

	return result;
}

void CoreState::setDepthFunc(GLenum func)
{
	if (!is_acceptable_depth_func(func)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_drawParameters.depthStencilState.depthTestParams.depthFunc = func;
	return;
}

void CoreState::setDepthWritemask(GLboolean flag)
{
	m_drawParameters.depthStencilState.writemaskParams.depthWritemask = flag;
	return;
}

void CoreState::setDepthRange(float n, float f)
{
	m_drawParameters.viewportParams.depthRange[0] = n;
	m_drawParameters.viewportParams.depthRange[1] = f;
	m_drawParameters.dirtyFlags |= VIEWPORT_DIRTY_BIT;
	return;
}

static inline bool is_acceptable_front_face(GLenum mode)
{
	bool result = false;
	switch (mode) {
	case GL_CW:
	case GL_CCW:
		result = true;
		break;
	default:
		break;
	}
	return result;
}

void CoreState::setFrontFace(GLenum mode)
{
	if (!is_acceptable_front_face(mode)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_drawParameters.cullFaceParams.frontFace = mode;
	m_drawParameters.dirtyFlags |= CULL_FACE_DIRTY_BIT;
	return;
}

void CoreState::setLineWidth(float width)
{
	m_lineWidth = width;
	return;
}

float CoreState::getLineWidth() const
{
	return m_lineWidth;
}

void CoreState::setPixelStore(GLenum pname, GLint param)
{
	switch (pname) {
	case GL_PACK_ROW_LENGTH:
		m_packParams.packRowLength = param;
		break;
	case GL_PACK_SKIP_ROWS:
		m_packParams.packSkipRows = param;
		break;
	case GL_PACK_SKIP_PIXELS:
		m_packParams.packSkipPixels = param;
		break;
	case GL_PACK_ALIGNMENT:
		m_packParams.packAlignment = param;
		break;
	case GL_UNPACK_ROW_LENGTH:
		m_unpackParams.unpackRowLength = param;
		break;
	case GL_UNPACK_IMAGE_HEIGHT:
		m_unpackParams.unpackImageHeight = param;
		break;
	case GL_UNPACK_SKIP_ROWS:
		m_unpackParams.unpackSkipRows = param;
		break;
	case GL_UNPACK_SKIP_PIXELS:
		m_unpackParams.unpackSkipPixels = param;
		break;
	case GL_UNPACK_SKIP_IMAGES:
		m_unpackParams.unpackSkipImages = param;
		break;
	case GL_UNPACK_ALIGNMENT:
		m_unpackParams.unpackAlignment = param;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreState::setPolygonOffset(float factor, float units)
{
	m_drawParameters.polygonOffsetParams.polygonOffsetFactor = factor;
	m_drawParameters.polygonOffsetParams.polygonOffsetUnits = units;
	m_drawParameters.dirtyFlags |= POLYGON_OFFSET_DIRTY_BIT;
	return;
}

void CoreState::setSampleCoverage(float value, GLboolean invert)
{
	m_drawParameters.renderPipelineState.sampleCoverageParams.sampleCoverageValue = value;
	m_drawParameters.renderPipelineState.sampleCoverageParams.sampleCoverageInvert = invert;
	return;
}

void CoreState::setScissorBox(GLint x, GLint y, GLint width, GLint height)
{
	m_drawParameters.scissorParams.scissorBox[0] = x;
	m_drawParameters.scissorParams.scissorBox[1] = y;
	m_drawParameters.scissorParams.scissorBox[2] = width;
	m_drawParameters.scissorParams.scissorBox[3] = height;
	m_drawParameters.dirtyFlags |= SCISSOR_DIRTY_BIT;
	return;
}

static inline bool is_acceptable_stancil_func(GLenum func)
{
	bool result = false;
	switch (func) {
	case GL_NEVER:
	case GL_LESS:
	case GL_LEQUAL:
	case GL_GREATER:
	case GL_GEQUAL:
	case GL_EQUAL:
	case GL_NOTEQUAL:
	case GL_ALWAYS:
		result = true;
		break;
	default:
		break;
	}
	return result;
}

void CoreState::setStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	if (!is_acceptable_stancil_func(func)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	StencilTestParams* dst = &m_drawParameters.depthStencilState.stencilTestParams;
	dst->stencilFunc = func;
	dst->stencilValueMask = mask;
	dst->stencilBackFunc = func;
	dst->stencilBackValueMask = mask;
	StencilReference* st_ref = &m_drawParameters.stencilReference;
	st_ref->stencilRef = ref;
	st_ref->stencilBackRef = ref;
	m_drawParameters.dirtyFlags |= STENCIL_REFERENCE_DIRTY_BIT;
	return;
}

static inline bool is_acceptable_stancil_face(GLenum face)
{
	bool result = false;
	switch (face) {
	case GL_FRONT:
	case GL_BACK:
	case GL_FRONT_AND_BACK:
		result = true;
		break;
	default:
		break;
	}
	return result;
}

void CoreState::setStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
	if (!is_acceptable_stancil_face(face)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_stancil_func(func)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	StencilTestParams* dst = &m_drawParameters.depthStencilState.stencilTestParams;
	if ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
		dst->stencilFunc = func;
		dst->stencilValueMask = mask;
		m_drawParameters.stencilReference.stencilRef = ref;
	}
	if ((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
		dst->stencilBackFunc = func;
		dst->stencilBackValueMask = mask;
		m_drawParameters.stencilReference.stencilBackRef = ref;
	}
	m_drawParameters.dirtyFlags |= STENCIL_REFERENCE_DIRTY_BIT;
	return;
}

void CoreState::setStencilWritemask(GLuint mask)
{
	DepthStencilWritemaskParams* dst = &m_drawParameters.depthStencilState.writemaskParams;
	dst->stencilWritemask = mask;
	dst->stencilBackWritemask = mask;
	return;
}

void CoreState::setStencilWritemaskSeparate(GLenum face, GLuint mask)
{
	if (!is_acceptable_stancil_face(face)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	DepthStencilWritemaskParams* dst = &m_drawParameters.depthStencilState.writemaskParams;
	if ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
		dst->stencilWritemask = mask;
	}
	if ((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
		dst->stencilBackWritemask = mask;
	}
	return;
}

static inline bool is_acceptable_stancil_op(GLenum op)
{
	bool result = false;
	switch (op) {
	case GL_KEEP:
	case GL_ZERO:
	case GL_REPLACE:
	case GL_INCR:
	case GL_INCR_WRAP:
	case GL_DECR:
	case GL_DECR_WRAP:
	case GL_INVERT:
		result = true;
		break;
	default:
		break;
	}
	return result;
}

void CoreState::setStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	if (!is_acceptable_stancil_op(fail)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_stancil_op(zfail)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_stancil_op(zpass)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	StencilTestParams* dst = &m_drawParameters.depthStencilState.stencilTestParams;
	dst->stencilFail = fail;
	dst->stencilPassDepthFail = zfail;
	dst->stencilPassDepthPass = zpass;
	dst->stencilBackFail = fail;
	dst->stencilBackPassDepthFail = zfail;
	dst->stencilBackPassDepthPass = zpass;
	return;
}

void CoreState::setStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	if (!is_acceptable_stancil_face(face)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_stancil_op(sfail)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_stancil_op(dpfail)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	if (!is_acceptable_stancil_op(dppass)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	StencilTestParams* dst = &m_drawParameters.depthStencilState.stencilTestParams;
	if ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
		dst->stencilFail = sfail;
		dst->stencilPassDepthFail = dpfail;
		dst->stencilPassDepthPass = dppass;
	}
	if ((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
		dst->stencilBackFail = sfail;
		dst->stencilBackPassDepthFail = dpfail;
		dst->stencilBackPassDepthPass = dppass;
	}
	return;
}

void CoreState::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	ViewportParams* dst = &m_drawParameters.viewportParams;
	dst->viewport[0] = x;
	dst->viewport[1] = y;
	dst->viewport[2] = width;
	dst->viewport[3] = height;
	m_drawParameters.dirtyFlags |= VIEWPORT_DIRTY_BIT;
	return;
}

static inline bool is_acceptable_read_buffer(GLenum src)
{
	bool result = false;
	switch (src) {
	case GL_NONE:
	case GL_BACK:
		result = true;
		break;
	default:
		if ((src - GL_COLOR_ATTACHMENT0) < AXGL_MAX_COLOR_ATTACHMENTS) {
			result = true;
		}
		break;
	}
	return result;
}

void CoreState::setReadBuffer(GLenum src)
{
	if (!is_acceptable_read_buffer(src)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_readBuffer = src;
	return;
}

GLenum CoreState::getReadBuffer() const
{
	return m_readBuffer;
}

static inline bool is_acceptable_draw_buffer(GLenum src)
{
	bool result = false;
	switch (src) {
	case GL_NONE:
	case GL_BACK:
		result = true;
		break;
	default:
		if ((src - GL_COLOR_ATTACHMENT0) < AXGL_MAX_COLOR_ATTACHMENTS) {
			result = true;
		}
		break;
	}
	return result;
}

void CoreState::setDrawBuffers(GLsizei n, const GLenum* bufs)
{
	AXGL_ASSERT(bufs != nullptr);
	for (int x = 0; x < n; n++) {
		if (!is_acceptable_draw_buffer(bufs[x])) {
			setErrorCode(GL_INVALID_ENUM);
			return;
		}
	}
	int i = 0;
	for (; i < n; i++) {
		m_drawBuffer[i] = bufs[i];
	}
	for (; i < AXGL_MAX_DRAW_BUFFERS; i++) {
		m_drawBuffer[i] = GL_NONE;
	}
	return;
}

GLenum CoreState::getDrawBuffer(GLenum pname)
{
	int index = pname - GL_DRAW_BUFFER0;
	if ((index < 0) || (index >= AXGL_MAX_DRAW_BUFFERS)) {
		return GL_NONE;
	}
	return m_drawBuffer[index];
}

void CoreState::setTransformFeedbackActive(GLboolean active)
{
	m_transformFeedbackParams.transformFeedbackActive = active;
	return;
}

static inline bool is_acceptable_transform_feedback_primitive(GLenum mode)
{
	bool result = false;
	switch (mode) {
	case GL_POINTS:
	case GL_LINES:
	case GL_TRIANGLES:
		result = true;
		break;
	default:
		break;
	}
	return result;
}

void CoreState::setTransformFeedbackPrimitive(GLenum primitive)
{
	if (!is_acceptable_transform_feedback_primitive(primitive)) {
		setErrorCode(GL_INVALID_ENUM);
		return;
	}
	m_transformFeedbackParams.transformFeedbackPrimitive = primitive;
	return;
}

void CoreState::setBufferRange(CoreContext* context, GLenum target, GLuint index, CoreBuffer* buffer, GLintptr offset, GLsizeiptr size)
{
	switch (target) {
	case GL_TRANSFORM_FEEDBACK_BUFFER:
		if (index < AXGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS) {
			setIndexedBuffer(context, &m_indexedTransformFeedbackBuffer[index], buffer, offset, size);
		} else {
			setErrorCode(GL_INVALID_VALUE);
		}
		break;
	case GL_UNIFORM_BUFFER:
		if (index < AXGL_MAX_UNIFORM_BUFFER_BINDINGS) {
			setIndexedBuffer(context, &m_drawParameters.uniformBuffer[index], buffer, offset, size);
			m_drawParameters.dirtyFlags |= UNIFORM_BUFFER_BINDING_DIRTY_BIT;
		} else {
			setErrorCode(GL_INVALID_VALUE);
		}
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}

	return;
}

void CoreState::setBufferBase(CoreContext* context, GLenum target, GLuint index, CoreBuffer* buffer)
{
	// NOTE: glBindBufferBase also binds the range to the generic buffer binding point specified by target.
	switch (target) {
	case GL_TRANSFORM_FEEDBACK_BUFFER:
		if (index < AXGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS) {
			setIndexedBuffer(context, &m_indexedTransformFeedbackBuffer[index], buffer, 0, 0);
			setCoreBuffer(context, &m_pTransformFeedbackBuffer, buffer);
		} else {
			setErrorCode(GL_INVALID_VALUE);
		}
		break;
	case GL_UNIFORM_BUFFER:
		if (index < AXGL_MAX_UNIFORM_BUFFER_BINDINGS) {
			setIndexedBuffer(context, &m_drawParameters.uniformBuffer[index], buffer, 0, 0);
			setCoreBuffer(context, &m_pUniformBuffer, buffer);
			m_drawParameters.dirtyFlags |= UNIFORM_BUFFER_BINDING_DIRTY_BIT;
		} else {
			setErrorCode(GL_INVALID_VALUE);
		}
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreState::setHint(GLenum target, GLenum mode)
{
	switch (target) {
	case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
		m_fragmentShaderDerivativeHint = mode;
		break;
	case GL_GENERATE_MIPMAP_HINT:
		m_generateMipmapHint = mode;
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}

	return;
}

GLenum CoreState::getHint(GLenum target) const
{
	// NOTE: glGet経由で呼ばれる
	GLenum mode = GL_DONT_CARE;
	switch (target) {
	case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
		mode = m_fragmentShaderDerivativeHint;
		break;
	case GL_GENERATE_MIPMAP_HINT:
		mode = m_generateMipmapHint;
		break;
	default:
		break;
	}
	return mode;
}

void CoreState::clearDrawParameterDirtyFlags()
{
	m_drawParameters.dirtyFlags = 0;
	return;
}

void CoreState::setupTextureSampler(CoreContext* context)
{
	for (int i = 0; i < AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++) {
		CoreSampler* smp = m_drawParameters.samplers[i];
		if (smp != nullptr) {
			// use sampler object
			bool result = smp->setup(context);
			if (!result) {
				AXGL_DBGOUT("CoreSampler::setup() failed in unit[%d]\n", i);
			}
		} else {
			// use texture parameters in texture object
			CoreTexture* tex_2d = m_drawParameters.texture2d[i];
			if (tex_2d != nullptr) {
				bool result = tex_2d->setupSampler(context);
				if (!result) {
					AXGL_DBGOUT("CoreTexture::setupSampler() failed in 2D unit[%d]\n", i);
				}
			}
			CoreTexture* tex_3d = m_drawParameters.texture3d[i];
			if (tex_3d != nullptr) {
				bool result = tex_3d->setupSampler(context);
				if (!result) {
					AXGL_DBGOUT("CoreTexture::setupSampler() failed in 3D unit[%d]\n", i);
				}
			}
			CoreTexture* tex_2d_array = m_drawParameters.texture2dArray[i];
			if (tex_2d_array != nullptr) {
				bool result = tex_2d_array->setupSampler(context);
				if (!result) {
					AXGL_DBGOUT("CoreTexture::setupSampler() failed in 2DArray unit[%d]\n", i);
				}
			}
			CoreTexture* tex_cube = m_drawParameters.textureCube[i];
			if (tex_cube != nullptr) {
				bool result = tex_cube->setupSampler(context);
				if (!result) {
					AXGL_DBGOUT("CoreTexture::setupSampler() failed in Cube unit[%d]\n", i);
				}
			}
		}
	}
	return;
}

void CoreState::setInstancedRendering(bool isInstanced)
{
	// set to render pipeline state
	m_drawParameters.renderPipelineState.isInstanced = isInstanced;
	return;
}

const BlendParams& CoreState::getBlendParams() const
{
	return m_drawParameters.renderPipelineState.blendParams;
}

const DepthTestParams& CoreState::getDepthTestParams() const
{
	return m_drawParameters.depthStencilState.depthTestParams;
}

const StencilTestParams& CoreState::getStencilTestParams() const
{
	return m_drawParameters.depthStencilState.stencilTestParams;
}

const StencilReference& CoreState::getStencilReference() const
{
	return m_drawParameters.stencilReference;
}

const ViewportParams& CoreState::getViewportParams() const
{
	return m_drawParameters.viewportParams;
}

const ClearParameters& CoreState::getClearParams() const
{
	return m_clearParams;
}

const ScissorParams& CoreState::getScissorParams() const
{
	return m_drawParameters.scissorParams;
}

const CullFaceParams& CoreState::getCullFaceParams() const
{
	return m_drawParameters.cullFaceParams;
}

const ColorWritemaskParams& CoreState::getColorWritemaskParams() const
{
	return m_drawParameters.renderPipelineState.writemaskParams;
}

const DepthStencilWritemaskParams& CoreState::getDepthStencilWritemaskParams() const
{
	return m_drawParameters.depthStencilState.writemaskParams;
}

const CoreState::PackParams& CoreState::getPackParams() const
{
	return m_packParams;
}

const CoreState::UnpackParams& CoreState::getUnpackParams() const
{
	return m_unpackParams;
}

const SampleCoverageParams& CoreState::getSampleCoverageParams() const
{
	return m_drawParameters.renderPipelineState.sampleCoverageParams;
}

const PolygonOffsetParams& CoreState::getPolygonOffsetParams() const
{
	return m_drawParameters.polygonOffsetParams;
}

const CoreState::TransformFeedbackParams& CoreState::getTransformFeedbackParams() const
{
	return m_transformFeedbackParams;
}

const IndexedBuffer* CoreState::getIndexedBuffer(GLenum target, GLuint index) const
{
	const IndexedBuffer* rptr = nullptr;
	if ((target == GL_TRANSFORM_FEEDBACK_BUFFER) && (index < AXGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS)) {
		rptr = &m_indexedTransformFeedbackBuffer[index];
	} else if ((target == GL_UNIFORM_BUFFER) && (index < AXGL_MAX_UNIFORM_BUFFER_BINDINGS)) {
		rptr = &m_drawParameters.uniformBuffer[index];
	}
	return rptr;
}

const DrawParameters& CoreState::getDrawParameters() const
{
	return m_drawParameters;
}

bool CoreState::elementArrayBufferAvailable() const
{
	CoreBuffer* eab = nullptr;
	if (m_drawParameters.vertexArray != nullptr) {
		eab = m_drawParameters.vertexArray->getIndexBuffer();
	} else {
		eab = m_drawParameters.indexBuffer;
	}
	return (eab != nullptr);
}

void CoreState::setCoreBuffer(CoreContext* context, CoreBuffer** dst, CoreBuffer* buf)
{
	AXGL_ASSERT(dst != nullptr);
	if (buf != nullptr) {
		buf->addRef();
	}
	if (*dst != nullptr) {
		(*dst)->release(context);
	}
	*dst = buf;
	return;
}

void CoreState::setCoreTexture(CoreContext* context, CoreTexture** dst, CoreTexture* tex)
{
	AXGL_ASSERT(dst != nullptr);
	if (tex != nullptr) {
		tex->addRef();
	}
	if (*dst != nullptr) {
		(*dst)->release(context);
	}
	*dst = tex;
	return;
}

void CoreState::setCoreFramebuffer(CoreContext* context, CoreFramebuffer** dst, CoreFramebuffer* framebuffer)
{
	AXGL_ASSERT(dst != nullptr);
	if (framebuffer != nullptr) {
		framebuffer->addRef();
	}
	if (*dst != nullptr) {
		(*dst)->release(context);
	}
	*dst = framebuffer;
	return;
}

void CoreState::setCoreSampler(CoreContext* context, CoreSampler** dst, CoreSampler* sampler)
{
	AXGL_ASSERT(dst != nullptr);
	if (sampler != nullptr) {
		sampler->addRef();
	}
	if (*dst != nullptr) {
		(*dst)->release(context);
	}
	*dst = sampler;
	return;
}

void CoreState::setCoreQuery(CoreContext* context, CoreQuery** dst, CoreQuery* query)
{
	AXGL_ASSERT(dst != nullptr);
	if (query != nullptr) {
		query->addRef();
	}
	if (*dst != nullptr) {
		(*dst)->release(context);
	}
	*dst = query;
	return;
}

void CoreState::setIndexedBuffer(CoreContext* context, IndexedBuffer* dst, CoreBuffer* buf, GLintptr offset, GLsizeiptr size)
{
	AXGL_ASSERT(dst != nullptr);
	if (buf != nullptr) {
		buf->addRef();
	}
	if (dst->buffer != nullptr) {
		dst->buffer->release(context);
	}
	dst->buffer = buf;
	// TODO: size, offset, alignment check
	dst->offset = offset;
	dst->size = size;
	return;
}

void CoreState::updatePipelineStateAttachments()
{
	TargetAttachment* dst = &m_drawParameters.renderPipelineState.targetAttachment;
	if (m_pDrawFramebuffer == nullptr) {
		// clear TargetAttachment
		for (int i = 0; i < AXGL_MAX_COLOR_ATTACHMENTS; i++) {
			dst->colorFormat[i] = 0;
		}
		dst->depthFormat = 0;
		dst->stencilFormat = 0;
		dst->samples = 1;
	} else {
		m_pDrawFramebuffer->setupTargetAttachment(dst);
	}
	return;
}

bool CoreState::isIntegerType(GLenum type)
{
	bool rval = false;
	switch (type) {
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
	case GL_INT:
	case GL_UNSIGNED_INT:
		rval = true;
		break;
	default:
		break;
	}
	return rval;
}

} // namespace axgl
