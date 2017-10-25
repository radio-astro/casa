from __future__ import print_function
import distutils.cmd
import distutils.log
import os
import fileinput
import shlex
import setuptools
from setuptools.command.build_py import build_py
import subprocess

from contextlib import closing

SVN_REV = subprocess.check_output(shlex.split('svnversion')).strip()


class BuildMyTasksCommand(distutils.cmd.Command):
    description = 'Generate the CASA CLI bindings'
    user_options = [('inplace', 'i', 'Generate CLI bindings in src directory')]
    boolean_options = ['inplace']

    def __init__(self, dist):
        distutils.cmd.Command.__init__(self, dist)

    def initialize_options(self):
        """Set default values for options."""
        self.inplace = None

    def finalize_options(self):
        # get the path to this file
        dir_path = os.path.dirname(os.path.realpath(__file__))

        if self.inplace:
            self.build_path = dir_path
        else:
            # get the path to the build directory
            build_py_cmd = self.get_finalized_command('build_py')
            self.build_path = os.path.join(dir_path, build_py_cmd.build_lib)

    def run(self):
        for d in ['h', 'hif', 'hifa', 'hifv', 'hsd']:
            package_path = os.path.join('pipeline', d, 'cli')
            srcdir = os.path.join(self.build_path, package_path)

            if not os.path.exists(srcdir):
                continue

            output_module = '{!s}.py'.format(d)

            self.announce('Building tasks for package: {!s}'.format(d), level=distutils.log.INFO)
            subprocess.check_output([
                'buildmytasks',
                '-i={!s}'.format(srcdir),
                '-o={!s}'.format(output_module),
            ], cwd=srcdir)

            # buildmytasks hard-codes the module locations into the generated
            # code, from which it can locate the XML definitions for the help.
            # This replaces the hard-coded paths with dynamic resolution of
            # the module locations.
            output_file = os.path.join(self.build_path, package_path, output_module)
            replacement_text = 'target_dir'

            with closing(fileinput.FileInput(output_file, inplace=True)) as file:
                for line in file:
                    if file.isfirstline():
                        print('import os.path\n'
                              'import sys\n'
                              'import {!s} as target_cli\n'
                              '\n'
                              'target_dir = os.path.dirname(os.path.realpath(target_cli.__file__))\n'
                              ''.format(package_path.replace('/', '.')))

                    print(line.replace("'%s'" % srcdir, replacement_text), end='')


class PipelineBuildPyCommand(build_py):
    def run(self):
        build_py.run(self)
        self.run_command('buildmytasks')


packages = setuptools.find_packages()
packages += ['pipeline.infrastructure.renderer.templates.resources.css',
             'pipeline.infrastructure.renderer.templates.resources.fonts',
             'pipeline.infrastructure.renderer.templates.resources.img',
             'pipeline.infrastructure.renderer.templates.resources.js']

setuptools.setup(
    name='Pipeline',
    version='5.3',
    description='CASA pipeline package',
    cmdclass={
        'buildmytasks': BuildMyTasksCommand,
        'build_py': PipelineBuildPyCommand,
    },
    # install_requires=[
    #     'cachetools',
    #     'intervaltree',
    #     'Mako',
    #     'pyparsing',
    #     'sortedcontainers'
    # ],
    options=dict(egg_info=dict(tag_build="-r" + SVN_REV)),
    packages=packages,
    package_data={'': ['*.css',
                       '*.egg',
                       '*.eot',
                       '*.gif',
                       '*.html',
                       '*.jpg',
                       '*.js',
                       '*.mak',
                       '*.mako',
                       '*.otf',
                       '*.png',
                       '*.svg',
                       '*.ttf',
                       '*.xml',
                       '*.zip']},
    zip_safe=True
)
