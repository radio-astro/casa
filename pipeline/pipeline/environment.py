"""
environment.py defines functions and variables related to the execution environment.
"""
import multiprocessing
import os
import pkg_resources
import platform
import re
import string
import subprocess


__all__ = ['cpu_type', 'hostname', 'host_distribution', 'logical_cpu_cores', 'memory_size', 'pipeline_revision']


def _cpu_type():
    """
    Get a user-friendly string description of the host CPU.

    :return: CPU description
    """
    system = platform.system()
    if system == 'Linux':
        all_info = subprocess.check_output('cat /proc/cpuinfo', shell=True).strip()
        model_names = {line for line in all_info.split('\n') if line.startswith('model name')}
        if len(model_names) != 1:
            return 'N/A'
        # get the text after the colon
        token = ''.join(model_names.pop().split(':')[1:])
        # replace any multispaces with one space
        return re.sub('\s+', ' ', token.strip())
    elif system == 'Darwin':
        return subprocess.check_output(['sysctl', '-n', 'machdep.cpu.brand_string']).strip()
    else:
        raise NotImplemented('Could not get CPU type for system {!s}'.format(system))


def _logical_cpu_cores():
    """
    Get the number of logical (not physical) CPU cores in this machine.

    :return: number of cores
    """
    return multiprocessing.cpu_count()


def _host_distribution():
    """
    Get a description of the host operating system.

    :return: host OS description
    """
    system = platform.system()
    if system == 'Linux':
        return ' '.join(platform.linux_distribution())
    elif system == 'Darwin':
        return 'MacOS {!s}'.format(platform.mac_ver()[0])
    else:
        raise NotImplemented('Could not get host OS for system {!s}'.format(system))


def _hostname():
    """
    Get the FQDN for this machine.

    :return: FQDN of this machine
    """
    return platform.node()


def _memory_size():
    """
    Get the amount of memory on this machine.

    :return: memory size, in bytes
    :rtype: int
    """
    try:
        return os.sysconf('SC_PAGE_SIZE') * os.sysconf('SC_PHYS_PAGES')
    except ValueError:
        # SC_PHYS_PAGES doesn't always exist on OS X
        system = platform.system()
        if system == 'Darwin':
            return int(subprocess.check_output(['sysctl', '-n', 'hw.memsize']).strip())
        else:
            raise NotImplemented('Could not determine memory size for system {!s}'.format(system))


# find pipeline revision using svnversion
def _pipeline_revision():
    """
    Get a string describing the pipeline revision and branch of the executing
    pipeline distribution.

    Note: requires SVN tools to be installed on the host machine to function.

    :return: SVN description
    """
    try:
        # get SVN revision using svnversion as it gives information when the
        # directory has been modified
        args = ['svnversion', '.']

        p = subprocess.Popen(args, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, shell=True,
                             cwd=pkg_resources.resource_filename(__name__, ''))
        (stdout, _) = p.communicate()

        if p.returncode is not 0:
            return 'Unknown'

        revision = string.strip(stdout)

        # get SVN branch using svn info
        args = ['svn info .']
        myenv = os.environ.copy()
        myenv['LC_MESSAGES'] = 'en_US.UTF_8'
        p = subprocess.Popen(args, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, shell=True,
                             cwd=pkg_resources.resource_filename(__name__, ''),
                             env=myenv)
        (stdout, _) = p.communicate()

        if p.returncode is not 0:
            return revision

        kv = [s.split(':', 1) for s in stdout.splitlines()]
        # subindex kv as last item in splitlines is []
        d = {k: v.strip() for (k, v) in kv[0:-1]}

        url = d['URL']
        # pipeline trunk and branches live within this common directory
        common_svn_prefix = '/branches/project/'
        root = d['Repository Root'] + common_svn_prefix

        branch = os.path.split(os.path.relpath(url, root))[0]
        if branch == 'pipeline':
            branch = 'trunk'

        return '%s (%s)' % (revision, branch)
    except:
        return 'Unknown'


cpu_type = _cpu_type()
hostname = _hostname()
host_distribution = _host_distribution()
logical_cpu_cores = _logical_cpu_cores()
memory_size = _memory_size()
try:
    # try getting the version from the egg first
    pipeline_revision = pkg_resources.get_distribution('pipeline').version
except:
    # but if not, fall back to the old svn routine
    pipeline_revision = _pipeline_revision()
