// CoreProgram.cpp
// Programクラスの実装
#include "CoreProgram.h"
#include "CoreShader.h"
#include "CoreContext.h"
#include "../backend/BackendProgram.h"
#include "../backend/BackendContext.h"

namespace axgl {

CoreProgram::CoreProgram()
{
	m_objectType = TYPE_PROGRAM;
}

CoreProgram::~CoreProgram()
{
}

void CoreProgram::attachShader(CoreShader* shader)
{
	if (shader == nullptr) {
		return;
	}
	GLenum type = shader->getType();
	switch (type) {
	case GL_VERTEX_SHADER:
		if (m_pVertexShader == nullptr) {
			shader->addRef();
			m_pVertexShader = shader;
		} else {
			// GL_INVALID_OPERATION is generated if a shader of the same type as shader is already attached to program.
			setErrorCode(GL_INVALID_OPERATION);
		}
		break;
	case GL_FRAGMENT_SHADER:
		if (m_pFragmentShader == nullptr) {
			shader->addRef();
			m_pFragmentShader = shader;
		} else {
			// GL_INVALID_OPERATION is generated if a shader of the same type as shader is already attached to program.
			setErrorCode(GL_INVALID_OPERATION);
		}
		break;
	default:
		break;
	}
	return;
}

void CoreProgram::detachShader(CoreContext* context, CoreShader* shader)
{
	if (shader == nullptr) {
		return;
	}
	GLenum type = shader->getType();
	switch (type) {
	case GL_VERTEX_SHADER:
		if (m_pVertexShader == shader) {
			shader->release(context);
			m_pVertexShader = nullptr;
		} else {
			// GL_INVALID_OPERATION is generated if shader is not attached to program.
			setErrorCode(GL_INVALID_OPERATION);
		}
		break;
	case GL_FRAGMENT_SHADER:
		if (m_pFragmentShader == shader) {
			shader->release(context);
			m_pFragmentShader = nullptr;
		} else {
			// GL_INVALID_OPERATION is generated if shader is not attached to program.
			setErrorCode(GL_INVALID_OPERATION);
		}
		break;
	default:
		break;
	}
	return;
}

void CoreProgram::getAttachedShaders(GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
	AXGL_ASSERT(shaders != nullptr);
	GLsizei cnt = 0;
	if ((m_pVertexShader != nullptr) && (cnt < maxCount)) {
		shaders[cnt] = m_pVertexShader->getId();
		cnt++;
	}
	if ((m_pFragmentShader != nullptr) && (cnt < maxCount)) {
		shaders[cnt] = m_pFragmentShader->getId();
		cnt++;
	}
	if (count != nullptr) {
		*count = cnt;
	}
	return;
}

void CoreProgram::bindAttribLocation(GLuint index, const GLchar* name)
{
	AXGL_ASSERT(name != nullptr);
	if (index >= AXGL_MAX_VERTEX_ATTRIBS) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	m_bindAttribMap.emplace(name, index);
	return;
}

GLint CoreProgram::getAttribLocation(const GLchar* name)
{
	AXGL_ASSERT(name != nullptr);
	auto it = m_attribLocations.find(name);
	if (it == m_attribLocations.end()) {
		return -1;
	}
	return it->second;
}

GLint CoreProgram::getUniformLocation(const GLchar* name)
{
	AXGL_ASSERT(name != nullptr);
	auto uniform_it = m_uniformLocations.find(name);
	if (uniform_it != m_uniformLocations.end()) {
		return uniform_it->second;
	}
	return -1;
}

GLint CoreProgram::getFragDataLocation(const GLchar* name)
{
	AXGL_ASSERT(name != nullptr);
	auto it = m_fragDataLocations.find(name);
	if (it == m_fragDataLocations.end()) {
		return -1;
	}
	return it->second;
}

void CoreProgram::getActiveAttrib(GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	AXGL_ASSERT((size != nullptr) && (type != nullptr) && (name != nullptr));
	if (index >= m_vertexShaderInputs.size()) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	if (m_pBackendProgram == nullptr) {
		return;
	}
	const ProgramVariable& attrib = m_vertexShaderInputs[index];
	// name
	size_t name_len = attrib.name.length();
	size_t copy_count = ((bufSize - 1) < name_len) ? (bufSize - 1) : name_len;
	strncpy(name, attrib.name.c_str(), copy_count);
	name[copy_count] = '\0';
	// size, type
	*size = attrib.info.m_Size;
	*type = attrib.info.m_Type;
	// length : nullable
	if (length != nullptr) {
		*length = static_cast<GLsizei>(copy_count);
	}
	return;
}

void CoreProgram::getActiveUniform(GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	AXGL_ASSERT((size != nullptr) && (type != nullptr) && (name != nullptr));
	const ProgramUniform* uniform = nullptr;
	GLuint n_uniform = static_cast<GLuint>(m_uniforms.size());
	if (index < n_uniform) {
		uniform = &m_uniforms[index];
	}
	if (uniform == nullptr) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	// name
	size_t name_len = uniform->name.length();
	size_t copy_count = ((bufSize - 1) < name_len) ? (bufSize - 1) : name_len;
	strncpy(name, uniform->name.c_str(), copy_count);
	name[copy_count] = '\0';
	// size, type
	*size = uniform->info.m_Size;
	*type = uniform->info.m_Type;
	// length : nullable
	if (length != nullptr) {
		*length = static_cast<GLsizei>(copy_count);
	}
	return;
}

void CoreProgram::getUniformIndices(GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
	AXGL_ASSERT((uniformNames != nullptr) && (uniformIndices != nullptr));
	for (int i = 0; i < uniformCount; i++) {
		auto it = m_uniformBlockIndices.find(uniformNames[i]);
		if (it != m_uniformBlockIndices.end()) {
			uniformIndices[i] = it->second;
		} else {
			uniformIndices[i] = GL_INVALID_INDEX;
		}
	}
	return;
}

void CoreProgram::getActiveUniformsiv(GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
	AXGL_ASSERT((uniformIndices != nullptr) && (params != nullptr));
	for (int i = 0; i < uniformCount; i++) {
		GLuint index = uniformIndices[i];
		GLuint n_uniform = static_cast<GLuint>(m_uniforms.size());
		if (index < n_uniform) {
			const ProgramUniform& uniform = m_uniforms[index];
			getShaderUniformParam(uniform, pname, &params[i]);
		}
	}
	return;
}

GLuint CoreProgram::getUniformBlockIndex(const GLchar* uniformBlockName)
{
	AXGL_ASSERT(uniformBlockName != nullptr);
	auto it = m_uniformBlockIndices.find(uniformBlockName);
	if (it == m_uniformBlockIndices.end()) {
		return GL_INVALID_INDEX;
	}
	return it->second;
}

void CoreProgram::getActiveUniformBlockiv(GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
	AXGL_ASSERT(params != nullptr);
	if (uniformBlockIndex >= m_uniformBlocks.size()) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const ProgramUniformBlock& program_ub = m_uniformBlocks[uniformBlockIndex];
	const BackendProgram::ShaderUniformBlock* ub = &program_ub.info;
	switch (pname) {
	case GL_UNIFORM_BLOCK_BINDING:
		// Backendで保持しているbindingを取得
		*params = m_pBackendProgram->getUniformBlockBinding(ub->m_NativeIndex);
		break;
	case GL_UNIFORM_BLOCK_DATA_SIZE:
		*params = ub->m_DataSize;
		break;
	case GL_UNIFORM_BLOCK_NAME_LENGTH:
	{
		*params = static_cast<GLint>(program_ub.name.length());
	}
	break;
	case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
		*params = ub->m_ActiveUniforms;
		break;
	case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
	{
		int n_out = 0;
		int cur_index = 0;
		for (const auto& uniform : m_uniforms) {
			const BackendProgram::ShaderUniform& info = uniform.info;
			if (info.m_IsUniformBlockMember == GL_TRUE) {
				params[n_out] = cur_index;
				n_out++;
			}
			cur_index++;
		}
		AXGL_ASSERT(n_out == ub->m_ActiveUniforms);
	}
	break;
	case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
		*params = GL_TRUE; // TODO: stage情報から
		break;
	case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
		*params = GL_TRUE; // TODO: stage情報から
		break;
	default:
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreProgram::getActiveUniformBlockName(GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
	AXGL_ASSERT((length != nullptr) && (uniformBlockName != nullptr));

	if (uniformBlockIndex >= m_uniformBlocks.size()) {
		setErrorCode(GL_INVALID_VALUE);
		return;
	}
	const ProgramUniformBlock& program_ub = m_uniformBlocks[uniformBlockIndex];
	size_t copy_len = program_ub.name.length();
	if ((bufSize - 1) < copy_len) {
		copy_len = bufSize - 1;
	}
	strncpy(uniformBlockName, program_ub.name.c_str(), copy_len);
	uniformBlockName[copy_len] = '\0';
	if (length != nullptr) {
		*length = static_cast<GLsizei>(copy_len);
	}
	return;
}

void CoreProgram::link(CoreContext* context)
{
	if ((context == nullptr) || (m_pBackendProgram == nullptr)) {
		return;
	}
	BackendShader* backend_vs = nullptr;
	BackendShader* backend_fs = nullptr;
	if (m_pVertexShader != nullptr) {
		backend_vs = m_pVertexShader->getBackendShader();
	}
	if (m_pFragmentShader != nullptr) {
		backend_fs = m_pFragmentShader->getBackendShader();
	}
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	bool link_result = m_pBackendProgram->link(backend_context, backend_vs, backend_fs);
	m_linkStatus = GL_FALSE;
	if (link_result) {
		// vertex attributes
		int32_t num_va = m_pBackendProgram->getNumActiveAttribs();
		m_vertexShaderInputs.resize(num_va);
		for (int32_t i = 0; i < num_va; i++) {
			ProgramVariable* dst = &m_vertexShaderInputs[i];
			const char* name = m_pBackendProgram->getActiveVertexAttribName(i);
			AXGL_ASSERT(name != nullptr);
			dst->name = name;
			bool result = m_pBackendProgram->getActiveVertexAttrib(i, &dst->info);
			AXGL_ASSERT(result);
			GLint location;
			if (dst->info.m_LayoutSpecified == GL_TRUE) {
				// layout qualifier で location が指定されている場合は優先
				location = m_pBackendProgram->getActiveVertexAttribIndex(i);
				AXGL_ASSERT(location != -1);
			} else {
				// location 指定されていない場合、glBindAttribLocation で指定されているか検索
				auto it = m_bindAttribMap.find(name);
				if (it != m_bindAttribMap.end()) {
					// 指定されている場合: 指定された値を使用
					location = it->second;
				} else {
					// 指定されていない場合: Backend側で指定した値を使用
					location = m_pBackendProgram->getActiveVertexAttribIndex(i);
					AXGL_ASSERT(location != -1);
				}
			}
			// vertex attribute location
			m_attribLocations.emplace(name, location);
			// attribute index array from location
			m_pBackendProgram->setAttribLocation(i, location);
		}
		// uniforms
		int32_t num_vu = static_cast<int>(m_pBackendProgram->getNumActiveUniforms());
		m_uniforms.resize(num_vu);
		for (int32_t i = 0; i < num_vu; i++) {
			// uniform を追加
			ProgramUniform* dst = &m_uniforms[i];
			const char* name = m_pBackendProgram->getActiveUniformName(i);
			AXGL_ASSERT(name != nullptr);
			dst->name = name;
			bool result = m_pBackendProgram->getActiveUniform(i, &dst->info);
			AXGL_ASSERT(result);
			// 名前とインデックスを関連付け
			m_uniformLocations.emplace(name, i);
		}
		// uniform blocks
		int32_t num_vub = m_pBackendProgram->getNumActiveUniformBlocks();
		if (num_vub > 0) {
			m_uniformBlocks.resize(num_vub);
			for (int32_t i = 0; i < num_vub; i++) {
				// get uniform block information
				ProgramUniformBlock* dst = &m_uniformBlocks[i];
				const char* name = m_pBackendProgram->getActiveUniformBlockName(i);
				AXGL_ASSERT(name != nullptr);
				dst->name = name;
				bool result = m_pBackendProgram->getActiveUniformBlock(backend_context, i, &dst->info);
				AXGL_ASSERT(result);
				// add to indices map
				m_uniformBlockIndices.emplace(name, i);
			}
		}
		// frag data
		int32_t num_frag_data = m_pBackendProgram->getNumFragData();
		if (num_frag_data > 0) {
			for (int32_t i = 0; i < num_frag_data; i++) {
				const char* name = m_pBackendProgram->getFragDataName(i);
				GLint loc = static_cast<GLint>(m_pBackendProgram->getFragDataLocation(i));
				m_fragDataLocations.emplace(name, loc);
			}
		}
		// link status : true
		m_linkStatus = GL_TRUE;
	}
	// TODO: m_TransformFeedbackVaryings が設定されている場合、
	//       m_TransformFeedbackOutputs の情報を作成する
	return;
}

void CoreProgram::setUniform1f(GLint location, GLfloat v0)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformf(GL_FLOAT, native_index, 1, &v0);
	}
	return;
}

void CoreProgram::setUniform1fv(GLint location, GLsizei count, const GLfloat* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformf(GL_FLOAT, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform1i(GLint location, GLint v0)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	// NOTE: glUniform1i() is used to set unit index for sampler uniform
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		if (uniform.info.m_IsSampler) {
			m_pBackendProgram->setUniformSampler(native_index, v0);
		} else {
			m_pBackendProgram->setUniformi(GL_INT, native_index, 1, &v0);
		}
	}
	return;
}

void CoreProgram::setUniform1iv(GLint location, GLsizei count, const GLint* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	// NOTE: glUniform1i() is used to set unit index for sampler uniform
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		if (uniform.info.m_IsSampler) {
			m_pBackendProgram->setUniformSampler(native_index, *value);
		} else {
			m_pBackendProgram->setUniformi(GL_INT, native_index, count, value);
		}
	}
	return;
}

