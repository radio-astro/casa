"""
The ppr module contains utility functions useful to PPR processing functions.

TODO There is a lot of duplication between executeppr and vlaexecuteppr. Can
this module be eliminated if a common PPR base class was created?
"""
import datetime
import os
from xml.dom import minidom

from .. import logging
from .. import task_registry

__all__ = ['check_ppr', 'write_errorexit_file']

LOG = logging.get_logger(__name__)


def check_ppr(pprfile):
    """Check PPR to make sure all tasks exist in the CASA task dictionary.

    Compares the <Command> elements of the PPR to the keys of
    the CASA task dictionary.  This is useful as a quick sanity check
    before executing the pipeline.

    Args:
        pprfile: A path to the PPR file (string).
                 e.g. 'PPR_VLAT003.xml' or 'mydata/working/PPR_VLAT003.xml'

    Returns:
        False on error.
        True  on success.
    """
    casatasks = {m.casa_task for m in task_registry.task_map}
    try:
        doc = minidom.parse(pprfile)
    except (IOError, Exception):
        LOG.info('Please provide a valid xml PPR file as input and try again.')
        return False

    tasks_in_ppr = set([str(x.firstChild.data) for x in doc.getElementsByTagName('Command')])

    undefined_tasks = (tasks_in_ppr - casatasks)
    for suspect_task in undefined_tasks:
        LOG.info("{} is not a recognized CASA task.  Check your PPR and try again.".format(suspect_task))

    if not undefined_tasks:
        LOG.info("Passed.  All tasks exist in the CASA task dictionary.")
        return True
    else:
        return False


def write_errorexit_file(path, root, extension):
    # Get formatted UTC time
    now = datetime.datetime.utcnow().strftime('%Y%m%dT%H%M%S')

    # Root name is composed of root, the current UTC time, and the extension
    basename = root + '-' + now + '.' + extension

    # Generate the root name
    if path:
        errorfile = os.path.join(path, basename)
    else:
        errorfile = basename
    with open(errorfile, 'w'):
        pass

    if os.path.exists(errorfile):
        return errorfile
    else:
        return ''
