//-------------------------------------------------------------
/**
\file      GxSingleCamColor.cpp
\brief     The Angle of the extracted image is detected
\version   1.0.1901.9311
\date      2021.5.17
*/
//-------------------------------------------------------------

#include "GxIAPI.h"
#include "DxImageProc.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

/*********JW************/
//#include "saveData.h"
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <vector>
/***********JW*************/

using namespace cv;
using namespace std;

#define ACQ_BUFFER_NUM          5               ///< Acquisition Buffer Qty.
#define ACQ_TRANSFER_SIZE       (64 * 1024)     ///< Size of data transfer block
#define ACQ_TRANSFER_NUMBER_URB 64              ///< Qty. of data transfer block
#define FILE_NAME_LEN           50              ///< Save image file name length

#define PIXFMT_CVT_FAIL             -1             ///< PixelFormatConvert fail
#define PIXFMT_CVT_SUCCESS          0              ///< PixelFormatConvert success

/************JW***************/
//截图
#define x_position  100
#define y_position  200
#define x_offset 500
#define y_offset 300
/************JW***************/

//Show error message
#define GX_VERIFY(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)     \
    {                                      \
        GetErrorString(emStatus);          \
        return emStatus;                   \
    }

//Show error message, close device and lib
#define GX_VERIFY_EXIT(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)     \
    {                                      \
        GetErrorString(emStatus);          \
        GXCloseDevice(g_hDevice);          \
        g_hDevice = NULL;                  \
        GXCloseLib();                      \
        printf("<App Exit!>\n");           \
        return emStatus;                   \
    }

GX_DEV_HANDLE g_hDevice = NULL;                     ///< Device handle
bool g_bColorFilter = false;                        ///< Color filter support flag
int64_t g_i64ColorFilter = GX_COLOR_FILTER_NONE;    ///< Color filter of device
bool g_bAcquisitionFlag = false;                    ///< Thread running flag
bool g_bSavePPMImage = false;                       ///< Save raw image flag
pthread_t g_nAcquisitonThreadID = 0;                ///< Thread ID of Acquisition thread

unsigned char* g_pRGBImageBuf = NULL;               ///< Memory for RAW8toRGB24
unsigned char* g_pRaw8Image = NULL;                 ///< Memory for RAW16toRAW8

Mat m_image,CannyOut,imgRGB, BinaryImage,BlurImage,cut_image;
int64_t m_width = 0;
int64_t m_height = 0;
char* m_rgb_image = NULL;
int64_t m_pixel_color = 0;
int64_t g_nPayloadSize = 0;
bool m_isImplemented = false;                       ///< Payload size
VideoWriter g_vid;

//Allocate the memory for pixel format transform
void PreForAcquisition();

//Release the memory allocated
void UnPreForAcquisition();

//Convert frame date to suitable pixel format
int PixelFormatConvert(PGX_FRAME_BUFFER);

//Save one frame to PPM image file
void SavePPMFile(uint32_t, uint32_t);

//Acquisition thread function
void *ProcGetImage(void*);

//Get description of error
void GetErrorString(GX_STATUS);



