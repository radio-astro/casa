'''
Created on 8 Sep 2014

@author: sjw
'''
import atexit
import fnmatch
import os
import shutil
import tempfile

import mako.lookup
import mako.template

import pipeline.infrastructure.logging
import pipeline.infrastructure.renderer.templates

LOG = pipeline.infrastructure.logging.get_logger(__name__)


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

    templates_path = pipeline.infrastructure.renderer.templates.__file__
    _templates_dir = os.path.dirname(templates_path)
    lookup = mako.lookup.TemplateLookup(directories=[_templates_dir],
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

    for template_path in abspaths:
        if not os.path.exists(template_path):
            raise IOError('Template not found: %s', template_path)

    # TODO replace with explicit registration for control over URIs
    if directory not in TEMPLATE_LOOKUP.directories:
        TEMPLATE_LOOKUP.directories.append(directory)
        
        # postponed until task import is removed from htmlrenderer
# 
#         root, _ = os.path.splitext(os.path.basename(template_path))
#         uri = os.path.join(prefix, root)
#
#         t = mako.template.Template(filename=template_path, 
#                                    format_exceptions=True,
#                                    module_directory=TEMPLATE_LOOKUP.module_directory,
#                                    lookup=TEMPLATE_LOOKUP,
#                                    uri=uri)
# 
#         TEMPLATE_LOOKUP.put_template(uri, t)
#         LOG.trace('%s registered to URI %s', template_path, uri)

TEMP_RENDERER_MAP = {}
def add_renderer(task_cls, renderer):
    TEMP_RENDERER_MAP[task_cls] = renderer
    

