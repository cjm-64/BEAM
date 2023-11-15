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

sudo ./BEAM

cp output.csv "/media/beam/$file_name.csv"
cp output.csv "Data/$file_name.csv"
rm output.csv

#ps aux | grep -i | awk {'print $2'} | xargs kill -9
