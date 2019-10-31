#ifndef __OPENVINOHANDLER_H__
#define __OPENVINOHANDLER_H__

#include <iostream>
#include <opencv2/core.hpp>
#include <inference_engine.hpp>
#include <samples/common.hpp>

void configModelInterfaces(InferenceEngine::CNNNetwork Model, std::string *inLayerName, std::string *outLayerName);
InferenceEngine::CNNNetwork ReadModel(char* modelDir, char *modelName);
float measureInference(InferenceEngine::InferRequest Infer);
void printArchitecture(InferenceEngine::CNNNetwork Model, InferenceEngine::Core InfEngine, std::string device);

#endif
