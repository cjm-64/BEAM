#!/bin/bash

scp {globals,main,mainwindow,settime}.cpp {globals,mainwindow,settime}.h {mainwindow,settime}.ui pupil-libuvc-qt.pro OpenKeyboard.sh beam@128.235.117.33:/home/beam/code/pupil-uvc-qt 

ssh beam@128.235.117.33 "cd /home/beam/code/pupil-uvc-qt && qmake pupil-libuvc-qt.pro && make && ls -la"
