#!/bin/sh

mkdir -p develop/
export PYTHONPATH="$(pwd)/develop/"
export PATH=$(pwd)/develop:$PATH
echo "PYTHONPATH=$PYTHONPATH"
echo "To build the project, execute the command:"
echo "python3 setup.py develop --install-dir=develop/"
