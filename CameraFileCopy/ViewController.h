//
//  ViewController.h
//  CameraFileCopy
//
//  Created by Jim Learning on 2025/3/26.
//

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h> // For UIDocumentPickerViewController with UTTypes (iOS 14+)

//遵循必要的协议
@interface ViewController : UIViewController <AVCaptureVideoDataOutputSampleBufferDelegate, UIDocumentPickerDelegate>

// --- UI Elements ---
// 在 Storyboard 或代码中连接这些
@property (weak, nonatomic) IBOutlet UIView *previewView; // 用于显示摄像头预览的视图
@property (weak, nonatomic) IBOutlet UISwitch *modeSwitch; // 模式切换开关

// --- AVFoundation Properties ---
@property (nonatomic, strong) AVCaptureSession *captureSession;
@property (nonatomic, strong) AVCaptureVideoPreviewLayer *previewLayer;
@property (nonatomic, strong) dispatch_queue_t videoDataOutputQueue; // 处理视频帧的队列
@property (nonatomic, assign) BOOL isCaptureSessionConfigured;

// --- State Properties ---
@property (nonatomic, assign) int modeVal;           // 当前模式值 (0 或 detectedMode)
@property (nonatomic, assign) int detectedMode;      // C++库检测到的模式 (例如 4 或 68)
@property (nonatomic, strong) NSString *activePath;    // 当前正在处理的临时文件的完整路径

@end

