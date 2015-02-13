import babeltrace

'''trace of frame events:
  [
    { 'frames':
      [
        { 'co_name': str,
          'co_filename': str,
          'lineno': int,
        }, ...
      ]
    }, ...
  ]
'''

def test_load_trace():
    traces = babeltrace.TraceCollection()
    ret = traces.add_trace("nosetests/data/traceback/ust/uid/1000/64-bit/", "ctf")
    assert(ret != None)
    ecount = 0
    for event in traces.events:
        ecount += 1
        frame = event['frames'][0]
        assert(type(frame.get('co_name')) == str)
        assert(type(frame.get('co_filename')) == str)
        assert(type(frame.get('lineno')) == int)
    assert(ecount == 1)
