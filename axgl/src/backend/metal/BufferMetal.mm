// BufferMetal.mm
#include "BufferMetal.h"
#include "ContextMetal.h"
#include "../../AXGLAllocatorImpl.h"

#include <algorithm>

namespace axgl {

static constexpr size_t DYNAMIC_BUFFER_SIZE_THRESHOLD = 4096;

// BackendBufferクラスの実装 --------
BackendBuffer* BackendBuffer::create()
{
	BufferMetal* buffer = AXGL_NEW(BufferMetal);
	return buffer;
}
	
void BackendBuffer::destroy(BackendBuffer* buffer)
{
	if (buffer == nullptr) {
		return;
	}
	AXGL_DELETE(buffer);
	return;
}

// BufferMetalクラスの実装 --------
BufferMetal::BufferMetal()
{
}

BufferMetal::~BufferMetal()
{
}

bool BufferMetal::initialize(BackendContext* context)
{
	AXGL_UNUSED(context);
	// 念のため初期値でクリア
	m_mtlBufferDirty = false;
	m_mapAccessFlags = 0;
	m_mapOffset = 0;
	m_mapLength = 0;
	m_dirtyStart = 0;
	m_dirtyEnd = 0;
	m_u8u16ConversionMode = false;
	m_setDataSize = 0;
	m_convertedStride = UINT32_MAX;
	m_convertedMode = ConversionModeNone;
	m_convertedOffset = 0;
	m_convertedSize = 0;
	m_convertedFirst = 0;
	m_convertedCount = 0;
	m_shadowBufferState = SHADOW_BUFFER_STATE_INITIAL;
	return true;
}

void BufferMetal::terminate(BackendContext* context)
{
	AXGL_UNUSED(context);
	m_shadowBuffer.releaseResources();
	m_srcBuffer = nil;
	m_mtlBuffer = nil;
	m_mtlBufferDirty = true;
	m_mapAccessFlags = 0;
	m_mapOffset = 0;
	m_mapLength = 0;
	return;
}

bool BufferMetal::setData(BackendContext* context, GLsizeiptr size, const void* data, GLenum usage)
{
	AXGL_ASSERT(context != nullptr);
	ContextMetal* mtl_context = static_cast<ContextMetal*>(context);
	const uint8_t* src_data = static_cast<const uint8_t*>(data);
	setupMTLBuffer(mtl_context, size, src_data);
	if (usage == GL_STATIC_DRAW) {
		// GL_STATIC_DRAWが指定されている場合は、基本的にバッファを書き換えないはず
		// シャドウバッファが必要になった時に作成する
		m_shadowBufferState = SHADOW_BUFFER_STATE_RESERVED;
	} else {
		// 他のusageはシャドウバッファを作成
		setupShadowBuffer(size, src_data);
	}
	m_setDataSize = size;
	m_usage = usage;
	// 変換情報をクリア
	m_convertedMode = ConversionModeNone;
	m_convertedStride = UINT32_MAX;
	m_convertedOffset = 0;
	m_convertedSize = 0;
	m_convertedFirst = 0;
	m_convertedCount = 0;
	return true;
}

bool BufferMetal::setSubData(BackendContext* context, GLintptr offset, GLsizeiptr size, const void* data)
{
	if (data == nullptr) {
		return true;
	}
	// シャドウバッファ未作成の場合は作成
	setupShadowBufferForReserved();
	// サイズによるオフセットチェック
	size_t shadow_size = m_shadowBuffer.getSize();
	if (offset >= shadow_size) {
		return false;
	}
	// シャドウバッファに対してコピー
	const uint8_t* src_data = static_cast<const uint8_t*>(data);
	size_t shadow_remain = shadow_size - offset;
	size_t copy_size = (size < shadow_remain) ? size : shadow_remain;
	std::copy(src_data, src_data + copy_size, m_shadowBuffer.getPointer() + offset);
	// ダーティ領域を更新
	if (m_dirtyStart == m_dirtyEnd) {
		m_dirtyStart = offset;
		m_dirtyEnd = offset + size;
	} else {
		m_dirtyStart = std::min(m_dirtyStart, offset);
		m_dirtyEnd = std::max(m_dirtyEnd, offset + size);
	}
	// 変換情報をクリアしておく
	m_convertedMode = ConversionModeNone;
	m_convertedStride = UINT32_MAX;
	m_convertedOffset = 0;
	m_convertedSize = 0;
	m_convertedFirst = 0;
	m_convertedCount = 0;
	return true;
}

bool BufferMetal::mapRange(BackendContext* context, GLintptr offset, GLsizeiptr length, GLenum access, void** mapPointer)
{
	// 現サポート範囲ではGPUが書き換えることはなく、GL_MAP_READ_BITも本実装でマップする
	AXGL_ASSERT((context != nullptr) && (mapPointer != nullptr));
	AXGL_ASSERT(m_mapAccessFlags == 0);
	// シャドウバッファ未作成の場合は作成する
	setupShadowBufferForReserved();
	if ((offset + length) > m_shadowBuffer.getSize()) {
		return false; // レンジが正しくない
	}
	*mapPointer = m_shadowBuffer.getPointer() + offset;
	// アンマップ処理のために保持
	m_mapAccessFlags = access;
	m_mapOffset = offset;
	m_mapLength = length;
	return true;
}

bool BufferMetal::unmap(BackendContext* context)
{
	AXGL_UNUSED(context);
	if (m_mapAccessFlags & GL_MAP_WRITE_BIT) {
		if (m_dirtyStart == m_dirtyEnd) {
			m_dirtyStart = m_mapOffset;
			m_dirtyEnd = m_mapOffset + m_mapLength;
		} else {
			m_dirtyStart = std::min(m_dirtyStart, m_mapOffset);
			m_dirtyEnd = std::max(m_dirtyEnd, m_mapOffset + m_mapLength);
		}
		m_mapAccessFlags = 0;
	}
	// 変換情報をクリアしておく
	m_convertedMode = ConversionModeNone;
	m_convertedStride = UINT32_MAX;
	m_convertedOffset = 0;
	m_convertedSize = 0;
	m_convertedFirst = 0;
	m_convertedCount = 0;
	return true;
}

bool BufferMetal::flushMappedRange(BackendContext* context, GLintptr offset, GLsizeiptr length)
{
	AXGL_UNUSED(context);
	if (m_mapAccessFlags & GL_MAP_WRITE_BIT) {
		if (m_dirtyStart == m_dirtyEnd) {
			m_dirtyStart = offset;
			m_dirtyEnd = offset + length;
		} else {
			m_dirtyStart = std::min(m_dirtyStart, offset);
			m_dirtyEnd = std::max(m_dirtyEnd, offset + length);
		}
	}
	// 変換情報をクリアしておく
	m_convertedMode = ConversionModeNone;
	m_convertedStride = UINT32_MAX;
	m_convertedOffset = 0;
	m_convertedSize = 0;
	m_convertedFirst = 0;
	m_convertedCount = 0;	
	return true;
}

id<MTLBuffer> BufferMetal::getMtlBuffer() const
{
	return m_mtlBuffer;
}

void BufferMetal::setU8U16ConversionMode()
{
	m_u8u16ConversionMode = true;
}

bool BufferMetal::setupBufferInDraw(BackendContext* context, ConversionMode conversion, intptr_t offset, intptr_t size)
{
	AXGL_ASSERT(context != nullptr);
	ContextMetal* mtl_context = static_cast<ContextMetal*>(context);
	if (conversion != ConversionModeNone) {
		// データ内容の変換をともなうバッファセットアップを呼び出す
		return setupWithDataConversion(mtl_context, conversion, offset, size);
	}
	// ダーティ領域がない場合
	if (m_dirtyStart == m_dirtyEnd) {
		// uint8のインデックスとして使う場合は変換処理を行う
		if (m_u8u16ConversionMode && (m_mtlBuffer.length != (m_shadowBuffer.getSize() * 2))) {
			// shadow buffer 未作成の場合は作成する
			setupShadowBufferForReserved();
			//　m_shadowBufferをuint16変換したものを、m_mtlBufferに設定
			const uint32_t num_indices = static_cast<uint32_t>(m_shadowBuffer.getSize());
			setupMTLBuffer(static_cast<ContextMetal*>(context), sizeof(uint16_t) * num_indices, nullptr);
			const uint8_t* u8_src = m_shadowBuffer.getPointer();
			uint16_t* u16_dst = static_cast<uint16_t*>([m_mtlBuffer contents]);
			for (uint32_t i = 0; i < num_indices; i++) {
				u16_dst[i] = u8_src[i];
			}
		}
		return true;
	}
	// シャドウバッファからBlitの転送元にコピーする
	{
		// shadow buffer 未作成の場合は作成する
		setupShadowBufferForReserved();
		// NOTE: m_srcBufferは常に作り直すほうが良いかも
		if (m_srcBuffer != nil) {
			uint8_t* dst = static_cast<uint8_t*>([m_srcBuffer contents]);
			if (dst != nullptr) {
				const uint8_t* src = m_shadowBuffer.getPointer();
				if (!m_u8u16ConversionMode) {
					memcpy(dst + m_dirtyStart, src + m_dirtyStart, m_dirtyEnd - m_dirtyStart);
				} else {
					// uint16に変換しつつ転送
					uint16_t* dst_u16 = reinterpret_cast<uint16_t*>(dst);
					for (intptr_t i = m_dirtyStart; i < m_dirtyEnd; i++) {
						dst_u16[i] = src[i];
					}
				}
			}
		} else {
			id<MTLDevice> mtlDevice = mtl_context->getDevice();
			const uint8_t* data_ptr = m_shadowBuffer.getPointer();
			intptr_t data_size = m_shadowBuffer.getSize();
			if (!m_u8u16ConversionMode) {
				m_srcBuffer = [mtlDevice newBufferWithBytes:data_ptr length:data_size options:MTLResourceStorageModeShared];
			} else {
				// uint16に変換したBlit転送元を作成
				const uint32_t num_indices = static_cast<uint32_t>(m_shadowBuffer.getSize());
				m_srcBuffer = [mtlDevice newBufferWithLength:data_size * sizeof(uint16_t) options:MTLResourceStorageModeShared];
				const uint8_t* u8_src = m_shadowBuffer.getPointer();
				uint16_t* u16_dst = static_cast<uint16_t*>([m_srcBuffer contents]);
				for (uint32_t i = 0; i < num_indices; i++) {
					u16_dst[i] = u8_src[i];
				}
			}
		}
	}
	// uint8から変換した場合の対処
	if (m_u8u16ConversionMode) {
		if (m_mtlBuffer.length != m_srcBuffer.length) {
			// 転送先がuint16のサイズでない場合は作り直す
			id<MTLDevice> mtlDevice = mtl_context->getDevice();
			MTLResourceOptions options = MTLResourceStorageModePrivate;
			m_mtlBuffer = [mtlDevice newBufferWithLength:m_srcBuffer.length options:options];
			m_mtlBufferDirty = true;
		}
		// ダーティ領域を調整
		m_dirtyStart *= 2;
		m_dirtyEnd *= 2;
	}
	id<MTLBlitCommandEncoder> command_encoder = mtl_context->getBlitCommandEncoder();
	[command_encoder copyFromBuffer:m_srcBuffer sourceOffset:m_dirtyStart toBuffer:m_mtlBuffer destinationOffset:m_dirtyStart size:(m_dirtyEnd - m_dirtyStart)];
	// clear dirty area
	m_dirtyStart = 0;
	m_dirtyEnd = 0;

	return true;
}

bool BufferMetal::needUpdateWithStrideConversion(uint32_t stride, uint32_t convertedStride) const
{
	bool rval = false;
	AXGL_ASSERT(m_convertedMode == ConversionModeNone);
	if ((m_dirtyStart != m_dirtyEnd) || (m_convertedStride != convertedStride)) {
		rval = true; // 変換済みのストライドと異なる場合
	}
	return rval;
}

bool BufferMetal::needUpdateWithStrideAndDataConversion(uint32_t stride, uint32_t convertedStride, ConversionMode conversion, GLint first, GLsizei count) const
{
	bool rval = false;
	if (conversion == ConversionModeNone) {
		// データ変換が無い場合、ストライド処理のメソッド
		rval = needUpdateWithStrideConversion(stride, convertedStride);
	} else {
		if ((m_convertedMode != conversion) || (convertedStride != m_convertedStride)
			|| (first != m_convertedFirst) || (count != m_convertedCount)) {
			rval = true; // VBOフォーマット変換済みデータと異なる場合
		}
	}
	return rval;
}

bool BufferMetal::needUpdateWithoutConversion() const
{
	AXGL_ASSERT(m_convertedMode == ConversionModeNone);
	return (m_dirtyStart != m_dirtyEnd);
}

int BufferMetal::needUpdateWithIndexConversion(ConversionMode conversion, intptr_t iboOffset, intptr_t iboSize, bool isUbyte) const
{
	int rval = NoUpdateRequired;
	if (conversion != ConversionModeNone) {
		if ((conversion != m_convertedMode) || (m_dirtyStart != m_dirtyEnd)
			|| (m_convertedOffset != iboOffset) || (m_convertedSize != iboSize)) {
			// IBOフォーマット変換済みのインデックスと異なる場合: バッファを新たに作成するため Blit は行わない
			rval = UpdateRequiredWithoutBlitCommand;
		}
	} else {
		if (m_dirtyStart != m_dirtyEnd) {
			// バッファが書き換えられている場合: 既存バッファの書き換えを行うため Blit を行う
			rval = UpdateRequiredWithBlitCommand;
		} else if (isUbyte && (m_mtlBuffer.length != (m_shadowBuffer.getSize() * 2))) {
			// Ubyteインデックスで未変換の場合: バッファをリサイズして再作成するため Blit は行わない
			rval = UpdateRequiredWithoutBlitCommand;
		}
	}
	return rval;
}

bool BufferMetal::setupBufferWithStrideConversion(ContextMetal* context, uint32_t stride, uint32_t convertedStride)
{
	if ((m_dirtyStart == m_dirtyEnd) && (m_convertedStride == convertedStride)) {
		return true;
	}
	AXGL_ASSERT(context != nullptr);
	// シャドウバッファ未作成の場合は作成する
	setupShadowBufferForReserved();
	// 古いバッファをリリース
	m_mtlBuffer = nil;
	m_mtlBufferDirty = true;
	// 拡大方向のみ正しく機能する
	AXGL_ASSERT(convertedStride > stride);
	// 変換後のデータサイズを算出
	intptr_t copy_count = m_setDataSize / stride;
	intptr_t converted_size = copy_count * convertedStride;
	// ワークバッファを確保
	void* work_buf = AXGL_ALLOC(converted_size);
	bool result = false;
	if (work_buf != nullptr) {
		// ストライド変換しながらコピー
		const uint8_t* sp = m_shadowBuffer.getPointer();
		AXGL_ASSERT(sp != nullptr);
		uint8_t* dp = static_cast<uint8_t*>(work_buf);
		for (intptr_t i = 0; i < copy_count; i++) {
			memcpy(dp, sp, stride);
			sp += stride;
			dp += convertedStride;
		}
		// 変換後のデータでバッファを作成
		id<MTLDevice> mtlDevice = context->getDevice();
		AXGL_ASSERT(mtlDevice != nil);
		m_mtlBuffer = [mtlDevice newBufferWithBytes:work_buf length:converted_size options:MTLResourceStorageModeShared];
		if (m_mtlBuffer != nil) {
			m_convertedStride = convertedStride;
			result = true;
		}
		AXGL_ASSERT(m_mtlBuffer != nil);
		// ワークを解放
		AXGL_FREE(work_buf);
	} else {
		AXGL_ASSERT(0);
	}
	return result;
}

bool BufferMetal::setupWithStrideAndDataConversion(ContextMetal* context,
	uint32_t stride, uint32_t convertedStride, ConversionMode conversion, GLint first, GLsizei count)
{
	if (conversion == ConversionModeNone) {
		// データ変換が無い場合、ストライド処理のみのメソッドを呼び出して終了
		return setupBufferWithStrideConversion(context, stride, convertedStride);
	}
	if ((m_convertedMode == conversion) && (convertedStride == m_convertedStride) && (first == m_convertedFirst) && (count == m_convertedCount)) {
		return true; // 変換済み
	}
	// ここに到達するのは頂点データのTriangleFan変換のみ
	AXGL_ASSERT(conversion == ConversionModeTriFanVertices);
	if (count < 3) {
		return false; // 描画されないので変換不要
	}
	// TriangleFanの頂点データ書き換え
	{
		// shadow buffer 未作成の場合は作成する
		setupShadowBufferForReserved();
		// 古いバッファをリリース
		AXGL_ASSERT(context != nullptr);
		id<MTLDevice> mtl_device = context->getDevice();
		AXGL_ASSERT(mtl_device != nil);
		m_mtlBuffer = nil;
		m_mtlBufferDirty = true;
		// トライアングル数
		int32_t num_triangle = count - 2;
		AXGL_ASSERT(num_triangle > 0);
		// 変換後データサイズでバッファを作成
		uint32_t converted_size = 3 * convertedStride * num_triangle;
		m_mtlBuffer = [mtl_device newBufferWithLength:converted_size options:MTLResourceStorageModeShared];
		AXGL_ASSERT(m_mtlBuffer != nil);
		uint8_t* dst_buffer = static_cast<uint8_t*>([m_mtlBuffer contents]);
		AXGL_ASSERT(dst_buffer != nullptr);
		// シャドウバッファを変換元として参照
		const uint8_t* sp = reinterpret_cast<uint8_t*>(m_shadowBuffer.getPointer() + (stride * first));
		AXGL_ASSERT(sp != nullptr);
		const uint8_t* first_sp = sp;
		// 変換済みの頂点は常にバッファ先頭から格納
		uint8_t* dp = dst_buffer;
		// 最初の三角形
		memcpy(dp, sp, stride);
		sp += stride;
		dp += convertedStride;
		memcpy(dp, sp, stride);
		sp += stride;
		dp += convertedStride;
		memcpy(dp, sp, stride);
		dp += convertedStride;
		// 2番目以降の三角形
		for (int32_t j = 1; j < num_triangle; j++) {
			memcpy(dp, first_sp, stride);
			dp += convertedStride;
			memcpy(dp, sp, stride);
			sp += stride;
			dp += convertedStride;
			memcpy(dp, sp, stride);
			dp += convertedStride;
		}
	}
	// 変換情報を保持
	m_convertedMode   = conversion;
	m_convertedStride = convertedStride;
	m_convertedFirst  = first;
	m_convertedCount  = count;
	return true;
}

bool BufferMetal::isMTLBufferDirty() const
{
	return m_mtlBufferDirty;
}

void BufferMetal::clearMTLBufferDirty()
{
	m_mtlBufferDirty = false;
	return;
}

bool BufferMetal::isDynamicBuffer() const
{
	if (m_mtlBuffer == nil) {
		return false;
	}
	// DYNAMIC_DRAWかつ閾値以下の場合に、動的なバッファとして扱う
	return (m_usage == GL_DYNAMIC_DRAW) && (m_mtlBuffer.length <= DYNAMIC_BUFFER_SIZE_THRESHOLD);
}

size_t BufferMetal::getBufferDataSize() const
{
	size_t data_size = 0;
	if (m_shadowBufferState == SHADOW_BUFFER_STATE_CREATED) {
		// シャドウバッファ作成済みでオリジナルデータを保持
		data_size = m_shadowBuffer.getSize();
	} else if (m_shadowBufferState == SHADOW_BUFFER_STATE_RESERVED) {
		// シャドウバッファ未作成でMTLBufferがオリジナルデータ
		data_size = m_mtlBuffer.length;
	}
	return data_size;
}

void BufferMetal::copyToDynamicBuffer(id<MTLBuffer> dynamicBuffer, size_t offset) const
{
	if (dynamicBuffer == nil) {
		return;
	}
	// GPUと競合しない領域を書き換えるため同期等は行わない
	uint8_t* dst = static_cast<uint8_t*>([dynamicBuffer contents]);
	size_t data_size = m_shadowBuffer.getSize();
	const uint8_t* src = m_shadowBuffer.getPointer();
	AXGL_ASSERT((dst != nullptr) && (src != nullptr));
	dst += offset;
	AXGL_ASSERT((offset + data_size) <= [dynamicBuffer length]);
	memcpy(dst, src, data_size);
	return;
}

//--------
bool BufferMetal::setupMTLBuffer(ContextMetal* context, size_t size, const uint8_t* data)
{
	id<MTLBuffer> mtlBuffer;
	id<MTLDevice> mtlDevice = context->getDevice();
	if (data != nullptr) {
		mtlBuffer = [mtlDevice newBufferWithBytes:data length:size options:MTLResourceStorageModeShared];
	} else {
		// 後でシャドウバッファ作成時にCPUが読み出す可能性があるためSharedで作成
		mtlBuffer = [mtlDevice newBufferWithLength:size options:MTLResourceStorageModeShared];
	}
	if (mtlBuffer == nil) {
		return false;
	}
	m_mtlBuffer = mtlBuffer;
	m_mtlBufferDirty = true;
	return true;
}

bool BufferMetal::setupShadowBuffer(size_t size, const uint8_t* data)
{
	AXGL_ASSERT(size > 0);
	// シャドウバッファをリサイズ
	size_t buf_size = m_shadowBuffer.getSize();
	if ((buf_size == 0) || (size > buf_size)) {
		if (!m_shadowBuffer.resize(size)) {
			return false;
		}
	}
	// データコピー
	if (data != nullptr) {
		std::copy(data, data + size, m_shadowBuffer.getPointer());
	}
	// 状態を作成済みに変更
	m_shadowBufferState = SHADOW_BUFFER_STATE_CREATED;
	return true;
}

bool BufferMetal::setupShadowBufferForReserved()
{
	if (m_shadowBufferState != SHADOW_BUFFER_STATE_RESERVED) {
		// シャドウバッファ作成済みの場合は、何もせずに終了
		return true;
	}
	// 設定データサイズのシャドウバッファを確保
	size_t buf_size = m_shadowBuffer.getSize();
	if ((buf_size == 0) || (m_setDataSize > buf_size)) {
		if (!m_shadowBuffer.resize(m_setDataSize)) {
			return false;
		}
	}
	// 描画に使用するMTLBufferを必ずSharedで作成しているためコピーする
	AXGL_ASSERT(m_mtlBuffer != nil);
	const void* src = [m_mtlBuffer contents];
	void* dst = m_shadowBuffer.getPointer();
	AXGL_ASSERT((src != nullptr) && (dst != nullptr));
	memcpy(dst, src, m_setDataSize);
	// 状態を作成済みに変更
	m_shadowBufferState = SHADOW_BUFFER_STATE_CREATED;
	return true;
}

bool BufferMetal::setupWithDataConversion(ContextMetal* context,
	ConversionMode conversion, intptr_t offset, intptr_t size)
{
	if ((conversion == m_convertedMode) && (m_dirtyStart == m_dirtyEnd)
		&& (m_convertedOffset == offset) && (m_convertedSize == size)) {
		return true;
	}
	AXGL_ASSERT(conversion != ConversionModeNone);
	// MetalがサポートしないTriangleFanのインデックスを変換する処理
	bool result = true;
	switch (conversion) {
	case ConversionModeTriFanIndices8:
		result = convertTriFanIndices8(context, offset, size);
		break;
	case ConversionModeTriFanIndices16:
		result = convertTriFanIndices16(context, offset, size);
		break;
	case ConversionModeTriFanIndices32:
		result = convertTriFanIndices32(context, offset, size);
		break;
	default:
		break;
	}
	return result;
}

bool BufferMetal::convertTriFanIndices8(ContextMetal* context, intptr_t offset, intptr_t size)
{
	// シャドウバッファ未作成の場合は作成する
	setupShadowBufferForReserved();
	AXGL_ASSERT(context != nullptr);
	id<MTLDevice> mtl_device = context->getDevice();
	// 古いバッファをリリース
	m_mtlBuffer = nil;
	m_mtlBufferDirty = true;
	// トライアングル数
	int32_t num_triangle = static_cast<int32_t>(size / sizeof(uint8_t)) - 2;
	AXGL_ASSERT(num_triangle > 0);
	// 変換後のデータサイズ
	uint32_t converted_size = 3 * sizeof(uint16_t) * num_triangle;
	// バッファを確保
	m_mtlBuffer = [mtl_device newBufferWithLength:converted_size options:MTLResourceStorageModeShared];
	AXGL_ASSERT(m_mtlBuffer != nil);
	uint16_t* dst_buffer = static_cast<uint16_t*>([m_mtlBuffer contents]);
	AXGL_ASSERT(dst_buffer != nullptr);
	// シャドウバッファから変換しながら格納
	{
		// 正しいパラメータならバッファの範囲を越えない
		AXGL_ASSERT(((offset + size) <= m_setDataSize) && (offset + size) <= m_shadowBuffer.getSize());
		const uint8_t* sp = reinterpret_cast<uint8_t*>(m_shadowBuffer.getPointer() + offset);
		AXGL_ASSERT(sp != nullptr);
		// 変換されたデータは必ずバッファ先頭から格納
		uint16_t* dp = dst_buffer;
		// 最初の三角形
		uint16_t center_index = static_cast<uint16_t>(sp[0]);
		dp[0] = center_index;
		dp[1] = static_cast<uint16_t>(sp[1]);
		dp[2] = static_cast<uint16_t>(sp[2]);
		sp += 2;
		dp += 3;
		// 2番目以降の三角形
		for (int32_t i = 1; i < num_triangle; i++) {
			dp[0] = center_index;
			dp[1] = static_cast<uint16_t>(sp[0]);
			dp[2] = static_cast<uint16_t>(sp[1]);
			sp++;
			dp += 3;
		}
	}
	// 変換情報を保持
	m_convertedMode = ConversionModeTriFanIndices8;
	m_convertedOffset = offset;
	m_convertedSize = size;
	return true;
}

bool BufferMetal::convertTriFanIndices16(ContextMetal* context, intptr_t offset, intptr_t size)
{
	// シャドウバッファ未作成の場合は作成する
	setupShadowBufferForReserved();
	AXGL_ASSERT(context != nullptr);
	id<MTLDevice> mtl_device = context->getDevice();
	// 古いバッファをリリース
	m_mtlBuffer = nil;
	m_mtlBufferDirty = true;
	// トライアングル数
	int32_t num_triangle = static_cast<int32_t>(size / sizeof(uint16_t)) - 2;
	AXGL_ASSERT(num_triangle > 0);
	// 変換後のデータサイズ
	uint32_t converted_size = 3 * sizeof(uint16_t) * num_triangle;
	// バッファを確保
	m_mtlBuffer = [mtl_device newBufferWithLength:converted_size options:MTLResourceStorageModeShared];
	AXGL_ASSERT(m_mtlBuffer != nil);
	uint16_t* dst_buffer = static_cast<uint16_t*>([m_mtlBuffer contents]);
	AXGL_ASSERT(dst_buffer != nullptr);
	// シャドウバッファから変換しながら格納
	{
		// 正しいパラメータならバッファの範囲を越えない
		AXGL_ASSERT(((offset + size) <= m_setDataSize) && (offset + size) <= m_shadowBuffer.getSize());
		const uint16_t* sp = reinterpret_cast<uint16_t*>(m_shadowBuffer.getPointer() + offset);
		AXGL_ASSERT(sp != nullptr);
		// 変換されたデータは必ずバッファ先頭から格納
		uint16_t* dp = dst_buffer;
		// 最初の三角形
		uint16_t center_index = sp[0];
		dp[0] = center_index;
		dp[1] = sp[1];
		dp[2] = sp[2];
		sp += 2;
		dp += 3;
		// 2番目以降の三角形
		for (int32_t i = 1; i < num_triangle; i++) {
			dp[0] = center_index;
			dp[1] = sp[0];
			dp[2] = sp[1];
			sp++;
			dp += 3;
		}
	}
	// 変換情報を保持
	m_convertedMode = ConversionModeTriFanIndices16;
	m_convertedOffset = offset;
	m_convertedSize = size;
	return true;
}

bool BufferMetal::convertTriFanIndices32(ContextMetal* context, intptr_t offset, intptr_t size)
{
	// シャドウバッファ未作成の場合は作成する
	setupShadowBufferForReserved();
	AXGL_ASSERT(context != nullptr);
	id<MTLDevice> mtl_device = context->getDevice();
	// 古いバッファをリリース
	m_mtlBuffer = nil;
	m_mtlBufferDirty = true;
	// トライアングル数
	int32_t num_triangle = static_cast<int32_t>(size / sizeof(uint32_t)) - 2;
	AXGL_ASSERT(num_triangle > 0);
	// 変換後のデータサイズ
	uint32_t converted_size = 3 * sizeof(uint32_t) * num_triangle;
	// バッファを確保
	m_mtlBuffer = [mtl_device newBufferWithLength:converted_size options:MTLResourceStorageModeShared];
	AXGL_ASSERT(m_mtlBuffer != nil);
	uint32_t* dst_buffer = static_cast<uint32_t*>([m_mtlBuffer contents]);
	AXGL_ASSERT(dst_buffer != nullptr);
	// シャドウバッファから変換しながら格納
	{
		// 正しいパラメータならバッファの範囲を越えない
		AXGL_ASSERT(((offset + size) <= m_setDataSize) && (offset + size) <= m_shadowBuffer.getSize());
		const uint32_t* sp = reinterpret_cast<uint32_t*>(m_shadowBuffer.getPointer() + offset);
		AXGL_ASSERT(sp != nullptr);
		// 変換されたデータは必ずバッファ先頭から格納
		uint32_t* dp = dst_buffer;
		// 最初の三角形
		uint32_t center_index = sp[0];
		dp[0] = center_index;
		dp[1] = sp[1];
		dp[2] = sp[2];
		sp += 2;
		dp += 3;
		// 2番目以降の三角形
		for (int32_t i = 1; i < num_triangle; i++) {
			dp[0] = center_index;
			dp[1] = sp[0];
			dp[2] = sp[1];
			sp++;
			dp += 3;
		}
	}
	// 変換情報を保持
	m_convertedMode = ConversionModeTriFanIndices32;
	m_convertedOffset = offset;
	m_convertedSize = size;
	return true;
}

} // namespace axgl
