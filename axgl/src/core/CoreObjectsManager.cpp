// CoreObjectsManager.cpp
// オブジェクト管理クラスの実装
#include "CoreObjectsManager.h"
#include "CoreBuffer.h"
#include "CoreFramebuffer.h"
#include "CoreProgram.h"
#include "CoreQuery.h"
#include "CoreRenderbuffer.h"
#include "CoreSampler.h"
#include "CoreShader.h"
#include "CoreSync.h"
#include "CoreTexture.h"
#include "CoreTransformFeedback.h"
#include "CoreVertexArray.h"

#include "../AXGLAllocatorImpl.h"

namespace axgl {

// コンストラクタ
CoreObjectsManager::CoreObjectsManager()
{
}

// デストラクタ
CoreObjectsManager::~CoreObjectsManager()
{
}

// Bufferの生成
void CoreObjectsManager::genBuffers(CoreContext* context, GLsizei n, GLuint* buffers)
{
	if ((n <= 0) || (buffers == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			CoreBuffer* core_buffer = AXGL_NEW(CoreBuffer);
			if (core_buffer == nullptr) {
				// TODO: error メモリ確保失敗
				break;
			}
			GLuint next_id = getNextBufferId();

			core_buffer->addRef();
			core_buffer->setId(next_id);
			core_buffer->initialize(context);

			m_buffers.emplace(next_id, core_buffer);

			buffers[i] = next_id;
		}
	}
	return;
}

// Bufferの削除
void CoreObjectsManager::deleteBuffers(CoreContext* context, GLsizei n, const GLuint* buffers)
{
	if ((n <= 0) || (buffers == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			auto it = m_buffers.find(buffers[i]);
			if (it != m_buffers.end()) {
				if (it->second != nullptr) {
					it->second->release(context);
				}
				m_buffers.erase(it);
			}
		}
	}
	return;
}

// CoreBufferを取得
CoreBuffer* CoreObjectsManager::getBuffer(GLuint id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_buffers.find(id);
	if (it == m_buffers.end()) {
		return nullptr;
	}
	return it->second;
}

// Framebufferの作成
void CoreObjectsManager::genFramebuffers(CoreContext* context, GLsizei n, GLuint* framebuffers)
{
	if ((n <= 0) || (framebuffers == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		for (GLsizei i = 0; i < n; i++) {
			CoreFramebuffer* core_framebuffer = AXGL_NEW(CoreFramebuffer);
			if (core_framebuffer == nullptr) {
				// TODO: error メモリ確保失敗
				break;
			}
			if (!core_framebuffer->initialize(context)) {
				AXGL_DBGOUT("CoreFramebuffer::initialize() failedn");
			}

			GLuint next_id = getNextFramebufferId();
			core_framebuffer->addRef();
			core_framebuffer->setId(next_id);
			m_framebuffers.emplace(next_id, core_framebuffer);

			framebuffers[i] = next_id;
		}
	}
	return;
}

// Framebufferの削除
void CoreObjectsManager::deleteFramebuffers(CoreContext* context, GLsizei n, const GLuint* framebuffers)
{
	if ((n <= 0) || (framebuffers == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		for (GLsizei i = 0; i < n; i++) {
			auto it = m_framebuffers.find(framebuffers[i]);
			if (it != m_framebuffers.end()) {
				if (it->second != nullptr) {
					it->second->release(context);
				}
				m_framebuffers.erase(it);
			}
		}
	}
	return;
}

// CoreFramebufferを取得
CoreFramebuffer* CoreObjectsManager::getFramebuffer(GLuint id)
{
	if (id != 0) {
		std::lock_guard<std::mutex> lock(m_mutex);

		auto it = m_framebuffers.find(id);
		if (it == m_framebuffers.end()) {
			return nullptr;
		}
		return it->second;
	}
	return nullptr;
}

// Programの作成
GLuint CoreObjectsManager::createProgram(CoreContext* context)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	CoreProgram* core_program = AXGL_NEW(CoreProgram);
	if (core_program == nullptr) {
		// TODO: error
		return 0;
	}
	GLuint next_id = getNextProgramId();
	core_program->addRef();
	core_program->setId(next_id);
	core_program->initialize(context);
	m_programs.emplace(next_id, core_program);
	return next_id;
}

// Programの削除
void CoreObjectsManager::deleteProgram(CoreContext* context, GLuint program)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_programs.find(program);
	if (it != m_programs.end()) {
		if (it->second != nullptr) {
			// NOTE: Program は GL_DELETE_STATUS を保持
			it->second->setDeleteStatus();
			it->second->release(context);
		}
		m_programs.erase(it);
	}
	return;
}

// CoreProgramの取得
CoreProgram* CoreObjectsManager::getProgram(GLuint id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_programs.find(id);
	if (it == m_programs.end()) {
		return nullptr;
	}
	return it->second;
}

// Queryの生成
void CoreObjectsManager::genQueries(CoreContext* context, GLsizei n, GLuint *ids)
{
	if ((n <= 0) || (ids == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			CoreQuery* core_query = AXGL_NEW(CoreQuery);
			if (core_query == nullptr) {
				// TODO: error
				break;
			}
			GLuint next_id = getNextQueryId();
			core_query->addRef();
			core_query->setId(next_id);
			core_query->initialize(context);
			m_queries.emplace(next_id, core_query);
			ids[i] = next_id;
		}
	}
	return;
}

// Queryの削除
void CoreObjectsManager::deleteQueries(CoreContext* context, GLsizei n, const GLuint *ids)
{
	if ((n <= 0) || (ids == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			auto it = m_queries.find(ids[i]);
			if (it != m_queries.end()) {
				if (it->second != nullptr) {
					it->second->release(context);
				}
				m_queries.erase(it);
			}
		}
	}
	return;
}

// CoreQueryの取得
CoreQuery* CoreObjectsManager::getQuery(GLuint id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_queries.find(id);
	if (it == m_queries.end()) {
		return nullptr;
	}
	return it->second;
}

// Renderbufferの生成
void CoreObjectsManager::genRenderbuffers(CoreContext* context, GLsizei n, GLuint* renderbuffers)
{
	if ((n <= 0) || (renderbuffers == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			CoreRenderbuffer* core_renderbuffer = AXGL_NEW(CoreRenderbuffer);
			if (core_renderbuffer == nullptr) {
				// TODO: error
				break;
			}
			if (!core_renderbuffer->initialize(context)) {
				AXGL_DBGOUT("CoreRenderbuffer::initialize() failed\n");
			}
			GLuint next_id = getNextRenderbufferId();
			core_renderbuffer->addRef();
			core_renderbuffer->setId(next_id);
			m_renderbuffers.emplace(next_id, core_renderbuffer);
			renderbuffers[i] = next_id;
		}
	}
	return;
}

// Renderbufferの削除
void CoreObjectsManager::deleteRenderbuffers(CoreContext* context, GLsizei n, const GLuint* renderbuffers)
{
	if ((n <= 0) || (renderbuffers == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			auto it = m_renderbuffers.find(renderbuffers[i]);
			if (it != m_renderbuffers.end()) {
				if (it->second != nullptr) {
					it->second->release(context);
				}
				m_renderbuffers.erase(it);
			}
		}
	}
	return;
}

// CoreRenderbufferの取得
CoreRenderbuffer* CoreObjectsManager::getRenderbuffer(GLuint id)
{
	if (id != 0) {
		std::lock_guard<std::mutex> lock(m_mutex);
		auto it = m_renderbuffers.find(id);
		if (it == m_renderbuffers.end()) {
			return nullptr;
		}
		return it->second;
	}
	return nullptr;
}

// Samplerの生成
void CoreObjectsManager::genSamplers(GLsizei n, GLuint* samplers)
{
	if ((n <= 0) || (samplers == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			CoreSampler* core_sampler = AXGL_NEW(CoreSampler);
			if (core_sampler == nullptr) {
				// TODO: error
				break;
			}
			GLuint next_id = getNextSamplerId();

			core_sampler->addRef();
			core_sampler->setId(next_id);
			m_samplers.emplace(next_id, core_sampler);
			samplers[i] = next_id;
		}
	}
	return;
}

// Samplerの削除
void CoreObjectsManager::deleteSamplers(CoreContext* context, GLsizei n, const GLuint* samplers)
{
	if ((n <= 0) || (samplers == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			auto it = m_samplers.find(samplers[i]);
			if (it != m_samplers.end()) {
				if (it->second != nullptr) {
					it->second->release(context);
				}
				m_samplers.erase(it);
			}
		}
	}
	return;
}

// CoreSamplerの取得
CoreSampler* CoreObjectsManager::getSampler(GLuint id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_samplers.find(id);
	if (it == m_samplers.end()) {
		return nullptr;
	}
	return it->second;
}

// Shaderの作成
GLuint CoreObjectsManager::createShader(CoreContext* context, GLenum type)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	CoreShader* core_shader = AXGL_NEW(CoreShader(type));
	if (core_shader == nullptr) {
		// TODO: error
		return 0;
	}
	GLuint next_id = getNextShaderId();
	core_shader->addRef();
	core_shader->setId(next_id);
	core_shader->initialize(context);
	m_shaders.emplace(next_id, core_shader);
	return next_id;
}

