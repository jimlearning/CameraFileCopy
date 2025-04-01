/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */

// 公共导入，不管什么平台都需要
#include <iostream>
#include <string>
#include <vector>

// 导入项目相关头文件
#include "cimbar_js/cimbar_js.h"
#include "cimb_translator/Config.h"
#include "serialize/str.h"
#include "util/File.h"
#include "cxxopts/cxxopts.hpp"

// 平台特定代码
#if defined(__APPLE__) && defined(CIMBAR_IOS_PLATFORM)
    // iOS平台使用POSIX API
    #include <unistd.h>     // 用于usleep
    #include <sys/time.h>   // 用于gettimeofday
#else
    // 非iOS平台使用标准C++库
    #include <chrono>
    #include <thread>
#endif

using std::string;
using std::vector;

namespace {

    // 获取当前时间（毫秒）
    #if defined(__APPLE__) && defined(CIMBAR_IOS_PLATFORM)
    // iOS平台使用gettimeofday
    inline uint64_t get_current_time_millis()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
    }
    #else
    // 非iOS平台使用chrono
    inline uint64_t get_current_time_millis()
    {
        using namespace std::chrono;
        return (uint64_t)duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
    }
    #endif

    // 统一的帧延迟函数
    inline uint64_t wait_for_frame_time(uint32_t delay, uint64_t start_time)
    {
        uint64_t current_time = get_current_time_millis();
        uint64_t elapsed = current_time - start_time;
        
        if (delay > elapsed) {
            uint32_t wait_time = (uint32_t)(delay - elapsed);
    #if defined(__APPLE__) && defined(CIMBAR_IOS_PLATFORM)
            // iOS平台使用usleep
            usleep(wait_time * 1000); // 转换为微秒
    #else
            // 非iOS平台使用std::this_thread::sleep_for
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
    #endif
        }
        
        return get_current_time_millis();
    }

}

int main(int argc, char** argv)
{
	cxxopts::Options options("cimbar video encoder", "Draw a bunch of weird static on the screen!");

	unsigned colorBits = cimbar::Config::color_bits();
	unsigned compressionLevel = cimbar::Config::compression_level();
	unsigned ecc = cimbar::Config::ecc_bytes();
	unsigned defaultFps = 15;
	options.add_options()
		("i,in", "Source file", cxxopts::value<vector<string>>())
		("c,colorbits", "Color bits. [0-3]", cxxopts::value<int>()->default_value(turbo::str::str(colorBits)))
		("e,ecc", "ECC level", cxxopts::value<unsigned>()->default_value(turbo::str::str(ecc)))
		("f,fps", "Target FPS", cxxopts::value<unsigned>()->default_value(turbo::str::str(defaultFps)))
		("m,mode", "Select a cimbar mode. B is new to 0.6.x. 4C is the 0.5.x config. [B,4C]", cxxopts::value<string>()->default_value("4C"))
		("z,compression", "Compression level. 0 == no compression.", cxxopts::value<int>()->default_value(turbo::str::str(compressionLevel)))
		("h,help", "Print usage")
	;
	options.show_positional_help();
	options.parse_positional({"in"});
	options.positional_help("<in...>");

	auto result = options.parse(argc, argv);
	if (result.count("help") or !result.count("in"))
	{
	  std::cout << options.help() << std::endl;
	  return 0;
	}

	vector<string> infiles = result["in"].as<vector<string>>();

	colorBits = std::min(3, result["colorbits"].as<int>());
	compressionLevel = result["compression"].as<int>();
	ecc = result["ecc"].as<unsigned>();

	bool legacy_mode = false;
	if (result.count("mode"))
	{
		string mode = result["mode"].as<string>();
		legacy_mode = (mode == "4c") or (mode == "4C");
	}

	unsigned fps = result["fps"].as<unsigned>();
	if (fps == 0)
		fps = defaultFps;
	unsigned delay = 1000 / fps;

	int window_size = cimbar::Config::image_size() + 32;
	if (!initialize_GL(window_size, window_size))
	{
		std::cerr << "failed to create window :(" << std::endl;
		return 70;
	}

	configure(colorBits, ecc, compressionLevel, legacy_mode);

	// 使用统一的计时API
	uint64_t start = get_current_time_millis();
	while (true)
		for (uint32_t i = 0; i < infiles.size(); ++i)
		{
			// delay, then try to read file
			start = wait_for_frame_time(delay, start);
			// TODO: maybe delay is the wrong thing to do here. Might be best to just kick out any files that fail to read?
			// we can then error out properly if all inputs are bad, which would be nice.
			{
				string contents = File(infiles[i]).read_all();
				if (contents.empty())
				{
					std::cerr << "failed to read file " << infiles[i] << std::endl;
					continue;
				}

				// start encode_id is 109. This is mostly unimportant (it only needs to wrap between [0,127]), but useful
				// for the decoder -- because it gives it a better distribution of colors in the first frame header it sees.
				if (!encode(reinterpret_cast<unsigned char*>(contents.data()), contents.size(), static_cast<int>(i+109)))
				{
					std::cerr << "failed to encode file " << infiles[i] << std::endl;
					continue;
				}
			}

			// after loading our current file, render frames to the screen until next_frame() loops
			int frameCount = 0;
			do {
				start = wait_for_frame_time(delay, start);
				if (render() < 0)
					return 0;
			}
			while (++frameCount == next_frame()); // when next_frame() finally loops, we roll to the next file
		}

	return 0; // should never reach here
}
