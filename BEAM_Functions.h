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

//Struct to hold bounding box information
struct BoundingBox{
    int startX; //Top left X pixel value
    int startY; //Top left Y pixel value
    int endX; //Bottom right X pixel value
    int endY; //Bottom right Y pixel value
    int startOrEndFlag; //Flag to track which corner is being moved
};

//Struct to hold frame processing information
struct FrameProcessingInformation{
    int thresh_val; //Threshold value for gray to binary conversion
    int max_radius; //Max radius circle to search for in HoughCircles()
    int CED; //For Circle tracking, read OCV documentation
    int Cent_D; //For Circle tracking, read OCV documentation
};

//Struct intermiediate steps for image processing
struct ImageManipulation{
    Mat image;
    Mat greyIMG;
    Mat binaryIMG;
    Mat binaryOneMask;
    Mat greyPlusColor;
    Mat finalImage;
};

vector<string> getCameraNames(){
    //Intialize variables to get the names of the cameras
    uvc_context_t *ctx;
    uvc_device_t **device_list;
    uvc_device_t *dev;
    uvc_device_descriptor_t *desc;
    uvc_error_t res;

    vector<string> camNames(3);

    //Create a UVC context
    //Get the list of the devices
    //Iterate through list of devices and record the names of all the cameras
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


class Camera {
public:
    //Struct to house information about the camera
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

    //Struct to house information about stream for the camera
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
    ImageManipulation imageManip;

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
        //Assuming it is used properly the user will update top left then bottom right
        //Assigned using user click on screen
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

        //However incase the user does it the wrong way, this flips the points that need to be flipped
        //End X&Y should always be greater than start X&Y because 0,0 is top left of image
        if(this->boundBox.startX > this->boundBox.endX){
            int temp = this->boundBox.startX;
            this->boundBox.startX = this->boundBox.endX;
            this->boundBox.endX = temp;
        }
        if(this->boundBox.startY > this->boundBox.endY){
            int temp = this->boundBox.startY;
            this->boundBox.startY = this->boundBox.endY;
            this->boundBox.endY = temp;
        }

        //Bounding box cannot have start and ends at the same point, so if this occurs, move the end positions by 1 pixel
        if(this->boundBox.startX == this->boundBox.endX){
            this->boundBox.endX = this->boundBox.endX + 1;
        }
        if(this->boundBox.startY == this->boundBox.endY){
            this->boundBox.endY = this->boundBox.endY + 1;
        }

    }

private:
    //Set initial values
    void initFrameProc(){
        frameProcInfo.thresh_val = 50;
        frameProcInfo.max_radius = 50;
        frameProcInfo.CED = 1;
        frameProcInfo.Cent_D = 1;
    }

    //Set initial values
    void initBoundingBox(){
        boundBox.startX = 10;
        boundBox.startY = 10;
        boundBox.endX = 182;
        boundBox.endY = 182;
        boundBox.startOrEndFlag = 0;
    }

    //Initialize the camera
    void initCamera(string name, int i){
        //Save name and num to Class's caminfo struct
        this->camInfo.camName = name;
        this->camInfo.camNum = i;

        //Open UVC context
        this->streamInfo.res = uvc_init(&this->streamInfo.ctx, NULL);
        if (this->streamInfo.res < 0){
            uvc_perror(this->streamInfo.res, "uvc_find_device");
        }

        //Get device list
        this->streamInfo.res = uvc_find_devices(this->streamInfo.ctx, &streamInfo.deviceList, 0, 0, NULL);
        if (this->streamInfo.res < 0) {
            uvc_perror(this->streamInfo.res, "uvc_init");
        }

        //Open the camera at location in the list
        this->streamInfo.res = uvc_open(this->streamInfo.deviceList[this->camInfo.camNum], &this->streamInfo.devh, 1);
        if (this->streamInfo.res < 0) {
            uvc_perror(this->streamInfo.res, "uvc_find_device"); /* no devices found */
        }
        else{
            cout << "Opened " << this->camInfo.camName << endl;
        }

        //Format camera into MJPEG
        this->streamInfo.format_desc = uvc_get_format_descs(this->streamInfo.devh);
        this->streamInfo.format_desc = this->streamInfo.format_desc->next;
        this->streamInfo.frame_desc = this->streamInfo.format_desc->frame_descs->next;
        this->streamInfo.frame_format = UVC_FRAME_FORMAT_MJPEG;

        //Format the frame to 192x192, 120 fps
        if(this->streamInfo.frame_desc){
            this->camInfo.width = this->streamInfo.frame_desc->wWidth;
            this->camInfo.height = this->streamInfo.frame_desc->wHeight;
            this->camInfo.fps = 10000000 / this->streamInfo.frame_desc->intervals[2];
        }
        printf("\n%s: \n    format: %4s\n    size: %dx%d\n    frame rate: %d fps\n\n", this->camInfo.camName.c_str(), this->streamInfo.format_desc->fourccFormat, this->camInfo.width, this->camInfo.height, this->camInfo.fps);


        //Begin steps of getting the camera stream
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

        //Open the stream of the camera
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
    //Struct for information for file writing
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

    //Struct for information for retrieving frame and converting to Mat
    struct FrameGetting{
        Mat frame2image;
        Mat dummy;
        int frameW;
        int frameH;
        long unsigned int frameBytes;
    } FG;

    //Decompression
    tjhandle decompressor = tjInitDecompress();

    int boxThickness = 2;
    Scalar boxColour = Scalar(119, 3, 252);
    Scalar circleColour = Scalar(255, 0, 0); //Color for drawing on frame

    //Gets frame from stream and feeds to function that converts to Mat
    Mat getFrame(uvc_stream_handle_t *streamHandle, FrameGetting FG){

        uvc_frame_t *frame;
        uvc_error_t res;
        res = uvc_stream_get_frame(streamHandle, &frame, 1 * pow(10,6));
        if(res < 0){
            uvc_perror(res, "Failed to get frame");
        }
        else{
//                printf("got frame\n");
        }

        Mat image = frame2Mat(frame, FG);

        return image;

    }

    //Take grey image from stream and process to create final image
    Mat processFrame(Mat image, ImageManipulation imageManip, FrameProcessingInformation frameProcInfo, BoundingBox boundBox, float currentTime) {

        //Mat is greyscaled but it stored as RGB so it needs to be converted
        cvtColor(image, imageManip.greyIMG, COLOR_BGR2GRAY);

        //Convert greyscale to black and white based on threshold set by slider
        threshold(imageManip.greyIMG, imageManip.binaryIMG, frameProcInfo.thresh_val, 255, 1);

        //Find the circls in the black and white image
        Vec3i c = findCircle(imageManip.binaryIMG, frameProcInfo, boundBox, currentTime);


        inRange(imageManip.binaryIMG, Scalar(255, 255, 255), Scalar(255, 255, 255), imageManip.binaryOneMask);
        imageManip.greyIMG.setTo(Scalar(255, 255, 255), imageManip.binaryOneMask);

        cvtColor(imageManip.greyIMG, imageManip.greyPlusColor, COLOR_GRAY2RGB);

            
        imageManip.finalImage = composeFinalImage(imageManip.greyPlusColor, c, boundBox);
        return imageManip.finalImage;

    }



private:
    //Converts from UVC frame to opencv Mat
    Mat frame2Mat(uvc_frame_t *frame, FrameGetting FG){
        //Allocate buffers for conversions
        FG.frameW = frame->width;
        FG.frameH = frame->height;
        FG.frameBytes = frame->data_bytes;

        //Check type, MJPEG = 7, Other = 3
        if (frame->frame_format == 7){
            //Allocate buffer, decrompress frame into buffer, write image to Mat
            unsigned char buffer[FG.frameW*FG.frameH*3];
            tjDecompress2(decompressor, (unsigned char *)frame->data, FG.frameBytes, buffer, FG.frameW, 0, FG.frameH, TJPF_RGB, TJFLAG_FASTDCT);
            FG.frame2image = Mat(FG.frameH, FG.frameW, CV_8UC3, buffer);
        }
        else if (frame->frame_format == 3){
            //Convert frame from stream to RGB, write RGB frame to Mat
            uvc_frame_t *rgb;
            rgb = uvc_allocate_frame(FG.frameW * FG.frameH * 3);
            if (!rgb) {
                printf("unable to allocate bgr frame!\n");
                return FG.frame2image;
            }
            uvc_error_t res = uvc_yuyv2rgb(frame, rgb);
            if (res < 0){
                printf("Unable to copy frame to bgr!\n");
            }
            FG.frame2image = Mat(rgb->height, rgb->width, CV_8UC3, rgb->data);
            uvc_free_frame(rgb);
        }
        else {
            printf("Error, somehow you got to a frame format that doesn't exist.\nBravo tbh\n");
        }

        return FG.frame2image;
    }

    Vec3i findCircle(Mat binaryImage, FrameProcessingInformation frameProcInfo, BoundingBox boundBox, float currentTime) {

        //Create binary image but only the part inside the bounding box
        Mat binaryROI = binaryImage(Rect(Point(boundBox.startX, boundBox.startY), Point(boundBox.endX, boundBox.endY)));

        //Create place to store data about circle
        vector<Vec3f> circles;

        //Find 1 circle inside the image, using the radius value set by user via slider
        HoughCircles(binaryROI, circles, HOUGH_GRADIENT, 1, 1000, frameProcInfo.CED, frameProcInfo.Cent_D, frameProcInfo.max_radius - 1, frameProcInfo.max_radius + 1);

        //Convert from float to int
        Vec3i c;
        for (size_t i = 0; i < circles.size(); i++) {
            c = circles[i];
        }

        //Check if circle is found. If X & Y position of circle == 0 or if radius == 0
        int found = 1;
        if ((c[0] == 0 && c[1] == 0) || c[2] == 0){
            found = 0;
        }


        //Write circle position to file
        if(this->FWI.step != 0){
            //If file is not open, open in, write the top line, then write data
            //Else just write data
            if(!this->FWI.outputFile.is_open()){
                this->FWI.outputFile.open(this->FWI.fileName);
                this->FWI.outputFile << "Header,Right_Eye_X,Right_Eye_Y,Right_Eye_Radius,Right_Eye_Found,Left_Eye_X,Left_Eye_Y,Left_Eye_Radius,Left_Eye_Found,Time_s" << endl;
                writeToFile(this->FWI.outputFile, c, found, boundBox, currentTime);
            }
            else{
                writeToFile(this->FWI.outputFile, c, found, boundBox, currentTime);
            }
        }
        return c;
    }

    void writeToFile(ofstream &outputFile, Vec3i circle, int found, BoundingBox boundBox, float currentTime){

        //Check 2 things, which eye and if the section header has been written
        //Right eye = 0, left = 1
        //If data is from right eye and section header has not been written, start data write out with header at front
        //If data is from right eye and section header has been written, start data write with empty space at front
        //If data is from left eye, append data write out and write to file
        if (this->FWI.rightOrLeftEye == 0 && this->FWI.headerWritten == 0){
            if (this->FWI.step == 3){
                this->FWI.dataOut = this->FWI.headers[this->FWI.step-1][0] + "_" + this->FWI.testTime + ",";
            }
            else{
                this->FWI.dataOut = this->FWI.headers[this->FWI.step-1][0] + "_" + to_string(this->FWI.calibrationNumber) + "_pd" + ",";
            }
            this->FWI.dataOut = this->FWI.dataOut + to_string(circle[0]+boundBox.startX) + "," + to_string(circle[1]+boundBox.startY) + "," + to_string(circle[2]) + "," + to_string(found) + ",";
            this->FWI.headerWritten = 1;
            this->FWI.rightOrLeftEye = 1;
        }
        else if(this->FWI.rightOrLeftEye == 0 && this->FWI.headerWritten == 1){
            this->FWI.dataOut = " ," + to_string(circle[0]+boundBox.startX) + "," + to_string(circle[1]+boundBox.startY) + "," + to_string(circle[2]) + "," + to_string(found) + ",";
            this->FWI.rightOrLeftEye = 1;
        }
        else{
            this->FWI.dataOut = this->FWI.dataOut + to_string(circle[0]+boundBox.startX) + "," + to_string(circle[1]+boundBox.startY) + "," + to_string(circle[2]) + "," + to_string(found) + "," + to_string(currentTime);
            this->FWI.rightOrLeftEye = 0;
            outputFile << this->FWI.dataOut << endl;
        }
    }

    Mat composeFinalImage(Mat greyPlusColor, Vec3i c, BoundingBox boundBox){
        //Draw Bounding Box using points from struct
        rectangle(greyPlusColor, Point(boundBox.startX, boundBox.startY), Point(boundBox.endX, boundBox.endY), boxColour, boxThickness);

        //Draw Circles on image using outputs from findCircle
        circle(greyPlusColor, Point(c[0]+boundBox.startX, c[1]+boundBox.startY), 1, circleColour,2,LINE_8);
        circle(greyPlusColor, Point(c[0]+boundBox.startX, c[1]+boundBox.startY), c[2], circleColour,2,LINE_8);
        return greyPlusColor;
    }
};

#endif // BEAM_FUNCTIONS_H
