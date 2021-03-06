// BSD 3-Clause License

// Copyright (c) 2017, Rishabh Biyani
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.

// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.

// * Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//         SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


/**
* @file VisionManger.cpp Class Implementations to detect the object
* and return the co-ordinates in the camera frame
* @author rishabh1b(Rishabh Biyani)
* @copyright BSD 3-Clause License (c) Rishabh Biyani 2017
*/


#include "robotarm_grasping/VisionManager.h"

VisionManager::VisionManager(float length, float breadth) {
	this->table_length = length;
	this->table_breadth = breadth;
	pixel_size_identified = false;
}

void VisionManager::get2DLocation(cv::Mat img, float&x, float& y) {
	this->curr_img = img;
	img_centre_x_ = img.rows / 2;
	img_centre_y_ = img.cols / 2;
	if (!pixel_size_identified) {
		detectTable();
	}
	detect2DObject(x, y);
	convertToMM(x, y);
}

void VisionManager::detectTable() {
	// Extract Table from the image and assign values to pixel_per_mm fields
	cv::Mat RGB[3];
	cv::Mat image = curr_img.clone();
	split(image, RGB);
	cv::Mat gray_image_red = RGB[2];
	cv::Mat gray_image_green = RGB[1];
	cv::Mat denoiseImage;
    cv::medianBlur(gray_image_red, denoiseImage, 3);

	// Threshold the Image
	cv::Mat binaryImage = denoiseImage;
	for (int i = 0; i < binaryImage.rows; i++) {
		for (int j = 0; j < binaryImage.cols; j++) {
			int editValue = binaryImage.at<uchar>(i, j);
			int editValue2 = gray_image_green.at<uchar>(i, j);

			if ((editValue > 34) && (editValue < 50) && (editValue2 > 18) && (editValue2 < 30)) { // check whether value is within range.
				binaryImage.at<uchar>(i, j) = 255;
			} else {
				binaryImage.at<uchar>(i, j) = 0;
			}
		}
	}
	dilate(binaryImage, binaryImage, cv::Mat());

	// Get the centroid of the of the blob
	std::vector<cv::Point> nonZeroPoints;
	cv::findNonZero(binaryImage, nonZeroPoints);
	cv::Rect bbox = cv::boundingRect(nonZeroPoints);
	cv::Point pt;
	pt.x = bbox.x + bbox.width / 2;
	pt.y = bbox.y + bbox.height / 2;
	cv::circle(image, pt, 2, cv::Scalar(0, 0, 255), -1, 8);

	// Update pixels_per_mm fields
	pixels_permm_y = bbox.height / table_length;
	pixels_permm_x = bbox.width / table_breadth;

	// Test the conversion values
	std::cout << "Pixels in y" << pixels_permm_y << std::endl;
	std::cout << "Pixels in x" << pixels_permm_x << std::endl;

	// Update the boolean to indicate measurement has been received
	pixel_size_identified = true;

	// Draw Contours - For Debugging
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(binaryImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
	for (int i = 0; i < contours.size(); i++) {
		cv::Scalar color = cv::Scalar(255, 0, 0);
		cv::drawContours(image, contours, i, color, 1, 8, hierarchy, 0, cv::Point() );
	}

	// cv::namedWindow("Table Detection", cv::WINDOW_AUTOSIZE);
	// cv::imshow("Table Detection", image);
}

void VisionManager::detect2DObject(float& pixel_x, float& pixel_y) {
	// Implement Color Thresholding and contour findings to get the location of object to be grasped in 2D
	cv::Mat image, gray_image_green;
	cv::Mat RGB[3];
	image = curr_img.clone();
	cv::split(image, RGB);

	gray_image_green = RGB[1];

	// Denoise the Image
	cv::Mat denoiseImage;
	cv::medianBlur(gray_image_green, denoiseImage, 3);

	// Threshold the Image
	cv::Mat binaryImage;
	cv::threshold(denoiseImage, binaryImage, 160, 255, cv::THRESH_BINARY);

	// Get the centroid of the of the blob
	std::vector<cv::Point> nonZeroPoints;
	cv::findNonZero(binaryImage, nonZeroPoints);
	cv::Rect bbox = cv::boundingRect(nonZeroPoints);
	cv::Point pt;
	pixel_x = bbox.x + bbox.width / 2;
	pixel_y = bbox.y + bbox.height / 2;

	// For Drawing
	pt.x = bbox.x + bbox.width / 2;
	pt.y = bbox.y + bbox.height / 2;
	cv::circle(image, pt, 2, cv::Scalar(0, 0, 255), -1, 8);

	// Draw Contours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(binaryImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	for ( int i = 0; i < contours.size(); i++ ) {
		cv::Scalar color = cv::Scalar(255, 0, 0);
		cv::drawContours(image, contours, i, color, 1, 8, hierarchy, 0, cv::Point());
	}

	// cv::namedWindow("Centre point", cv::WINDOW_AUTOSIZE);
	// cv::imshow("Centre point", image);
}

void VisionManager::convertToMM(float& x, float& y) {
	// Convert from pixel to world co-ordinates in the camera frame
	x = (x - img_centre_x_) / pixels_permm_x;
	y = (y - img_centre_y_) / pixels_permm_y;
}

// Temporary Main Function for testing- This should go away later
/*
int main(int argc, char** argv ) {
	if ( argc != 2 )
    {
        printf("usage: VisionManager <Image_Path>\n");
        return -1;
    }

    cv::Mat image;
    image = cv::imread( argv[1], 1 );

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }

    float length = 1;
    float breadth = 0.6;
    float obj_x, obj_y;

    VisionManager vm(length, breadth);
    vm.get2DLocation(image, obj_x, obj_y);
    std::cout<< " X-Co-ordinate in Camera Frame :" << obj_x << std::endl;
    std::cout<< " Y-Co-ordinate in Camera Frame :" << obj_y << std::endl;

    cv::waitKey(0);
}*/




