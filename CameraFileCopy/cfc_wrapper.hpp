//
//  cfc_wrapper.hpp
//  CameraFileCopy
//
//  Created by Jim Learning on 2025/3/26.
//

#ifndef cfc_wrapper_h
#define cfc_wrapper_h

#include <string>

// 使用前向声明来避免在此头文件中包含完整的 OpenCV 头文件
// 这样可以加快编译速度，减少依赖
namespace cv {
    class Mat;
}

/**
 * @brief 处理图像帧。
 * @param image 输入/输出的 cv::Mat 图像 (期望是 RGBA 格式)。函数可能会修改它用于预览。
 * @param dataPath 用于存储临时文件的目录路径。
 * @param mode 当前的操作模式。
 * @return 如果检测到模式，返回以 "/" 开头的字符串 (例如 "/4")；
 *         如果完成文件传输，返回文件名；
 *         如果什么都没发生，返回空字符串。
 */
std::string processImageCPP(cv::Mat& image, const std::string& dataPath, int mode);

/**
 * @brief 清理 C++ 库使用的任何资源。
 */
void shutdownCPP();

#endif /* cfc_wrapper_h */

