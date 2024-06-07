#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include "libuvc/libuvc.h"
#include "turbojpeg.h"
#include "string"
#include "iostream"
#include "fstream"

#ifndef BEAM_FUNCTIONS_H
#define BEAM_FUNCTIONS_H

// ID0 = right, ID1 = Left, ID2 = World



using namespace std;
using namespace cv;

struct BoundingBox{
    int startX;
    int startY;
    int endX;
    int endY;
    int startOrEndFlag;
};

struct FrameProcessingInformation{
    int thresh_val; //Threshold value for gray to binary conversion
    int max_radius; //Max radius circle to search for in HoughCircles()
    int CED; //For Circle tracking, read OCV documentation
    int Cent_D; //For Circle tracking, read OCV documentation
};

vector<string> getCameraNames(){
    uvc_context_t *ctx;
    uvc_device_t **device_list;
    uvc_device_t *dev;
    uvc_device_descriptor_t *desc;
    uvc_error_t res;

    vector<string> camNames(3);

    res = uvc_init(&ctx,NULL);
    if(res < 0) {
        uvc_perror(res, "uvc_init");
    }
    res = uvc_get_device_list(ctx,&device_list);
    if (res < 0) {
        uvc_perror(res, "uvc_find_device");
    }
    int i = 0;
    while (true) {
        dev = device_list[i];
        if (dev == NULL) {
            break;
        } else {
            uvc_get_device_descriptor(dev, &desc);
        }
        camNames[i] = desc->product;

        uvc_free_device_descriptor(desc);
        i++;
    }
    uvc_exit(ctx);
    return camNames;
};

tuple<int, int> getShifts(BoundingBox boundBox){
    int xShift = 0;
    int yShift = 0;
    if(boundBox.startX < boundBox.endX){
        xShift = boundBox.startX;
    }
    else{
        xShift = boundBox.endX;
    }


    if(boundBox.startY < boundBox.endY){
        yShift = boundBox.startY;
    }
    else{
        yShift = boundBox.endY;
    }
    return make_tuple(xShift, yShift);
}



class Camera {
public:
    struct CamInformation{
        string camName;
        int camNum;
        int width;
        int height;
        int fps;
        uint8_t vID;
        uint8_t pID;
        string uid;
        uvc_device_t *dev;
    } camInfo;

    struct StreamInformation{
        uvc_context_t *ctx;
        uvc_error_t res;
        uvc_device_t **deviceList;
        uvc_device_t *dev;
        uvc_device_handle_t *devh;
        uvc_stream_ctrl_t ctrl;
        uvc_stream_handle_t *strmh;
        const uvc_format_desc_t *format_desc;
        uvc_frame_desc_t *frame_desc;
        enum uvc_frame_format frame_format;
    } streamInfo;

    int clickXOffset;

    BoundingBox boundBox;
    FrameProcessingInformation frameProcInfo;

    void init(string name, int i){
        initFrameProc();
        initBoundingBox();
        initCamera(name, i);
    }

    void close(uvc_context_t *ctx){
        uvc_exit(ctx);
    }



    //Function to update the bounding box for drawing
    void updateBoundingBox(int xPos, int yPos){
        if (this->boundBox.startOrEndFlag == 0){
            //Top Left of box aka start
            this->boundBox.startX = xPos-clickXOffset;
            this->boundBox.startY = yPos;
            this->boundBox.startOrEndFlag = 1;
        }
        else{
            //Bottom right aka end
            this->boundBox.endX = xPos-clickXOffset;
            this->boundBox.endY = yPos;
            this->boundBox.startOrEndFlag = 0;
        }
    }

private:
    void initFrameProc(){
        frameProcInfo.thresh_val = 50;
        frameProcInfo.max_radius = 50;
        frameProcInfo.CED = 1;
        frameProcInfo.Cent_D = 1;
    }

    void initBoundingBox(){
        boundBox.startX = 10;
        boundBox.startY = 10;
        boundBox.endX = 182;
        boundBox.endY = 182;
        boundBox.startOrEndFlag = 0;
    }

