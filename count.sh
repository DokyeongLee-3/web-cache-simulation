#! /bin/bash

count=0
line_count=0
zero_size=0
zero_count=0
one_size=0
one_count=0
two_size=0
two_count=0
size=0

while read line
do
	for word in $line
       	do
		if (( count%4==2 )); then
			((size= size + word))
			 
		fi

		#if (( count%4==3 )); then
                #	param=$word
                	#echo 'param is '$param
		#	if ((param == 0)); then
		#		((zero_size=zero_size+size))
		#		((zero_count=zero_count+1))
				#echo 'zero size is '$zero_size
		#	elif ((param == 1)); then
		#		((one_size=one_size+size))
		#		((one_count=one_count+1))
				#echo 'one size is '$one_size
		#	elif ((param == 2)); then
		#		((two_size=two_size+size))
		#		((two_count=two_count+1))
				#echo 'two size is '$two_size
			#elif ((param == 10)); then
			#	echo 'whole line is '$line
			#	echo 'line count is '$line_count
			#	echo 'feature paramter is 10'
		#	fi
		#fi
		#((count=count+1))
	done
	#((line_count=line_count+1))
done < wiki2018.tr 

echo $size
#echo 'zero average is ' ((zero_size/zero_count))
