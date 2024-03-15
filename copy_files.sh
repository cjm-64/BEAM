#!/bin/bash

scp {globals,main,mainwindow,settime}.cpp {globals,mainwindow,settime}.h {mainwindow,settime}.ui BEAM.pro OpenKeyboard.sh beam@128.235.117.60:/home/beam/code/BEAM 

ssh beam@128.235.117.60 "cd /home/beam/code/BEAM && qmake BEAM.pro && make && ls -la"