// Shaderの削除
void CoreObjectsManager::deleteShader(CoreContext* context, GLuint shader)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_shaders.find(shader);
	if (it != m_shaders.end()) {
		if (it->second != nullptr) {
			// NOTE: Shader は GL_DELETE_STATUS を保持
			it->second->setDeleteStatus();
			it->second->release(context);
		}
		m_shaders.erase(it);
	}
	return;
}

// CoreShaderの取得
CoreShader* CoreObjectsManager::getShader(GLuint id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_shaders.find(id);
	if (it == m_shaders.end()) {
		return nullptr;
	}
	return it->second;
}

// Textureの生成
void CoreObjectsManager::genTextures(GLsizei n, GLuint* textures)
{
	if ((n <= 0) || (textures == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			CoreTexture* core_texture = AXGL_NEW(CoreTexture);
			if (core_texture == nullptr) {
				// TODO: error
				break;
			}
			GLuint next_id = getNextTextureId();

			core_texture->addRef();
			core_texture->setId(next_id);
			m_textures.emplace(next_id, core_texture);
			textures[i] = next_id;
		}
	}
	return;
}

// Textureの削除
void CoreObjectsManager::deleteTextures(CoreContext* context, GLsizei n, const GLuint* textures)
{
	if ((n <= 0) || (textures == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			auto it = m_textures.find(textures[i]);
			if (it != m_textures.end()) {
				if (it->second != nullptr) {
					it->second->release(context);
				}
				m_textures.erase(it);
			}
		}
	}
	return;
}

