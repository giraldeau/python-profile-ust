from linuxProfile.tools import ProfileTree

def test_print():
    assert("(42: 0)" == str(ProfileTree(42)))

def check_list(exp, gen):
    act = list((x[0].key, x[1]) for x in gen())
    eq = (exp == act)
    if (not eq):
        print("\nexp: {}\nact: {}".format(exp, act))
    assert(eq)

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
    check_list(exp, n00.preorder)

def test_create():
    root = ProfileTree("root")
    ev = ["foo", "bar", "baz"]
    root.get_or_create_branch(ev)
    exp = [("root", 0), ("foo", 1), ("bar", 2), ("baz", 3)]
    check_list(exp, root.preorder)

def test_property_value():
    root = ProfileTree("foo")
    root.value += 1
    assert(root.value == 1)

def test_profile():
    root = ProfileTree("root")
    trace = [["a", "b", "c"], ["a", "c"]]
    # (path, total, self, children)
    exp = [ ("/a", 2, 0, 2), ("/a/b", 1, 0, 1), ("/a/b/c", 1, 1, 0), ("/a/c", 1, 1, 0) ]
    for ev in trace:
        leaf = root.get_or_create_branch(ev)
        leaf.value += 1
