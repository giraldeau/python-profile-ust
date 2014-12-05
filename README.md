# LTTng-UST for CPython #

The prototype of UST provider records function entry an return from C and
Python code. Requires CPython >= 3.

Available events:
* python:call
* python:c_call
* python:return
* python:c_return

## Install on Ubuntu 14.04 ##

Install dependencies and build:
```
sudo apt-get install lttng-tools liblttng-ust-dev python3-dev
python3 setup.py build
sudo python3 setup.py install
```

## Usage ##

```python
from linuxProfile import api
api.enable_ust()
# do something
api.disable_ust()
```

Events will be recorded to any active LTTng-UST session. (see go.sh for an
example of trace management script). Due to a bug in LTTng-UST, baddr events
must not be enabled. If they are, a deadlock occurs. (Bug:
https://bugs.lttng.org/issues/865)

Exceptions are not supported yet. I was not able to trigger PyTrace_EXCEPTION
event using "raise" or similar (see nosetests/call_exception.py)

Function name is a start, but the complete namespace should be recorded to
avoid ambiguity (such as package.module.class.method). Probably that file and
lineno should also be recorded. Suggestions welcome.

Here is an example of the trace for the following `foo.py` code:

```python
def baz():
    pass

def bar():
    baz()

def foo():
    bar()

if __name__=='__main__':
    foo()
```

```bash
lttng create
lttng enable-event -u python:call,python:c_call,python:return,python:c_return
lttng start
python3 -m linuxProfile.ust foo.py
lttng stop
lttng view
```

Trace produced:
```
[17:27:14.772391868] (+?.?????????) berta python:c_call: { cpu_id = 4 }, { co_name = "exec" }
[17:27:14.772405974] (+0.000014106) berta python:call: { cpu_id = 4 }, { co_name = "<module>" }
[17:27:14.772417488] (+0.000011514) berta python:call: { cpu_id = 4 }, { co_name = "foo" }
[17:27:14.772425974] (+0.000008486) berta python:call: { cpu_id = 4 }, { co_name = "bar" }
[17:27:14.772435480] (+0.000009506) berta python:call: { cpu_id = 4 }, { co_name = "baz" }
[17:27:14.772444008] (+0.000008528) berta python:return: { cpu_id = 4 }, { }
[17:27:14.772453087] (+0.000009079) berta python:return: { cpu_id = 4 }, { }
[17:27:14.772461700] (+0.000008613) berta python:return: { cpu_id = 4 }, { }
[17:27:14.772470012] (+0.000008312) berta python:return: { cpu_id = 4 }, { }
[17:27:14.772478886] (+0.000008874) berta python:c_return: { cpu_id = 4 }, { }
[17:27:14.772496402] (+0.000017516) berta python:c_call: { cpu_id = 4 }, { co_name = "disable" }
```

