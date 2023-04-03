// CoreObjectsManager.h
// オブジェクト管理クラスの宣言
#ifndef __CoreObjectsManager_h_
#define __CoreObjectsManager_h_

#include "../common/axglCommon.h"
#include <mutex>

namespace axgl {

class CoreContext;
class CoreBuffer;
class CoreFramebuffer;
class CoreProgram;
class CoreObject;
class CoreQuery;
class CoreRenderbuffer;
class CoreSampler;
class CoreShader;
class CoreTexture;
class CoreTransformFeedback;
class CoreVertexArray;
class CoreSync;

// オブジェクト管理クラス
class CoreObjectsManager
{
public:
	CoreObjectsManager();
	~CoreObjectsManager();
	// Buffer
	void genBuffers(CoreContext* context, GLsizei n, GLuint* buffers);
	void deleteBuffers(CoreContext* context, GLsizei n, const GLuint* buffers);
	CoreBuffer* getBuffer(GLuint id);
	// Framebuffer
	void genFramebuffers(CoreContext* context, GLsizei n, GLuint* framebuffers);
	void deleteFramebuffers(CoreContext* context, GLsizei n, const GLuint* framebuffers);
	CoreFramebuffer* getFramebuffer(GLuint id);
	// Program
	GLuint createProgram(CoreContext* context);
	void deleteProgram(CoreContext* context, GLuint program);
	CoreProgram* getProgram(GLuint id);
	// Query
	void genQueries(CoreContext* context, GLsizei n, GLuint *ids);
	void deleteQueries(CoreContext* context, GLsizei n, const GLuint *ids);
	CoreQuery* getQuery(GLuint id);
	// Renderbuffer
	void genRenderbuffers(CoreContext* context, GLsizei n, GLuint* renderbuffers);
	void deleteRenderbuffers(CoreContext* context, GLsizei n, const GLuint* renderbuffers);
	CoreRenderbuffer* getRenderbuffer(GLuint id);
	// Sampler
	void genSamplers(GLsizei n, GLuint* samplers);
	void deleteSamplers(CoreContext* context, GLsizei n, const GLuint* samplers);
	CoreSampler* getSampler(GLuint id);
	// Shader
	GLuint createShader(CoreContext* context, GLenum type);
	void deleteShader(CoreContext* context, GLuint shader);
	CoreShader* getShader(GLuint id);
	// Texture
	void genTextures(GLsizei n, GLuint* textures);
	void deleteTextures(CoreContext* context, GLsizei n, const GLuint* textures);
	CoreTexture* getTexture(GLuint id);
	// TransformFeedback
	void genTransformFeedbacks(GLsizei n, GLuint* ids);
	void deleteTransformFeedbacks(CoreContext* context, GLsizei n, const GLuint* ids);
	CoreTransformFeedback* getTransformFeedback(GLuint id);
	// VertexArray
	void genVertexArrays(CoreContext* context, GLsizei n, GLuint* arrays);
	void deleteVertexArrays(CoreContext* context, GLsizei n, const GLuint* arrays);
	CoreVertexArray* getVertexArray(GLuint id);
	// Sync
	CoreSync* createSync(GLenum condition, GLbitfield flags);
	void deleteSync(CoreContext* context, GLsync sync);
	CoreSync* getSync(GLsync sync);
	// Reference count
	void addRef();
	void release(CoreContext* context);

private:
	GLuint getNextBufferId();
	GLuint getNextFramebufferId();
	GLuint getNextProgramId();
	GLuint getNextQueryId();
	GLuint getNextRenderbufferId();
	GLuint getNextSamplerId();
	GLuint getNextShaderId();
	GLuint getNextTextureId();
	GLuint getNextTransformFeedbackId();
	GLuint getNextVertexArrayId();
	void destroyAllObjects(CoreContext* context);

private:
	std::mutex m_mutex;
	AXGLUnorderedMap<GLuint, CoreBuffer*> m_buffers;
	GLuint m_lastBufferId = 0;
	AXGLUnorderedMap<GLuint, CoreFramebuffer*> m_framebuffers;
	GLuint m_lastFramebufferId = 0;
	AXGLUnorderedMap<GLuint, CoreProgram*> m_programs;
	GLuint m_lastProgramId = 0;
	AXGLUnorderedMap<GLuint, CoreQuery*> m_queries;
	GLuint m_lastQueryId = 0;
	AXGLUnorderedMap<GLuint, CoreRenderbuffer*> m_renderbuffers;
	GLuint m_lastRenderbufferId = 0;
	AXGLUnorderedMap<GLuint, CoreSampler*> m_samplers;
	GLuint m_lastSamplerId = 0;
	AXGLUnorderedMap<GLuint, CoreShader*> m_shaders;
	GLuint m_lastShaderId = 0;
	AXGLUnorderedMap<GLuint, CoreTexture*> m_textures;
	GLuint m_lastTextureId = 0;
	AXGLUnorderedMap<GLuint, CoreTransformFeedback*> m_transformFeedbacks;
	GLuint m_lastTransformFeedbackId = 0;
	AXGLUnorderedMap<GLuint, CoreVertexArray*> m_vertexArrays;
	GLuint m_lastVertexArrayId = 0;
	AXGLUnorderedSet<CoreSync*> m_syncSet;
	uint32_t m_referenceCount = 0;
};

} // namespace axgl

#endif // __CoreObjectsManager_h_
