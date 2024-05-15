#!/bin/bash

matchbox-keyboard&

name=$(zenity --entry \
		--title="File Name Input" \
		--text="Please input participant identifier:" \
		--entry-text "Name")
		

echo $name

killall matchbox-keyboard

timeStamp=$(date +%s | cut -b1-13)

fileName=$name"_"$timeStamp
echo $fileName

sudo ./BEAM $fileName

cp $fileName".csv" "/media/beam/BEAM/BEAM_DATA/$fileName.csv"
cp $fileName".csv" "Data/$fileName.csv"

