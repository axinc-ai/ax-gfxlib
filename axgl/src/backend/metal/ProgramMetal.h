// ProgramMetal.h
#ifndef __ProgramMetal_h_
#define __ProgramMetal_h_

#include "BackendMetal.h"
#include "../BackendProgram.h"
#include "../../AXGLString.h"

namespace axgl {

class ProgramSpirvMsl;
class BackendContext;
class ContextMetal;
class ShaderMetal;

class ProgramMetal : public BackendProgram
{
public:
	ProgramMetal();
	virtual ~ProgramMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual void terminate(BackendContext* context) override;
	virtual bool link(BackendContext* context, BackendShader* vs, BackendShader* fs) override;
	virtual int32_t getNumActiveAttribs() const override;
	virtual int32_t getNumActiveUniforms() const override;
	virtual int32_t getNumActiveUniformBlocks() const override;
	virtual int32_t getNumFragData() const override;
	virtual const char* getActiveVertexAttribName(int32_t index) const override;
	virtual const char* getActiveUniformName(int32_t index) const override;
	virtual const char* getActiveUniformBlockName(int32_t index) const override;
	virtual const char* getFragDataName(int32_t index) const override;
	virtual int32_t getActiveVertexAttribIndex(int32_t index) const override;
	virtual bool getActiveVertexAttrib(int32_t index, ShaderVariable* attrib) const override;
	virtual bool getActiveUniform(int32_t index, ShaderUniform* uniform) const override;
	virtual bool getActiveUniformBlock(BackendContext* context, int32_t index, ShaderUniformBlock* uniformBlock) const override;
	virtual uint32_t getUniformBlockBinding(int32_t index) const override;
	virtual int32_t getFragDataLocation(int32_t index) const override;
	virtual bool setUniformf(GLenum type, int32_t index, int32_t num, const float* value) override;
	virtual bool setUniformi(GLenum type, int32_t index, int32_t num, const int32_t* value) override;
	virtual bool setUniformui(GLenum type, int32_t index, int32_t num, const uint32_t* value) override;
	virtual bool setUniformSampler(int32_t index, int32_t value) override;
	virtual bool setUniformBlockBinding(int32_t index, uint32_t binding) override;
	virtual void setAttribLocation(int32_t index, int32_t location) override;

public:
	// 描画に使用するサンプラパラメータ
	struct SamplerDrawParams {
		int32_t unit = -1;
		int32_t type = 0;
	};

public:
	id<MTLFunction> getVertexFunction() const;
	id<MTLFunction> getFragmentFunction() const;
	const void* getGlobalBlockMemory() const;
	size_t getGlobalBlockSize() const;
	int32_t getGlobalBlockMetalIndex(int32_t type) const;
	const SamplerDrawParams* getSamplerDrawParams() const;
	const int32_t* getUniformBlockBinding() const;
	const int32_t* getVsUniformBlockMetalIndices() const;
	const int32_t* getFsUniformBlockMetalIndices() const;
	const int32_t* getAttribLocations() const;
	bool isProgramDirty() const;
	bool isUniformBlockBindingDirty() const;
	bool isSamplerUnitDirty() const;
	void clearDirty();
	const int32_t* getVsTextureSamplerMetalIndices() const;
	const int32_t* getFsTextureSamplerMetalIndices() const;
	int32_t getNumSampler() const;

private:
	bool setupGlobalBuffers(ContextMetal* context, ShaderMetal* vs, ShaderMetal* fs);
	void releaseGlobalBuffers();
	
private:
	ProgramSpirvMsl* m_pProgramMsl = nullptr;
	// unit index for sampler
	SamplerDrawParams m_samplerParams[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
	// Metal texture/sampler indices
	int32_t m_vsTextureSamplerIndices[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
	int32_t m_fsTextureSamplerIndices[AXGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
	// binding index for uniform block
	int32_t m_uniformBlockBinding[AXGL_MAX_UNIFORM_BUFFER_BINDINGS];
	// Metal buffer indices for uniform block
	int32_t m_vsUniformBlockIndices[AXGL_MAX_UNIFORM_BUFFER_BINDINGS];
	int32_t m_fsUniformBlockIndices[AXGL_MAX_UNIFORM_BUFFER_BINDINGS];
	// vertex attribute locations
	int32_t m_attribLocations[AXGL_MAX_VERTEX_ATTRIBS];
	bool m_programDirty = false;
	bool m_uniformBlockBindingDirty = false;
	bool m_samplerUnitDirty = false;
	id<MTLLibrary> m_vsLibrary = nil;
	id<MTLLibrary> m_fsLibrary = nil;
	id<MTLFunction> m_vsFunction = nil;
	id<MTLFunction> m_fsFunction = nil;
	uint32_t m_globalBlockIndex = 0;
	void* m_globalBlockMemory = nullptr;
	size_t m_globalBlockSize = 0;
};

} // namespace axgl

#endif // __ProgramMetal_h_
