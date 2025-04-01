//
//  ViewController.m
//  CameraFileCopy
//
//  Created by Jim Learning on 2025/3/26.
//

// 包含 OpenCV 和 C++ 包装器头文件
// 使用 #ifdef __cplusplus 确保 C++ 代码只在 .mm 文件中编译
#ifdef __cplusplus
#import <opencv2/opencv.hpp>
#import <opencv2/imgcodecs/ios.h> // 可能需要用于 Mat <-> UIImage 转换（虽然这里没直接用）
#include "cfc_wrapper.hpp" // 包含你的 C++ 接口头文件
#endif

#import "ViewController.h"
#import <Photos/Photos.h> // 用于请求权限

@interface ViewController ()
// 私有属性或方法可以放在这里
@end

@implementation ViewController

// --- Lifecycle Methods ---

- (void)viewDidLoad {
    [super viewDidLoad];

    // 初始化状态
    self.modeVal = 0;
    self.detectedMode = 68; // 默认值
    self.isCaptureSessionConfigured = NO;

    // 配置模式开关
    [self.modeSwitch addTarget:self action:@selector(modeSwitchChanged:) forControlEvents:UIControlEventValueChanged];
    // 根据 detectedMode 初始化开关状态（如果需要）
    self.modeSwitch.on = NO; // 初始设为关闭

    // 请求权限并设置摄像头
    [self checkAndRequestCameraPermissionWithCompletion:^(BOOL granted) {
        if (granted) {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self setupCameraSession];
            });
        } else {
            // 在主线程上显示提示，告知用户需要权限
            dispatch_async(dispatch_get_main_queue(), ^{
                [self showPermissionAlert];
            });
        }
    }];

    // 显示提示信息，类似 Android 的 Toast
    [self showToast:@"请在 cimbar.org 编码数据! :)"];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    // 仅当会话已配置且应用可见时启动会话
    if (self.isCaptureSessionConfigured && !self.captureSession.isRunning) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
             [self.captureSession startRunning];
        });
    }
}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    // 应用不可见时停止会话
    if (self.captureSession.isRunning) {
         dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
              [self.captureSession stopRunning];
         });
    }
}

- (void)dealloc {
#ifdef __cplusplus
    // 调用 C++ 清理函数
    shutdownCPP();
#endif
    // 移除 KVO 或通知观察者（如果添加了的话）
    NSLog(@"ViewController deallocated");
}

// --- Permissions ---

- (void)checkAndRequestCameraPermissionWithCompletion:(void (^)(BOOL granted))completion {
    AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    switch (status) {
        case AVAuthorizationStatusAuthorized:
            // 已授权
            completion(YES);
            break;
        case AVAuthorizationStatusNotDetermined: {
            // 未决定，请求权限
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
                completion(granted);
            }];
            break;
        }
        case AVAuthorizationStatusDenied:
        case AVAuthorizationStatusRestricted:
            // 已拒绝或受限
            completion(NO);
            break;
    }
}

- (void)showPermissionAlert {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"需要相机权限"
                                                                   message:@"请在设置中开启相机权限以使用此功能。"
                                                            preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"取消" style:UIAlertActionStyleCancel handler:nil]];
    [alert addAction:[UIAlertAction actionWithTitle:@"去设置" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
        [[UIApplication sharedApplication] openURL:[NSURL URLWithString:UIApplicationOpenSettingsURLString] options:@{} completionHandler:nil];
    }]];
    [self presentViewController:alert animated:YES completion:nil];
}

// --- Camera Setup (AVFoundation) ---

