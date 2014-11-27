#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER python

#undef TRACEPOINT_INCLUDE_FILE
#define TRACEPOINT_INCLUDE_FILE ./provider.h

#if !defined(_PYTHON_TP_PROVIDER_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _PYTHON_TP_PROVIDER_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(
    python,
    call,
    TP_ARGS(char *, name),
    TP_FIELDS(
        ctf_string(co_name, name)
    )
)

TRACEPOINT_EVENT(
    python,
    ret,
    TP_ARGS(),
    TP_FIELDS(
    )
)

#endif /* _PYTHON_TP_PROVIDER_H */
#include <lttng/tracepoint-event.h>
