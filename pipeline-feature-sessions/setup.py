from __future__ import print_function
import os
import fileinput
import setuptools
from setuptools.command.build_py import build_py
import subprocess

from contextlib import closing


class PipelineBuildPyCommand(build_py):
    def run(self):
        build_py.run(self)

        dir_path = os.path.dirname(os.path.realpath(__file__))
        build_path = os.path.join(dir_path, self.build_lib)

        for d in ['h', 'hif', 'hifa', 'hifv', 'hsd']:
            package_path = os.path.join('pipeline', d, 'cli')
            srcdir = os.path.join(build_path, package_path)

            if not os.path.exists(srcdir):
                continue

            output_module = '{!s}.py'.format(d)

            print('Building tasks for package: %s' % d)
            subprocess.check_output([
                'buildmytasks',
                '-i={!s}'.format(srcdir),
                '-o={!s}'.format(output_module),
            ], cwd=srcdir)

            # buildmytasks hard-codes the module locations into the generated
            # code, from which it can locate the XML definitions for the help.
            # This replaces the hard-coded paths with dynamic resolution of
            # the module locations.
            output_file = os.path.join(build_path, package_path, output_module)
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


packages = setuptools.find_packages()
packages += ['pipeline.infrastructure.renderer.templates.resources.css',
             'pipeline.infrastructure.renderer.templates.resources.fonts',
             'pipeline.infrastructure.renderer.templates.resources.img',
             'pipeline.infrastructure.renderer.templates.resources.js']

setuptools.setup(
    name='Pipeline',
    version='0.1',
    description='Pipeline package',
    cmdclass={
        'build_py': PipelineBuildPyCommand,
    },
    # install_requires=[
    #     'cachetools',
    #     'intervaltree',
    #     'Mako',
    #     'pyparsing',
    #     'sortedcontainers'
    # ],
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
    zip_safe=False
)
