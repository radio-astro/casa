"""
Created on 8 Sep 2014

@author: sjw
"""
import atexit
import fnmatch
import os
import pkg_resources
import shutil
import tempfile

import mako.lookup
import mako.template

import pipeline.infrastructure.logging
import pipeline.infrastructure.renderer.templates

LOG = pipeline.infrastructure.logging.get_logger(__name__)


# enumerations for registering web log renderers
UNGROUPED = 'ungrouped'
BY_SESSION = 'by_session'


def _get_template_lookup():
    """
    Create a Mako TemplateLookup object to which all pipeline templates will
    be registered. Compiled templates are stored in a temporary working
    directory which is deleted on process exit.
    """
    tmpdir = tempfile.mkdtemp()
    LOG.trace('Mako module directory = %s' % tmpdir)
    # Remove temporary Mako codegen directory on termination of Python process
    atexit.register(lambda: shutil.rmtree(tmpdir,
                                          ignore_errors=True))

    templates_path = pkg_resources.resource_filename(pipeline.infrastructure.renderer.templates.__name__, '')
    lookup = mako.lookup.TemplateLookup(directories=[templates_path],
                                        module_directory=tmpdir)
    return lookup
TEMPLATE_LOOKUP = _get_template_lookup()


def register_mako_templates(directory, prefix=''):
    """
    Find Mako templates in the given directory, registering them to the module
    template lookup. Templates will be registered to a URI composed of the URI
    prefix argument (optional) plus the template filename, minus filename
    extension.
    
    For example, a call with prefix='hif' finding a file called
    'importdata.mako' would register the template to the Mako URI 
    'hif/importdata'.
    """
    # get relative paths to all Mako templates in the directory
    relpaths = fnmatch.filter(os.listdir(directory), '*.mako')
    # convert them to absolute paths for lookup registration
    abspaths = [os.path.join(directory, t) for t in relpaths]

    if directory not in TEMPLATE_LOOKUP.directories:
        TEMPLATE_LOOKUP.directories.append(directory)

    # # TODO replace with explicit registration for control over URIs
    # for template_path in abspaths:
    #     # postponed until task import is removed from htmlrenderer
    #     root, _ = os.path.splitext(os.path.basename(template_path))
    #     uri = os.path.join(prefix, root)
    #
    #     t = mako.template.Template(filename=template_path,
    #                                format_exceptions=True,
    #                                module_directory=TEMPLATE_LOOKUP.module_directory,
    #                                lookup=TEMPLATE_LOOKUP,
    #                                uri=uri)
    #
    #     TEMPLATE_LOOKUP.put_template(uri, t)
    #     LOG.trace('%s registered to URI %s', template_path, uri)


# holds registrations of renderers that should be used in all situations,
# unless a context-specific registration takes precedence.
_UNIVERSAL_RENDERER_MAP = {}

# holds registrations of renderers that should be used in context-specific
# situations
_SPECIFIC_RENDERER_MAP = {}

# holds functions that should be used to return a context-specific key, which
# can be used to retrieve the correct renderer for that context
_SELECTOR_FN_MAP = {}

RENDER_BY_SESSION = set()
RENDER_UNGROUPED = set()


def add_renderer(task_cls, renderer, group_by=None, key_fn=None, key=None):
    """
    Register a renderer to be used to generate HTML for results from a given
    task.

    There are two modes of registration:

    1. registration of a context-specific renderer
    2. registration of a universal renderer, which wil be used if no
       context-specific renderer is found

    Context-specific renderers are registered by supplying key and key_fn
    arguments. key_fn should be a function that accepts a context and returns
    a key from it. This key is used to look up the renderer. Specifying a
    key value of 'X' says 'this renderer should be used for this task if this
    key_fn returns 'X' for this context'.

    :param task_cls: the target pipeline Task class
    :param renderer: the renderer to use for the task
    :param group_by: grouping directive - either "session" or "ungrouped"
    :param key: optional key to retrieve this renderer by
    :param key_fn: optional function that accepts a pipeline context and returns the renderer key
    :return:
    """
    if key is not None and key_fn is None:
        msg = ('Renderer registration invalid for {!s}.\n'
               'Must supply a renderer selector function when defining a renderer selector key'.format(task_cls))
        LOG.error(msg)
        raise ValueError(msg)

    # Registering without a key says that the renderer is not context
    # dependent, and the same renderer should be returned for all context
    # values.
    if key is None:
        _UNIVERSAL_RENDERER_MAP[task_cls] = renderer
    else:
        # Registering with a key says that the renderer is context dependent,
        # and a function should be used to extract the key value from that
        # context
        if task_cls not in _SPECIFIC_RENDERER_MAP:
            _SPECIFIC_RENDERER_MAP[task_cls] = {}
        _SPECIFIC_RENDERER_MAP[task_cls][key] = renderer
        _SELECTOR_FN_MAP[task_cls] = key_fn

    if group_by == 'session':
        RENDER_BY_SESSION.add(task_cls.__name__)
    elif group_by == 'ungrouped':
        RENDER_UNGROUPED.add(task_cls.__name__)
    else:
        LOG.warning('%s did not register a renderer group type. Assuming it is'
                    ' grouped by session', task_cls.__name__)
        RENDER_BY_SESSION.add(task_cls.__name__)


def get_renderer(cls, context):
    """
    Get the registered renderer for a class.

    The pipeline context argument may be passed to a registered function that
    returns the key for the given context.

    :param cls:  the class to look up
    :param context: pipeline context
    :return: registered renderer class, or KeyError if no renderer was registered
    """
    if cls in _SPECIFIC_RENDERER_MAP:
        select_fn = _SPECIFIC_RENDERER_MAP[cls]
        key = select_fn(context)
        if key in _SPECIFIC_RENDERER_MAP:
            return _SPECIFIC_RENDERER_MAP[key]

    # either not a specific renderer or key not found, so return universal
    # renderer for that task
    return _UNIVERSAL_RENDERER_MAP[cls]
