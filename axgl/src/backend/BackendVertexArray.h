// BackendVertexArray.h
#ifndef __BackendVertexArray_h_
#define __BackendVertexArray_h_

#include "../common/axglCommon.h"

namespace axgl {

class BackendContext;
class BackendBuffer;

class BackendVertexArray
{
public:
	virtual ~BackendVertexArray() {}
	virtual bool initialize(BackendContext* context) = 0;
	virtual bool terminate(BackendContext* context) = 0;
	virtual void setEnable(uint32_t index, bool enable) = 0;
	virtual void setVertexAttrib(uint32_t index, int32_t size, int32_t type, int32_t normalized, uint32_t stride, uint32_t offset, BackendBuffer* buffer) = 0;
	virtual void setDivisor(uint32_t index, uint32_t divisor) = 0;
	virtual void setIndexBuffer(BackendBuffer* buffer) = 0;

public:
	static BackendVertexArray* create();
	static void destroy(BackendVertexArray* vertexArray);
};

} // namespace axgl

#endif // __BackendVertexArray_h_
