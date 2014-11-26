#!/bin/sh

/home/francis/cpython-bin/bin/python3 setup.py build

lttng destroy -a
lttng create
lttng enable-event -a -u
lttng start
/home/francis/cpython-bin/bin/python3 go.py
lttng stop
lttng view
lttng destroy -a
