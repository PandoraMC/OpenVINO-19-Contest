#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <memory>
#include <string>
#include <samples/common.hpp>
#include <math.h>
#include "ImageProcessor.h"
#include "StringHandler.h"
#include "OpenVINOHandler.h"
#include <inference_engine.hpp>
#include <ext_list.hpp>

#define DIR_OFFSET 2
#define LEFT_KEY 83
#define RIGHT_KEY 81
#define ESC_KEY 27

using namespace cv;
using namespace std;
using namespace InferenceEngine;

typedef chrono::high_resolution_clock Time;
typedef chrono::duration<double, ratio<1, 1000>> ms;
typedef chrono::duration<float> fsec;

static UNUSED Blob::Ptr wrapMat2Blob(const Mat &mat);

const string lblClasses[3] = {"Common Nevus", "Atypical Nevus", "Melanoma"};

int main(int argc, char** argv){
	int fileNumber = 0, filesAvailable, key = 83;
	string fileName, inSegLayerName, inClassLayerName, outSegLayerName, outClassLayerName;
	srand(time(NULL));
	char *imageDir = getDirectory(argv, 1);
	list<string> lstFileNames = getFileNames(imageDir);
	filesAvailable = lstFileNames.size() - DIR_OFFSET;
	fileNumber = rand()%filesAvailable;
	cout << "Image directory " << imageDir << " with " << filesAvailable << " files available" << endl;
	char *modelDir = getDirectory(argv, 2);
	char *SegModelName = *(argv + 3);
	char *ClassModelName = *(argv + 4);
	string device = "HETERO:FPGA,CPU";
	string wndName = "Dermoscopic Image Processor";
	Mat inImage, outImage;
	float Ecc;
	int Area;
	cout << "Segmentation model path: " << modelDir << SegModelName << ".*" << endl;	
	cout << "Classification model path: " << modelDir << ClassModelName << ".*" << endl;
	// 1. Load inference Engine instance
	Core InfEngine;
	CNNNetwork SegNet, ClassNet;
	ExecutableNetwork SegExcNet, ClassExcNet;
	InferRequest SegInfRequest, ClassInfRequest;
	try{
		InfEngine.AddExtension(make_shared<Extensions::Cpu::CpuExtensions>(), "CPU");
		InfEngine.SetConfig({ { PluginConfigParams::KEY_DEVICE_ID, {} } }, "FPGA");
		cout << "Device:" << endl << InfEngine.GetVersions("HETERO:FPGA,CPU") << endl;
		// 2. Read IR Generated by ModelOptimizer (.xml and .bin files)
		SegNet = ReadModel(modelDir, SegModelName);	
		ClassNet = ReadModel(modelDir, ClassModelName);
		QueryNetworkResult SegRes = InfEngine.QueryNetwork(SegNet, device, { });
		QueryNetworkResult ClassRes = InfEngine.QueryNetwork(ClassNet, device, { });
		// 3. Configure input & output
		// 3.1 Input configuration. Mark input as resizable by setting of a resize algorithm.
		configModelInterfaces(SegNet, &inSegLayerName, &outSegLayerName);
		configModelInterfaces(ClassNet, &inClassLayerName, &outClassLayerName);

		cout << endl << "[Segmentation architecture]" << endl;
		printArchitecture(SegNet, InfEngine, device);			
		cout << "[Classification architecture]" << endl;
		printArchitecture(ClassNet, InfEngine, device);			
		// 4. Loading model to the device
		SegExcNet = InfEngine.LoadNetwork(SegNet, device);
		ClassExcNet = InfEngine.LoadNetwork(ClassNet, device);
		// 5. Create infer request
		SegInfRequest = SegExcNet.CreateInferRequest();
		ClassInfRequest = ClassExcNet.CreateInferRequest();
	}catch(const std::exception & ex){
		cout << "Can't get executable graph: " << ex.what() << endl;
		return -1;
	}
	do{
		if(key == 83 || key == 81){
			fileName = getListFileName(lstFileNames, fileNumber);
			string strFilePath = imageDir + fileName;
			inImage = imread(strFilePath, IMREAD_COLOR);   // Read the file
			if(!inImage.data) {                             // Check for invalid input
				cout <<  "Could not open or find the image" << endl ;
				return -1;
			}
			auto t0 = Time::now();
			// 6. Prepare input
			Blob::Ptr imgBlob = wrapMat2Blob(inImage);  // just wrap Mat data by Blob::Ptr without allocating of new memory
			SegInfRequest.SetBlob(inSegLayerName, imgBlob);  // SegInfRequest accepts input blob of any size
			ClassInfRequest.SetBlob(inClassLayerName, imgBlob);  // SegInfRequest accepts input blob of any size			
			// 7. Do inference
			// Running the request synchronously
			float SegTime = measureInference(SegInfRequest);
			float ClassTime = measureInference(ClassInfRequest);

			cout << "Segmentation inference duration: " << SegTime << " ms." << endl << "Throughput: " << 1000/SegTime << " FPS" << endl;
			cout << "Classification inference duration: " << ClassTime << " ms." << endl << "Throughput: " << 1000/ClassTime << " FPS" << endl;

			// 8. Process output
			// 8.1 Segmentation
			Blob::Ptr SegOut = SegInfRequest.GetBlob(outSegLayerName);
			size_t inH, inW;
			size_t SegOut_blob_shape_size = SegOut->getTensorDesc().getDims().size();

			if (SegOut_blob_shape_size == 3) {
				inH = imgBlob->getTensorDesc().getDims().at(1);
				inW = imgBlob->getTensorDesc().getDims().at(2);
			} else if (SegOut_blob_shape_size == 4) {
				inH = imgBlob->getTensorDesc().getDims().at(2);
				inW = imgBlob->getTensorDesc().getDims().at(3);
			} else {
				throw std::logic_error("Unexpected SegOut blob shape. Only 4D and 3D SegOut blobs are supported.");
			}
			Mat classPredict = composeClassSeg(SegOut);
			resize(classPredict, classPredict, Size(inW, inH), 0, 0, INTER_NEAREST);
			outImage = inImage.clone();
			colorObjective(outImage, classPredict, Scalar(255, 0, 255), 0.70, 0);
			// 8.2 Classification
			Blob::Ptr ClassOut = ClassInfRequest.GetBlob(outClassLayerName);
			int prdClass = getClass(ClassOut);
			// 9. Show Data
			drawFeatures(outImage, getObjective(classPredict, 0), &Ecc, &Area);
			Mat imgResults = composePresentation(inImage, outImage);
			setResults(imgResults, Point(10, 5), 3, "Navigate [<-][->], Quit [Esc]", ("File: " + fileName).c_str(), ("Dimensions: " + to_string(inH) + 'x' + to_string(inW) + " px").c_str());
			setResults(imgResults, Point(30 + inW, 5), 3, ("Predicted: " + lblClasses[prdClass]).c_str(), ("Area: " + to_string(Area) + " px^2").c_str(), ("Eccentricity: " + to_string(Ecc)).c_str());

			auto t1 = Time::now();
			fsec fs = t1-t0;
			ms d = chrono::duration_cast<ms>(fs);
			cout << endl <<"Full processing duration: " << d.count() << " ms" << endl;
			cout << "Throughput: " << 1000 / d.count() << " FPS" << endl << endl;

			imshow(wndName, imgResults);
		}
		key = waitKey(0);
		if(key == LEFT_KEY)	fileNumber--;
		if(key == RIGHT_KEY)	fileNumber++;
		fileNumber = (fileNumber > filesAvailable)? 0: (fileNumber < 0)? filesAvailable - 1: fileNumber;
	}while(key != ESC_KEY);
	destroyAllWindows();
	return 0;
}

static UNUSED Blob::Ptr wrapMat2Blob(const Mat &mat) {
    size_t channels = mat.channels();
    size_t height = mat.size().height;
    size_t width = mat.size().width;

    size_t strideH = mat.step.buf[0];
    size_t strideW = mat.step.buf[1];

    bool is_dense = strideW == channels && strideH == channels * width;

    if (!is_dense) THROW_IE_EXCEPTION << "Doesn't support conversion from not dense cv::Mat";

    TensorDesc tDesc(Precision::U8, {1, channels, height, width}, Layout::NHWC);

    return make_shared_blob<uint8_t>(tDesc, mat.data);
}
