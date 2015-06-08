import os
import collections

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishApplycalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    #FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')

    def __init__(self, template='hsd_applycal.mako', 
                 description='Apply calibration tables',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishApplycalRenderer, self).__init__(template,
                                                                     description,
                                                                     always_rerender)

    def get_display_context(self, context, result):
        super_cls = super(T2_4MDetailsSingleDishApplycalRenderer, self)
        ctx = super_cls.get_display_context(context, result)

        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % result.stage_number)

        calapps = {}
        for r in result:
            calapps = utils.dict_merge(calapps,
                                       self.calapps_for_result(r))

        caltypes = {}
        for r in result:
            caltypes = utils.dict_merge(caltypes,
                                        self.caltypes_for_result(r))

        # return all agents so we get ticks and crosses against each one
        agents = ['before', 'applycal']

        ctx.update({'calapps'  : calapps,
                    'caltypes' : caltypes,
                    'agents'   : agents,
                    'dirname'  : weblog_dir})
        return ctx

    def calapps_for_result(self, result):
        calapps = collections.defaultdict(list)
        #for calapp in result.applied:
        #for calapp in result.outcome:
        calapplist = result.outcome
        for calapp in calapplist:
            infile = os.path.basename(calapp.infile)
            calapps[infile].append(calapp)
        return calapps

    def caltypes_for_result(self, result):
        type_map = {
            'ps' : 'Sky',
            'otf' : 'Sky',
            'otfraster'  : 'Sky',
            'tsys'     : 'T<sub>sys</sub>',
        }
        
        d = {}
        #for calapp in result.applied:
        calapplist = result.outcome
        for calapp in calapplist:
            for calfrom in calapp.calfrom:
                caltype = type_map.get(calfrom.caltype, calfrom.caltype)
                
                caltype += self.get_gain_solution_type(calfrom)
                
                d[calfrom.gaintable] = caltype

        return d
                
    def get_gain_solution_type(self, calfrom):
        if calfrom.caltype != 'tsys':
            return ' (%s)'%(calfrom.caltype)
        return ''

