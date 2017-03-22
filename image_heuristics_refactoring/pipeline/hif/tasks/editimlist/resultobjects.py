from __future__ import absolute_import
import copy

import pipeline.infrastructure.basetask as basetask


class EditimlistResult(basetask.Results):
    def __init__(self):
        super(EditimlistResult, self).__init__()
        self.targets = []
        self._max_num_targets = 0

    def add_target(self, target):
        self.targets.append(target)

    def merge_with_context(self, context):
        new_targets = self.targets

        new_targets_names = [x['field'] for x in self.targets]
        # new_targets_keys = [(targ['phasecenter', targ['imsize'], targ['cell']) for targ in self.targets]
        # print 'new targets', new_targets_names

        if not hasattr(context, 'clean_list_pending'):
            context.clean_list_pending = []

        existing_targets = context.clean_list_pending
        existing_targets_names = [x['field'] for x in context.clean_list_pending]
        # print 'existing targets', existing_targets_names

        for new_target in new_targets:
            if new_target['field'] not in existing_targets_names:
                context.clean_list_pending.append(new_target)
            else:
                indices = [i for i, x in enumerate(existing_targets) if x['field'] == new_target['field']]
                if len(indices) > 1:
                    # print('Multiple targets with the same field name')
                    # print('Replacing the first occurrence')
                    context.clean_list_pending[indices[0]] = new_target
                if len(indices) == 1:
                    # print('Replacing ' + new_target['field'] + ' with ' + existing_targets[indices[0]]['field'])
                    tmp_new_target = copy.deepcopy(context.clean_list_pending[indices[0]])
                    for k, v in new_target.iteritems():
                        # print k, '=', v
                        tmp_new_target[k] = v
                    context.clean_list_pending[indices[0]] = copy.deepcopy(tmp_new_target)
                    self.targets[0] = copy.deepcopy(context.clean_list_pending[indices[0]])

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
