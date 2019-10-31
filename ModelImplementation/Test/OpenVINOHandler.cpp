#include <iostream>
#include <opencv2/core.hpp>
#include <inference_engine.hpp>
#include <samples/common.hpp>
#include "OpenVINOHandler.h"
#include "StringHandler.h"

using namespace cv;
using namespace std;
using namespace InferenceEngine;

typedef chrono::high_resolution_clock Time;
typedef chrono::duration<double, ratio<1, 1000>> ms;
typedef chrono::duration<float> fsec;

void printArchitecture(CNNNetwork Model, Core InfEngine, string device){
	QueryNetworkResult Network = InfEngine.QueryNetwork(Model, device, { });
	for (auto && layer : Network.supportedLayersMap)
		cout << layer.first << endl;
	cout << endl << "Input layer name: " << Model.getInputsInfo().begin()->first << endl;
	cout << "Output layer name: " << Model.getOutputsInfo().begin()->first << endl << endl;
}

float measureInference(InferRequest Inference){
	auto t0 = Time::now();
	Inference.Infer();
	auto t1 = Time::now();
	fsec fs = t1-t0;
	ms d = chrono::duration_cast<ms>(fs);
	return d.count();
}

void configModelInterfaces(CNNNetwork Model, string *inLayerName, string *outLayerName){
	InputInfo::Ptr ModelInputInfo = Model.getInputsInfo().begin()->second;
	*inLayerName = Model.getInputsInfo().begin()->first;
        ModelInputInfo->setLayout(Layout::NHWC);
        ModelInputInfo->setPrecision(Precision::U8);
	ModelInputInfo->getPreProcess().setResizeAlgorithm(RESIZE_BILINEAR);
	DataPtr ModelOutputInfo = Model.getOutputsInfo().begin()->second;
        *outLayerName = Model.getOutputsInfo().begin()->first;
        ModelOutputInfo->setPrecision(Precision::FP16);
}

CNNNetwork ReadModel(char* modelDir, char *modelName){
	CNNNetReader NetReader;
	NetReader.ReadNetwork(composeFilePath(modelDir, modelName, (char*)".xml"));
	NetReader.ReadWeights(composeFilePath(modelDir, modelName, (char*)".bin"));
        return NetReader.getNetwork();
}
