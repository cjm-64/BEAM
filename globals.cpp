#include "globals.h"

//Test Timing
int Test_Time = 0; //Total time in seconds
int Test_Time_Frames = 0; //Time in frames (seconds * 30)
int Test_Time_M = 1; //Minutes component of test time
int Test_Time_H = 0; //Hours components of test time
int SetUpTime = 300000; //5 minutes to set up tracking
int CalibrationTime = 3000; //3 seconds of calibration (3*30fps = 90)
int TimeCount = 0; //Frame counter for timing
int RecordingTimer = 0; //Flag to set how long to record for
int TestCompleteFlag = 0; //Flag for if the test was completed

//Misc
int DisplaySelector = 0; //Select which display to use; 0 = w/Panel 1 = w/o
int DataSavingFlag = 0; //Where to save the data; 0 = None, 1 = CalR, 2 = CalL 3 = Full Test
int ColorOrBW = 1; //Color Video or BW Video; Color = 0, BW = 1


//Tracking
int CalRFrame = 0;
int CalLFrame = 0;
int TestFrame = 0;
int CalFrames = 0;
int X_Point = 0; //X of center of tracked circle (pupil)
int Y_Point = 0; //Y of center of tracked circle (pupil)
int Radius = 0; //Radius of tracked circle (pupil)

globals::globals()
{

}
