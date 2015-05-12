"""
Tools to analyze the LTTng-UST trace
"""
import babeltrace
import os
from operator import attrgetter
from linuxProfile.utils import NullProgressBar
from collections import namedtuple
import math

class StatItem(namedtuple("StatItem", "path total value children_sum")):
    pass

class ProfileTree(object):
    def __init__(self, key=None):
        self.key = key
        self._parent = None
        self._value = 0
        self._total = 0
        self._children_sum = 0
        self._children_map = {}
        self._dirty = False
        self._children_sorted = []

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

    @property
    def children_sorted(self):
        self._children_sorted = sorted(self._children_map.values(),
                                       key=attrgetter('key'),
                                       reverse=False)
        for x in self._children_sorted:
            yield x

    def preorder(self):
        queue = [self]
        level = [1]
        while(len(queue) > 0):
            node = queue.pop(0)
            depth = len(level) - 1
            level[0] -= 1
            yield (node, depth)
            nr_children = len(list(node.children_sorted))
            if (nr_children > 0):
                level.insert(0, nr_children)
                queue = list(node.children_sorted) + queue
            if level[0] == 0:
                level.pop(0)

    def query(self, path):
        items = path
        if isinstance(path, str):
            items = path.strip("/").split("/")
        node = self
        if len(items) == 0:
            return None
        if len(items) == 1:
            return node
        for item in items[1:]:
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
        if (event.name != "python:traceback"):
            return
        frames = event.get("frames", [])
        frames.reverse()
        node = self.root
        for frame in frames:
            node = node.get_or_create_child(frame.get("co_name", "unkown"))
        # increment the leaf
        node.value += 1
        self.count += 1

# The number of events and the timestamp_end are unkown
# approximation for the progress bar using trace size and event size
# And it is shitty
BYTES_PER_EVENT = 400

def getFolderSize(folder):
    total_size = os.path.getsize(folder)
    for item in os.listdir(folder):
        itempath = os.path.join(folder, item)
        if os.path.isfile(itempath):
            total_size += os.path.getsize(itempath)
        elif os.path.isdir(itempath):
            total_size += getFolderSize(itempath)
    return total_size

def build_profile(trace, root, bar=None):
    if bar is None:
        bar = NullProgressBar()
    handler = PythonTracebackEventHandler(root)
    if (hasattr(trace, "size")):
        bar.total_work = trace.size
    x = 0
    for event in trace.events:
        handler.handle(event)
        x += BYTES_PER_EVENT
        bar.update(x)
    bar.done()
    return handler

def make_list_gen(lst):
    def list_gen():
        for item in lst:
            yield item
    return list_gen

def make_path_stats(gen):
    res = []
    for node, depth in gen():
        path = "/" + "/".join([x.key for x in node.path])
        item = (path, node.total, node.value, node.children_sum)
        res.append(item)
    return res

# FIXME: will return 0 if there is no intersection between profile
# it should return instead the maximum error
def profile_rms_error(stats, root):
    root_total = float(root.total)
    stat_total = 0.0
    for stat in stats:
        stat_total += float(stat.value)
    rms = 0.0
    for stat in stats:
        node = root.query(stat.path)
        p1 = 0.0
        if node != None and root_total > 0.0:
            p1 = node.total / root_total
        p2 = 0.0
        if stat_total > 0.0:
            p2 = stat.total / stat_total
        err = p1 - p2
        # print("{} {} {} {} {}".format(node, stat, p1, p2, err))
        rms += err * err
    rms = math.sqrt(rms)
    return rms

def find(dir, fname):
    for parent, dirs, files in os.walk(dir):
        if fname in files:
            yield parent

def load_trace(paths):
    if isinstance(paths, str):
        paths = [paths]
    trace = babeltrace.TraceCollection()
    trace.size = 0
    for path in paths:
        if not os.path.isdir(path):
                raise IOError("Path is not a directory")
        for dir in find(path, "metadata"):
            ret = trace.add_trace(dir, "ctf")
            if ret == None:
                raise IOError("failed to load trace %s" % (repr(path)))
            trace.size += getFolderSize(dir)
    return trace

class CalltreeReport(object):
    def __init__(self):
        pass
    def write(self, file, root):
        # symbol | self | total
        file.write("%-30s %6s %6s\n" % ("symbol", "self (%)", "total (%)"))
        total = float(root.total) / 100
        for node, depth in root.preorder():
            indent = " " * depth
            p1 = p2 = 0.0
            if (total > 0):
                p1 = node.value / total
                p2 = node.total / total
            file.write("%-30s %6d (%4.1f%%) %6d (%4.1f%%)\n" %
                  (indent + node.key,
                   node.value, p1,
                   node.total, p2))
