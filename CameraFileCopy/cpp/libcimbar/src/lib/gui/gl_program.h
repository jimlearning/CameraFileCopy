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
#include <string>

// 定义CIMBAR_IOS_PLATFORM确保所有iOS兼容代码生效
#if defined(__APPLE__)
#ifndef CIMBAR_IOS_PLATFORM
#define CIMBAR_IOS_PLATFORM
#endif
#endif

namespace cimbar {

class gl_program
{
public:
	gl_program(GLuint vertexShader, GLuint fragmentShader, const std::string& vertextVarName)
	{
		_p = build(vertexShader, fragmentShader, vertextVarName);
	}

	~gl_program()
	{
		if (_p)
			glDeleteProgram(_p);
		_p = 0;
	}

	bool use() const
	{
		if (!_p)
			return false;
		glUseProgram(_p);
		return true;
	}

	operator GLuint() const
	{
		return _p;
	}

protected:
	GLuint build(GLuint vertexShader, GLuint fragmentShader, const std::string& vertextVarName)
	{
		GLuint program = glCreateProgram();
		if (vertexShader)
			glAttachShader(program, vertexShader);
		if (fragmentShader)
			glAttachShader(program, fragmentShader);

		if (!vertextVarName.empty())
			glBindAttribLocation(program, 0, vertextVarName.c_str());
		glLinkProgram(program);

		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
			GLint logLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength)
			{
				char* logBuffer = new char[logLength];
				glGetProgramInfoLog(program, logLength, NULL, logBuffer);
				std::cerr << "Error linking program: " << logBuffer << std::endl;
				delete[] logBuffer;
			}
			glDeleteProgram(program);
			return 0;
		}
		return program;
	}

private:
	GLuint _p;
};

}
