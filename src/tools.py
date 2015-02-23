"""
Tools to analyze the LTTng-UST trace
"""

class TreeVisitor(object):
    def __init__(self):
        pass
    def visit(self, node, depth):
        pass

class RecordVisitor(TreeVisitor):
    def __init__(self):
        self.visited = []
    def visit(self, node, depth):
        self.visited.append((node.key, depth))
    def get_visited(self):
        return self.visited

class ProfileTree(object):
    def __init__(self, key=None):
        self.parent = None
        self.key = key
        self.value = 0
        self.children = {}
        self.children_sum = 0
        self.dirty = False

    def add_child(self, child):
        child.set_parent(self)
        self.children[child.key] = child
        return child

    def set_parent(self, parent):
        self.parent = parent

    def get_child(self, key):
        return self.children.get(key, None)

    def get_or_create_child(self, key):
        if key in self.children:
            return self.children.get(key)
        return self.add_child(ProfileTree(key))

    def get_or_create_branch(self, path):
        res = [self]
        for p in path:
            res.append(node.get_or_create_child(p))
        return res

    def set_dirty(self):
        self.dirty = True
        self.parent.set_dirty()

    def set_value(self, value):
        self.value = value
        self.set_dirty()

    def get_total(self):
        total = self.value
        if self.dirty:
            self.refresh()
        for child in self.children.values():
            total += child.get_total()
        return total

    def get_value(self):
        return self.value

    def refresh(self):
        self.children_sum = 0
        for child in self.children.values():
            child.refresh()
            self.children_sum += child.get_children()

    def preorder(self, visitor):
        queue = [self]
        level = [1]
        while(len(queue) > 0):
            node = queue.pop(0)
            depth = len(level) - 1
            level[0] -= 1
            visitor.visit(node, depth)
            nr_children = len(node.children)
            if (nr_children > 0):
                level.insert(0, nr_children)
                queue = list(node.children.values()) + queue
            if level[0] == 0:
                level.pop(0)

    def depthfirst(self, visitor):
        self._depthfirst(visitor, self, 0)

    def _depthfirst(self, visitor, node, depth):
        for child in node.children.values():
            self._depthfirst(visitor, child, depth + 1)
        visitor.visit(node, depth)

    def __repr__(self):
        return "({}: {})".format(self.key, self.value)
