/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "gl_program.h"
#include "gl_shader.h"
#include "util/loop_iterator.h"

// iOS平台使用不同的OpenGL ES头文件路径
#if defined(__APPLE__)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#else
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

#include <memory>

namespace cimbar {

class gl_2d_display
{
protected:
	static constexpr GLfloat PLANE[] = {
	    -1.0f, -1.0f, 0.0f,
	     1.0f, -1.0f, 0.0f,
	    -1.0f,  1.0f, 0.0f,
	     1.0f, -1.0f, 0.0f,
	    -1.0f,  1.0f, 0.0f,
	     1.0f,  1.0f, 0.0f
	};

	static constexpr GLfloat TEXTURE_UVS[] = {
	    0.0f, 1.0f,
	    1.0f, 1.0f,
	    0.0f, 0.0f,
	    1.0f, 1.0f,
	    0.0f, 0.0f,
	    1.0f, 0.0f
	};

public:
	static cimbar::gl_program create_vertex_fragment_program(const std::string& fragSource, const std::string& vertSource=_default_vertex_shader())
	{
		cimbar::gl_shader vert(GL_VERTEX_SHADER, vertSource);
		bool success = vert.check_compile_error("vertex_shader");
		if (!success)
			return cimbar::gl_program(0, 0, ""); // 返回空程序而不是整数0

		cimbar::gl_shader frag(GL_FRAGMENT_SHADER, fragSource);
		success = frag.check_compile_error("fragment_shader");
		if (!success)
			return cimbar::gl_program(0, 0, ""); // 返回空程序而不是整数0

		return cimbar::gl_program(vert, frag, "position");
	}

	static GLuint create_vao(const gl_program& program)
	{
		if (!program)
			return 0;

		GLuint vao = 0;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo[2];
		glGenBuffers(2, vbo);

		// position
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(PLANE), PLANE, GL_STATIC_DRAW);
		GLint positionAttrib = glGetAttribLocation(program, "position");
		glEnableVertexAttribArray(positionAttrib);
		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// uv
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_UVS), TEXTURE_UVS, GL_STATIC_DRAW);
		GLint uvAttrib = glGetAttribLocation(program, "tex_coord");
		if (uvAttrib != -1)
		{
			glEnableVertexAttribArray(uvAttrib);
			glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}

		//unbind
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return vao;
	}

	static const std::string& _default_fragment_shader()
	{
		static const std::string shader = R"(
			precision mediump float;
			uniform sampler2D tex;
			varying vec2 texCoord;
			void main()
			{
				gl_FragColor = texture2D(tex, texCoord);
			}
		)";
		return shader;
	}

	static const std::string& _default_vertex_shader()
	{
		static const std::string shader = R"(
			precision mediump float;
			attribute vec3 position;
			attribute vec2 tex_coord;
			varying vec2 texCoord;
			void main()
			{
				gl_Position = vec4(position, 1.0);
				texCoord = tex_coord;
			}
		)";
		return shader;
	}

public:
	gl_2d_display(const std::string& fragmentSource=_default_fragment_shader())
	    : _program(create_vertex_fragment_program(fragmentSource)) // 使用初始化列表初始化_program
	{
		_vao = create_vao(_program);
	}

	~gl_2d_display()
	{
		if (_vao)
			glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}

	bool good() const
	{
		return _vao != 0 and _program;
	}

	void draw() const
	{
		// use our texture and vao
		set_program();
		glBindVertexArray(_vao);
		//glActiveTexture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void set_program() const
	{
		_program.use();
	}

protected:
	cimbar::gl_program _program;
	GLuint _vao;
};
