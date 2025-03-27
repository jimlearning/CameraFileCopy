/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <cstdio>
// 检测平台
#if defined(__APPLE__)
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE
		// iOS平台: 使用iOS特定的文件操作API
		#define CIMBAR_IOS_PLATFORM
		#include <string>
		#include <sys/stat.h>
		#include <unistd.h>
	#else
		// macOS平台: 使用标准库的filesystem
		#include <filesystem>
		namespace fs = std::filesystem;
	#endif
#else
	// 其他平台: 使用experimental/filesystem
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif
#include <string>

// 为iOS平台定义路径类型
#ifdef CIMBAR_IOS_PLATFORM
	typedef std::string path_type;
	
	// 为iOS提供路径组合操作
	inline path_type operator/(const path_type& lhs, const path_type& rhs)
	{
		if (lhs.empty())
			return rhs;
		if (rhs.empty())
			return lhs;
			
		if (lhs.back() == '/')
			return lhs + rhs;
		else
			return lhs + "/" + rhs;
	}
	
	inline path_type operator/(const path_type& lhs, const char* rhs)
	{
		return lhs / path_type(rhs);
	}
#else
	typedef fs::path path_type;
#endif

class MakeTempDirectory
{
public:
	MakeTempDirectory(bool cleanup=true)
		: _cleanup(cleanup)
	{
#ifdef CIMBAR_IOS_PLATFORM
		// iOS平台特定实现
		char tmpdir[L_tmpnam];
		std::tmpnam(tmpdir);
		_path = tmpdir;
		// 使用mkdir替代filesystem的create_directory
		mkdir(_path.c_str(), 0700);
#else
		// 其他平台实现
		_path = std::tmpnam(nullptr);
		fs::create_directory(_path);
#endif
	}

	~MakeTempDirectory()
	{
		if (_cleanup)
		{
#ifdef CIMBAR_IOS_PLATFORM
			// iOS平台特定实现
			remove(_path.c_str());
#else
			// 其他平台实现
			std::error_code ec;
			fs::remove_all(_path, ec);
#endif
		}
	}

	// 统一接口，返回路径
	path_type path() const
	{
		return _path;
	}

protected:
	bool _cleanup;
	path_type _path;
};
