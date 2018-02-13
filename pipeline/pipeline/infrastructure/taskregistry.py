import textwrap


class TaskMapping(object):
    def __init__(self, casa_task, pipeline_class, comment):
        self.casa_task = casa_task
        self.pipeline_class = pipeline_class
        self.comment = comment

    def __repr__(self):
        return '<TaskMapping({!r}, {!r}, {!r})>'.format(self.casa_task, self.pipeline_class, self.comment)


class TaskRegistry(object):
    # The object used to map pipeline classes to CASA tasks
    task_mapping_class = TaskMapping

    def __init__(self):
        self.task_map = []

    def get_casa_task(self, cls):
        """
        Get the CASA task for a pipeline class.

        Raises KeyError if no mapping is found.

        :param cls: pipeline class
        :return: name of CASA task as a string
        """
        matching = [c.casa_task for c in self.task_map if c.pipeline_class == cls]
        if not matching:
            raise KeyError('No task registered for {!s}'.format(cls.__name__))
        return matching[0]

    def get_comment(self, cls, format=True):
        """
        Get the casa_commands.log entry for a pipeline class.

        :param cls: the pipeline class to get a comment for
        :param format: True if comment should be wrapped
        :return: comment as a string
        """
        matching = [c for c in self.task_map if c.pipeline_class == cls]
        if not matching:
            raise KeyError('No comment registered for {!s}'.format(cls.__name__))

        match = matching[0]
        comment = match.comment
        if comment is None:
            comment = 'No comment registered for {!s}'.format(match.casa_task)

        if not format:
            return comment

        wrapped = textwrap.wrap('# ' + comment, subsequent_indent='# ', width=78, break_long_words=False)
        return '%s\n#\n' % '\n'.join(wrapped)

    def get_pipeline_class(self, name):
        """
        Get a pipeline class by name.

        Raises KeyError if no mapping is found.

        :param name: name of the pipeline class as a string
        :return: pipeline class
        """
        matching = [c.pipeline_class for c in self.task_map if c.pipeline_class.__name__ == name]
        if not matching:
            raise KeyError('No pipeline class registered for {!s}'.format(name))
        return matching[0]

    def get_pipeline_class_for_task(self, task_name):
        """
        Get the pipeline class used to execute a CASA task.

        Raises KeyError if no mapping is found.

        :param task_name: name of CASA task
        :return: pipeline class
        """
        matching = [c.pipeline_class for c in self.task_map if c.casa_task == task_name]
        if not matching:
            raise KeyError('No pipeline class registered for {!s}'.format(task_name))
        return matching[0]

    def set_casa_commands_comment(self, comment):
        """A decorator that is used to register the descriptive text that
        preceeds the list of CASA commands invoked by this task, as seen in
        casa_commands.log

        :param comment: comment to add to casa_commands.log as a string
        """
        def decorator(pipeline_class):
            self.set_comment_for_class(pipeline_class, comment)
            return pipeline_class

        return decorator

    def set_comment_for_class(self, pipeline_class, comment):
        """
        Set the comment for a task mapping.

        :param pipeline_class: pipeline class to map
        :param comment: comment to set as a string
        """
        existing = [o for o in self.task_map if o.pipeline_class == pipeline_class]
        to_add = []

        if not existing:
            mapping = self.task_mapping_class(None, pipeline_class, comment)
            to_add.append(mapping)

        for o in existing:
            mapping = self.task_mapping_class(o.casa_task, pipeline_class, comment)
            to_add.append(mapping)

        for o in existing:
            self.task_map.remove(o)
        for o in to_add:
            self.task_map.append(o)

    def set_equivalent_casa_task(self, cli_task):
        """A decorator that is used to register the mapping between a pipeline
        class and its identity in the CASA command line interface.

        :param cli_task: the CASA task name as a string
        """
        def decorator(f):
            self.set_task_identifier(cli_task, f)
            return f

        return decorator

    def set_task_identifier(self, casa_task, pipeline_class):
        """
        Set the CASA task for a task mapping.

        :param comment: casa task name as a string
        :param pipeline_class: pipeline class to map
        """
        # assume 1:1 mapping between pipeline class and CASA tasks
        existing = [o for o in self.task_map if o.pipeline_class == pipeline_class]
        to_add = []

        if not existing:
            mapping = self.task_mapping_class(casa_task, pipeline_class, None)
            to_add.append(mapping)

        for o in existing:
            mapping = self.task_mapping_class(casa_task, pipeline_class, o.comment)
            to_add.append(mapping)

        for o in existing:
            self.task_map.remove(o)
        for o in to_add:
            self.task_map.append(o)


task_registry = TaskRegistry()
