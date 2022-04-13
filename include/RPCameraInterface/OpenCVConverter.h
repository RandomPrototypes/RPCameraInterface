#pragma once

#include <memory>
#include "ImageData.h"
#include <opencv2/opencv.hpp>

namespace RPCameraInterface
{


inline std::shared_ptr<ImageData> createImageDataFromMat(const cv::Mat& img, uint64_t timestampMs, bool with_copy = true)
{
	ImageFormat imgFormat;
	if(img.type() == CV_8UC1)
		imgFormat.type = ImageType::GRAY8;
	else if(img.type() == CV_8UC3)
		imgFormat.type = ImageType::BGR24;
	else return std::shared_ptr<ImageData>();

	imgFormat.width = img.cols;
	imgFormat.height = img.rows;
	std::shared_ptr<ImageData> imgData = createImageData(imgFormat);
	int size = img.cols * img.rows * img.channels();
	if(with_copy) {
		imgData->allocData(size);
		memcpy(img.data, imgData->getDataPtr(), size);
	} else {
		imgData->setDataPtr(img.data);
		imgData->setDataSize(size);
	}
	imgData->setDataReleasedWhenDestroy(with_copy);
	imgData->setTimestamp(timestampMs);
	return imgData;
}

}
