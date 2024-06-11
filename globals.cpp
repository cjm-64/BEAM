#include "globals.h"

//Test Timing
int Test_Time = 0; //Total time in seconds
int Test_Time_M = 0; //Minutes to run
int Test_Time_H = 0; //Hours to run
int SetUpTime = 300000; //5 minutes (300 seconds) to set up tracking
int CalibrationTime = 3000; //3 seconds in ms
int RecordingTimer = 0; //Flag to set how long to record for
int TestCompleteFlag = 0; //Flag for if the test was completed

//Misc
int DisplaySelector = 0; //Select which display to use; 0 = w/Panel 1 = w/o
int DataSavingFlag = 0; //Where to save the data; 0 = None, 1 = CalR, 2 = CalL 3 = Full Test
int ColorOrBW = 1; //Color Video or BW Video; Color = 0, BW = 1

globals::globals()
{

}
