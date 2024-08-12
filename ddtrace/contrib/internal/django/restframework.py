import rest_framework.views

from ddtrace.contrib.trace_utils import iswrapped
from ddtrace.contrib.trace_utils import with_traced_module
from ddtrace.vendor.wrapt import wrap_function_wrapper as wrap


@with_traced_module
def _traced_handle_exception(django, pin, wrapped, instance, args, kwargs):
    """Sets the error message, error type and exception stack trace to the current span
    before calling the original exception handler.
    """
    span = pin.tracer.current_span()

    if span is not None:
        span.set_traceback()

    return wrapped(*args, **kwargs)


def patch_restframework(django):
    """Patches rest_framework app.

    To trace exceptions occurring during view processing we currently use a TraceExceptionMiddleware.
    However the rest_framework handles exceptions before they come to our middleware.
    So we need to manually patch the rest_framework exception handler
    to set the exception stack trace in the current span.
    """

    # trace the handle_exception method
    if not iswrapped(rest_framework.views.APIView, "handle_exception"):
        wrap("rest_framework.views", "APIView.handle_exception", _traced_handle_exception(django))