// CoreTextureの取得
CoreTexture* CoreObjectsManager::getTexture(GLuint id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_textures.find(id);
	if (it == m_textures.end()) {
		return nullptr;
	}
	return it->second;
}

// TransformFeedbackの生成
void CoreObjectsManager::genTransformFeedbacks(GLsizei n, GLuint* ids)
{
	if ((n <= 0) || (ids == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			CoreTransformFeedback* core_transform_feedback = AXGL_NEW(CoreTransformFeedback);
			if (core_transform_feedback == nullptr) {
				// TODO: error
				break;
			}
			GLuint next_id = getNextTransformFeedbackId();
			core_transform_feedback->addRef();
			core_transform_feedback->setId(next_id);
			m_transformFeedbacks.emplace(next_id, core_transform_feedback);
			ids[i] = next_id;
		}
	}
	return;
}

// TransformFeedbackの削除
void CoreObjectsManager::deleteTransformFeedbacks(CoreContext* context, GLsizei n, const GLuint* ids)
{
	if ((n <= 0) || (ids == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			auto it = m_transformFeedbacks.find(ids[i]);
			if (it != m_transformFeedbacks.end()) {
				if (it->second != nullptr) {
					it->second->release(context);
				}
				m_transformFeedbacks.erase(it);
			}
		}
	}
	return;
}

