"""
The utils package contains a set of utility classes and functions that are
useful to the pipeline framework and to tasks manipulating pipeline framework
objects, Python data types, and CASA data types.

The utils package is intended to be free of any task-specific logic. Code that
assumes knowledge or logic beyond that of the task-independent framework should
be housed in the h.common package (or hif.common, hifv.common, hsd.common, etc.
as appropriate).
"""
from importlib import import_module

from .conversion import *
from .diagnostics import *
from .framework import *
from .imaging import *
from .ppr import *
from .sorting import *
from .utils import *
from .weblog import *

# IMPORTANT! If you import from a new submodule, please add it to the list below
_all_modules = ['conversion', 'diagnostics', 'framework', 'imaging', 'ppr', 'sorting', 'utils', 'weblog']


def _ensure_no_multiple_definitions(module_names):
    """
    Raise an ImportError if references are exported with the same name.

    The aim of this function is to prevent functions with the same name being
    imported into the same namespace. For example, import
    module_a.my_function and module_b.my_function would raise an error.

    This function depends on __all__ being defined correctly in the package
    modules.

    :param module_names: names of submodules to check
    """
    package_modules = [import_module('.{}'.format(m), package=__name__) for m in module_names]
    names_and_declarations = [(m, set(m.__all__)) for m in package_modules]

    all_declarations = set()
    for module_name, declaration in names_and_declarations:
        if declaration.isdisjoint(all_declarations):
            all_declarations.update(declaration)
        else:
            raise ImportError('Utility module {} contains duplicate definitions: {}'
                              ''.format(module_name.__name__,
                                        ','.join(d for d in declaration.intersection(all_declarations))))


_ensure_no_multiple_definitions(_all_modules)
