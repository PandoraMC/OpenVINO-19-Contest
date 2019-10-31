#ifndef __IMAGEPROCESSOR_H__
#define __IMAGEPROCESSOR_H__

#include <opencv2/core.hpp>
#include <inference_engine.hpp>
#include <stdarg.h>

void drawFeatures(cv::Mat refImage, cv::Mat binImage, float *Ecc, int *Area);
void BBPoints(cv::Mat binImage, float angle, cv::Point *A, cv::Point *B, cv::Point *C, cv::Point *D);
cv::Mat composeClassSeg(InferenceEngine::Blob::Ptr ptrBlobProb);
cv::Mat colorObjective(cv::Mat imageMat, cv::Mat classMat, cv::Scalar colorMask, float alpha, int objetive);
int getClass(InferenceEngine::Blob::Ptr ptrBlobProb);
cv::Mat getObjective(cv::Mat Classes, int obj);
cv::Mat composePresentation(cv::Mat imgReference, cv::Mat imgResults);
void setResults(cv::Mat imgFrame, cv::Point pntReference, int strNumber, ...);

#endif
