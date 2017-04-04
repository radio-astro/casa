import traceback
from IPython.core.error import InputRejected
from casa_stack_manip import stack_find

__casa = stack_find('casa')


if __casa.has_key('state') and __casa['state'].has_key('init_version') and __casa['state']['init_version'] > 0:

    #### this provides two functions:
    ####      register_builtin(str or [str]) - register builtin values
    ####      enable_builtin_protection( )   - turn on builtin value protection
    import ast

    def static_var(varname, value):
        def decorate(func):
            setattr(func, varname, value)
            return func
        return decorate

    @static_var("builtin_dict", stack_find('casa_builtins','root'))
    def register_builtin( obj ):
        if isinstance(obj,str):
            register_builtin.builtin_dict[obj] = True
        elif isinstance(obj, list) and all(isinstance(elem, str) for elem in obj):
            for b in obj: register_builtin.builtin_dict[b] = True
        else:
            raise RuntimeError("parameter is not a string or list of strings")

    class __check_builtin(ast.NodeTransformer):
        """prevent assignment to builtin values"""
        def __init__(self):
            self.casa_builtins = stack_find('casa_builtins','root')
        def visit_FunctionDef(self, node):
            return node
        def visit_Assign(self, node):
            try:
                for n in node.targets:
                    # CASA <1>: def foobar( ): return (1,2,3)
                    # CASA <2>: True,b,c = foobar( )
                    #
                    #           generates a [<ast.Tuple>] for 'node'
                    #           and tuple.elts provides access to the
                    #           strings/names within the ast.Tuple
                    #
                    tgts = n.elts if isinstance(n,ast.Tuple) else [n]
                    for t in tgts:
                        if not isinstance(t,ast.Attribute) and \
                           not isinstance(t,ast.Subscript):
                            if __builtins__.has_key(t.id):
                                raise InputRejected("attempt to modify a python builtin value")
                            if self.casa_builtins.has_key(t.id):
                                raise InputRejected("attempt to modify a casa builtin value")
            except InputRejected as ir:
                raise ir
            except:
                print "-----------------------------------------------------------------"
                print "internal error in CASA assignment filter..."
                print traceback.format_exc()
                print "-----------------------------------------------------------------"
            return node
        def visit_Lambda(self, node):
            return node

    @static_var("ip",get_ipython( ))
    def enable_builtin_protection( ):
        enable_builtin_protection.ip.ast_transformers.append(__check_builtin( ))

else:

    def enable_builtin_protection( ):
        pass

    def register_builtin( obj ):
        if not isinstance(obj,str) or not isinstance(obj, list) or not all(isinstance(elem, str) for elem in obj):
            raise RuntimeError("parameter is not a string or list of strings")
