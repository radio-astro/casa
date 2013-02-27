from __future__ import absolute_import

import copy
import os.path

import pipeline.infrastructure.api as api


class MakeCleanListResult(api.Results):
    def __init__(self):
        self.targets = []

    def add_target(self, target):
        self.targets.append(target)

    def merge_with_context(self, context):
        context.clean_list_pending = copy.deepcopy(self.targets)

    def __repr__(self):
        repr = 'MakeCleanList:'

        if len(self.targets) == 0:
            repr += '\n is empty'
            return repr

        field_width = len('field')
        intent_width = len('intent')
        spw_width = len('spw')
        phasecenter_width = len('phasecenter')
        cell_width = len('cell')
        imsize_width = len('imsize')
        imagename_width = len('imagename')
        mode_width = len('mode')
        for target in self.targets:
            field_width = max(field_width, len(target['field']))
            intent_width = max(intent_width, len(target['intent']))
            spw_width = max(spw_width, len(target['spw']))
            phasecenter_width = max(phasecenter_width,
              len(target['phasecenter']))
            cell_width = max(cell_width, len(str(target['cell'])))
            imsize_width = max(imsize_width, len(str(target['imsize'])))
            imagename = os.path.basename(target['imagename'])
            imagename_width = max(imagename_width, len(imagename))
            if 'mode' in target.keys():
                mode_width = max(mode_width, len(target['mode']))

        field_width += 1
        intent_width += 1
        spw_width += 1
        phasecenter_width += 1
        cell_width += 1
        imsize_width += 1
        imagename_width += 1
        mode_width += 1

        repr += '\n'
        repr += '{0:{1}}'.format('field', field_width)
        repr += '{0:{1}}'.format('intent', intent_width)
        repr += '{0:{1}}'.format('spw', spw_width)
        repr += '{0:{1}}'.format('phasecenter', phasecenter_width)
        repr += '{0:{1}}'.format('cell', cell_width)
        repr += '{0:{1}}'.format('imsize', imsize_width)
        repr += '{0:{1}}'.format('imagename', imagename_width)
        if 'mode' in target.keys():
            repr += '{0:{1}}'.format('mode', mode_width)

        for target in self.targets:
            repr += '\n'
            repr += '{0:{1}}'.format(target['field'], field_width)
            repr += '{0:{1}}'.format(target['intent'], intent_width)
            repr += '{0:{1}}'.format(target['spw'], spw_width)
            repr += '{0:{1}}'.format(target['phasecenter'],
              phasecenter_width)
            repr += '{0:{1}}'.format(target['cell'], cell_width)
            repr += '{0:{1}}'.format(target['imsize'], imsize_width)
            repr += '{0:{1}}'.format(os.path.basename(target['imagename']),
              imagename_width)
            if 'mode' in target.keys():
                repr += '{0:{1}}'.format(target['mode'], mode_width)

        return repr
