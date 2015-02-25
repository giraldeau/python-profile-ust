#!/bin/sh

trap "lttng destroy -a" 2

lttng destroy -a
lttng create
lttng enable-event -u python:traceback
lttng start
#perf record -c 1 -e faults -- python3 -m linuxProfile.perf $@
perf record -c 1 -e cache-misses -- python3 -m linuxProfile.perf $@
lttng stop
lttng destroy -a
