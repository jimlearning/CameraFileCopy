/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

// iOS平台使用不同的OpenGL ES头文件路径
#if defined(__APPLE__)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#else
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

#include <iostream>

// 定义CIMBAR_IOS_PLATFORM确保所有iOS兼容代码生效
#if defined(__APPLE__)
#ifndef CIMBAR_IOS_PLATFORM
#define CIMBAR_IOS_PLATFORM
#endif
#endif

namespace cimbar {

class gl_shader
{
public:
	gl_shader(GLenum type, const std::string& source)
	{
		_s = compile(type, source.c_str());
	}

	operator GLuint() const
	{
		return _s;
	}

	~gl_shader()
	{
		if (_s)
			glDeleteShader(_s);
		_s = 0;
	}

	bool check_compile_error(const std::string& context="compile_shader")
	{
		GLint compiled = GL_FALSE;
		glGetShaderiv(_s, GL_COMPILE_STATUS, &compiled);
		if (compiled != GL_TRUE)
		{
			GLint logLength = 0;
			glGetShaderiv(_s, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength)
			{
				char* logBuffer = new char[logLength];
				glGetShaderInfoLog(_s, logLength, NULL, logBuffer);
				std::cerr << context << " error: " << logBuffer << std::endl;
				delete[] logBuffer;
			}
			return false;
		}
		return true;
	}

protected:
	GLuint compile(GLenum type, const char* source)
	{
		if (!source)
			return 0;

		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		GLint compileStatus = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus != GL_TRUE)
		{
			GLint logLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength)
			{
				char* logBuffer = new char[logLength];
				glGetShaderInfoLog(shader, logLength, NULL, logBuffer);
				std::cerr << "Error compiling shader: " << logBuffer << std::endl;
				delete[] logBuffer;
			}
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}

private:
	GLuint _s;
};

}