// CoreTransformFeedbackの取得
CoreTransformFeedback* CoreObjectsManager::getTransformFeedback(GLuint id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_transformFeedbacks.find(id);
	if (it == m_transformFeedbacks.end()) {
		return nullptr;
	}
	return it->second;
}

// VertexArrayの生成
void CoreObjectsManager::genVertexArrays(CoreContext* context, GLsizei n, GLuint* arrays)
{
	if ((n <= 0) || (arrays == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			CoreVertexArray* core_vertex_array = AXGL_NEW(CoreVertexArray);
			if (core_vertex_array == nullptr) {
				// TODO: error
				break;
			}
			GLuint next_id = getNextVertexArrayId();
			core_vertex_array->addRef();
			core_vertex_array->setId(next_id);
			core_vertex_array->initialize(context);
			m_vertexArrays.emplace(next_id, core_vertex_array);
			arrays[i] = next_id;
		}
	}
	return;
}

// VertexArrayの削除
void CoreObjectsManager::deleteVertexArrays(CoreContext* context, GLsizei n, const GLuint* arrays)
{
	if ((n <= 0) || (arrays == nullptr)) {
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (GLsizei i = 0; i < n; i++) {
			auto it = m_vertexArrays.find(arrays[i]);
			if (it != m_vertexArrays.end()) {
				if (it->second != nullptr) {
					it->second->release(context);
				}
				m_vertexArrays.erase(it);
			}
		}
	}
	return;
}

// CoreVertexArrayの取得
CoreVertexArray* CoreObjectsManager::getVertexArray(GLuint id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_vertexArrays.find(id);
	if (it == m_vertexArrays.end()) {
		return nullptr;
	}
	return it->second;
}

// Syncの作成
CoreSync* CoreObjectsManager::createSync(GLenum condition, GLbitfield flags)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE) {
		setErrorCode(GL_INVALID_ENUM);
		return nullptr;
	}
	if (flags != 0) {
		setErrorCode(GL_INVALID_VALUE);
		return nullptr;
	}
	CoreSync* core_sync = AXGL_NEW(CoreSync(condition, flags));
	if (core_sync == nullptr) {
		// TODO: error
		return nullptr;
	}
	core_sync->addRef();
	m_syncSet.emplace(core_sync);
	return core_sync;
}

// Syncの削除
void CoreObjectsManager::deleteSync(CoreContext* context, GLsync sync)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	CoreSync* core_sync = reinterpret_cast<CoreSync*>(sync);
	if (core_sync != nullptr) {
		auto it = m_syncSet.find(core_sync);
		if (it != m_syncSet.end()) {
			m_syncSet.erase(it);
			core_sync->release(context);
		} else {
			setErrorCode(GL_INVALID_VALUE);
		}
	}
	return;
}

// CoreSyncの取得
CoreSync* CoreObjectsManager::getSync(GLsync sync)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	CoreSync* core_sync = reinterpret_cast<CoreSync*>(sync);
	if (m_syncSet.find(core_sync) == m_syncSet.end()) {
		return nullptr;
	}
	return core_sync;
}

// リファレンスカウントを増加
void CoreObjectsManager::addRef()
{
	AXGL_ASSERT(m_referenceCount <= UINT32_MAX);
	m_referenceCount++;
	return;
}

// リリース
void CoreObjectsManager::release(CoreContext* context)
{
	AXGL_ASSERT(m_referenceCount > 0);
	m_referenceCount--;
	if (m_referenceCount == 0) {
		destroyAllObjects(context);
		AXGL_DELETE(this);
	}
	return;
}

