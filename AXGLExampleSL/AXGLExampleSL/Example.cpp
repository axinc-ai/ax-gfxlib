// Example.cpp
// GLES sample program
#if 1
#include <axgl/ES3/gl.h>
#include <axgl/ES3/glext.h>
#else
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#endif
#include "ExampleInterface.h"
#include "ExampleUtil.h"
#include <stdio.h>

typedef struct VsUniforms_t {
	float mvp[16];
} VsUniforms;

typedef struct ExampleResources_t {
	GLuint program = 0;
	GLuint vboPosition = 0;
	GLuint vboColor = 0;
	GLuint vboTexCoord = 0;
	GLuint uboVs = 0;
	GLuint texture = 0;
	GLuint sampler = 0;
	GLuint vao = 0;
	float rotY = 0.0f;
} ExampleResources;
static ExampleResources s_resources;

// Vertex shader GLSL
static const char c_vs_glsl[] = {
	"#version 300 es\n"
	"layout(location = 0) in vec3 a_position;\n"
	"layout(location = 1) in vec4 a_color;\n"
	"layout(location = 2) in vec2 a_texcoord;\n"
	"layout(binding = 0) uniform VS_UBO {\n"
	"  mat4 mvp;\n"
	"} vs_ubo;\n"
	"out vec4 v_color;\n"
	"out vec2 v_texcoord;\n"
	"void main() {\n"
	"  v_color = a_color;\n"
	"  v_texcoord = a_texcoord;\n"
	"  gl_Position = (vs_ubo.mvp * vec4(a_position, 1.0));\n"
	"}\n"
};
// Fragment shader GLSL
static const char c_fs_glsl[] = {
	"#version 300 es\n"
	"precision mediump float;\n"
	"in vec4 v_color;\n"
	"in vec2 v_texcoord;\n"
	"uniform vec4 u_color;\n"
	"layout(binding = 0) uniform sampler2D u_sampler;\n"
	"layout(location = 0) out vec4 o_frag_color;\n"
	"void main() {\n"
	"  vec4 tex_color = texture(u_sampler, v_texcoord);\n"
	"  o_frag_color = v_color * tex_color * u_color;\n"
	"}\n"
};

// Initialization
void* exampleInitialize()
{
	// Execute initialization if necessary
	return static_cast<void*>(&s_resources);
}

// Termination
void exampleTerminate(void* instance)
{
	if (instance == nullptr) {
		return;
	}
	exampleDestroyResources(instance);
	// Execute termination if necessary
	return;
}

