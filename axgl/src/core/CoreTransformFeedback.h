// CoreTransformFeedback.h
// TransformFeedbackクラスの宣言
#ifndef __CoreTransformFeedback_h_
#define __CoreTransformFeedback_h_

#include "CoreObject.h"

namespace axgl {

// TransformFeedbackクラス
class CoreTransformFeedback : public CoreObject
{
public:
	CoreTransformFeedback();
	~CoreTransformFeedback();
	void pause();
	void resume();

private:
	int m_dummy = 0;
};

} // namespace axgl

#endif // __CoreTransformFeedback_h_
