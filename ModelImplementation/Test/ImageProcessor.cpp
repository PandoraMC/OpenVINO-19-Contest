#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <inference_engine.hpp>
#include "ImageProcessor.h"

using namespace cv;
using namespace std;
using namespace InferenceEngine;

void drawFeatures(Mat refImage, Mat binImage, float *Ecc, int *Area){
	Moments mnt = moments(binImage, false);
	*Area = mnt.m00;
	float cx = mnt.m10/(*Area);
	float cy = mnt.m01/(*Area);
	float angle = atan2(2*mnt.mu11, mnt.mu20 - mnt.mu02)/2;
	float a1 = (mnt.mu20 + mnt.mu02 + sqrt(pow(mnt.mu20 - mnt.mu02, 2) + 4*pow(mnt.mu11, 2)));
	float a2 = (mnt.mu20 + mnt.mu02 - sqrt(pow(mnt.mu20 - mnt.mu02, 2) + 4*pow(mnt.mu11, 2)));
	*Ecc = a1/a2;
	float ra = sqrt(2*a1/(*Area));
	float rb = sqrt(2*a2/(*Area));
	/*float ebx = cos(angle), eby = sin(angle);
	float eax = sin(angle), eay = -cos(angle);
	size_t H = binImage.size().height;
	size_t W = binImage.size().width;
	float amin = W, amax = 0.0;
	float bmin = H, bmax = 0.0;
	for (size_t w = 0; w < W; w++)
		for (size_t h = 0; h < H; h++)
			if((int)binImage.at<uchar>(h, w)){
				float a = w*eax + h*eay;
				amin = min(amin, a);
				amax = max(amax, a);
				float b = w*ebx + h*eby;
				bmin = min(bmin, b);
				bmax = max(bmax, b);
			}
	Point A(amin*eax + bmin*ebx, amin*eay + bmin*eby);
	Point B(amin*eax + bmax*ebx, amin*eay + bmax*eby);
	Point C(amax*eax + bmax*ebx, amax*eay + bmax*eby);
	Point D(amax*eax + bmin*ebx, amax*eay + bmin*eby);
	line(refImage, A, B, Scalar(0,227,255), 2, LINE_AA);
	line(refImage, B, C, Scalar(217,255,0), 2, LINE_AA);
	line(refImage, C, D, Scalar(0,227,255), 2, LINE_AA);
	line(refImage, D, A, Scalar(217,255,0), 2, LINE_AA);
*/
	line(refImage, Point(cx - ra*cos(angle), cy - ra*sin(angle)), Point(cx + ra*cos(angle), cy + ra*sin(angle)), Scalar(0,227,255), 2, LINE_AA);
	angle += CV_PI/2;
	line(refImage, Point(cx - rb*cos(angle), cy - rb*sin(angle)), Point(cx + rb*cos(angle), cy + rb*sin(angle)), Scalar(217,255,0), 2, LINE_AA);
}

Mat composeClassSeg(Blob::Ptr ptrBlobProb){
	const auto output_data = ptrBlobProb->buffer().as<float*>();
        size_t C, H, W;
	size_t output_blob_shape_size = ptrBlobProb->getTensorDesc().getDims().size();

	if (output_blob_shape_size == 3) {
		C = 1;
		H = ptrBlobProb->getTensorDesc().getDims().at(1);
		W = ptrBlobProb->getTensorDesc().getDims().at(2);
        } else if (output_blob_shape_size == 4) {
		C = ptrBlobProb->getTensorDesc().getDims().at(1);
		H = ptrBlobProb->getTensorDesc().getDims().at(2);
		W = ptrBlobProb->getTensorDesc().getDims().at(3);
        } else {
		throw std::logic_error("Unexpected output blob shape. Only 4D and 3D output blobs are supported.");
        }
	Mat classMat(H, W, CV_8UC1, Scalar(0));
	vector<vector<size_t>> classArray(H, vector<size_t>(W, 0));
	vector<vector<float>> probArray(H, vector<float>(W, 0.));
	for (size_t w = 0; w < W; w++)
		for (size_t h = 0; h < H; h++)
			if (C == 1)
				classArray[h][w] = static_cast<size_t>(output_data[W * h + w]);
			else
				for (size_t classID = 0; classID < C; classID++) {
					auto data = output_data[W * H * classID + W * h + w];
					if (data > probArray[h][w]) {
						classArray[h][w] = classID;
						probArray[h][w] = data;
						classMat.at<uchar>(h,w) = classID;
					}
				}
	return classMat;
}

Mat colorObjective(Mat imageMat, Mat classMat, Scalar colorMask, float alpha, int objetive){
	size_t H = classMat.rows, W = classMat.cols;
	Vec3b pixelData;
	for (size_t w = 0; w < W; w++)
		for (size_t h = 0; h < H; h++)
			if(classMat.at<uchar>(h, w) == objetive){
				pixelData = imageMat.at<Vec3b>(h, w);
				pixelData[0] = pixelData[0]*alpha + colorMask[0]*(1-alpha);
				pixelData[1] = pixelData[1]*alpha + colorMask[1]*(1-alpha);
				pixelData[2] = pixelData[2]*alpha + colorMask[2]*(1-alpha);
				imageMat.at<Vec3b>(h, w) = pixelData;
			}
	return imageMat;
}

int getClass(Blob::Ptr ptrBlobProb){
	size_t ClassOut_size = ptrBlobProb->getTensorDesc().getDims().at(1);
	const auto output_data = ptrBlobProb->buffer().as<float*>();
	int classID = 0;
	float ptr = 0;
	for (size_t prob = 0; prob < ClassOut_size; prob++){
		auto data = output_data[prob];
		if(data > ptr){
			classID = prob;
			ptr = data;
		}
	}
	return classID;
}

Mat getObjective(Mat Classes, int obj){
	Mat classMask(Classes.size(), CV_8UC1, Scalar(0));
	size_t H = Classes.size().height;
	size_t W = Classes.size().width;
	for (size_t w = 0; w < W; w++)
		for (size_t h = 0; h < H; h++)
			classMask.at<uchar>(h, w) = (Classes.at<uchar>(h, w) == obj);
	return classMask;
}
