from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.contfilehandler as contfilehandler

import numpy

LOG = infrastructure.get_logger(__name__)

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
        clean_list_pending = []
        for i, target in enumerate(context.clean_list_pending):
            new_target = target
            target_ok = True
            if (target['specmode'] in ('mfs', 'cont')):
                source_name = utils.dequote(target['field'])
                spwids = target['spw']
                new_spwids = []
                new_spw_sel = 'NEW' in [self.result_cont_ranges[source_name][spwid]['status'] for spwid in spwids.split(',')]

                if (new_spw_sel):
                    spwsel = {}
                    for spwid in spwids.split(','):
                        if ((self.cont_ranges['fields'][source_name][spwid] == ['NONE']) and (target['intent'] == 'TARGET')):
                            spwsel['spw%s' % (spwid)] = ''
                            LOG.warn('No continuum frequency range information found for %s, spw %s.' % (target['field'], spwid))
                        else:
                            new_spwids.append(spwid)
                            spwsel['spw%s' % (spwid)] = ';'.join(['%s~%sGHz' % (cont_range['range'][0], cont_range['range'][1]) for cont_range in self.cont_ranges['fields'][source_name][spwid]])
                            refers = numpy.array([cont_range['refer'] for cont_range in self.cont_ranges['fields'][source_name][spwid]])
                            if ((refers == 'TOPO').all()):
                                refer = 'TOPO'
                            elif ((refers == 'LSRK').all()):
                                refer = 'LSRK'
                            else:
                                refer = 'UNDEFINED'
                            spwsel['spw%s' % (spwid)] = '%s %s' % (spwsel['spw%s' % (spwid)], refer)

                    new_spwids = ','.join(new_spwids)
                    if ((new_spwids == '') and (target['intent'] == 'TARGET')):
                        LOG.warn('No continuum selection for target %s, spw %s. Will not image this selection.' % (new_target['field'], new_target['spw']))
                        target_ok = False
                    else:
                        new_target['spw'] = new_spwids
                        new_target['spwsel'] = spwsel

            if (target_ok):
                clean_list_pending.append(new_target)

        context.clean_list_pending = clean_list_pending

    def __repr__(self):
        repr = 'FindCont:\n'
        for source_name in self.result_cont_ranges.iterkeys():
            repr += ' Source %s\n' % (source_name)
            for spwid in self.result_cont_ranges[source_name].iterkeys():
                repr += '  SpW %s:' % (spwid)
                if (self.result_cont_ranges[source_name][spwid]['cont_ranges'] == ['NONE']):
                    repr += '   Ranges: None\n'
                else:
                    repr += '   Ranges: %s' % (';'.join(['%s~%sGHz' % (cont_range['range'][0], cont_range['range'][1]) for cont_range in self.result_cont_ranges[source_name][spwid]['cont_ranges']]))
                    refers = numpy.array([cont_range['refer'] for cont_range in self.result_cont_ranges[source_name][spwid]['cont_ranges']])
                    if ((refers == 'TOPO').all()):
                        refer = 'TOPO'
                    elif ((refers == 'LSRK').all()):
                        refer = 'LSRK'
                    else:
                        refer = 'UNDEFINED'
                    repr += ' ' + refer + '\n'
                repr += '   Status: %s\n' % (self.result_cont_ranges[source_name][spwid]['status'])

        return repr