    void initCamera(string name, int i){
        this->camInfo.camName = name;
        this->camInfo.camNum = i;

        this->streamInfo.res = uvc_init(&this->streamInfo.ctx, NULL);
        if (this->streamInfo.res < 0){
            uvc_perror(this->streamInfo.res, "uvc_find_device");
        }
        this->streamInfo.res = uvc_find_devices(this->streamInfo.ctx, &streamInfo.deviceList, 0, 0, NULL);
        if (this->streamInfo.res < 0) {
            uvc_perror(this->streamInfo.res, "uvc_init");
        }
        this->streamInfo.res = uvc_open(this->streamInfo.deviceList[this->camInfo.camNum], &this->streamInfo.devh, 1);
        if (this->streamInfo.res < 0) {
            uvc_perror(this->streamInfo.res, "uvc_find_device"); /* no devices found */
        }
        else{
            cout << "Name: " << this->camInfo.camName << endl;
        }
        this->streamInfo.format_desc = uvc_get_format_descs(this->streamInfo.devh);
        this->streamInfo.format_desc = this->streamInfo.format_desc->next;
        this->streamInfo.frame_desc = this->streamInfo.format_desc->frame_descs->next;
        this->streamInfo.frame_format = UVC_FRAME_FORMAT_MJPEG;

        if(this->streamInfo.frame_desc){
            this->camInfo.width = this->streamInfo.frame_desc->wWidth;
            this->camInfo.height = this->streamInfo.frame_desc->wHeight;
            this->camInfo.fps = 10000000 / this->streamInfo.frame_desc->intervals[2];
        }
        printf("\n%s: \n    format: %4s\n    size: %dx%d\n    frame rate: %d fps\n\n", this->camInfo.camName.c_str(), this->streamInfo.format_desc->fourccFormat, this->camInfo.width, this->camInfo.height, this->camInfo.fps);


        this->streamInfo.res = uvc_get_stream_ctrl_format_size(this->streamInfo.devh, &this->streamInfo.ctrl, this->streamInfo.frame_format, this->camInfo.width, this->camInfo.height, this->camInfo.fps, 1);
        if (this->streamInfo.res < 0){
            uvc_perror(this->streamInfo.res, "uvc_get_stream_ctrl_format_size");
        }
        else{
            printf("Eye %s stream control formatted\n", this->camInfo.camName.c_str());
//            uvc_print_stream_ctrl(&this->streamInfo.ctrl, stderr);
        }
        this->streamInfo.res = uvc_stream_open_ctrl(this->streamInfo.devh, &this->streamInfo.strmh, &this->streamInfo.ctrl,1);
        if (this->streamInfo.res < 0){
            uvc_perror(this->streamInfo.res, "uvc_stream_open_ctrl");
        }
        else{
            printf("Eye %s stream opened\n", this->camInfo.camName.c_str());
        }
        this->streamInfo.res = uvc_stream_start(this->streamInfo.strmh, nullptr, nullptr,1.6,0);
        if (this->streamInfo.res < 0){
            uvc_perror(this->streamInfo.res, "uvc_stream_start");
        }
        else{
            printf("Eye %s stream started\n", this->camInfo.camName.c_str());
        }
        printf("\n\n\n");
    }
};



class Frame{
public:
    struct fileWritingInformation{
        ofstream outputFile;
        string dataOut;
        string fileName;

        string headers[3][1] = {{"Right_Calibration"}, {"Left_Calibration"}, {"Test_Data"}};
        int headerNum = 0;
        int calibrationNumber;
        int headerWritten = 0;
        int rightOrLeftEye = 0; // 0 = right, 1 = left
        int step = 0;
        string testTime;
    } FWI;

    //Decompression
    tjhandle decompressor = tjInitDecompress();

    int boxThickness = 2;
    Scalar boxColour = Scalar(119, 3, 252);
    Scalar circleColour = Scalar(255, 0, 0); //Color for drawing on frame

    Mat getFrame(uvc_stream_handle_t *streamHandle){

        uvc_frame_t *frame;
        uvc_error_t res;
        res = uvc_stream_get_frame(streamHandle, &frame, 1 * pow(10,6));
        if(res < 0){
            uvc_perror(res, "Failed to get frame");
        }
        else{
//                printf("got frame\n");
        }

        Mat image = frame2Mat(frame);

        return image;

    }

    Mat processFrame(Mat image, FrameProcessingInformation frameProcInfo, BoundingBox boundBox, float currentTime) {
        Mat greyIMG;
        cvtColor(image, greyIMG, COLOR_BGR2GRAY);

        Mat binaryIMG;
        threshold(greyIMG, binaryIMG, frameProcInfo.thresh_val, 255, 1);

        Mat binaryOneMask;
        inRange(binaryIMG, Scalar(255, 255, 255), Scalar(255, 255, 255), binaryOneMask);
        greyIMG.setTo(Scalar(255, 255, 255), binaryOneMask);

        Mat greyPlusColor;
        cvtColor(greyIMG, greyPlusColor, COLOR_GRAY2RGB);
        greyIMG.release();
        binaryOneMask.release();

        Vec3i c = findCircle(binaryIMG, frameProcInfo, boundBox, currentTime);
        Mat finalImage = composeFinalImage(greyPlusColor, c, boundBox);
        return finalImage;

    }



private:
    Mat frame2Mat(uvc_frame_t *frame){
        //Allocate buffers for conversions
        Mat image;
        int frameW = frame->width;
        int frameH = frame->height;
        long unsigned int frameBytes = frame->data_bytes;

        if (frame->frame_format == 7){
//                printf("Frame Format: MJPEG\n");
            long unsigned int _jpegSize = frameBytes;
            unsigned char buffer[frameW*frameH*3];
            tjDecompress2(decompressor, (unsigned char *)frame->data, _jpegSize, buffer, frameW, 0, frameH, TJPF_RGB, TJFLAG_FASTDCT);
            Mat placeholder(frameH, frameW, CV_8UC3, buffer);
            placeholder.copyTo(image);
            placeholder.release();
        }
        else if (frame->frame_format == 3){
//            printf("Frame Format: Other\n");
            uvc_frame_t *rgb;
            rgb = uvc_allocate_frame(frameW * frameH * 3);
            if (!rgb) {
                printf("unable to allocate bgr frame!\n");
                return image;
            }
            uvc_error_t res = uvc_yuyv2rgb(frame, rgb);
            if (res < 0){
                printf("Unable to copy frame to bgr!\n");
            }
            Mat placeholder(rgb->height, rgb->width, CV_8UC3, rgb->data);
            placeholder.copyTo(image);
            placeholder.release();
            uvc_free_frame(rgb);
        }
        else {
            printf("Error, somehow you got to a frame format that doesn't exist.\nBravo tbh\n");
        }

        return image;
    }

