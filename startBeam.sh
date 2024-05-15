#!/bin/bash

matchbox-keyboard&

file_name=$(zenity --entry \
		--title="File Name Input" \
		--text="Please input participant identifier:" \
		--entry-text "Name")
		
echo Hello
echo $file_name
echo World!

killall matchbox-keyboard

sudo ./BEAM $file_name

cp output.csv "/media/beam/BEAM_DATA/$file_name.csv"
cp output.csv "Data/$file_name.csv"