- (void)setupCameraSession {
    if (self.isCaptureSessionConfigured) {
        return; // 防止重复配置
    }

    self.captureSession = [[AVCaptureSession alloc] init];
    // self.captureSession.sessionPreset = AVCaptureSessionPreset1280x720; // 可以尝试设置分辨率，但要确保设备支持

    // 1. 获取输入设备 (后置摄像头)
    AVCaptureDevice *videoDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    if (!videoDevice) {
        NSLog(@"Error: No video device found");
        self.captureSession = nil;
        return;
    }

    // 尝试设置连续自动对焦
    if ([videoDevice isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus]) {
        NSError *lockError = nil;
        if ([videoDevice lockForConfiguration:&lockError]) {
            videoDevice.focusMode = AVCaptureFocusModeContinuousAutoFocus;
            [videoDevice unlockForConfiguration];
        } else {
            NSLog(@"Could not lock device for focus configuration: %@", lockError);
        }
    }

    // 2. 创建输入
    NSError *inputError = nil;
    AVCaptureDeviceInput *videoInput = [AVCaptureDeviceInput deviceInputWithDevice:videoDevice error:&inputError];
    if (!videoInput || inputError) {
        NSLog(@"Error creating video input: %@", inputError);
        self.captureSession = nil;
        return;
    }

    if ([self.captureSession canAddInput:videoInput]) {
        [self.captureSession addInput:videoInput];
    } else {
        NSLog(@"Error: Cannot add video input to session");
        self.captureSession = nil;
        return;
    }

    // 3. 创建输出
    AVCaptureVideoDataOutput *videoDataOutput = [[AVCaptureVideoDataOutput alloc] init];
    // 设置像素格式为 BGRA，这与 OpenCV 的 cv::Mat(height, width, CV_8UC4, data) 兼容
    NSDictionary *videoSettings = @{(id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)};
    videoDataOutput.videoSettings = videoSettings;
    videoDataOutput.alwaysDiscardsLateVideoFrames = YES; // 丢弃延迟的帧，保持实时性

    // 4. 设置输出代理和队列
    // 创建一个串行队列来处理视频帧，避免阻塞主线程
    self.videoDataOutputQueue = dispatch_queue_create("VideoDataOutputQueue", DISPATCH_QUEUE_SERIAL);
    [videoDataOutput setSampleBufferDelegate:self queue:self.videoDataOutputQueue];

    if ([self.captureSession canAddOutput:videoDataOutput]) {
        [self.captureSession addOutput:videoDataOutput];
    } else {
        NSLog(@"Error: Cannot add video output to session");
        self.captureSession = nil;
        self.videoDataOutputQueue = nil;
        return;
    }

    // 5. 创建预览层
    self.previewLayer = [AVCaptureVideoPreviewLayer layerWithSession:self.captureSession];
    self.previewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill; // 填充整个预览视图
    self.previewLayer.frame = self.previewView.bounds; // 设置预览层大小
    [self.previewView.layer addSublayer:self.previewLayer]; // 添加到视图层级

    self.isCaptureSessionConfigured = YES;

    // 配置完成后可以立即启动，或者在 viewWillAppear 中启动
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
         [self.captureSession startRunning];
    });

    NSLog(@"Camera session configured successfully.");
}

// --- AVCaptureVideoDataOutputSampleBufferDelegate ---

- (void)captureOutput:(AVCaptureOutput *)output didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
    // 检查会话是否还在运行，以及是否有 C++ 支持
#ifndef __cplusplus
    return; // 如果没有 C++ 环境，直接返回
#else
    if (!self.captureSession.isRunning) {
        return;
    }

    // 1. 从 CMSampleBufferRef 获取 CVPixelBufferRef
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (!imageBuffer) {
        return;
    }

    // 2. 锁定 Pixel Buffer 以访问其内存
    // 使用 kCVPixelBufferLock_ReadOnly 因为我们通常只读（虽然 C++ 函数可能修改 Mat 用于预览）
    // 如果 C++ 函数确实修改了 Mat 数据并希望它反映回 CVPixelBuffer（通常不这样做），
    // 则需要使用 0 (读写锁) 并在结束后解锁。
    if (CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess) {
        NSLog(@"Failed to lock pixel buffer");
        return;
    }

    // 3. 获取 Pixel Buffer 信息
    void *baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);

    // 4. 创建指向 Pixel Buffer 内存的 cv::Mat (BGRA 格式)
    // 注意：这里没有数据拷贝，Mat 直接引用 imageBuffer 的内存
    cv::Mat matBGRA(static_cast<int>(height), static_cast<int>(width), CV_8UC4, baseAddress, bytesPerRow);

    // 5. 转换颜色空间：AVFoundation 输出是 BGRA，但 Android 示例似乎传递 RGBA 给 JNI。
    // 假设你的 C++ 函数 processImageCPP 期望 RGBA 格式。
    cv::Mat matRGBA;
    cv::cvtColor(matBGRA, matRGBA, cv::COLOR_BGRA2RGBA);

    // --- 调用 C++ 处理函数 ---
    std::string result = "";
    try {
        // 获取临时文件目录路径
        std::string dataPathStd = [NSTemporaryDirectory() UTF8String];
        // 调用 C++ 处理函数
        result = processImageCPP(matRGBA, dataPathStd, self.modeVal);
    } catch (const cv::Exception& e) {
        NSLog(@"OpenCV Exception: %s", e.what());
    } catch (const std::exception& e) {
        NSLog(@"Standard Exception: %s", e.what());
    } catch (...) {
        NSLog(@"Unknown C++ Exception occurred");
    }
    // --- C++ 处理结束 ---

    // 6. 解锁 Pixel Buffer
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);

    // 7. 处理 C++ 返回的结果 (在主线程进行 UI 更新或跳转)
    if (!result.empty()) {
        NSString *resultString = [NSString stringWithUTF8String:result.c_str()];
        dispatch_async(dispatch_get_main_queue(), ^{
            [self handleProcessResult:resultString];
        });
    }

    // 注意：如果 C++ 函数修改了 matRGBA 并且你想在预览中看到变化，
    // 你需要将 matRGBA 转换回 BGRA，然后写回 CVPixelBuffer（如果锁定了读写）。
    // 或者，更常见的是，让 C++ 函数返回修改后的 Mat，然后在主线程上
    // 将其转换为 UIImage 并显示在单独的 UIImageView 上。
    // 这里我们假设 C++ 函数只是分析图像，或者修改 Mat 用于其内部目的。