int main()
{
    printf("\n");
    printf("-------------------------------------------------------------\n");
    printf(" The Angle of the extracted image is detected.\n");
    printf("version: 1.0.1901.9311\n");
    printf("-------------------------------------------------------------\n");
    printf("\n");
    printf("Initializing......");
    printf("\n\n");

    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    uint32_t ui32DeviceNum = 0;

    //Initialize libary
    emStatus = GXInitLib();
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        return emStatus;
    }

    //Get device enumerated number
    emStatus = GXUpdateDeviceList(&ui32DeviceNum, 1000);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        GXCloseLib();
        return emStatus;
    }

    //If no device found, app exit
    if(ui32DeviceNum <= 0)
    {
        printf("<No device found>\n");
        GXCloseLib();
        return emStatus;
    }

    //Open first device enumerated
    emStatus = GXOpenDeviceByIndex(1, &g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        GXCloseLib();
        return emStatus;
    }


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	emStatus = GXIsImplemented(g_hDevice,GX_ENUM_PIXEL_COLOR_FILTER,&m_isImplemented);
	GXGetInt(g_hDevice,GX_INT_WIDTH,&m_width);
	GXGetInt(g_hDevice,GX_INT_HEIGHT,&m_height);
	if(m_isImplemented)
	{
		m_image.create(m_height,m_width,CV_8UC3);
		m_rgb_image = new char[m_width*m_height*3];
	}
	else
	{
		m_image.create(m_height,m_width,CV_8UC1);
	}
	emStatus = GXSetFloat(g_hDevice,GX_FLOAT_EXPOSURE_TIME,50000);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //Get Device Info
    printf("***********************************************\n");
    //Get libary version
    printf("<Libary Version : %s>\n", GXGetLibVersion());
    size_t nSize = 0;
    //Get string length of Vendor name
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_VENDOR_NAME, &nSize);
    GX_VERIFY_EXIT(emStatus);
    //Alloc memory for Vendor name
    char *pszVendorName = new char[nSize];
    //Get Vendor name
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_VENDOR_NAME, pszVendorName, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszVendorName;
        pszVendorName = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Vendor Name : %s>\n", pszVendorName);
    //Release memory for Vendor name
    delete[] pszVendorName;
    pszVendorName = NULL;

    //Get string length of Model name
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_MODEL_NAME, &nSize);
    GX_VERIFY_EXIT(emStatus);
    //Alloc memory for Model name
    char *pszModelName = new char[nSize];
    //Get Model name
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_MODEL_NAME, pszModelName, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszModelName;
        pszModelName = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Model Name : %s>\n", pszModelName);
    //Release memory for Model name
    delete[] pszModelName;
    pszModelName = NULL;

    //Get string length of Serial number
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, &nSize);
    GX_VERIFY_EXIT(emStatus);
    //Alloc memory for Serial number
    char *pszSerialNumber = new char[nSize];
    //Get Serial Number
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, pszSerialNumber, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszSerialNumber;
        pszSerialNumber = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Serial Number : %s>\n", pszSerialNumber);
    //Release memory for Serial number
    delete[] pszSerialNumber;
    pszSerialNumber = NULL;

    //Get string length of Device version
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_VERSION, &nSize);
    GX_VERIFY_EXIT(emStatus);
    char *pszDeviceVersion = new char[nSize];
    //Get Device Version
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_VERSION, pszDeviceVersion, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszDeviceVersion;
        pszDeviceVersion = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Device Version : %s>\n", pszDeviceVersion);
    //Release memory for Device version
    delete[] pszDeviceVersion;
    pszDeviceVersion = NULL;
    printf("***********************************************\n");

    //Get the type of Bayer conversion. whether is a color camera.
    emStatus = GXIsImplemented(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &g_bColorFilter);
    GX_VERIFY_EXIT(emStatus);

    //This app only support color cameras
    if (!g_bColorFilter)
    {
        printf("<This app only support color cameras! App Exit!>\n");
        GXCloseDevice(g_hDevice);
        g_hDevice = NULL;
        GXCloseLib();
        return 0;
    }
    else
    {
        emStatus = GXGetEnum(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &g_i64ColorFilter);
        GX_VERIFY_EXIT(emStatus);
    }

    emStatus = GXGetInt(g_hDevice, GX_INT_PAYLOAD_SIZE, &g_nPayloadSize);
    GX_VERIFY(emStatus);

    printf("\n");
    printf("Press [a] or [A] and then press [Enter] to start acquisition\n");
    printf("Press [s] or [S] and then press [Enter] to save one ppm image\n");
    printf("Press [x] or [X] and then press [Enter] to Exit the Program\n");
    printf("\n");

    char chStartKey = 0;
    bool bWaitStart = true;
    while (bWaitStart)
    {
        chStartKey = getchar();
        switch(chStartKey)
        {
            //press 'a' and [Enter] to start acquisition;
            //press 'x' and [Enter] to exit.
            case 'a':
            case 'A':
                //Start to acquisition
				g_vid.open("cam.avi",CV_FOURCC('M', 'J', 'P', 'G'),25,Size(m_width,m_height),1);
                bWaitStart = false;
                break;
            case 'S':
            case 's':
                printf("<Please start acquisiton before saving image!>\n");
                break;
            case 'x':
            case 'X':
                //App exit
                GXCloseDevice(g_hDevice);
                g_hDevice = NULL;
                GXCloseLib();
				g_vid.release();
                printf("<App exit!>\n");
                return 0;
            default:
                break;
        }
    }

    //Set acquisition mode
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    GX_VERIFY_EXIT(emStatus);

    //Set trigger mode
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
    GX_VERIFY_EXIT(emStatus);

    //Set buffer quantity of acquisition queue
    uint64_t nBufferNum = ACQ_BUFFER_NUM;
    emStatus = GXSetAcqusitionBufferNumber(g_hDevice, nBufferNum);
    GX_VERIFY_EXIT(emStatus);

    bool bStreamTransferSize = false;
    emStatus = GXIsImplemented(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE, &bStreamTransferSize);
    GX_VERIFY_EXIT(emStatus);

    if(bStreamTransferSize)
    {
        //Set size of data transfer block
        emStatus = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE, ACQ_TRANSFER_SIZE);
        GX_VERIFY_EXIT(emStatus);
    }

    bool bStreamTransferNumberUrb = false;
    emStatus = GXIsImplemented(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, &bStreamTransferNumberUrb);
    GX_VERIFY_EXIT(emStatus);

    if(bStreamTransferNumberUrb)
    {
        //Set qty. of data transfer block
        emStatus = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, ACQ_TRANSFER_NUMBER_URB);
        GX_VERIFY_EXIT(emStatus);
    }

    //Set Balance White Mode : Continuous
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_CONTINUOUS);
    GX_VERIFY_EXIT(emStatus);

    //Allocate the memory for pixel format transform
    PreForAcquisition();

    //Device start acquisition
    emStatus = GXStreamOn(g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        //Release the memory allocated
        UnPreForAcquisition();
        GX_VERIFY_EXIT(emStatus);
    }

    //Start acquisition thread, if thread create failed, exit this app
    int nRet = pthread_create(&g_nAcquisitonThreadID, NULL, ProcGetImage, NULL);
    if(nRet != 0)
    {
        //Release the memory allocated
        UnPreForAcquisition();

        GXCloseDevice(g_hDevice);
        g_hDevice = NULL;
        GXCloseLib();

        printf("<Failed to create the acquisition thread, App Exit!>\n");
        exit(nRet);
    }

    //Main loop
    bool bRun = true;
    while(bRun == true)
    {
        char chKey = getchar();
        //press 's' and [Enter] to save image;
        //press 'x' and [Enter] to exit.
        switch(chKey)
        {
            //Save PPM Image
            case 'S':
            case 's':
                g_bSavePPMImage = true;
                break;
            //Exit app
            case 'X':
            case 'x':
                bRun = false;
                break;
            default:
                break;
        }
    }

    //Stop Acquisition thread
    g_bAcquisitionFlag = false;
    pthread_join(g_nAcquisitonThreadID, NULL);

    //Device stop acquisition
    emStatus = GXStreamOff(g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        //Release the memory allocated
        UnPreForAcquisition();
        GX_VERIFY_EXIT(emStatus);
    }

    //Release the resources and stop acquisition thread
    UnPreForAcquisition();

    //Close device
    emStatus = GXCloseDevice(g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        g_hDevice = NULL;
        GXCloseLib();
        return emStatus;
    }

    //Release libary
    emStatus = GXCloseLib();
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        return emStatus;
    }

    printf("<App exit!>\n");
    return 0;
}

