#!/bin/bash

for ((i=1;;i++))
{
	dir="$(ls | grep "^$i.\ ")"
	if [ -d "$dir" ]; then echo -e "\x1B[32mChanging dir to: \033[0m$dir"; else break; fi
	cd "$dir"

	for ((j=1;;j++))
	{
		subdir="$(ls | grep "^$j.\ ")"
		if [ -d "$subdir" ]; 
		then 
			echo -n -e "$subdir ... "
			cd "$subdir"
			make clean > /dev/null
			rm bin/* 2> /dev/null
			rm -r build 2> /dev/null
			cd ..
			echo "done"
		else 
			cd ..
			break; 
		fi
	}
}
