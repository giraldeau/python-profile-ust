#!/bin/sh

rm -rf build
python3 setup.py build

lttng destroy -a
lttng create
#lttng enable-event -u python:call
lttng enable-event -a -u
lttng start
python3 go.py
lttng stop
lttng view
lttng destroy -a
