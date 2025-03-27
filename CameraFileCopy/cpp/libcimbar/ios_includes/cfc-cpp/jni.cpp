#include "MultiThreadedDecoder.h"
#include "cimb_translator/CimbDecoder.h"
#include "cimb_translator/CimbReader.h"
#include "encoder/Decoder.h"
#include "extractor/Scanner.h"
#include "serialize/format.h"

// iOS平台特有的头文件
#if defined(__APPLE__)
// 暂时不包含任何特定header，但可能需要引入UIKit相关头文件
// #include <Foundation/Foundation.h>
#else
#include <jni.h>
#include <android/log.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <memory>
#include <mutex>
#include <sstream>

// 定义CIMBAR_IOS_PLATFORM确保所有iOS兼容代码生效
#if defined(__APPLE__)
#ifndef CIMBAR_IOS_PLATFORM
#define CIMBAR_IOS_PLATFORM
#endif
#define TAG "CameraFileCopyiOS"
#else
#define TAG "CameraFileCopyCPP"
#endif

using namespace std;
using namespace cv;

namespace {
    std::shared_ptr<MultiThreadedDecoder> _proc;
    std::mutex _mutex; // for _proc
    std::set<std::string> _completed;

    unsigned _calls = 0;
    int _transferStatus = 0;
    clock_t _frameDecodeSnapshot = 0;
    clock_t _frameSuccessSnapshot = 0;

    unsigned millis(unsigned num, unsigned denom)
    {
        if (!denom)
            denom = 1;
        return (num / denom) * 1000 / CLOCKS_PER_SEC;
    }

    unsigned percent(unsigned num, unsigned denom)
    {
        if (!denom)
            denom = 1;
        return (num * 100) / denom;
    }

    void drawGuidance(cv::Mat& mat, int in_progress)
    {
        int minsz = std::min(mat.cols, mat.rows);
        int guideWidth = minsz >> 7;
        int outlineWidth = guideWidth + (minsz >> 8);
        int guideLength = guideWidth << 3;

        // Guide lines (green): intended for screen corners
        Scalar color = Scalar(0, 255, 0);
        Scalar outline = Scalar(0, 0, 0);

        int offsetX = outlineWidth;
        int offsetY = outlineWidth;

        // Top left
        line(mat, Point(offsetX, offsetY), Point(offsetX + guideLength, offsetY), outline, outlineWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX, offsetY + guideLength), outline, outlineWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX + guideLength, offsetY), color, guideWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX, offsetY + guideLength), color, guideWidth);

        // Top right
        offsetX = mat.cols - outlineWidth;
        line(mat, Point(offsetX, offsetY), Point(offsetX - guideLength, offsetY), outline, outlineWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX, offsetY + guideLength), outline, outlineWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX - guideLength, offsetY), color, guideWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX, offsetY + guideLength), color, guideWidth);

        // Bottom left
        offsetX = outlineWidth;
        offsetY = mat.rows - outlineWidth;
        line(mat, Point(offsetX, offsetY), Point(offsetX + guideLength, offsetY), outline, outlineWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX, offsetY - guideLength), outline, outlineWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX + guideLength, offsetY), color, guideWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX, offsetY - guideLength), color, guideWidth);

        // Bottom right
        offsetX = mat.cols - outlineWidth;
        line(mat, Point(offsetX, offsetY), Point(offsetX - guideLength, offsetY), outline, outlineWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX, offsetY - guideLength), outline, outlineWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX - guideLength, offsetY), color, guideWidth);
        line(mat, Point(offsetX, offsetY), Point(offsetX, offsetY - guideLength), color, guideWidth);
    }

    void drawProgress(cv::Mat& mat, const std::vector<double>& progress)
    {
        if (progress.empty())
            return;

        // progress bars on the right side of the screen
        int pbWidth = 80;
        int pbHeight = 18;
        int pbGap = 8;

        Point tl, br;
        br.x = mat.cols - 28;
        tl.x = br.x - pbWidth;

        for (unsigned i = 0; i < progress.size(); ++i)
        {
            if (progress[i] <= 0)
                continue;

            int rows_from_top = i;
            tl.y = 30 + ((pbHeight + pbGap) * rows_from_top);
            br.y = tl.y + pbHeight;

            Scalar color = Scalar(255, 153, 51); // orange

            // pb outline
            cv::rectangle(mat, tl, br, Scalar(0, 0, 0), 2);

            // pb fill
            int fillWidth = (progress[i] * pbWidth) / 100;
            Point brFill = Point(tl.x + fillWidth, br.y);
            cv::rectangle(mat, tl, brFill, color, -1);
        }
    }

    void drawDebugInfo(cv::Mat& mat, MultiThreadedDecoder& proc)
    {
        unsigned scanMillis = 0;
        unsigned extractMillis = 0;
        unsigned decodeMillis = 0;
        unsigned calls = 0;
        unsigned scanned = 0;
        unsigned perfect = 0;
        unsigned decoded = 0;
        {
            std::lock_guard<std::mutex> guard(_mutex);
            calls = _calls;
            scanMillis = millis(MultiThreadedDecoder::scanTicks, MultiThreadedDecoder::scanned);
            extractMillis = millis(MultiThreadedDecoder::extractTicks, MultiThreadedDecoder::scanned);
            decodeMillis = millis(MultiThreadedDecoder::decodeTicks, MultiThreadedDecoder::decoded);
            scanned = MultiThreadedDecoder::scanned;
            perfect = MultiThreadedDecoder::perfect;
            decoded = MultiThreadedDecoder::decoded;
        }

        std::stringstream ss;
        ss << "Frames: " << calls;
        ss << " | Scan: " << scanned << " (" << percent(scanned, calls) << "%) " << scanMillis << "ms";
        ss << " | Loc: " << perfect << " (" << percent(perfect, scanned) << "%) " << extractMillis << "ms";
        ss << " | Dec: " << decoded << " (" << percent(decoded, perfect) << "%) " << decodeMillis << "ms";
        ss << " | Mode: " << proc.mode();
        if (proc.detected_mode() != proc.mode())
            ss << " (" << proc.detected_mode() << ")";

        // bottom of the screen, middle
        putText(mat, ss.str(), Point(30, mat.rows - 30), FONT_HERSHEY_DUPLEX, 0.9, Scalar(0, 0, 0), 4);
        putText(mat, ss.str(), Point(30, mat.rows - 30), FONT_HERSHEY_DUPLEX, 0.9, Scalar(51, 153, 255), 2);
    }

