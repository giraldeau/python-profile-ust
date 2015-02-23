from linuxProfile.tools import ProfileTree

def test_print():
    assert("(42: 0)" == str(ProfileTree(42)))

def make_key_depth(gen):
    return list((node.key, depth) for node, depth in gen())

def make_path_stats(gen):
    res = []
    print()
    for node, depth in gen():
        print("%-10s %3d %3d %3d" % (" " * depth + node.key,
                                   node.total,
                                   node.value,
                                   node.children_sum))
        res.append(node)
    return res

def check_list(exp, act):
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
    exp = [ ("/a", 2, 0, 2), ("/a/b", 1, 0, 1), ("/a/b/c", 1, 1, 0), ("/a/c", 1, 1, 0) ]
    for ev in trace:
        leaf = root.get_or_create_branch(ev)
        leaf.value += 1
    res = make_path_stats(root.preorder)
    print(res)
