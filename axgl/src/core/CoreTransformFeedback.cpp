// CoreTransformFeedback.cpp
// TransformFeedbackクラスの実装
#include "CoreTransformFeedback.h"

namespace axgl {

CoreTransformFeedback::CoreTransformFeedback()
{
	m_objectType = TYPE_TRANSFORM_FEEDBACK;
}

CoreTransformFeedback::~CoreTransformFeedback()
{
}

void CoreTransformFeedback::pause()
{
	return;
}

void CoreTransformFeedback::resume()
{
	return;
}

} // namespace axgl
