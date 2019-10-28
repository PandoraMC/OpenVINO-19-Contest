#ifndef __IMAGEPROCESSOR_H__
#define __IMAGEPROCESSOR_H__

#include <opencv2/core.hpp>
#include <inference_engine.hpp>

void drawFeatures(cv::Mat refImage, cv::Mat binImage, float *Ecc, int *Area);
cv::Mat composeClassSeg(InferenceEngine::Blob::Ptr ptrBlobProb);
cv::Mat colorObjective(cv::Mat imageMat, cv::Mat classMat, cv::Scalar colorMask, float alpha, int objetive);
int getClass(InferenceEngine::Blob::Ptr ptrBlobProb);
cv::Mat getObjective(cv::Mat Classes, int obj);

#endif
