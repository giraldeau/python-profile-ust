"""
Tools to analyze the LTTng-UST trace
"""

class ProfileTree(object):
    def __init__(self, key=None):
        self.key = key
        self._parent = None
        self._value = 0
        self._children_map = {}
        self._children_sum = 0
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

    def get_total(self):
        total = self.value
        if self.dirty:
            self.refresh()
        for child in self.children.values():
            total += child.get_total()
        return total

    def get_value(self):
        return self.value

    def get_path(self):
        path = []
        node = self
        while (node is not None):
            path.append(node)
            node = node.parent
        path.reverse()
        return path

    def refresh(self):
        self.children_sum = 0
        for child in self.children:
            child.refresh()
            self.children_sum += child.get_children()

    @property
    def children(self):
        return list(self._children_map.values())

    def preorder(self):
        queue = [self]
        level = [1]
        while(len(queue) > 0):
            node = queue.pop(0)
            depth = len(level) - 1
            level[0] -= 1
            yield (node, depth)
            nr_children = len(node.children)
            if (nr_children > 0):
                level.insert(0, nr_children)
                queue = node.children + queue
            if level[0] == 0:
                level.pop(0)

    def __repr__(self):
        return "({}: {})".format(self.key, self.value)
