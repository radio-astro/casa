"""
Verify a new CASA pipeline task.

The purpose of this check a newly created pipeline task.
"""

import os
import sys
import argparse

import pipeline


class NewTask():

    def __init__(self):
        self.repository_path = os.environ['SCIPIPE_HEURISTICS']

    def parse_command_line(self, argv):
        script_args = ''
        for idx, arg in enumerate(argv):
            if 'verify_new_pipeline_task.py' in arg:
                script_args = argv[idx + 1:]

        parser = argparse.ArgumentParser()
        parser.add_argument('--package', help="Pipeline package.  One of 'h', 'hif', 'hifa', 'hifa', 'hifv', or 'hsd'.",
                            type=str, choices=['h', 'hif', 'hifa', 'hifa', 'hifv', 'hsd'], required=True)
        parser.add_argument('--task', help='New task name', type=str, required=True)
        parser.add_argument('--test-ms', dest='vis', help='Test Measurement Set', type=str, required=True)

        try:
            args = parser.parse_args(script_args)
        except:
            print('Problem with new_pipeline_task arguments.')
            return '', '', ''

        area = args.package
        task_name = args.task
        vis = args.vis
        return area, task_name, vis

    def verify(self, area, task_name, vis):
        # ---------------------------
        #  prepare to check the task
        # ---------------------------
        h_init()
        h_save()
        hifv_importdata(vis=[vis], session=['session_1'], overwrite=False)
        h_save()

        context = pipeline.Pipeline(context='last').context

        # ---------------------------
        #  execute the task
        # ---------------------------
        inputs = eval('pipeline.{area}.tasks.{task}.{captask}.Inputs(context)'.format(area=area, task=task_name, captask=task_name.capitalize()))
        task = eval('pipeline.{area}.tasks.{task}.{captask}(inputs)'.format(area=area, task=task_name, captask=task_name.capitalize()))
        result = task.execute(dry_run=False)
        result.accept(context)
        context.save()

        # --------------------------------
        #   run as a registered CASA task
        # --------------------------------
        try:
            eval('{area}_{task}()'.format(area=area, task=task_name))
            h_save()
        except NameError as ee:
            print('ERROR: {msg}'.format(msg=ee.message))
            print('\tTry using runsetup to register the new task with CASA first.')

if __name__ == '__main__':

    new_task = NewTask()
    area, task_name, vis = new_task.parse_command_line(sys.argv)
    if not area or not task_name or not vis:
        print('area, task_name and test-ms all need to be defined')
    else:
        new_task.verify(area, task_name, vis)
