import os
import collections
import shutil

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

JyperKTR = collections.namedtuple('JyperKTR', 'msname spw antenna pol factor')

class T2_4MDetailsSingleDishK2JyCalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='hsd_k2jycal.mako', 
                 description='Generate Kelvin to Jy calibration table.',
                 always_rerender=True):
        super(T2_4MDetailsSingleDishK2JyCalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
    
    def update_mako_context(self, ctx, context, results):            
        jyperk = collections.defaultdict(lambda: collections.defaultdict(
                                         lambda: collections.defaultdict(
                                         lambda: collections.defaultdict(lambda: 'N/A (1.0)'))))
        reffile = None
        row_values = []
        for r in results:
            # rearrange jyperk factors
            for ms in context.observing_run.measurement_sets:
                vis = ms.basename
                for spw in ms.get_spectral_windows(science_windows_only=True):
                    spwid = spw.id
                    ddid = ms.get_data_description(spw=spwid)
                    for ant in ms.get_antenna():
                        ant_name = ant.name
                        corrs = map(ddid.get_polarization_label, range(ddid.num_polarizations))
#                         # an attempt to collapse pol rows
#                         # corr_collector[factor] = [corr0, corr1, ...]
#                         corr_collector = collections.defaultdict(lambda: [])
                        for corr in corrs:
                            factor = self.__get_factor(r.factors, vis, spwid, ant_name, corr)
#                             corr_collector[factor].append(corr)
#                         for factor, corrlist in corr_collector.items():
#                             corr = str(', ').join(corrlist)
                            if factor is not None:
                                jyperk[vis][spwid][ant_name][corr] = factor
                            tr = JyperKTR(vis, spwid, ant_name, corr,
                                          jyperk[vis][spwid][ant_name][corr])
                            row_values.append(tr)
            reffile = r.reffile
        reffile_copied = None
        if reffile is not None and os.path.exists(reffile):
            stage_dir = os.path.join(context.report_dir, 'stage%s'%(results.stage_number))
            LOG.debug('copying %s to %s'%(reffile, stage_dir))
            shutil.copy2(reffile, stage_dir)
            reffile_copied = os.path.join(stage_dir, os.path.basename(reffile))
        ctx.update({'jyperk_rows': utils.merge_td_columns(row_values),
                    'reffile': reffile_copied})

    def __get_factor(self, factor_dict, vis, spwid, ant_name, pol_name):
        '''
        Returns a factor correxponding to vis, spwid, ant_name, and pol_name from
        a factor_dict[vis][spwid][ant_name][pol_name] = factor
        If factor_dict lack corresponding factor, the method returns None.
        '''
        if not factor_dict.has_key(vis) or not factor_dict[vis].has_key(spwid) \
            or not factor_dict[vis][spwid].has_key(ant_name) \
            or not factor_dict[vis][spwid][ant_name].has_key(pol_name):
            return None
        return factor_dict[vis][spwid][ant_name][pol_name]
    