// Create GLES resources
void exampleCreateResources(void* instance)
{
	if (instance == nullptr) {
		return;
	}
	ExampleResources* rsc = reinterpret_cast<ExampleResources*>(instance);

	// Vertex positions
	static const float c_positions[] = {
		-0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
		-0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
		-0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
		 0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f
	};
	glGenBuffers(1, &rsc->vboPosition);
	glBindBuffer(GL_ARRAY_BUFFER, rsc->vboPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(c_positions), c_positions, GL_STATIC_DRAW);
	// Vertex colors
	static const float c_color[] = {
		1.0f,0.0f,0.0f,1.0f, 1.0f,0.0f,0.0f,1.0f, 1.0f,0.0f,0.0f,1.0f, 1.0f,0.0f,0.0f,1.0f,
		0.0f,1.0f,0.0f,1.0f, 0.0f,1.0f,0.0f,1.0f, 0.0f,1.0f,0.0f,1.0f, 0.0f,1.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,1.0f, 0.0f,0.0f,1.0f,1.0f, 0.0f,0.0f,1.0f,1.0f, 0.0f,0.0f,1.0f,1.0f,
		1.0f,0.0f,1.0f,1.0f, 1.0f,0.0f,1.0f,1.0f, 1.0f,0.0f,1.0f,1.0f, 1.0f,0.0f,1.0f,1.0f,
		1.0f,1.0f,0.0f,1.0f, 1.0f,1.0f,0.0f,1.0f, 1.0f,1.0f,0.0f,1.0f, 1.0f,1.0f,0.0f,1.0f,
		0.0f,1.0f,1.0f,1.0f, 0.0f,1.0f,1.0f,1.0f, 0.0f,1.0f,1.0f,1.0f, 0.0f,1.0f,1.0f,1.0f
	};
	glGenBuffers(1, &rsc->vboColor);
	glBindBuffer(GL_ARRAY_BUFFER, rsc->vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(c_color), c_color, GL_STATIC_DRAW);
	// Texture coordinates
	static const float c_texcoords[] = {
		0.0f,1.0f, 0.0f,0.0f, 1.0f,1.0f, 1.0f,0.0f,
		0.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f,
		0.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f,
		0.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f,
		0.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f,
		0.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f
	};
	glGenBuffers(1, &rsc->vboTexCoord);
	glBindBuffer(GL_ARRAY_BUFFER, rsc->vboTexCoord);
	glBufferData(GL_ARRAY_BUFFER, sizeof(c_texcoords), c_texcoords, GL_STATIC_DRAW);
	// Unbind buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Uniform buffer
	static const VsUniforms c_vs_uniform = {
		{1.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f,1.0f}
	};
	glGenBuffers(1, &rsc->uboVs);
	glBindBuffer(GL_UNIFORM_BUFFER, rsc->uboVs);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(c_vs_uniform), &c_vs_uniform, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Texture
	static const uint8_t c_texels[] = {
		0xff,0xff,0xff,0xff, 0x80,0x80,0x80,0xff, 0xff,0xff,0xff,0xff, 0x80,0x80,0x80,0xff,
		0x80,0x80,0x80,0xff, 0xff,0xff,0xff,0xff, 0x80,0x80,0x80,0xff, 0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff, 0x80,0x80,0x80,0xff, 0xff,0xff,0xff,0xff, 0x80,0x80,0x80,0xff,
		0x80,0x80,0x80,0xff, 0xff,0xff,0xff,0xff, 0x80,0x80,0x80,0xff, 0xff,0xff,0xff,0xff
	};
	glGenTextures(1, &rsc->texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rsc->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, c_texels);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Sampler
	glGenSamplers(1, &rsc->sampler);
	glSamplerParameteri(rsc->sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(rsc->sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(rsc->sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(rsc->sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Shaders
	const GLchar* src;
	GLint compile_status;
	src = c_vs_glsl;
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	if (vs != 0) {
		glShaderSource(vs, 1, &src, nullptr);
		glCompileShader(vs);
		glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_status);
		if (compile_status != GL_TRUE) {
			printf("VS compile FAILED\n");
		}
	}
	src = c_fs_glsl;
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	if (fs != 0) {
		glShaderSource(fs, 1, &src, nullptr);
		glCompileShader(fs);
		glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_status);
		if (compile_status != GL_TRUE) {
			printf("FS compile FAILED\n");
		}
	}
	GLuint pgm = glCreateProgram();
	if (pgm != 0) {
		GLint link_status;
		glAttachShader(pgm, vs);
		glAttachShader(pgm, fs);
		glLinkProgram(pgm);
		glGetProgramiv(pgm, GL_LINK_STATUS, &link_status);
		if (link_status != GL_TRUE) {
			printf("Program link FAILED\n");
		}
	}
	glDeleteShader(vs);
	glDeleteShader(fs);
	rsc->program = pgm;

	// Vertex Array Object
	glGenVertexArrays(1, &rsc->vao);
	glBindVertexArray(rsc->vao);
	{
		static const GLint loc_position = 0;
		glBindBuffer(GL_ARRAY_BUFFER, rsc->vboPosition);
		glEnableVertexAttribArray(loc_position);
		glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		static const GLint loc_color = 1;
		glBindBuffer(GL_ARRAY_BUFFER, rsc->vboColor);
		glEnableVertexAttribArray(loc_color);
		glVertexAttribPointer(loc_color, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

		static const GLint loc_texcoord = 2;
		glBindBuffer(GL_ARRAY_BUFFER, rsc->vboTexCoord);
		glEnableVertexAttribArray(loc_texcoord);
		glVertexAttribPointer(loc_texcoord, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Initialize the rotation angle
	rsc->rotY = 0.0f;

	return;
}

// Destroy GLES resources
void exampleDestroyResources(void* instance)
{
	if (instance == nullptr) {
		return;
	}
	ExampleResources* rsc = reinterpret_cast<ExampleResources*>(instance);
	if (rsc->program != 0) {
		glDeleteProgram(rsc->program);
		rsc->program = 0;
	}
	if (rsc->vboPosition != 0) {
		glDeleteBuffers(1, &rsc->vboPosition);
		rsc->vboPosition = 0;
	}
	if (rsc->vboColor != 0) {
		glDeleteBuffers(1, &rsc->vboColor);
		rsc->vboColor = 0;
	}
	if (rsc->vboTexCoord != 0) {
		glDeleteBuffers(1, &rsc->vboTexCoord);
		rsc->vboTexCoord = 0;
	}
	if (rsc->uboVs != 0) {
		glDeleteBuffers(1, &rsc->uboVs);
		rsc->uboVs = 0;
	}
	if (rsc->texture != 0) {
		glDeleteTextures(1, &rsc->texture);
		rsc->texture = 0;
	}
	if (rsc->sampler != 0) {
		glDeleteSamplers(1, &rsc->sampler);
		rsc->sampler = 0;
	}
	if (rsc->vao != 0) {
		glDeleteVertexArrays(1, &rsc->vao);
		rsc->vao = 0;
	}

	return;
}

// Render the scene
void exampleRender(void* instance, uint32_t framebuffer)
{
	if (instance == nullptr) {
		return;
	}
	ExampleResources* rsc = reinterpret_cast<ExampleResources*>(instance);
	// Bind framebuffer managed by UIView
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Projection matrix
	float pm[16];
	{
		GLint viewport[4];
		float m0[16];
		float m1[16];
		glGetIntegerv(GL_VIEWPORT, viewport);
		float aspect = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);
		if (viewport[2] >= viewport[3]) {
			utilOrthoMat4(m0, -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
		} else {
			float recp_aspect = 1.0f / aspect;
			utilOrthoMat4(m0, -1.0f, 1.0f, -recp_aspect, recp_aspect, -1.0f, 1.0f);
		}
		utilRotateXMat4(m1, EXAMPLE_UTIL_PI / 6.0f);
		utilMultiplyMat4(pm, m0, m1);
	}

	// Clear
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set the program object to current rendering state
	glUseProgram(rsc->program);

	// Set values to a uniform variable
	// u_color : Get a location because it is not specified to "u_color"
	GLint loc_u_color = glGetUniformLocation(rsc->program, "u_color");
	if (loc_u_color >= 0) {
		static const float fs_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glUniform4fv(loc_u_color, 1, fs_color);
	}

	// Update uniform buffers
	VsUniforms vs_uniform;
	{
		// Model-View matrix
		float mvm[16];
		utilRotateYMat4(mvm, rsc->rotY);
		// Model-View-Projection matrix
		utilMultiplyMat4(vs_uniform.mvp, pm, mvm);
	}
	rsc->rotY += 0.01f;
	if (rsc->rotY >= (2.0f * EXAMPLE_UTIL_PI)) {
		rsc->rotY -= (2.0f * EXAMPLE_UTIL_PI);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, rsc->uboVs);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VsUniforms), &vs_uniform);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, rsc->uboVs);

	// Bind the texture object
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rsc->texture);
	// Bind the sampler object
	glBindSampler(0, rsc->sampler);
		
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);

	// Draw a cube
	glBindVertexArray(rsc->vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 24);
	glBindVertexArray(0);

	GLenum ec = glGetError();
	if (ec != GL_NO_ERROR) {
		printf("GL ERROR\n");
	}

	return;
}
