from linuxProfile.tools import ProfileTree, RecordVisitor, ProfileVisitor

def test_print():
    assert("(42: 0)" == str(ProfileTree(42)))

def check_tree(op, visitor, exp):
    op(visitor)
    eq = (exp == visitor.visited)
    if (not eq):
        print("\nexp: {}\nact: {}".format(exp, visitor.visited))
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

    exp_depthfirst = [(20, 2), (21, 2), (22, 2), (10, 1), (11, 1), (12, 1), (0, 0)]
    exp_preorder = [(0, 0), (10, 1), (20, 2), (21, 2), (22, 2), (11, 1), (12, 1)]
    check_tree(n00.preorder, RecordVisitor(), exp_preorder)
    check_tree(n00.depthfirst, RecordVisitor(), exp_depthfirst)

def test_create():
    root = ProfileTree("root")
    ev = ["foo", "bar", "baz"]
    root.get_or_create_branch(ev)
    exp = [("root", 0), ("foo", 1), ("bar", 2), ("baz", 3)]
    check_tree(root.preorder, RecordVisitor(), exp)

def test_profile():
    root = ProfileTree("root")
    trace = [["a", "b", "c"], ["a", "c"]]
    # (path, total, self, children)
    exp = [ ("/a",2,0,2), ("/a/b",1,0,1), ("/a/b/c",1,1,0), ("/a/c",1,1,0) ]
    for ev in trace:
        node = root.get_or_create_branch(ev)
        node.value += 1