void CoreProgram::setUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	GLfloat values[2] = {
		v0, v1
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformf(GL_FLOAT_VEC2, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformf(GL_FLOAT_VEC2, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform2i(GLint location, GLint v0, GLint v1)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	GLint values[2] = {
		v0, v1
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformi(GL_INT_VEC2, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform2iv(GLint location, GLsizei count, const GLint* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformi(GL_INT_VEC2, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	GLfloat values[3] = {
		v0, v1, v2
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformf(GL_FLOAT_VEC3, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformf(GL_FLOAT_VEC3, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	GLint values[3] = {
		v0, v1, v2
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformi(GL_INT_VEC3, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform3iv(GLint location, GLsizei count, const GLint* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformi(GL_INT_VEC3, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	const float values[4] = {
		v0, v1, v2, v3
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformf(GL_FLOAT_VEC4, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformf(GL_FLOAT_VEC4, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	const GLint values[4] = {
		v0, v1, v2, v3
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformi(GL_INT_VEC4, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform4iv(GLint location, GLsizei count, const GLint* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformi(GL_INT_VEC4, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT2, native_index, count, value);
	return;
}

void CoreProgram::setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT3, native_index, count, value);
	return;
}

void CoreProgram::setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT4, native_index, count, value);
	return;
}

void CoreProgram::setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT2x3, native_index, count, value);
	return;
}

void CoreProgram::setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT3x2, native_index, count, value);
	return;
}

void CoreProgram::setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT2x4, native_index, count, value);
	return;
}

void CoreProgram::setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT4x2, native_index, count, value);
	return;
}

void CoreProgram::setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT3x4, native_index, count, value);
	return;
}

void CoreProgram::setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	if ((m_pBackendProgram == nullptr) || (location >= m_uniforms.size())) {
		return;
	}
	AXGL_UNUSED(transpose); // unsupported in GLES
	const ProgramUniform& uniform = m_uniforms[location];
	int32_t native_index = uniform.info.m_NativeIndex;
	m_pBackendProgram->setUniformf(GL_FLOAT_MAT4x3, native_index, count, value);
	return;
}

void CoreProgram::setUniform1ui(GLint location, GLuint v0)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformui(GL_UNSIGNED_INT, native_index, 1, &v0);
	}
	return;
}

void CoreProgram::setUniform1uiv(GLint location, GLsizei count, const GLuint* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformui(GL_UNSIGNED_INT, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform2ui(GLint location, GLuint v0, GLuint v1)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	GLuint values[2] = {
		v0, v1
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformui(GL_UNSIGNED_INT_VEC2, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform2uiv(GLint location, GLsizei count, const GLuint* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformui(GL_UNSIGNED_INT_VEC2, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	GLuint values[3] = {
		v0, v1, v2
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformui(GL_UNSIGNED_INT_VEC3, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform3uiv(GLint location, GLsizei count, const GLuint* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformui(GL_UNSIGNED_INT_VEC3, native_index, count, value);
	}
	return;
}

void CoreProgram::setUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	GLuint values[4] = {
		v0, v1, v2, v3
	};
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformui(GL_UNSIGNED_INT_VEC4, native_index, 1, values);
	}
	return;
}

void CoreProgram::setUniform4uiv(GLint location, GLsizei count, const GLuint* value)
{
	if (m_pBackendProgram == nullptr) {
		return;
	}
	int n_uniform = static_cast<int>(m_uniforms.size());
	if (location < n_uniform) {
		const ProgramUniform& uniform = m_uniforms[location];
		int32_t native_index = uniform.info.m_NativeIndex;
		m_pBackendProgram->setUniformui(GL_UNSIGNED_INT_VEC4, native_index, count, value);
	}
	return;
}

void CoreProgram::getUniformfv(GLint location, GLfloat* params)
{
	// TODO:
	AXGL_UNUSED(location);
	AXGL_UNUSED(params);
	return;
}

void CoreProgram::getUniformiv(GLint location, GLint* params)
{
	// TODO:
	AXGL_UNUSED(location);
	AXGL_UNUSED(params);
	return;
}

void CoreProgram::getUniformuiv(GLint location, GLuint* params)
{
	// TODO:
	AXGL_UNUSED(location);
	AXGL_UNUSED(params);
	return;
}

void CoreProgram::validateProgram()
{
	return;
}

void CoreProgram::setTransformFeedbackVaryings(GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
	// TODO: GL_SEPARATE_ATTRIBS で count の GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS チェック
	m_transformFeedbackVaryings.resize(count);
	for (int i = 0; i < count; i++) {
		m_transformFeedbackVaryings[i] = varyings[i];
	}
	// TODO: bufferMode check, GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS
	m_transformFeedbackBufferMode = bufferMode;
	return;
}

void CoreProgram::getTransformFeedbackVarying(GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
	if (index >= m_transformFeedbackOutputs.size()) {
		// GL_INVALID_VALUE
		return;
	}
	const ProgramVariable& varying = m_transformFeedbackOutputs[index];
	// name
	size_t name_len = varying.name.length();
	size_t copy_count = ((bufSize - 1) < name_len) ? (bufSize - 1) : name_len;
	strncpy(name, varying.name.c_str(), copy_count);
	// size, type
	*size = varying.info.m_Size;
	*type = varying.info.m_Type;
	// length : nullable
	if (length != nullptr) {
		*length = static_cast<GLsizei>(copy_count);
	}
	return;
}

void CoreProgram::uniformBlockBinding(GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	if (uniformBlockIndex >= m_uniformBlocks.size()) {
		return;
	}
	int ub_index = uniformBlockIndex;
	ProgramUniformBlock& block = m_uniformBlocks[ub_index];
	int32_t native_index = block.info.m_NativeIndex;
	m_pBackendProgram->setUniformBlockBinding(native_index, uniformBlockBinding);
	return;
}

void CoreProgram::getProgramBinary(GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary)
{
	// TODO:
	AXGL_UNUSED(bufSize);
	AXGL_UNUSED(length);
	AXGL_UNUSED(binaryFormat);
	AXGL_UNUSED(binary);
	return;
}

void CoreProgram::setProgramBinary(GLenum binaryFormat, const void* binary, GLsizei length)
{
	// TODO:
	AXGL_UNUSED(binaryFormat);
	AXGL_UNUSED(binary);
	AXGL_UNUSED(length);
	return;
}

void CoreProgram::setProgramParameteri(GLenum pname, GLint value)
{
	switch (pname) {
	case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
		m_programBinaryRetrievableHint = static_cast<GLboolean>(value);
		break;
	default:
		// GL_INVALID_ENUM
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreProgram::getProgramiv(GLenum pname, GLint* params)
{
	AXGL_ASSERT(params != nullptr);
	switch (pname) {
	case GL_ACTIVE_ATTRIBUTES:
		*params = static_cast<GLint>(m_vertexShaderInputs.size());
		break;
	case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
	{
		// NOTE: 取得する可能性が低いパラメータであり、取得時に算出する
		GLint max_length = 0;
		for (const auto& attrib : m_vertexShaderInputs) {
			GLint len = static_cast<GLint>(attrib.name.length());
			if (len > max_length) {
				max_length = len;
			}
		}
		*params = max_length;
	}
	break;
	case GL_ACTIVE_UNIFORM_BLOCKS:
		*params = static_cast<GLint>(m_uniformBlocks.size());
		break;
	case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
	{
		// NOTE: 取得する可能性が低いパラメータであり、取得時に算出する
		GLint max_length = 0;
		for (const auto& ub : m_uniformBlocks) {
			GLint len = static_cast<GLint>(ub.name.length());
			if (len > max_length) {
				max_length = len;
			}
		}
		*params = max_length;
	}
	break;
	case GL_ACTIVE_UNIFORMS:
	{
		GLint n_default_uniform = static_cast<GLint>(m_uniforms.size());
		*params = n_default_uniform;
	}
	break;
	case GL_ACTIVE_UNIFORM_MAX_LENGTH:
	{
		// NOTE: 取得する可能性が低いパラメータであり、取得時に算出する
		GLint max_length = 0;
		for (const auto& uniform : m_uniforms) {
			GLint len = static_cast<GLint>(uniform.name.length());
			if (len > max_length) {
				max_length = len;
			}
		}
		*params = max_length;
	}
	break;
	case GL_ATTACHED_SHADERS:
	{
		GLint num_shaders = 0;
		if (m_pVertexShader != nullptr) {
			num_shaders++;
		}
		if (m_pFragmentShader != nullptr) {
			num_shaders++;
		}
		*params = num_shaders;
	}
	break;
	case GL_DELETE_STATUS:
		*params = m_deleteStatus;
		break;
	case GL_INFO_LOG_LENGTH:
		*params = static_cast<GLint>(m_infoLog.length());
		break;
	case GL_LINK_STATUS:
		*params = m_linkStatus;
		break;
	case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
		*params = m_programBinaryRetrievableHint;
		break;
	case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
		*params = m_transformFeedbackBufferMode;
		break;
	case GL_TRANSFORM_FEEDBACK_VARYINGS:
		*params = static_cast<GLint>(m_transformFeedbackOutputs.size());
		break;
	case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
	{
		// NOTE: 取得する可能性が低いパラメータであり、取得時に算出する
		GLint max_length = 0;
		for (const auto& tfv : m_transformFeedbackOutputs) {
			GLint len = static_cast<GLint>(tfv.name.length());
			if (len > max_length) {
				max_length = len;
			}
		}
		*params = max_length;
	}
	break;
	case GL_VALIDATE_STATUS:
		*params = m_validateStatus;
		break;
	default:
		// GL_INVALID_ENUM
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

void CoreProgram::getProgramInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
#if defined(AXGL_DEBUG)
	// デバッグ時は情報を出力
	GLsizei copy_len = static_cast<GLsizei>(m_infoLog.length());
	if ((bufSize - 1) < copy_len) {
		copy_len = bufSize - 1;
	}
	strncpy(infoLog, m_infoLog.c_str(), copy_len);
	if (length != nullptr) {
		*length = copy_len;
	}
#else
	AXGL_UNUSED(bufSize);
	AXGL_UNUSED(infoLog);
	// リリース時は情報出力なし
	if (length != nullptr) {
		*length = 0;
	}
#endif
	return;
}

void CoreProgram::setDeleteStatus()
{
	m_deleteStatus = GL_TRUE;
	return;
}

bool CoreProgram::initialize(CoreContext* context)
{
	AXGL_ASSERT(m_pBackendProgram == nullptr);
	m_pBackendProgram = BackendProgram::create();
	if (m_pBackendProgram == nullptr) {
		return false;
	}
	AXGL_ASSERT(context != nullptr);
	BackendContext* backend_context = context->getBackendContext();
	AXGL_ASSERT(backend_context != nullptr);
	return m_pBackendProgram->initialize(backend_context);
}

void CoreProgram::terminate(CoreContext* context)
{
	if (m_pBackendProgram != nullptr) {
		BackendContext* backend_context = nullptr;
		if (context != nullptr) {
			backend_context = context->getBackendContext();
		}
		if (backend_context != nullptr) {
			// discard caches
			backend_context->discardCachesAssociatedWithProgram(m_pBackendProgram);
		}
		// terminate backend
		m_pBackendProgram->terminate(backend_context);

		BackendProgram::destroy(m_pBackendProgram);
		m_pBackendProgram = nullptr;
	}
	return;
}

void CoreProgram::getShaderUniformParam(const ProgramUniform& us, GLenum pname, GLint* param)
{
	AXGL_ASSERT(param != nullptr);
	const BackendProgram::ShaderUniform& info = us.info;
	switch (pname) {
	case GL_UNIFORM_TYPE:
		*param = info.m_Type;
		break;
	case GL_UNIFORM_SIZE:
		*param = info.m_Size;
		break;
	case GL_UNIFORM_NAME_LENGTH:
		*param = static_cast<GLint>(us.name.length());
		break;
	case GL_UNIFORM_BLOCK_INDEX:
		*param = info.m_BlockIndex;
		break;
	case GL_UNIFORM_OFFSET:
		*param = info.m_Offset;
		break;
	case GL_UNIFORM_ARRAY_STRIDE:
		*param = info.m_ArrayStride;
		break;
	case GL_UNIFORM_MATRIX_STRIDE:
		*param = info.m_MatrixStride;
		break;
	case GL_UNIFORM_IS_ROW_MAJOR:
		*param = info.m_IsRowMajor;
		break;
	default:
		// GL_INVALID_ENUM
		setErrorCode(GL_INVALID_ENUM);
		break;
	}
	return;
}

} // namespace axgl
