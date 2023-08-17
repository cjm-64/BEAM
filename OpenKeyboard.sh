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

sudo ./pupil-libuvc-qt

mv output.csv "$file_name.csv"

#ps aux | grep -i | awk {'print $2'} | xargs kill -9