//-------------------------------------------------
/**
\brief Save PPM image
\param ui32Width[in]       image width
\param ui32Height[in]      image height
\return void
*/
//-------------------------------------------------
void SavePPMFile(uint32_t ui32Width, uint32_t ui32Height)
{
    char szName[FILE_NAME_LEN] = {0};

    static int nRawFileIndex = 0;
    FILE* phImageFile = NULL;
    sprintf(szName, "Frame_%d.ppm", nRawFileIndex++);
    phImageFile = fopen(szName, "wb");
    if (phImageFile == NULL)
    {
        printf("Save %s failed!\n", szName);
        return;
    }

    if(g_pRGBImageBuf != NULL)
    {
        //Save color image
        fprintf(phImageFile, "P6\n" "%u %u 255\n", ui32Width, ui32Height);
        fwrite(g_pRGBImageBuf, 1, g_nPayloadSize * 3, phImageFile);
        fclose(phImageFile);
        phImageFile = NULL;
        printf("Save %s successed!\n", szName);
    }
    else
    {
        printf("Save %s failed!\n", szName);
    }
}

//-------------------------------------------------
/**
\brief Convert frame date to suitable pixel format
\param pParam[in]           pFrameBuffer       FrameData from camera
\return void
*/
//-------------------------------------------------
int PixelFormatConvert(PGX_FRAME_BUFFER pFrameBuffer)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    VxInt32 emDXStatus = DX_OK;

    // Convert RAW8 or RAW16 image to RGB24 image
    switch (pFrameBuffer->nPixelFormat)
    {
        case GX_PIXEL_FORMAT_BAYER_GR8:
        case GX_PIXEL_FORMAT_BAYER_RG8:
        case GX_PIXEL_FORMAT_BAYER_GB8:
        case GX_PIXEL_FORMAT_BAYER_BG8:
        {
            // Convert to the RGB image
            emDXStatus = DxRaw8toRGB24((unsigned char*)pFrameBuffer->pImgBuf, g_pRGBImageBuf, pFrameBuffer->nWidth, pFrameBuffer->nHeight,
                              RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(g_i64ColorFilter), false);
            if (emDXStatus != DX_OK)
            {
                printf("DxRaw8toRGB24 Failed, Error Code: %d\n", emDXStatus);
                return PIXFMT_CVT_FAIL;
            }
            break;
        }
        case GX_PIXEL_FORMAT_BAYER_GR10:
        case GX_PIXEL_FORMAT_BAYER_RG10:
        case GX_PIXEL_FORMAT_BAYER_GB10:
        case GX_PIXEL_FORMAT_BAYER_BG10:
        case GX_PIXEL_FORMAT_BAYER_GR12:
        case GX_PIXEL_FORMAT_BAYER_RG12:
        case GX_PIXEL_FORMAT_BAYER_GB12:
        case GX_PIXEL_FORMAT_BAYER_BG12:
        {
            // Convert to the Raw8 image
            emDXStatus = DxRaw16toRaw8((unsigned char*)pFrameBuffer->pImgBuf, g_pRaw8Image, pFrameBuffer->nWidth, pFrameBuffer->nHeight, DX_BIT_2_9);
            if (emDXStatus != DX_OK)
            {
                printf("DxRaw16toRaw8 Failed, Error Code: %d\n", emDXStatus);
                return PIXFMT_CVT_FAIL;
            }
            // Convert to the RGB24 image
            emDXStatus = DxRaw8toRGB24((unsigned char*)g_pRaw8Image, g_pRGBImageBuf, pFrameBuffer->nWidth, pFrameBuffer->nHeight,
                              RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(g_i64ColorFilter), false);
            if (emDXStatus != DX_OK)
            {
                printf("DxRaw8toRGB24 Failed, Error Code: %d\n", emDXStatus);
                return PIXFMT_CVT_FAIL;
            }
            break;
        }
        default:
        {
            printf("Error : PixelFormat of this camera is not supported\n");
            return PIXFMT_CVT_FAIL;
        }
    }
    return PIXFMT_CVT_SUCCESS;
}

