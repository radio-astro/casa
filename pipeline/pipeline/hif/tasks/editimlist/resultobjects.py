from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask


class EditimlistResult(basetask.Results):
    def __init__(self):
        super(EditimlistResult, self).__init__()
        self.targets = []
        self._max_num_targets = 0

    def add_target(self, target):
        self.targets.append(target)

    def merge_with_context(self, context):
        if not hasattr(context, 'clean_list_pending'):
            context.clean_list_pending = []

        for new_target in self.targets:
            context.clean_list_pending.append(new_target)

    @property
    def num_targets(self):
        return len(self.targets)

    @property
    def max_num_targets(self):
        return self._max_num_targets

    def set_max_num_targets(self, max_num_targets):
        self._max_num_targets = max_num_targets

    def __repr__(self):
        return 'Editimlist:'
