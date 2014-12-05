#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER python

#undef TRACEPOINT_INCLUDE_FILE
#define TRACEPOINT_INCLUDE_FILE ./provider.h

#if !defined(_PYTHON_TP_PROVIDER_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _PYTHON_TP_PROVIDER_H

#include <lttng/tracepoint.h>

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

#endif /* _PYTHON_TP_PROVIDER_H */
#include <lttng/tracepoint-event.h>