#if !defined(CIMBAR_IOS_PLATFORM)
    // Android特有的字符串转换函数
    std::string jstring_to_cppstr(JNIEnv *env, const jstring& dataPathObj)
    {
        const char* data_path = env->GetStringUTFChars(dataPathObj, 0);
        std::string res = data_path;
        env->ReleaseStringUTFChars(dataPathObj, data_path);
        return res;
    }
#endif
}

#if defined(CIMBAR_IOS_PLATFORM)
// iOS特有的API函数 - 这些将需要从Swift/Objective-C进行调用

// 处理图像，返回处理结果
// 这个函数对应Android版本的Java_org_cimbar_camerafilecopy_MainActivity_processImageJNI
extern "C" const char* processImageiOS(void* matPtr, const char* dataPath, int modeInt)
{
    cv::Mat& mat = *static_cast<cv::Mat*>(matPtr);
    std::string result;
    
    {
        std::lock_guard<std::mutex> guard(_mutex);
        if (!_proc)
            _proc = std::make_shared<MultiThreadedDecoder>(dataPath, modeInt);
        ++_calls;
    }
    
    // 在阶段性更新时收集并返回已完成的文件
    std::vector<std::string> done = _proc->get_done();
    for (const std::string& d : done)
    {
        if (_completed.find(d) == _completed.end())
        {
            _completed.insert(d);
            result = d;
            break;
        }
    }
    
    // 获取进度信息并绘制UI元素
    std::vector<double> progress = _proc->get_progress();
    drawGuidance(mat, progress.size());
    drawProgress(mat, progress);
    drawDebugInfo(mat, *_proc);
    
    // 尝试解码图像
    _proc->add(mat);
    
    // 如果有结果，返回它；否则返回空字符串
    static std::string str_res;
    str_res = result;
    return str_res.c_str();
}

// 停止处理
// 这个函数对应Android版本的Java_org_cimbar_camerafilecopy_MainActivity_shutdownJNI
extern "C" void shutdowniOS()
{
    std::lock_guard<std::mutex> guard(_mutex);
    if (_proc)
    {
        _proc->stop();
        _proc.reset();
    }
}

#else
// Android特有的JNI API

extern "C" {

// 处理图像，返回处理结果
jstring Java_org_cimbar_camerafilecopy_MainActivity_processImageJNI(JNIEnv *env, jobject instance, jlong matAddr, jstring dataPathObj, jint modeInt)
{
    Mat& mat = *(Mat*)matAddr;
    std::string res;

    {
        std::lock_guard<std::mutex> guard(_mutex);
        if (!_proc)
            _proc = std::make_shared<MultiThreadedDecoder>(jstring_to_cppstr(env, dataPathObj), modeInt);
        ++_calls;
    }

    // occasionally return the name of any completed transfers
    std::vector<std::string> done = _proc->get_done();
    for (const std::string& d : done)
    {
        if (_completed.find(d) == _completed.end())
        {
            _completed.insert(d);
            res = d;
            break;
        }
    }

    std::vector<double> progress = _proc->get_progress();
    drawGuidance(mat, progress.size());
    drawProgress(mat, progress);
    drawDebugInfo(mat, *_proc);

    _proc->add(mat);

    return (res.empty())? NULL : env->NewStringUTF(res.c_str());
}

// 停止处理
void Java_org_cimbar_camerafilecopy_MainActivity_shutdownJNI(JNIEnv *env, jobject instance)
{
    std::lock_guard<std::mutex> guard(_mutex);
    if (_proc)
    {
        _proc->stop();
        _proc.reset();
    }
}

}
#endif
