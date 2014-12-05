#!/bin/sh

rm -rf build
python3 setup.py build

lttng destroy -a
lttng create
lttng enable-event -u python:call,python:c_call,python:return,python:c_return
#lttng enable-event -a -u
lttng start
python3 -m profileUST foo.py
lttng stop
lttng view
lttng destroy -a