//-------------------------------------------------
/**
\brief Allocate the memory for pixel format transform
\return void
*/
//-------------------------------------------------
void PreForAcquisition()
{
    g_pRGBImageBuf = new unsigned char[g_nPayloadSize * 3];
    g_pRaw8Image = new unsigned char[g_nPayloadSize];

    return;
}

//-------------------------------------------------
/**
\brief Release the memory allocated
\return void
*/
//-------------------------------------------------
void UnPreForAcquisition()
{
    //Release resources
    if (g_pRaw8Image != NULL)
    {
        delete[] g_pRaw8Image;
        g_pRaw8Image = NULL;
    }
    if (g_pRGBImageBuf != NULL)
    {
        delete[] g_pRGBImageBuf;
        g_pRGBImageBuf = NULL;
    }

    return;
}

//-------------------------------------------------
/**
\brief Acquisition thread function
\param pParam       thread param, not used in this app
\return void*
*/
//-------------------------------------------------
void *ProcGetImage(void* pParam)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    //Thread running flag setup
    g_bAcquisitionFlag = true;
    PGX_FRAME_BUFFER pFrameBuffer = NULL;

    time_t lInit;
    time_t lEnd;
    uint32_t ui32FrameCount = 0;
    uint32_t ui32AcqFrameRate = 0;

	namedWindow("original_image");
	namedWindow("canny_image");

    while(g_bAcquisitionFlag)
    {
        if(!ui32FrameCount)
        {
            time(&lInit);
        }

        // Get a frame from Queue
        emStatus = GXDQBuf(g_hDevice, &pFrameBuffer, 1000);
        if(emStatus != GX_STATUS_SUCCESS)
        {
            if (emStatus == GX_STATUS_TIMEOUT)
            {
                continue;
            }
            else
            {
                GetErrorString(emStatus);
                break;
            }
        }

        if(pFrameBuffer->nStatus != GX_FRAME_STATUS_SUCCESS)
        {
            printf("<Abnormal Acquisition: Exception code: %d>\n", pFrameBuffer->nStatus);
        }
        else
        {
            ui32FrameCount++;
            time (&lEnd);
            // Print acquisition info each second.
            if (lEnd - lInit >= 1)
            {
               // printf("<Successful acquisition: FrameCount: %u Width: %d Height: %d FrameID: %llu>\n",
                //  ui32FrameCount, pFrameBuffer->nWidth, pFrameBuffer->nHeight, pFrameBuffer->nFrameID);
                  ui32FrameCount = 0;
            }

			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			if(m_isImplemented)
			{
				emStatus = DxRaw8toRGB24((unsigned char*)pFrameBuffer->pImgBuf, m_rgb_image, pFrameBuffer->nWidth, pFrameBuffer->nHeight,
                          RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(BAYERBG), false);//g_i64ColorFilter//BAYERBG
				memcpy(m_image.data,m_rgb_image,m_width*m_height*3);
			}

/***********************JW***************************/

                                cvtColor(m_image,imgRGB,CV_BGR2GRAY);
								GaussianBlur(imgRGB, BlurImage, Size(15, 9), 15, 10);//高斯滤波处理******
								threshold(BlurImage, BinaryImage, 100, 255, CV_THRESH_BINARY_INV);//二值化处理*****
								vector< vector<Point> > contours, contours0;//子容器放点：二维数组（一组点为一个轮廓）
								vector<Vec4i> hireachy;//放4维向量
								Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));// 构建形态学操作的结构元，运算内核
								morphologyEx(BinaryImage, BinaryImage, MORPH_CLOSE, kernel, Point(-1, -1));//闭操作：闭运算是对图像先膨胀再腐蚀，排除小型黑洞
								morphologyEx(BinaryImage, BinaryImage, MORPH_OPEN, kernel, Point(-1, -1));//开操作：开运算是对图像先腐蚀再膨胀，可以排除小团的物体
								Rect temp_rect(x_position,y_position,x_offset,y_offset);
								Canny(BinaryImage, CannyOut, 50, 150, 3);
								cut_image=CannyOut(temp_rect);
								findContours(cut_image, contours, hireachy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

								if(contours.size()>0)
								{
								float current_angle=0;
									contours0.resize(contours.size());
									for (size_t k = 0; k < contours.size(); k++)
										approxPolyDP(Mat(contours[k]), contours0[k], 1, true);
									//计算轮廓矩
									vector<Moments> mu(contours.size());
									for (int i = 0; i < contours.size(); i++)
									{
										mu[i] = moments(contours[i], false);
									}
									//计算轮廓的质心
									vector<Point2f> mc(contours.size());
									for (int i = 0; i < contours.size(); i++)
									{
										mc[i] = Point2d(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
									}
									//画出最外围轮廓
									drawContours(imgRGB, contours0, -1, Scalar(0, 255, 255), 3, 8, hireachy);
									circle(m_image, Point(mc[0].x, mc[0].y), 2, Scalar(0, 0, 255), 1);
									//画出外围轮廓的最小外包矩形框
									Rect rect;
									Point2f vertices[4];
									rect = boundingRect(contours.at(0));
									Rect rect_offset(rect.x+x_position,rect.y+y_position,rect.width,rect.height);
									RotatedRect AngleRect = minAreaRect(contours.at(0));
									AngleRect.points(vertices);
									line(m_image, Point2f(vertices[0].x+x_position,vertices[0].y+y_position), Point2f(vertices[1].x+x_position,vertices[1].y+y_position), Scalar(0, 0, 255));
									line(m_image, Point2f(vertices[0].x+x_position,vertices[0].y+y_position), Point2f(vertices[3].x+x_position,vertices[3].y+y_position), Scalar(0, 0, 255));
									line(m_image, Point2f(vertices[1].x+x_position,vertices[1].y+y_position), Point2f(vertices[2].x+x_position,vertices[2].y+y_position), Scalar(0, 0, 255));
									line(m_image, Point2f(vertices[2].x+x_position,vertices[2].y+y_position), Point2f(vertices[3].x+x_position,vertices[3].y+y_position), Scalar(0, 0, 255));
									rectangle(m_image, rect_offset, Scalar(255, 0, 0), 1);
									//Draw_image = original_image;
									Size2f temp = AngleRect.size;

									if (temp.width <= temp.height)
									{
										current_angle = -AngleRect.angle;
									}
									else {
										current_angle = -(90 + AngleRect.angle);
									}
									if(current_angle==-180)
									{
										current_angle=0;
									}
									if(current_angle==-0)
									{
										current_angle=0;
									}
										//图片文本保留两位
										stringstream ss;
										ss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << current_angle;
										string a = ss.str();
										//获取文本框的长宽值
										int baseline;
										Size text_size = getTextSize(a.c_str(), FONT_HERSHEY_SIMPLEX, 0.5, 4, &baseline);
										//设定文本框在图片中的坐标值
										Point origin;
										origin.x = m_image.cols - text_size.width-80;//图片宽-文字宽
										origin.y = m_image.rows-text_size.height ;//图片长-文字长
										Point origin1;
										origin1.x = m_image.cols - text_size.width-240;//图片宽-文字宽
										origin1.y = m_image.rows-text_size.height ;//图片长-文字长
									    //在图片上输出文本
										putText(m_image,"Angle_Err:", origin1, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2, 8);
										putText(m_image,a, origin, FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2, 8);
								     //  ofstream file;
					                                                      // file.open("./Angle_Err.txt",ios::app);
					                                                      // file<<curr_angle<<endl;
                                                                                                                                        imshow("canny_image", cut_image);
								        imshow("original_image",m_image);
								        waitKey(1);

								}
								else
								{   string text = "The light is low or not aligned with the rod";
								    int font_face = FONT_HERSHEY_COMPLEX;
								    double font_scale = 2;
								    int thickness = 2;
								    int baseline;
									Size text_size = getTextSize(text, font_face, font_scale, thickness, &baseline);
									    Point origin;
								        origin.x =  m_image.cols -text_size.width /2;
								        origin.y = m_image.rows / 2 - text_size.height / 2;
									    putText(m_image,text,origin, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2, 8);
										imshow("canny_image", cut_image);
										imshow("original_image",m_image);
										waitKey(1);
								}
/*****************JW***********************************/

			//g_vid.write(m_image);



            if (g_bSavePPMImage)
            {
                int nRet = PixelFormatConvert(pFrameBuffer);
                if (nRet == PIXFMT_CVT_SUCCESS)
                {
                    SavePPMFile(pFrameBuffer->nWidth, pFrameBuffer->nHeight);
                }
                else
                {
                    printf("PixelFormat Convert failed!\n");
                }
                g_bSavePPMImage = false;
            }
        }

        emStatus = GXQBuf(g_hDevice, pFrameBuffer);
        if(emStatus != GX_STATUS_SUCCESS)
        {
            GetErrorString(emStatus);
            break;
        }
    }
    printf("<Acquisition thread Exit!>\n");

    return 0;
}

//----------------------------------------------------------------------------------
/**
\brief  Get description of input error code
\param  emErrorStatus  error code

\return void
*/
//----------------------------------------------------------------------------------
void GetErrorString(GX_STATUS emErrorStatus)
{
    char *error_info = NULL;
    size_t size = 0;
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    // Get length of error description
    emStatus = GXGetLastError(&emErrorStatus, NULL, &size);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        printf("<Error when calling GXGetLastError>\n");
        return;
    }

    // Alloc error resources
    error_info = new char[size];
    if (error_info == NULL)
    {
        printf("<Failed to allocate memory>\n");
        return ;
    }

    // Get error description
    emStatus = GXGetLastError(&emErrorStatus, error_info, &size);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        printf("<Error when calling GXGetLastError>\n");
    }
    else
    {
        printf("%s\n", (char*)error_info);
    }

    // Realease error resources
    if (error_info != NULL)
    {
        delete []error_info;
        error_info = NULL;
    }
}

