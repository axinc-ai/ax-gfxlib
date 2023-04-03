// SamplerMetal.h
#ifndef __SamplerMetal_h_
#define __SamplerMetal_h_
#include "BackendMetal.h"
#include "../BackendSampler.h"

namespace axgl {

class ContextMetal;

class SamplerMetal : public BackendSampler
{
public:
	SamplerMetal();
	virtual ~SamplerMetal();
	virtual bool initialize(BackendContext* context) override;
	virtual bool terminate(BackendContext* context) override;
	virtual bool setupSampler(BackendContext* context, const SamplerParameters& params) override;

public:
	id<MTLSamplerState> getMtlSamplerState() const;
	MTLSamplerDescriptor* getMtlSamplerDescriptor() const;
	bool isDirty() const;
	void clearDirty();

private:
	MTLSamplerDescriptor* m_pSamplerDesc = nil;
	id<MTLSamplerState> m_samplerState = nil;
	bool m_dirty = false;
};

} // namespace axgl

#endif // __SamplerMetal_h_
