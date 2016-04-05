import os
import collections
import shutil

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishK2JyCalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='hsdms_k2jycal.mako', 
                 description='Generate Kelvin to Jy calibration table.',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishK2JyCalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
    
    def update_mako_context(self, ctx, context, results):            
        jyperk = collections.defaultdict(lambda: collections.defaultdict(
                                         lambda: collections.defaultdict(
                                         lambda: collections.defaultdict(lambda: 'N/A (1.0)'))))
        reffile = None
        # for r in results:
        #     # rearrange jyperk factors
        #     for ms in pcontext.observing_run.measurement_sets:
        #         vis = ms.basename
        #         for ant in ms.get_antenna():
        #             ant_name = ant.name
        #             if not r.outcome['factors'].has_key(vis) or \
        #                     not r.outcome['factors'][vis].has_key(ant):
        #                 continue
        #             fs = r.outcome['factors'][vis][ant]
        #             for spw in ms.get_spectral_windows(science_windows_only=True):
        #                 spwid = spw.id
        #                 ddid = ms.get_data_description(spw=spwid)
        #                 corrs = map(ddid.get_polarization_label, range(ddid.num_polarizations))
        #                 if fs.has_key(spwid) and spw.is_target and spw.nchan > 1 and spw.nchan != 4:
        #                     fp = fs[spwid]
        #                     LOG.info('fp=%s'%(fp))
        #                     LOG.info('corrs=%s'%(corrs))
        #                     for corr in corrs:
        #                         if corr in fp.keys():
        #                             ckey = corr
        #                         elif 'I' in fp.keys() and corr in ['XX', 'YY', 'RR', 'LL']:
        #                             ckey = 'I'
        #                         else:
        #                             ckey = None
        #                         LOG.info('corr=%s ckey=%s'%(corr,ckey))
        #                         if ckey is not None:
        #                             jyperk[vis][ant][spwid][corr] = fp[ckey]
        #     reffile = r.outcome['reffile']
        reffile_copied = None
        if reffile is not None and os.path.exists(reffile):
            stage_dir = os.path.join(context.report_dir, 'stage%s'%(results.stage_number))
            LOG.debug('copying %s to %s'%(reffile, stage_dir))
            shutil.copy2(reffile, stage_dir)
            reffile_copied = os.path.join(stage_dir, os.path.basename(reffile))
        ctx.update({'jyperk': jyperk,
                    'reffile': reffile_copied})
            
