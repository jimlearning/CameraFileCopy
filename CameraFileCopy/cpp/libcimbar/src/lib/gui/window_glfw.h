/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "gl_2d_display.h"
#include "mat_to_gl.h"

// iOS平台特有的头文件
#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
// 这里应该根据实际需要引入UIKit或其他iOS图形框架
// #include <UIKit/UIKit.h>
#else
#include <GLFW/glfw3.h>
#endif

#include <chrono>
#include <string>
#include <thread>
#include <memory>

// 定义CIMBAR_IOS_PLATFORM确保所有iOS兼容代码生效
#if defined(__APPLE__)
#ifndef CIMBAR_IOS_PLATFORM
#define CIMBAR_IOS_PLATFORM
#endif
#endif

namespace cimbar {

class window_glfw
{
public:
    window_glfw(unsigned width, unsigned height, std::string title)
        : _width(width), _height(height), _title(title), _good(true)
    {
#if defined(CIMBAR_IOS_PLATFORM)
        // iOS平台不使用GLFW，而是依赖UIKit/Metal/OpenGL ES
        // 在实际的iOS实现中，这部分会由iOS应用层负责处理
        _display = std::make_shared<cimbar::gl_2d_display>();
        glGenTextures(1, &_texid);
        init_opengl(width, height);
#else
        if (!glfwInit())
        {
            _good = false;
            return;
        }

        _w = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!_w)
        {
            _good = false;
            return;
        }
        glfwMakeContextCurrent(_w);
        glfwSwapInterval(1);

        _display = std::make_shared<cimbar::gl_2d_display>();
        glGenTextures(1, &_texid);
        init_opengl(width, height);
#endif
    }

    ~window_glfw()
    {
#if !defined(CIMBAR_IOS_PLATFORM)
        if (_w)
            glfwDestroyWindow(_w);
        glfwTerminate();
#endif

        if (_texid)
            glDeleteTextures(1, &_texid);
    }

    bool is_good() const
    {
        return _good;
    }

    bool should_close() const
    {
#if defined(CIMBAR_IOS_PLATFORM)
        // iOS应用由系统控制生命周期，不需要此检查
        return false;
#else
        return glfwWindowShouldClose(_w);
#endif
    }

    void show(const cv::Mat& img)
    {
        if (!is_good())
            return;

        // 这部分OpenGL代码在iOS和其他平台上是兼容的
        cimbar::mat_to_gl::load_gl_texture(_texid, img);
        if (_display)
            _display->draw();

#if !defined(CIMBAR_IOS_PLATFORM)
        glfwSwapBuffers(_w);
        glfwPollEvents();
#endif
    }

    unsigned width() const
    {
        return _width;
    }

    unsigned height() const
    {
        return _height;
    }

protected:
    void init_opengl(unsigned width, unsigned height)
    {
        glEnable(GL_TEXTURE_2D);
        glViewport(0, 0, width, height);
    }

protected:
    unsigned _width;
    unsigned _height;
    std::string _title;
    bool _good;

#if !defined(CIMBAR_IOS_PLATFORM)
    GLFWwindow* _w;
#endif

    std::shared_ptr<cimbar::gl_2d_display> _display;
    GLuint _texid;
};

}
