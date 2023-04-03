// QueryMetal.mm
#include "QueryMetal.h"
#include "ContextMetal.h"
#include "../../AXGLAllocatorImpl.h"
#include "../../common/axglDebug.h"

// 1024 entry * 64bit
#define AXGL_VISIBILITY_RESULT_BUFFER_SIZE (1024*8)

namespace axgl {

// BackendQueryクラスの実装 --------
BackendQuery* BackendQuery::create()
{
	QueryMetal* query = AXGL_NEW(QueryMetal);
	return query;
}
	
void BackendQuery::destroy(BackendQuery* query)
{
	if (query == nullptr) {
		return;
	}
	AXGL_DELETE(query);
	return;
}

// QueryMetalクラスの実装 --------
QueryMetal::QueryMetal()
{
}

QueryMetal::~QueryMetal()
{
}

bool QueryMetal::initialize(BackendContext* context)
{
	if (context == nullptr) {
		return false;
	}
	AXGL_ASSERT(m_visibilityResultBuffer == nil);
	ContextMetal* context_metal = static_cast<ContextMetal*>(context);
	id<MTLDevice> mtlDevice = context_metal->getDevice();
	// 8bytes : 64bit result value
	NSUInteger size = AXGL_VISIBILITY_RESULT_BUFFER_SIZE;
	// VisibilityResultBuffer for GPU
	m_visibilityResultBuffer = [mtlDevice newBufferWithLength:size options:MTLResourceStorageModePrivate];
	// Copy destination buffer for CPU
	m_copyDestBuffer = [mtlDevice newBufferWithLength:size options:MTLResourceStorageModeShared];
	return true;
}

void QueryMetal::terminate(BackendContext* context)
{
	m_visibilityResultBuffer = nil;
	m_copyDestBuffer = nil;
	return;
}

bool QueryMetal::begin(BackendContext* context)
{
	if (context == nullptr) {
		return false;
	}
	// change result available
	m_resultAvailable = GL_FALSE;
	// reset offset
	m_visibilityResultBufferOffset = 0;
	ContextMetal* mtl_context = static_cast<ContextMetal*>(context);
	// create blit command
	id<MTLCommandQueue> command_queue = mtl_context->getCommandQueue();
	AXGL_ASSERT(command_queue != nil);
	id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
	AXGL_ASSERT(command_buffer != nil);
	id<MTLBlitCommandEncoder> blit_command_encoder = [command_buffer blitCommandEncoder];
	AXGL_ASSERT(blit_command_encoder != nil);
	static const NSRange c_fill_range { // struct
		0, // location
		AXGL_VISIBILITY_RESULT_BUFFER_SIZE // length
	};
	[blit_command_encoder fillBuffer:m_visibilityResultBuffer range:c_fill_range value:0];
	[blit_command_encoder endEncoding];
	// commit command buffer
	[command_buffer commit];
	return true;
}

bool QueryMetal::end(BackendContext* context)
{
	if (context == nullptr) {
		return false;
	}
	if (m_visibilityResultBufferOffset == 0) {
		// no buffer copy required
		m_resultAvailable = GL_TRUE;
		return true;
	}
	ContextMetal* mtl_context = static_cast<ContextMetal*>(context);
	// create blit command
	id<MTLCommandQueue> command_queue = mtl_context->getCommandQueue();
	AXGL_ASSERT(command_queue != nil);
	id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
	AXGL_ASSERT(command_buffer != nil);
	// encode a blit pass to copy data from private buffer to shared buffer
	id<MTLBlitCommandEncoder> blit_command_encoder = [command_buffer blitCommandEncoder];
	AXGL_ASSERT(blit_command_encoder != nil);
	[blit_command_encoder copyFromBuffer:m_visibilityResultBuffer sourceOffset:0
		toBuffer:m_copyDestBuffer destinationOffset:0 size:AXGL_VISIBILITY_RESULT_BUFFER_SIZE];
	[blit_command_encoder endEncoding];
	// block for completed handler
	void (^completed_func)(id<MTLCommandBuffer>) = ^(id<MTLCommandBuffer> commandBuffer){
		this->m_resultAvailable = GL_TRUE;
	};
	// add completed handler
	[command_buffer addCompletedHandler:completed_func];
	// commit command buffer
	[command_buffer commit];

	return true;
}

bool QueryMetal::getQueryuiv(GLenum pname, GLuint* params)
{
	if (params == nullptr) {
		return false;
	}
	if (pname == GL_QUERY_RESULT_AVAILABLE) {
		*params = m_resultAvailable;
	} else if(pname == GL_QUERY_RESULT) {
		if (m_copyDestBuffer == nil) {
			return false;
		}
		if (m_visibilityResultBufferOffset == 0) {
			// no results
			*params = 0; // initial value
		} else {
			// read copied visibility results
			uint64_t* result = static_cast<uint64_t*>([m_copyDestBuffer contents]);
			if (result == nullptr) {
				return false;
			}
			// check result for GL_ANY_SAMPLES_PASSED or GL_ANY_SAMPLES_PASSED_CONSERVATIVE
			int num_result = static_cast<int>(m_visibilityResultBufferOffset / 8);
			GLuint tmp = GL_FALSE;
			for (int i = 0; i < num_result; i++) {
				if (result[i] != 0) {
					tmp = GL_TRUE;
					break;
				}
			}
			*params = tmp;
		}
	}
	return true;
}

id<MTLBuffer> QueryMetal::getVisibilityResultBuffer() const
{
	return m_visibilityResultBuffer;
}

uint64_t QueryMetal::getVisibilityResultBufferOffset() const
{
	return m_visibilityResultBufferOffset;
}

bool QueryMetal::incrementOffset()
{
	bool result = true;
	// result:64bit
	m_visibilityResultBufferOffset += 8;
	if (m_visibilityResultBufferOffset > (AXGL_VISIBILITY_RESULT_BUFFER_SIZE - 8)) {
		AXGL_MSGOUT("BackendQueryMetal::incrementOffset> overflow offset value (%lld)", m_visibilityResultBufferOffset);
		m_visibilityResultBufferOffset = (AXGL_VISIBILITY_RESULT_BUFFER_SIZE - 8);
		result = false;
	}
	return result;
}

id<MTLBuffer> QueryMetal::getCopyDestBuffer() const
{
	return m_copyDestBuffer;
}

} // namespace axgl
