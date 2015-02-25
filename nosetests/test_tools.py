from linuxProfile.tools import ProfileTree, CalltreeReport
import sys # temp
from collections import namedtuple

def test_print():
    assert("(42: 0)" == str(ProfileTree(42)))

def make_key_depth(gen):
    return list((node.key, depth) for node, depth in gen())

def make_path_stats(gen):
    res = []
    print()
    for node, depth in gen():
        path = "/" + "/".join([x.key for x in node.path])
        item = (path, node.total, node.value, node.children_sum)
        res.append(item)
    return res

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

def yaml_trace(yam):
    with open(yam) as f:
        trace = yaml.load(f)
        print(trace)
        for event in trace['events']:
            yield event

class StubFrame(dict):
    def __init__(self, co_name, co_filename = "<module>", lineno = 42):
        super(StubFrame, self).__init__()
        self["co_name"] = co_name
        self["co_filename"] = co_filename
        self["lineno"] = lineno

traces = {
    "basic": [
        { "frames": [ StubFrame('baz'),
                      StubFrame('bar'),
                      StubFrame('foo'),]
        },
        { "frames": [ StubFrame('baz'),
                      StubFrame('foo'),]
        },
    ]
}

def test_profile_samples():
    root = ProfileTree("root")
    for name, events in traces.items():
        print("processing " + name)
        for event in events:
            frames = event["frames"]
            frames.reverse()
            print(frames)
            node = root
            for frame in frames:
                print(frame)
                node = node.get_or_create_child(frame["co_name"])
            node.value += 1
    report = CalltreeReport()
    report.write(sys.stdout, root)
    