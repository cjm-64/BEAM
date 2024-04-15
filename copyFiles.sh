#!/bin/bash

scp {globals,main,mainwindow,settime}.cpp {globals,mainwindow,settime}.h {mainwindow,settime}.ui BEAM.pro startBEAM.sh beam@128.235.117.91:/home/beam/code/BEAM 

ssh beam@128.235.117.91 "cd /home/beam/code/BEAM && qmake BEAM.pro && make && ls -la"
