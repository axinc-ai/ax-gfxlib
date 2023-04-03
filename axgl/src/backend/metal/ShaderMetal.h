// ShaderMetal.h
#ifndef __ShaderMetal_h_
#define __ShaderMetal_h_
#include "BackendMetal.h"
#include "../BackendShader.h"

namespace axgl {

class ShaderSpirvMsl;

class ShaderMetal : public BackendShader
{
public:
	ShaderMetal();
	virtual ~ShaderMetal();
	virtual bool initialize(BackendContext* context, GLenum type) override;
	virtual void terminate(BackendContext* context) override;
	virtual bool compileSource(BackendContext* context, const char* source) override;

public:
	// SpirvMslのシェーダを取得
	ShaderSpirvMsl* getShaderMsl() const { return m_pShaderMsl; }

private:
	GLenum m_type = 0;
	ShaderSpirvMsl* m_pShaderMsl = nullptr;
};

} // namespace axgl

#endif // __ShaderMetal_h_
