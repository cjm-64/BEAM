#!/bin/bash

scp {globals,main,mainwindow,settime}.cpp {BEAM_Functions,globals,mainwindow,settime}.h {mainwindow,settime}.ui BEAM.pro startBEAM.sh beam@$1:/home/beam/code/BEAM 

ssh beam@$1 "cd /home/beam/code/BEAM && qmake BEAM.pro && make && ls -la"
