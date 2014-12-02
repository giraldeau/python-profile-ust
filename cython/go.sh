#!/bin/sh -x

lttng destroy -a
lttng create
lttng enable-event -u python:call,python:c_call,python:return,python:c_return
lttng enable-event -u lttng_ust_cyg_profile_fast:func_entry,lttng_ust_cyg_profile_fast:func_exit
lttng enable-event -k sched_switch,sched_process_exec,sched_process_exit,sched_process_fork
lttng enable-event -k --all --syscall
lttng add-context -u -t vtid
lttng start
LD_PRELOAD=liblttng-ust-cyg-profile-fast.so python3 main.py
lttng stop
lttng destroy -a
