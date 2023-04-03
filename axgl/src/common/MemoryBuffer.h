// MemoryBuffer.h
#ifndef __MemoryBuffer_h_
#define __MemoryBuffer_h_

#include "../common/axglCommon.h"

namespace axgl {

// 汎用的なCPUメモリのバッファクラス
class MemoryBuffer
{
public:
    MemoryBuffer();
    ~MemoryBuffer();
    bool resize(size_t size);
    void releaseResources();
    size_t getSize() const { return m_size; }
    const uint8_t* getPointer() const { return m_data; }
    uint8_t* getPointer() { return m_data; }

private:
    size_t m_size = 0;
    uint8_t* m_data = nullptr;
};

} // namespace axgl

#endif // __MemoryBuffer_h_
