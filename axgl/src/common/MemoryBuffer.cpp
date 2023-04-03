// MemoryBuffer.cpp
#include "MemoryBuffer.h"
#include "../AXGLAllocatorImpl.h"

namespace axgl {

MemoryBuffer::MemoryBuffer()
{
}

MemoryBuffer::~MemoryBuffer()
{
    releaseResources();
}

bool MemoryBuffer::resize(size_t size)
{
    if (size == 0) {
        if (m_data != nullptr) {
            AXGL_FREE(m_data);
            m_data = nullptr;
        }
        m_size = 0;
        return true;
    }
    if (size == m_size) {
        return true;
    }
    uint8_t* new_mem = static_cast<uint8_t*>(AXGL_ALLOC(sizeof(uint8_t) * size));
    if (new_mem == nullptr) {
        return false;
    }
    if (m_data != nullptr) {
        size_t copy_size = (size < m_size) ? size : m_size;
        memcpy(new_mem, m_data, copy_size);
        AXGL_FREE(m_data);
    }
    m_data = new_mem;
    m_size = size;
    return true;
}

void MemoryBuffer::releaseResources()
{
    if (m_data != nullptr) {
        AXGL_FREE(m_data);
        m_data = nullptr;
    }
    return;
}

} // namespace axgl
