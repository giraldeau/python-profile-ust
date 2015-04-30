from linuxProfile.tools import ProfileTree, CalltreeReport, PythonTracebackEventHandler, StatItem, build_profile, make_path_stats, profile_rms_error, make_list_gen
import sys  # temp

class StubTrace(dict):
    pass

class StubEvent(dict):
    pass

class StubFrame(dict):
    def __init__(self, co_name, co_filename="<module>", lineno=42):
        super(StubFrame, self).__init__()
        self["co_name"] = co_name
        self["co_filename"] = co_filename
        self["lineno"] = lineno

def test_print():
    assert("(42: 0)" == str(ProfileTree(42)))

def make_key_depth(gen):
    return list((node.key, depth) for node, depth in gen())

def check_profile(root, stats):
    for stat in stats:
        node = root.query(stat.path)
        assert(node != None)
        assert(node.total == stat.total)
        assert(node.value == stat.value)
        assert(node.children_sum == stat.children_sum)

def check_list(exp, act):
    s1 = frozenset(exp)
    s2 = frozenset(act)
    diff = s1.symmetric_difference(s2)
    if len(diff) != 0:
        print("\nexp: {}\nact: {}\ndiff: {}\n".format(exp, act, diff))
    assert(len(diff) == 0)

def test_tree():
    n00 = ProfileTree(0)
    n10 = ProfileTree(10)
    n11 = ProfileTree(11)
    n12 = ProfileTree(12)
    n20 = ProfileTree(20)
    n21 = ProfileTree(21)
    n22 = ProfileTree(22)

    n00.add_child(n10)
    n00.add_child(n11)
    n00.add_child(n12)

    n10.add_child(n20)
    n10.add_child(n21)
    n10.add_child(n22)

    exp = [(0, 0), (10, 1), (20, 2), (21, 2), (22, 2), (11, 1), (12, 1)]
    check_list(exp, make_key_depth(n00.preorder))

def test_create():
    root = ProfileTree("root")
    ev = ["foo", "bar", "baz"]
    root.get_or_create_branch(ev)
    exp = [("root", 0), ("foo", 1), ("bar", 2), ("baz", 3)]
    check_list(exp, make_key_depth(root.preorder))

def test_property_value():
    root = ProfileTree("foo")
    root.value += 1
    assert(root.value == 1)

def test_profile():
    root = ProfileTree("root")
    trace = [["a", "b", "c"], ["a", "c"]]
    # (path, total, self, children)
    exp = [ ("/root", 2, 0, 2), ("/root/a", 2, 0, 2), ("/root/a/b", 1, 0, 1), ("/root/a/b/c", 1, 1, 0), ("/root/a/c", 1, 1, 0) ]
    for ev in trace:
        leaf = root.get_or_create_branch(ev)
        leaf.value += 1
    act = make_path_stats(root.preorder)
    check_list(exp, act)

    items = []
    for e in exp:
       items.append(StatItem(e[0], e[1], e[2], e[3]))
    rms = profile_rms_error(items, root)
    assert(rms < 0.0000001)

def yaml_trace(yam):
    with open(yam) as f:
        trace = yaml.load(f)
        for event in trace['events']:
            yield event

traces = {
    "basic": [
        { "frames": [ StubFrame('baz'),
                      StubFrame('bar'),
                      StubFrame('foo'), ]
        },
        { "frames": [ StubFrame('baz'),
                      StubFrame('foo'), ]
        },
    ]
}

exp_stats = {
    "basic": [StatItem("/foo", 2, 0, 2),
             ]
}

def wrap_events(events, attr, val):
    stubs = []
    for event in events:
        stub = StubEvent(event)
        setattr(stub, attr, val)
        stubs.append(stub)
    return stubs

def test_profile_samples():
    for name, events in traces.items():
        trace = StubTrace()
        trace.events = wrap_events(events, "name", "python:traceback")
        root = ProfileTree("root")
        build_profile(trace, root)
        check_profile(root, exp_stats[name])