    Vec3i findCircle(Mat binaryImage, FrameProcessingInformation frameProcInfo, BoundingBox boundBox, float currentTime) {
        Rect roi(Point(boundBox.startX, boundBox.startY), Point(boundBox.endX, boundBox.endY));
        Mat binaryROI = binaryImage(roi);
        vector<Vec3f> circles;
        HoughCircles(binaryROI, circles, HOUGH_GRADIENT, 1, 1000, frameProcInfo.CED, frameProcInfo.Cent_D, frameProcInfo.max_radius - 1, frameProcInfo.max_radius + 1);
        Vec3i c;
        for (size_t i = 0; i < circles.size(); i++) {
            c = circles[i];
        }

        int found = 1;
        if ((c[0] == 0 && c[1] == 0) || c[2] == 0){
            found = 0;
        }

        if(this->FWI.step != 0){
            if(this->FWI.outputFile.is_open()){
                writeToFile(this->FWI.outputFile, c, found, boundBox, currentTime);
            }
            else{
                this->FWI.outputFile.open(this->FWI.fileName);
                this->FWI.outputFile << "Header,Right_Eye_X,Right_Eye_Y,Right_Eye_Radius,Right_Eye_Found,Left_Eye_X,Left_Eye_Y,Left_Eye_Radius,Left_Eye_Found,Time_s" << endl;
                writeToFile(this->FWI.outputFile, c, found, boundBox, currentTime);
            }
        }
        return c;
    }

    void writeToFile(ofstream &outputFile, Vec3i circle, int found, BoundingBox boundBox, float currentTime){
        int xShift, yShift;
        tie (xShift, yShift) = getShifts(boundBox);

        if (this->FWI.rightOrLeftEye == 0 && this->FWI.headerWritten == 0){
            if (this->FWI.step == 3){
                this->FWI.dataOut = this->FWI.headers[this->FWI.step-1][0] + "_" + this->FWI.testTime + ",";
            }
            else{
                this->FWI.dataOut = this->FWI.headers[this->FWI.step-1][0] + "_" + to_string(this->FWI.calibrationNumber) + "_pd" + ",";
            }
            this->FWI.dataOut = this->FWI.dataOut + to_string(circle[0]+xShift) + "," + to_string(circle[1]+yShift) + "," + to_string(circle[2]) + "," + to_string(found) + ",";
            this->FWI.headerWritten = 1;
            this->FWI.rightOrLeftEye = 1;
        }
        else if(this->FWI.rightOrLeftEye == 0 && this->FWI.headerWritten == 1){
            this->FWI.dataOut = " ," + to_string(circle[0]+xShift) + "," + to_string(circle[1]+yShift) + "," + to_string(circle[2]) + "," + to_string(found) + ",";
            this->FWI.rightOrLeftEye = 1;
        }
        else{
            this->FWI.dataOut = this->FWI.dataOut + to_string(circle[0]+xShift) + "," + to_string(circle[1]+yShift) + "," + to_string(circle[2]) + "," + to_string(found) + "," + to_string(currentTime);
            this->FWI.rightOrLeftEye = 0;
            cout << this->FWI.dataOut;
            outputFile << this->FWI.dataOut << endl;
        }
    }

    Mat composeFinalImage(Mat greyPlusColor, Vec3i c, BoundingBox boundBox){
        rectangle(greyPlusColor, Point(boundBox.startX, boundBox.startY), Point(boundBox.endX, boundBox.endY), boxColour, boxThickness);

        //Draw Circles on image
        circle(greyPlusColor, Point(c[0]+boundBox.startX, c[1]+boundBox.startY), 1, circleColour,2,LINE_8);
        circle(greyPlusColor, Point(c[0]+boundBox.startX, c[1]+boundBox.startY), c[2], circleColour,2,LINE_8);
        return greyPlusColor;
    }
};

#endif // BEAM_FUNCTIONS_H
