#!/bin/sh

for f in $(find build/ -type d | grep linuxProfile); do 
	dir=$(dirname $(dirname $f))
	export PYTHONPATH=$(pwd)/${dir}
done
