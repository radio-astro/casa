from __future__ import absolute_import

import os

from numpy import sqrt

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class SDK2JyCalWorkerInputs(vdp.StandardInputs):
    caltype = vdp.VisDependentProperty(default='amp', readonly=True)
    
    def __init__(self, context, output_dir, vis, caltable, factors):
        super(self.__class__, self).__init__()
        
        self.context = context
        self.vis = vis
        self.output_dir = output_dir
        self.caltable = caltable
        self.factors = factors


    # Convert to CASA gencal task arguments.
    def to_casa_args(self):
        return {'vis': self.vis,
                'caltable': self.caltable,
                'caltype': self.caltype}


class SDK2JyCalWorkerResults(basetask.Results):
    def __init__(self, vis, calapp=None, factors={}):
        super(SDK2JyCalWorkerResults, self).__init__()
        self.calapp = calapp
        self.vis = vis
        self.ms_factors = factors
        self.factors_ok = (calapp is not None)

    def merge_with_context(self, context):
        if not os.path.exists(self.vis):
            LOG.error("Invalid MS name passed to Result class")
        if self.calapp is None:
            LOG.error("caltable generation failed")


class SDK2JyCalWorker(basetask.StandardTaskTemplate):
    """
    Per MS caltable creation
    """
    Inputs = SDK2JyCalWorkerInputs    

    def prepare(self):
        inputs = self.inputs
        vis = inputs.vis
        factors = inputs.factors

        polmap = {'XX': 'X', 'YY': 'Y', 'I': ''}

        if not os.path.exists(vis):
            LOG.error("Could not find MS '%s'" % vis)
            return SDK2JyCalWorkerResults(os.path.basename(vis))
        vis = os.path.basename(vis)
        if vis not in factors:
            return SDK2JyCalWorkerResults(vis)

        # make a note of the current inputs state before we start fiddling
        # with it. This origin will be attached to the final CalApplication.
        origin = callibrary.CalAppOrigin(task=SDK2JyCalWorker,
                                         inputs=inputs.to_casa_args())
        common_params = inputs.to_casa_args()
        factors_for_ms = factors[vis]
        factors_used = {}
        for spw, spw_factor in factors_for_ms.iteritems():
            factors_used[spw] = {}
            for ant, ant_factor in spw_factor.iteritems():
                factors_used[spw][ant] = {}
                # map polarization
                pol_list = ant_factor.keys()
                pols = str(',').join(map(polmap.get, pol_list))
                for pol in pol_list:
                    factors_used[spw][ant][pol] = ant_factor[pol]
                    LOG.debug("%s, SPW %d, %s, %s: factor=%f" %
                              (vis, spw, ant, pol, factors_used[spw][ant][pol]))
                # handle anonymous antenna
                ant_sel = '' if ant.upper() == 'ANONYMOUS' else ant
                gain_factor = [1./sqrt(ant_factor[pol]) for pol in pol_list]
                gencal_args = dict(spw=str(spw), antenna=ant_sel, pol=pols,
                                   parameter=gain_factor)
                gencal_args.update(common_params)
                gencal_job = casa_tasks.gencal(**gencal_args)
                self._executor.execute(gencal_job)
        # generate callibrary for the caltable
        calto = callibrary.CalTo(vis=common_params['vis'])
        calfrom = callibrary.CalFrom(common_params['caltable'], 
                                     caltype=inputs.caltype,
                                     gainfield='', spwmap=None,
                                     interp='nearest,nearest')
        calapp = callibrary.CalApplication(calto, calfrom, origin)
        return SDK2JyCalWorkerResults(vis, calapp=calapp, factors=factors_used)

    def analyse(self, result):
        """
        Define factors actually used and analyze if the factors are provided to all relevant data in MS.
        """
        name = result.vis
        if len(result.ms_factors) == 0:
            result.factors_ok = False
            LOG.warn("No Jy/K factor is given for MS '%s'" % name)
            return result

        ms = self.inputs.context.observing_run.get_ms(name)
        pol_to_map_i = ('XX', 'YY', 'RR', 'LL', 'I')
        for spw in ms.get_spectral_windows(science_windows_only=True):
            spwid = spw.id
            if spwid not in result.ms_factors:
                result.factors_ok = False
                LOG.warn("No Jy/K factor is given for Spw=%d of %s" % (spwid, name))
                continue
            ddid = ms.get_data_description(spw=spwid)
            pol_list = map(ddid.get_polarization_label,
                           range(ddid.num_polarizations))
            # mapping for anonymous antenna if necessary
            is_anonymous_ant = 'ANONYMOUS' in result.ms_factors[spwid]
            all_ant_factor = result.ms_factors[spwid].pop('ANONYMOUS') if is_anonymous_ant else {}
            for ant in ms.get_antenna():
                ant_name = ant.name
                if is_anonymous_ant:
                    result.ms_factors[spwid][ant_name] = all_ant_factor
                elif ant_name not in result.ms_factors[spwid]:
                    result.factors_ok = False
                    LOG.warn("No Jy/K factor is given for Spw=%d, Ant=%s of %s" % (spwid, ant_name, name))
                    continue
                is_anonymous_pol = 'I' in result.ms_factors[spwid][ant_name]
                all_pol_factor = result.ms_factors[spwid][ant_name].pop('I') if is_anonymous_pol else {}
                for pol in pol_list:
                    # mapping for stokes I if necessary
                    if is_anonymous_pol and pol in pol_to_map_i:
                        result.ms_factors[spwid][ant_name][pol] = all_pol_factor
                    # check factors provided for all spw, antenna, and pol
                    ok = self.__check_factor(result.ms_factors, spwid, ant_name, pol)
                    result.factors_ok &= ok
                    if not ok:
                        LOG.warn("No Jy/K factor is given for Spw=%d, Ant=%s, Pol=%s of %s" %
                                 (spwid, ant_name, pol, name))
        # LOG information
        sep = "*"*40
        LOG.info("")
        LOG.info(sep)
        LOG.info("Summary of Jy/K factors of %s" % name)
        LOG.info(sep)
        for spw, facs in result.ms_factors.iteritems():
            for ant, faca in facs.iteritems():
                for pol, facp in faca.iteritems():
                    LOG.info("SPW %d, %s, %s: %f" % (spw, ant, pol, facp))
        return result

    @staticmethod
    def __check_factor(factors, spw, ant, pol):
        if spw not in factors or factors[spw] is None:
            return False
        if ant not in factors[spw] or factors[spw][ant] is None:
            return False
        if pol not in factors[spw][ant] or factors[spw][ant][pol] is None:
            return False
        return True
