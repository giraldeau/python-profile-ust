#!/bin/sh

lttng destroy -a
lttng create
lttng enable-event -u python:traceback
lttng start
perf record -c 1 -e faults -- python3 -m linuxProfile.perf foo.py
lttng stop
lttng view
lttng destroy -a
