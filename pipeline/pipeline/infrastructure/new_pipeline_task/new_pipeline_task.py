"""
Create a new CASA pipeline task.

The purpose of this script is to make it easy to create
a generic pipeline task, save time and not miss any
essential steps.
"""

import argparse
import os
import shutil
import sys
import tempfile


class NewTask():

    def __init__(self):
        self.repository_path = os.environ['SCIPIPE_HEURISTICS']

    def parse_command_line(self, argv):

        script_args = ''
        for idx, arg in enumerate(argv):
            if 'new_pipeline_task.py' in arg:
                script_args = argv[idx + 1:]

        parser = argparse.ArgumentParser(prog="new_pipeline_task", add_help=False)
        parser.add_argument('--package', help="Pipeline package.  One of 'h', 'hif', 'hifa', 'hifa', 'hifv', or 'hsd'.",
                            type=str, choices=['h', 'hif', 'hifa', 'hifa', 'hifv', 'hsd'], required=True)
        parser.add_argument('--task', help='New task name', type=str, required=True)
        parser.add_argument('--module',
                            help="Optional module name.  e.g. if task is 'foo' "
                                 "and module is 'bar' then 'tasks/foo/bar.py' is created",
                            type=str, default='', required=False)

        try:
            args = parser.parse_args(script_args)
        except:
            parser.print_help()
            return '', '', ''

        area = args.package
        task_name = args.task

        if args.module:
            module_name = args.module
        else:
            module_name = task_name

        return area, task_name, module_name

    def create(self, area, task_name, module_name):

        from mako.template import Template

        task_name = task_name.lower()

        # -----------------------------------------------------------------------------
        # define the directories for new files
        # -----------------------------------------------------------------------------
        task_dir = '{repo}/pipeline/{area}/tasks/{task}'.format(repo=self.repository_path,
                                                                area=area, task=task_name)
        cli_dir = '{repo}/pipeline/{area}/cli'.format(repo=self.repository_path, area=area)

        # -----------------------------------------------------------------------------
        # create the task directory
        # -----------------------------------------------------------------------------
        print('1.')
        print('\tCreating {f}'.format(f=task_dir))
        try:
            os.mkdir(task_dir)
        except OSError as ee:
            if ee.errno == 17:
                pass
            else:
                raise

        # -----------------------------------------------------------------------------
        # define the new files
        # -----------------------------------------------------------------------------
        module_file = '{tdir}/{task}.py'.format(tdir=task_dir, task=module_name)
        init_file = '{tdir}/__init__.py'.format(tdir=task_dir)
        cli_file = '{cdir}/task_{area}_{task}.py'.format(cdir=cli_dir, area=area, task=task_name)
        cli_xml = '{cdir}/{area}_{task}.xml'.format(cdir=cli_dir, area=area, task=task_name)
        weblog_mako = '{repo}/pipeline/{area}/templates/{task}.mako'.format(repo=self.repository_path, area=area, task=task_name)

        # -----------------------------------------------------------------------------
        # instantiate the templates
        # -----------------------------------------------------------------------------
        module_template = Template(filename='{repo}/pipeline/infrastructure/new_pipeline_task/'
                                            'pipeline_task_module.mako'.format(repo=self.repository_path))
        cli_template = Template(filename='{repo}/pipeline/infrastructure/new_pipeline_task/'
                                         'pipeline_cli_module.mako'.format(repo=self.repository_path))
        cli_xml_template = Template(filename='{repo}/pipeline/infrastructure/new_pipeline_task/'
                                             'pipeline_cli_xml.mako'.format(repo=self.repository_path))
        init_template = Template(filename='{repo}/pipeline/infrastructure/new_pipeline_task/'
                                          'pipeline_task_init.mako'.format(repo=self.repository_path))

        # -----------------------------------------------------------------------------
        # create the files
        # -----------------------------------------------------------------------------

        print('\tCreating {f}'.format(f=module_file))
        with open(module_file, 'w+') as fd:
            fd.writelines(module_template.render(package=area, taskname=task_name))

        print('\tCreating {f}'.format(f=init_file))
        with open(init_file, 'w+') as fd:
            fd.writelines(init_template.render(taskname=task_name, modulename=module_name))

        print('\tCreating {f}'.format(f=cli_file))
        with open(cli_file, 'w+') as fd:
            fd.writelines(cli_template.render(package=area, taskname=task_name))

        print('\tCreating {f}'.format(f=cli_xml))
        with open(cli_xml, 'w+') as fd:
            fd.writelines(cli_xml_template.render(package=area, taskname=task_name))

        # print('\tCreating {f}'.format(f=weblog_mako))
        # with open(weblog_mako, 'w+') as fd:
        #     fd.writelines(web_template.render(taskname=task_name))

        # -----------------------------------------------------------------------------
        # -----------------------------------------------------------------------------

        print('''2.
        Check infrastructure/jobrequest.py to see if it lists all CASA tasks
        needed by your new pipeline task.  If not, add them to the
        CASATaskGenerator class.
        ''')

        # -----------------------------------------------------------------------------
        # Add import to package __init__.py
        # -----------------------------------------------------------------------------

        package_init_file = "{repo}/pipeline/{area}/tasks/__init__.py".format(repo=self.repository_path, area=area)

        with open(package_init_file) as fd:
            init_file_data = fd.readlines()

        last_import_line = 0
        task_already_in_init = False

        # look for the last "from " import line and
        # add the new module import on the next line
        for idx, line in enumerate(init_file_data):
            if task_name in line:
                task_already_in_init = True
            elif line.startswith('from '):
                last_import_line = idx

        if task_already_in_init is False:
            init_file_data.insert(last_import_line+1, 'from .{task} import {task_class}\n'.format(task=task_name,
                                                                                                  task_class=task_name.capitalize()))
        print('\tAdded "from .{task} import {task_class}" to {pfile}'.format(task=task_name,
                                                                             task_class=task_name.capitalize(),
                                                                             pfile=package_init_file))

        temp_init_file = tempfile.NamedTemporaryFile(delete=False)
        with open(temp_init_file.name, "w+") as fd:
            fd.writelines(init_file_data)

        shutil.copy(temp_init_file.name, package_init_file)
        os.unlink(temp_init_file.name)

        # -----------------------------------------------------------------------------
        # Say something about call library
        # -----------------------------------------------------------------------------

        print('''3.
        Consider adding code to infrastructure/callibrary.py if needed.
        ''')

        # -----------------------------------------------------------------------------
        # Say something about file namer
        # -----------------------------------------------------------------------------

        print('''4.
        Consider adding code to infrastructure/filenamer.py if needed for calibration tables.
        ''')

        # -----------------------------------------------------------------------------
        # Web log rendering
        # -----------------------------------------------------------------------------

        print('''5.
        Consider adding code to infrastructure/displays/ if needed for the web log.
        ''')

        print('''6.
        Later you might need to add a renderer.py file to {area}/tasks/{task}/ if needed for the web log.
        '''.format(area=area, task=task_name))

        # -----------------------------------------------------------------------------
        # mako template for weblog
        # -----------------------------------------------------------------------------
        mako_template = '{repo}/pipeline/infrastructure/new_pipeline_task/pipeline_weblog_task.mako'.format(repo=self.repository_path)

        print('7.')
        print('\tCreating {f}'.format(f=weblog_mako))

        temp_mako = tempfile.NamedTemporaryFile(delete=False)

        with open(mako_template) as infile, open(temp_mako.name, 'w+') as outfile:
            for line in infile:
                # print(line.replace("$$mytaskname$$", task_name.capitalize()))
                outfile.write(line.replace("$$mytaskname$$", task_name.capitalize()))

        shutil.copy(temp_mako.name, weblog_mako)
        os.unlink(temp_mako.name)

        print('\n\tNow use runsetup to make the new pipeline task visible within CASA.\n')


if __name__ == '__main__':

    new_task = NewTask()
    area, task_name, module_name = new_task.parse_command_line(sys.argv)
    if not area or not task_name:
        print('At least area and task_name need to be defined.')
    else:
        new_task.create(area, task_name, module_name)
