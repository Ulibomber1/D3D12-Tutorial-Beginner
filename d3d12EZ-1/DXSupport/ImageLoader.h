#pragma once

#include <WinSupport/WinInclude.h>
#include <WinSupport/ComPointer.h>

#include <vector>
#include <filesystem>
#include <algorithm>

#define __ImageLoader_CAR(expr) do {if(FAILED(expr)) {return false;}} while(false)


class ImageLoader
{
public:
	struct ImageData
	{
		std::vector<char> data = {};
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t bpp = 0; // bits per pixel
		uint32_t cc = 0; // channel count

		GUID wicpixelFormat = {};
		DXGI_FORMAT giPixelFormat = {};
	};

	static bool LoadImageFromDisk(const std::filesystem::path& imagePath, ImageData& data);

private:
	struct GUID_to_DXGI
	{
		GUID wic;
		DXGI_FORMAT gi;
	};

	static const std::vector<GUID_to_DXGI> s_lookupTable;

private:
	ImageLoader() = default;
	ImageLoader(const ImageLoader&) = default;
	ImageLoader& operator=(const ImageLoader&) = default;
};