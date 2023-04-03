// CoreObject.cpp
// オブジェクトクラスの実装
#include "CoreObject.h"
#include "../AXGLAllocatorImpl.h"

namespace axgl {

CoreObject::CoreObject()
{
}

CoreObject::~CoreObject()
{
}

void CoreObject::terminate(CoreContext* context)
{
	AXGL_UNUSED(context);
	return;
}

void CoreObject::addRef()
{
	AXGL_ASSERT(m_referenceCount <= UINT32_MAX);
	m_referenceCount++;
	return;
}

void CoreObject::release(CoreContext* context)
{
	AXGL_ASSERT(m_referenceCount > 0);
	m_referenceCount--;
	if (m_referenceCount == 0) {
		terminate(context);
		AXGL_DELETE(this);
	}
	return;
}

} // namespace axgl
