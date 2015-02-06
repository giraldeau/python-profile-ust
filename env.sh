#!/bin/sh

for f in $(find build/ -type f | grep cli.py); do
	dir=$(dirname $(dirname $f))
	export PYTHONPATH=$(pwd)/${dir}
done
echo "PYTHONPATH=$PYTHONPATH"
