import sys
from casa_stack_manip import stack_find

__casa = stack_find('casa')


if __casa.has_key('state') and __casa['state'].has_key('init_version') and __casa['state']['init_version'] > 0:

    import ast

    __builtins = stack_find('casa_builtins','root')

    def enable( ):
        enabled = stack_find('casa_builtins_enabled','root')
        enabled = True

    def disable( ):
        enabled = stack_find('casa_builtins_enabled','root')
        enabled = False

    def register_builtin( obj ):
        if isinstance(obj,str):
            __builtins[obj] = True
        elif isinstance(obj, list) and all(isinstance(elem, str) for elem in obj):
            for b in obj: __builtins[b] = True
        else:
            raise RuntimeError("parameter is not a string or list of strings")

    class check_casa_builtin(ast.NodeVisitor):
        """Wraps all integers in a call to Integer()"""
        def visit_Num(self, node):
            #return NodeVisitor.visit_Num(node)
            return node

#    class check_casa_builtin(ast.NodeVisitor):
#        """Wraps all integers in a call to Integer()"""
#        def visit_Num(self, node):
#            if isinstance(node.n, int):
#                return ast.Call(func=ast.Name(id='Integer', ctx=ast.Load()),
#                                args=[node], keywords=[])
#            return node

    get_ipython( ).ast_transformers.append(check_casa_builtin( ))

else:

    def enable( ):
        pass
    def disable( ):
        pass
    def register_builtin( obj ):
        if not isinstance(obj,str) or not isinstance(obj, list) or not all(isinstance(elem, str) for elem in obj):
            raise RuntimeError("parameter is not a string or list of strings")