#endif // __cplusplus
}

// --- Result Handling ---

- (void)handleProcessResult:(NSString *)resultString {
    if ([resultString hasPrefix:@"/"]) {
        // --- Mode Detection ---
        NSLog(@"Mode detected: %@", resultString);
        if (resultString.length >= 2) {
            unichar modeChar = [resultString characterAtIndex:1];
            if (modeChar == '4') {
                self.detectedMode = 4;
                // 更新 Switch 状态，确保它被选中并反映新模式
                 if (!self.modeSwitch.isOn) {
                    [self.modeSwitch setOn:YES animated:YES];
                 }
                 self.modeVal = self.detectedMode; // 确保 modeVal 更新
            } else {
                 self.detectedMode = 68; // 或其他默认/检测到的模式
                 if (!self.modeSwitch.isOn) {
                    [self.modeSwitch setOn:YES animated:YES];
                 }
                 self.modeVal = self.detectedMode; // 确保 modeVal 更新
            }
             // 可以根据需要更新 UI 提示当前检测到的模式类型
        }
    } else if (resultString.length > 0) {
        // --- File Received ---
        NSLog(@"File received: %@", resultString);
        NSString *filename = resultString;
        NSString *tempFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:filename];
        self.activePath = tempFilePath;

        // 弹出文件保存对话框
        [self presentDocumentPickerForExport:filename];
    }
}

// --- UI Actions ---

- (IBAction)modeSwitchChanged:(UISwitch *)sender {
    if (sender.isOn) {
        self.modeVal = self.detectedMode; // 使用检测到的模式
    } else {
        self.modeVal = 0; // 关闭模式
    }
    NSLog(@"Mode value changed to: %d", self.modeVal);
}

// --- File Saving (UIDocumentPickerViewController) ---

- (void)presentDocumentPickerForExport:(NSString *)filename {
    if (!self.activePath) {
        NSLog(@"Error: No active path to export.");
        return;
    }

    NSURL *fileURL = [NSURL fileURLWithPath:self.activePath];

    // 使用 UIDocumentPickerViewController 导出文件
    // asCopy:YES 表示系统会复制文件到用户选择的位置
    UIDocumentPickerViewController *picker;
    if (@available(iOS 14.0, *)) {
         // 使用 UTType (推荐)
         // 假设是通用二进制数据
        picker = [[UIDocumentPickerViewController alloc] initForExportingURLs:@[fileURL] asCopy:YES];
    } else {
        // Fallback on earlier versions (可能需要不同的初始化)
        // 对于导出，旧版本可能行为不同，这里简化处理
        picker = [[UIDocumentPickerViewController alloc] initWithURL:fileURL inMode:UIDocumentPickerModeExportToService];
    }

    if (picker) {
        picker.delegate = self;
        // 可以设置默认目录等 picker.directoryURL = ...
        [self presentViewController:picker animated:YES completion:nil];
    } else {
        NSLog(@"Failed to create document picker.");
        // 清理临时文件，因为无法保存
        [self cleanupTemporaryFile];
    }
}

// --- UIDocumentPickerDelegate Methods ---

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    // 用户选择了保存位置，并且系统（因为 asCopy:YES）已经处理了复制
    NSLog(@"File exported successfully to: %@", urls.firstObject);
    // 清理临时文件
    [self cleanupTemporaryFile];
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
    // 用户取消了保存
    NSLog(@"Document picker was cancelled.");
    // 清理临时文件
    [self cleanupTemporaryFile];
}

// --- Utility Methods ---

- (void)cleanupTemporaryFile {
    if (self.activePath) {
        NSError *error = nil;
        if ([[NSFileManager defaultManager] removeItemAtPath:self.activePath error:&error]) {
            NSLog(@"Temporary file deleted: %@", self.activePath);
        } else {
            NSLog(@"Error deleting temporary file: %@ - Error: %@", self.activePath, error);
        }
        self.activePath = nil; // 重置路径
    }
}

- (void)showToast:(NSString *)message {
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil
                                                                   message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];
    [self presentViewController:alert animated:YES completion:nil];
    // 2秒后自动消失
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [alert dismissViewControllerAnimated:YES completion:nil];
    });
}

// 确保预览层在设备旋转或视图布局改变时调整大小
- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    self.previewLayer.frame = self.previewView.bounds;
}

@end
