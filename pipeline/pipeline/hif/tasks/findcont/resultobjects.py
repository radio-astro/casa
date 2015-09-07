from __future__ import absolute_import

import os.path

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.contfilehandler as contfilehandler


class FindContResult(basetask.Results):
    def __init__(self, result_cont_ranges, cont_ranges, num_found, num_total):
        super(FindContResult, self).__init__()
        self.result_cont_ranges = result_cont_ranges
        self.cont_ranges = cont_ranges
        self.num_found = num_found
        self.num_total = num_total
        self.plot_path = None

    def merge_with_context(self, context):
        # write the new ranges to the continuum file
        contfile_handler = contfilehandler.ContFileHandler(context.contfile)
        contfile_handler.write(self.cont_ranges)

        # Store new selection for subsequent mfs or cont imaging step.
        # NOTE: This only works for the default setup, not for a user supplied list.
        # TODO: Catch user supplied list case.
        for i, target in enumerate(context.clean_list_pending):
            if (target['specmode'] in ('mfs', 'cont')):
                source_name = target['field']
                spwids = target['spw']
                new_spw_sel = 'NEW' in [self.result_cont_ranges[source_name][spwid]['status'] for spwid in spwids.split(',')]

                if (new_spw_sel):
                    spwsel = {}
                    for spwid in spwids.split(','):
                        if (self.cont_ranges[source_name][spwid] in [['NONE'], ['']]):
                            spwsel['spw%s' % (spwid)] = ''
                            LOG.warn('No continuum frequency range information available for %s, spw %s.' % (target, spwid))
                        else:
                            spwsel['spw%s' % (spwid)] = ';'.join(['%s~%sGHz' % (cont_range[0], cont_range[1]) for cont_range in self.cont_ranges[source_name][spwid]])
                    context.clean_list_pending[i]['spwsel'] = spwsel

    def __repr__(self):
        repr = 'FindCont:\n'
        for source_name in self.result_cont_ranges.iterkeys():
            repr += ' Source %s\n' % (source_name)
            for spwid in self.result_cont_ranges[source_name].iterkeys():
                repr += '  SpW %s:\n' % (spwid)
                if (self.result_cont_ranges[source_name][spwid]['cont_ranges'] == ['NONE']):
                    repr += '   Ranges: None'
                else:
                    repr += '   Ranges: %s\n' % (';'.join(['%s~%sGHz' % (cont_range[0], cont_range[1]) for cont_range in self.result_cont_ranges[source_name][spwid]['cont_ranges']]))
                repr += '   Status: %s\n' % (self.result_cont_ranges[source_name][spwid]['status'])

        return repr
