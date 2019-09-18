
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <iomanip>

#include <cv.h>
#include <opencv2/opencv.hpp>
#include <highgui.h>
#include <eigen3/Eigen/Dense>
#include "System.h"

using namespace std;
using namespace cv;
using namespace Eigen;

const int nDelayTimes = 2;
string sData_path = "/home/liu/my_prog/Dataset/EuRoC/MH_05_difficult/mav0";
string sConfig_path = "../config/";

std::shared_ptr<System> pSystem;

void PubImuData()
{
	string sImu_data_file = sData_path + "/imu0/data.csv";
	cout << "1 PubImuData start sImu_data_filea: " << sImu_data_file << endl;
	ifstream fsImu;
	fsImu.open(sImu_data_file.c_str());
	if (!fsImu.is_open())
	{
		cerr << "Failed to open imu file! " << sImu_data_file << endl;
		return;
	}

	std::string sImu_line;
	double dStampNSec = 0.0;
	Vector3d vAcc;
	Vector3d vGyr;

	std::string firstLine;
	std::getline(fsImu, firstLine);
	while (std::getline(fsImu, sImu_line) && !sImu_line.empty()) // read imu data
	{
		std::vector<double > outputStr;
		std::stringstream ss(sImu_line);
		std::string str;
		while (std::getline(ss, str, ','))
		{
			outputStr.push_back(std::stod(str));
		}
		dStampNSec = outputStr[0];
		vGyr.x() = outputStr[1];
		vGyr.y() = outputStr[2];
		vGyr.z() = outputStr[3];
		vAcc.x() = outputStr[4];
		vAcc.y() = outputStr[5];
		vAcc.z() = outputStr[6];

		// cout << "Imu t: " << fixed << dStampNSec << " gyr: " << vGyr.transpose() << " acc: " << vAcc.transpose() << endl;
		pSystem->PubImuData(dStampNSec / 1e9, vGyr, vAcc);
		usleep(5000*nDelayTimes);
	}
	fsImu.close();
}

void PubImageData()
{
	string sImage_file = sData_path + "/cam0/data.csv";

	cout << "1 PubImageData start sImage_file: " << sImage_file << endl;

	ifstream fsImage;
	fsImage.open(sImage_file.c_str());
	if (!fsImage.is_open())
	{
		cerr << "Failed to open image file! " << sImage_file << endl;
		return;
	}

	std::string sImage_line;
	double dStampNSec;
	string sImgFileName;

    std::string firstLine;
    std::getline(fsImage, firstLine);
    while (std::getline(fsImage, sImage_line) && !sImage_line.empty())
	{
        std::vector<std::string > outputStr;
        std::stringstream ss(sImage_line);
        std::string str;
        while (std::getline(ss, str, ','))
        {
            outputStr.push_back(str);
        }
        dStampNSec = std::stod(outputStr[0]);
        sImgFileName = outputStr[1];

		std::string imagePath = sData_path + "/cam0/data/" + sImgFileName;
		imagePath.resize(imagePath.size() - 1);
		cv::Mat img = cv::imread(imagePath.c_str(), 0);

		if (img.empty())
		{
			cerr << "image is empty! path: " << imagePath << endl;
			return;
		}
		pSystem->PubImageData(dStampNSec / 1e9, img);
		usleep(50000*nDelayTimes);
	}

	fsImage.close();
}

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		cerr << "./run_euroc PATH_TO_FOLDER/MH-05/mav0 PATH_TO_CONFIG/config \n" 
			<< "For example: ./run_euroc /home/stevencui/dataset/EuRoC/MH-05/mav0/ ../config/"<< endl;
		return -1;
	}
	sData_path = argv[1];
	sConfig_path = argv[2];

	pSystem.reset(new System(sConfig_path));
	
	std::thread thd_BackEnd(&System::ProcessBackEnd, pSystem);
		
	// sleep(5);
	std::thread thd_PubImuData(PubImuData);

	std::thread thd_PubImageData(PubImageData);
	
	std::thread thd_Draw(&System::Draw, pSystem);
	
	thd_PubImuData.join();
	thd_PubImageData.join();

	// thd_BackEnd.join();
	// thd_Draw.join();

	cout << "main end... see you ..." << endl;
	return 0;
}
