from __future__ import absolute_import

import copy
import os.path

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils


class MakeImListResult(basetask.Results):
    def __init__(self):
        super(MakeImListResult, self).__init__()
        self.targets = []
        self.clean_list_info = {}
        self._max_num_targets = 0
        self.clearlist = True
        self.mitigation_error = False
        self.synthesized_beams = None

    def add_target(self, target):
        self.targets.append(target)

    def set_info(self, info):
        self.clean_list_info = info

    def merge_with_context(self, context):
        if self.clearlist:
            context.clean_list_pending = copy.deepcopy(self.targets)
            context.clean_list_info = self.clean_list_info
        else:
            context.clean_list_pending.extend(copy.deepcopy(self.targets))
            for key, value in self.clean_list_info.iteritems():
                if context.clean_list_info.get(key, None) is not None:
                    context.clean_list_info[key] = '%s %s' % (context.clean_list_info[key], value)
                else:
                    context.clean_list_info[key] = value
        # Remove heuristics objects to avoid accumulating large amounts of unnecessary memory
        for target in self.targets:
            try:
                del target['heuristics']
            except:
                pass

        context.contfile = self.contfile
        context.linesfile = self.linesfile

        # Calculated beams for later stages
        if self.synthesized_beams is not None:
            if 'recalc' in self.synthesized_beams:
                context.synthesized_beams = copy.deepcopy(self.synthesized_beams)
                del context.synthesized_beams['recalc']
            else:
                utils.update_beams_dict(context.synthesized_beams, self.synthesized_beams)

    @property
    def num_targets(self):
        return len(self.targets)

    @property
    def max_num_targets(self):
        return self._max_num_targets

    def set_max_num_targets(self, max_num_targets):
        self._max_num_targets = max_num_targets

    def __repr__(self):
        repr = 'MakeImList:'

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
        specmode_width = len('specmode')
        start_width = len('start')
        width_width = len('width')
        nbin_width = len('nbin')
        nchan_width = len('nchan')
        uvrange_width = len('uvrange')

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
            if 'specmode' in target.keys():
                specmode_width = max(specmode_width, len(str(target['specmode'])))
            if 'start' in target.keys():
                start_width = max(start_width, len(str(target['start'])))
            if 'width' in target.keys():
                width_width = max(width_width, len(str(target['width'])))
            if 'nbin' in target.keys():
                nbin_width = max(nbin_width, len(str(target['nbin'])))
            if 'nchan' in target.keys():
                nchan_width = max(nchan_width, len(str(target['nchan'])))
            if 'uvrange' in target.keys():
                uvrange_width = max(uvrange_width, len(str(target['uvrange'])))

        field_width += 1
        intent_width += 1
        spw_width += 1
        phasecenter_width += 1
        cell_width += 1
        imsize_width += 1
        imagename_width += 1
        specmode_width += 1
        start_width += 1
        width_width += 1
        nbin_width += 1
        nchan_width += 1
        uvrange_width += 1

        repr += '\n'
        repr += '{0:{1}}'.format('field', field_width)
        repr += '{0:{1}}'.format('intent', intent_width)
        repr += '{0:{1}}'.format('spw', spw_width)
        repr += '{0:{1}}'.format('phasecenter', phasecenter_width)
        repr += '{0:{1}}'.format('cell', cell_width)
        repr += '{0:{1}}'.format('imsize', imsize_width)
        repr += '{0:{1}}'.format('imagename', imagename_width)
        if 'specmode' in target.keys():
            repr += '{0:{1}}'.format('specmode', specmode_width)
        if 'start' in target.keys():
            repr += '{0:{1}}'.format('start', start_width)
        if 'width' in target.keys():
            repr += '{0:{1}}'.format('width', width_width)
        if 'nbin' in target.keys():
            repr += '{0:{1}}'.format('nbin', nbin_width)
        if 'nchan' in target.keys():
            repr += '{0:{1}}'.format('nchan', nchan_width)
        if 'uvrange' in target.keys():
            repr += '{0:{1}}'.format('uvrange', uvrange_width)

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
            if 'specmode' in target.keys():
                repr += '{0:{1}}'.format(target['specmode'], specmode_width)
            if 'start' in target.keys():
                repr += '{0:{1}}'.format(str(target['start']), start_width)
            if 'width' in target.keys():
                repr += '{0:{1}}'.format(str(target['width']), width_width)
            if 'nbin' in target.keys():
                repr += '{0:{1}}'.format(str(target['nbin']), nbin_width)
            if 'nchan' in target.keys():
                repr += '{0:{1}}'.format(str(target['nchan']), nchan_width)
            if 'uvrange' in target.keys():
                repr += '{0:{1}}'.format(str(target['uvrange']), uvrange_width)

        return repr
