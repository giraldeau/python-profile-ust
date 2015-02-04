#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER python

#undef TRACEPOINT_INCLUDE_FILE
#define TRACEPOINT_INCLUDE_FILE ./tp.h

#if !defined(_PYTHON_TP_PROVIDER_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _PYTHON_TP_PROVIDER_H

#include <lttng/tracepoint.h>
#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>

#include "pmu.h"

/* disable template not working with ctf_struct patch */
/*
TRACEPOINT_EVENT_CLASS(python, call_template,
    TP_ARGS(const char *, name),
    TP_FIELDS(
        ctf_string(co_name, name)
    )
)

TRACEPOINT_EVENT_INSTANCE(python, call_template, call,
    TP_ARGS(const char *, name))

TRACEPOINT_EVENT_INSTANCE(python, call_template, c_call,
    TP_ARGS(const char *, name))

TRACEPOINT_EVENT_CLASS(python, ret_template,
    TP_ARGS(),
)

TRACEPOINT_EVENT_INSTANCE(python, ret_template, return,
    TP_ARGS())

TRACEPOINT_EVENT_INSTANCE(python, ret_template, c_return,
    TP_ARGS())

TRACEPOINT_EVENT(python, callstack,
		TP_ARGS(),
		TP_FIELDS()
)
*/

TRACEPOINT_EVENT(python, call,
    TP_ARGS(const char *, name),
    TP_FIELDS(
        ctf_string(co_name, name)
    )
)

TRACEPOINT_EVENT(python, c_call,
    TP_ARGS(const char *, name),
    TP_FIELDS(
        ctf_string(co_name, name)
    )
)

TRACEPOINT_EVENT(python, return,
    TP_ARGS(),
    TP_FIELDS()
)

TRACEPOINT_EVENT(python, c_return,
    TP_ARGS(),
    TP_FIELDS()
)

TRACEPOINT_STRUCT(python, frame,
    TP_ARGS(struct frame *, frame),
    TP_FIELDS(
        ctf_sequence_text(char, co_filename, frame->co_filename, size_t, frame->co_filename_len)
        ctf_sequence_text(char, co_name, frame->co_name, size_t, frame->co_name_len)
        ctf_integer(int, lineno, frame->lineno)
    )
)

TRACEPOINT_EVENT(python, traceback,
    TP_ARGS(struct frame *, framevar, int, depth),
    TP_FIELDS(
        ctf_sequence_of_struct(python, frame, frames, framevar, int, depth)
    )
)

#endif /* _PYTHON_TP_PROVIDER_H */
#include <lttng/tracepoint-event.h>
