"""
Tools to analyze the LTTng-UST trace
"""
import babeltrace
import os

class ProfileTree(object):
    def __init__(self, key=None):
        self.key = key
        self._parent = None
        self._value = 0
        self._total = 0
        self._children_sum = 0
        self._children_map = {}
        self._dirty = False

    def add_child(self, child):
        child.parent = self
        self._children_map[child.key] = child
        return child

    @property
    def parent(self):
        return self._parent
        self._parent = parent

    @parent.setter
    def parent(self, parent):
        self._parent = parent

    @property
    def dirty(self):
        return self._dirty

    @dirty.setter
    def dirty(self, state=True):
        self._dirty = state
        if state is True and self.parent is not None:
            self.parent.dirty = True

    def get_child(self, key):
        return self._children_map.get(key, None)

    def get_or_create_child(self, key):
        if key in self._children_map:
            return self._children_map.get(key)
        return self.add_child(ProfileTree(key))

    def get_or_create_branch(self, path):
        node = self
        for p in path:
            node = node.get_or_create_child(p)
        return node

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, value):
        self._value = value
        self.dirty = True

    @property
    def children_sum(self):
        return self.total - self.value

    @property
    def total(self):
        if self.dirty:
            self._total = self.value
            for child in self.children:
                self._total += child.total
            self.dirty = False
        return self._total

    @property
    def path(self):
        path = []
        node = self
        while (node is not None):
            path.append(node)
            node = node.parent
        path.reverse()
        return path

    @property
    def children(self):
        for x in self._children_map.values():
            yield x

    def preorder(self):
        queue = [self]
        level = [1]
        while(len(queue) > 0):
            node = queue.pop(0)
            depth = len(level) - 1
            level[0] -= 1
            yield (node, depth)
            nr_children = len(list(node.children))
            if (nr_children > 0):
                level.insert(0, nr_children)
                queue = list(node.children) + queue
            if level[0] == 0:
                level.pop(0)

    def query(self, path):
        items = path
        if isinstance(path, str):
            items = path.strip("/").split("/")
        node = self
        for item in items:
            node = node.get_child(item)
            if node == None:
                break
        return node

    def __repr__(self):
        return "({}: {})".format(self.key, self.value)

class PythonTracebackEventHandler(object):
    def __init__(self, root=None):
        self.root = root
        if self.root is None:
            self.root = ProfileTree("root")
        self.count = 0

    def handle(self, event):
        if event.name != "python:traceback":
            return
        frames = event.get("frames", [])
        frames.reverse()
        node = self.root
        for frame in frames:
            node = node.get_or_create_child(frame.get("co_name", "unkown"))
        # increment the leaf
        node.value += 1
        self.count += 1

def build_profile(trace, root):
    handler = PythonTracebackEventHandler(root)
    for event in trace.events:
        handler.handle(event)

def find(dir, fname):
    for parent, dirs, files in os.walk(dir):
        if fname in files:
            yield parent

def load_trace(paths):
    if isinstance(paths, str):
        paths = [paths]
    trace = babeltrace.TraceCollection()
    for path in paths:
        for dir in find(path, "metadata"):
            ret = trace.add_trace(dir, "ctf")
            if ret == None:
                raise IOError("failed to load trace %s" % (repr(path)))
    return trace

class CalltreeReport(object):
    def __init__(self):
        pass
    def write(self, file, root):
        # symbol | self | total
        file.write("%-30s %6s %6s\n" % ("symbol", "self", "total"))
        total = float(root.total) / 100
        for node, depth in root.preorder():
            indent = " " * depth
            file.write("%-30s %6.1f%% %6.1f%%\n" %
                  (indent + node.key,
                   node.value / total,
                   node.total / total))