// 次のBufferIDを取得
GLuint CoreObjectsManager::getNextBufferId()
{
	GLuint id = m_lastBufferId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_buffers.end();
		while (next_id == 0) {
			const auto it = m_buffers.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastBufferId = next_id;
	return next_id;
}

// 次のFramebufferIDを取得
GLuint CoreObjectsManager::getNextFramebufferId()
{
	GLuint id = m_lastFramebufferId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_framebuffers.end();
		while (next_id == 0) {
			const auto it = m_framebuffers.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastFramebufferId = next_id;
	return next_id;
}

// 次のProgramIDを取得
GLuint CoreObjectsManager::getNextProgramId()
{
	GLuint id = m_lastProgramId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_programs.end();
		while (next_id == 0) {
			const auto it = m_programs.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastProgramId = next_id;
	return next_id;
}

// 次のQueryIDを取得
GLuint CoreObjectsManager::getNextQueryId()
{
	GLuint id = m_lastQueryId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_queries.end();
		while (next_id == 0) {
			const auto it = m_queries.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastQueryId = next_id;
	return next_id;
}

// 次のRenderbufferIDを取得
GLuint CoreObjectsManager::getNextRenderbufferId()
{
	GLuint id = m_lastRenderbufferId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_renderbuffers.end();
		while (next_id == 0) {
			const auto it = m_renderbuffers.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastRenderbufferId = next_id;
	return next_id;
}

// 次のSamplerIDを取得
GLuint CoreObjectsManager::getNextSamplerId()
{
	GLuint id = m_lastSamplerId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_samplers.end();
		while (next_id == 0) {
			const auto it = m_samplers.find(id);

			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastSamplerId = next_id;
	return next_id;
}

// 次のShaderIDを取得
GLuint CoreObjectsManager::getNextShaderId()
{
	GLuint id = m_lastShaderId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_shaders.end();
		while (next_id == 0) {
			const auto it = m_shaders.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastShaderId = next_id;
	return next_id;
}

// 次のTextureIDを取得
GLuint CoreObjectsManager::getNextTextureId()
{
	GLuint id = m_lastTextureId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_textures.end();
		while (next_id == 0) {
			const auto it = m_textures.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastTextureId = next_id;
	return next_id;
}

// 次のTransformFeedbackIDを取得
GLuint CoreObjectsManager::getNextTransformFeedbackId()
{
	GLuint id = m_lastTransformFeedbackId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_transformFeedbacks.end();
		while (next_id == 0) {
			const auto it = m_transformFeedbacks.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastTransformFeedbackId = next_id;
	return next_id;
}

// 次のVertexArrayIDを取得
GLuint CoreObjectsManager::getNextVertexArrayId()
{
	GLuint id = m_lastVertexArrayId + 1;
	if (id == 0) {
		id = 1;
	}
	GLuint next_id = 0;
	{
		const auto it_end = m_vertexArrays.end();
		while (next_id == 0) {
			const auto it = m_vertexArrays.find(id);
			if (it == it_end) {
				next_id = id;
			} else {
				id++;
				if (id == 0) {
					id = 1;
				}
			}
		}
	}
	m_lastVertexArrayId = next_id;
	return next_id;
}

// 全オブジェクトを破棄
void CoreObjectsManager::destroyAllObjects(CoreContext* context)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	for (auto it = m_buffers.begin(); it != m_buffers.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_buffers.clear();
	for (auto it = m_framebuffers.begin(); it != m_framebuffers.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_framebuffers.clear();
	for (auto it = m_programs.begin(); it != m_programs.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_programs.clear();
	for (auto it = m_queries.begin(); it != m_queries.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_queries.clear();
	for (auto it = m_renderbuffers.begin(); it != m_renderbuffers.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_renderbuffers.clear();
	for (auto it = m_samplers.begin(); it != m_samplers.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_samplers.clear();
	for (auto it = m_shaders.begin(); it != m_shaders.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_shaders.clear();
	for (auto it = m_textures.begin(); it != m_textures.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_textures.clear();
	for (auto it = m_transformFeedbacks.begin(); it != m_transformFeedbacks.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_transformFeedbacks.clear();
	for (auto it = m_vertexArrays.begin(); it != m_vertexArrays.end(); it++) {
		if (it->second != nullptr) {
			it->second->terminate(context);
			AXGL_DELETE(it->second);
		}
	}
	m_vertexArrays.clear();
	for (auto it = m_syncSet.begin(); it != m_syncSet.end(); it++) {
		if (*it != nullptr) {
			(*it)->terminate(context);
			AXGL_DELETE(*it);
		}
	}
	m_syncSet.clear();
	return;
}

} // namespace axgl
