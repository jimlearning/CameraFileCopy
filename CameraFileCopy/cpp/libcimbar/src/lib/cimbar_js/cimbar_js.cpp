/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "cimbar_js.h"

#include "cimb_translator/Config.h"
#include "encoder/SimpleEncoder.h"

// 平台检测
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
    // iOS平台 - 使用iOS特定的头文件
    #define CIMBAR_IOS_PLATFORM
#else
    // macOS平台 - 使用GLFW
    #include "gui/window_glfw.h"
#endif
#else
    // 其他平台 - 使用GLFW
    #include "gui/window_glfw.h"
#endif

#include "util/byte_istream.h"

#include <iostream>
#include <sstream>


namespace {
#ifdef CIMBAR_IOS_PLATFORM
    // iOS平台特定实现
    cv::Mat _current;
    std::optional<cv::Mat> _next;
#else
    // 其他平台使用GLFW实现
    std::shared_ptr<cimbar::window_glfw> _window;
    std::optional<cv::Mat> _next;
#endif

    std::shared_ptr<fountain_encoder_stream> _fes;
    int _frameCount = 0;
    uint8_t _encodeId = 109;

    // settings
    unsigned _ecc = cimbar::Config::ecc_bytes();
    unsigned _colorBits = cimbar::Config::color_bits();
    int _compressionLevel = cimbar::Config::compression_level();
    bool _legacyMode = true;
}

extern "C" {

int initialize_GL(int width, int height)
{
#ifdef CIMBAR_IOS_PLATFORM
    // iOS平台实现
    std::cerr << "iOS: initializing " << width << " by " << height << " renderer";
    
    // 确保尺寸是4的倍数
    if (width % 4 != 0)
        width += (4 - width % 4);
    if (height % 4 != 0)
        height += (4 - height % 4);
    
    // 初始化图像缓冲区
    _current = cv::Mat(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 255));
    return 1;
#else
    // 原始GLFW实现
    if (_window)
        return 1;

    // must be divisible by 4???
    if (width % 4 != 0)
        width += (4 - width % 4);
    if (height % 4 != 0)
        height += (4 - height % 4);
    std::cerr << "initializing " << width << " by " << height << " window";

    _window = std::make_shared<cimbar::window_glfw>(width, height, "Cimbar Encoder");
    if (!_window or !_window->is_good())
        return 0;

    return 1;
#endif
}

// render() and next_frame() could be put in the same function,
// but it seems cleaner to split them.
// in any case, we're concerned with frame pacing (some encodes take longer than others)
int render()
{
#ifdef CIMBAR_IOS_PLATFORM
    // iOS平台实现
    if (_next)
    {
        // 更新图像缓冲区
        _current = *_next;
        _next.reset();
        return 1;
    }
    return 0;
#else
    // 原始GLFW实现
    if (!_window or !_fes or _window->should_close())
        return -1;

    if (_next)
    {
        _window->show(*_next, 0);
        _window->shake();
        return 1;
    }
    return 0;
#endif
}

int next_frame()
{
#ifdef CIMBAR_IOS_PLATFORM
    // iOS平台实现
    if (!_fes)
        return 0;

    // we generate 5x the amount of required symbol blocks -- unless everything fits in a single frame.
    // color blocks will contribute to this total, but only symbols are used for the initial calculation.
    // ... this way, if the color decode is failing, it won't get "stuck" failing to read a single frame.
    unsigned required = _fes->blocks_required();
    if (required > cimbar::Config::fountain_chunks_per_frame(cimbar::Config::symbol_bits(), _legacyMode))
        required = required*5;
    if (_fes->block_count() > required)
    {
        _fes->restart();
        _frameCount = 0;
    }

    SimpleEncoder enc(_ecc, cimbar::Config::symbol_bits(), _colorBits);
    if (_legacyMode)
        enc.set_legacy_mode();

    enc.set_encode_id(_encodeId);
    _next = enc.encode_next(*_fes, _current.cols);
    return ++_frameCount;
#else
    // 原始GLFW实现
    if (!_window or !_fes)
        return 0;

    // we generate 5x the amount of required symbol blocks -- unless everything fits in a single frame.
    // color blocks will contribute to this total, but only symbols are used for the initial calculation.
    // ... this way, if the color decode is failing, it won't get "stuck" failing to read a single frame.
    unsigned required = _fes->blocks_required();
    if (required > cimbar::Config::fountain_chunks_per_frame(cimbar::Config::symbol_bits(), _legacyMode))
        required = required*5;
    if (_fes->block_count() > required)
    {
        _fes->restart();
        _window->shake(0);
        _frameCount = 0;
    }

    SimpleEncoder enc(_ecc, cimbar::Config::symbol_bits(), _colorBits);
    if (_legacyMode)
        enc.set_legacy_mode();

    enc.set_encode_id(_encodeId);
    _next = enc.encode_next(*_fes, _window->width());
    return ++_frameCount;
#endif
}

int encode(unsigned char* buffer, unsigned size, int encode_id)
{
    _frameCount = 0;
    if (!FountainInit::init())
        std::cerr << "failed FountainInit :(" << std::endl;

    SimpleEncoder enc(_ecc, cimbar::Config::symbol_bits(), _colorBits);
    if (_legacyMode)
        enc.set_legacy_mode();

    if (encode_id < 0)
        enc.set_encode_id(++_encodeId); // increment _encodeId every time we change files
    else
        enc.set_encode_id(static_cast<uint8_t>(encode_id));

    cimbar::byte_istream bis(reinterpret_cast<char*>(buffer), size);
    _fes = enc.create_fountain_encoder(bis, _compressionLevel);

    if (!_fes)
        return 0;

    _next.reset();
    return 1;
}

int configure(unsigned color_bits, unsigned ecc, int compression, bool legacy_mode)
{
    // defaults
    if (color_bits > 3)
        color_bits = cimbar::Config::color_bits();
    if (ecc >= 150)
        ecc = cimbar::Config::ecc_bytes();
    if (compression < 0 or compression > 22)
        compression = cimbar::Config::compression_level();

    // check if we need to refresh the stream
    bool refresh = (color_bits != _colorBits or ecc != _ecc or compression != _compressionLevel or legacy_mode != _legacyMode);
    if (refresh)
    {
        // update config
        _colorBits = color_bits;
        _ecc = ecc;
        _compressionLevel = compression;
        _legacyMode = legacy_mode;

        // try to refresh the stream
        if (_fes)
        {
            unsigned buff_size_new = cimbar::Config::fountain_chunk_size(_ecc, cimbar::Config::symbol_bits() + _colorBits, _legacyMode);
            if (!_fes->restart_and_resize_buffer(buff_size_new))
            {
                // if the data is too small, we should throw out _fes -- and clear the canvas.
                _fes = nullptr;
                _next.reset();
            }
            _frameCount = 0;
        }
    }
    return 0;
}

}
