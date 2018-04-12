"""
Example usage:

inputs = pipeline.vla.tasks.priorcals.Priorcals.Inputs(context)
task = pipeline.vla.tasks.priorcals.Priocals(inputs)
result = task.exectue(dry_run=False)
result.accept(context)

"""

from __future__ import absolute_import

import datetime
import os
import urllib2

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.vdp as vdp
from pipeline.hif.tasks.antpos import Antpos
from pipeline.hifv.tasks.gaincurves import GainCurves
from pipeline.hifv.tasks.opcal import Opcal
from pipeline.hifv.tasks.rqcal import Rqcal
from pipeline.hifv.tasks.swpowcal import Swpowcal
from pipeline.hifv.tasks.tecmaps import TecMaps
from pipeline.infrastructure import task_registry
from . import resultobjects

LOG = infrastructure.get_logger(__name__)


def correct_ant_posns (vis_name, print_offsets=False):
    """
    Given an input visibility MS name (vis_name), find the antenna
    position offsets that should be applied.  This application should
    be via the gencal task, using caltype='antpos'.

    If the print_offsets parameter is True, will print out each of
    the found offsets (or indicate that none were found), otherwise
    runs silently.

    A list is returned where the first element is the returned error
    code, the second element is a string of the antennas, and the 
    third element is a list of antenna Bx,By,Bz offsets.  An example 
    return list might look like:
    [ 0, 'ea01,ea19', [0.0184, -0.0065, 0.005, 0.0365, -0.0435, 0.0543] ]

    Usage examples:

       CASA <1>: antenna_offsets = correct_ant_posns('test.ms')
       CASA <2>: if (antenna_offsets[0] == 0):
       CASA <3>:     gencal(vis='test.ms', caltable='cal.G', \
                     caltype='antpos', antenna=antenna_offsets[1], \
                     parameter=antenna_offsets[2])

    This function does NOT work for VLA datasets, only EVLA.  If an
    attempt is made to use the function for VLA data (prior to 2010),
    an error code of 1 is returned.

    The offsets are retrieved over the internet.  A description and the
    ability to manually examine and retrieve offsets is at:
    http://www.vla.nrao.edu/astro/archive/baselines/
    If the attempt to establish the internet connection fails, an error
    code of 2 is returned.

    Uses the same algorithm that the AIPS task VLANT does.


    bjb
    nrao
    spring 2012
    """

    MONTHS = [ 'JAN', 'FEB', 'MAR', 'APR', 'MAY', 'JUN',
               'JUL', 'AUG', 'SEP', 'OCT', 'NOV', 'DEC' ]
    URL_BASE = 'http://www.vla.nrao.edu/cgi-bin/evlais_blines.cgi?Year='

    #
    # get start date+time of observation
    #
    with casatools.TableReader(vis_name+'/OBSERVATION') as table:
        # observation = tb.open(vis_name+'/OBSERVATION')
        time_range = table.getcol('TIME_RANGE')

    MJD_start_time = time_range[0][0] / 86400
    q1 = casatools.quanta.quantity(time_range[0][0],'s')
    date_time = casatools.quanta.time(q1,form='ymd')
    # date_time looks like: '2011/08/10/06:56:49'
    [obs_year,obs_month,obs_day,obs_time_string] = date_time[0].split('/')
    if (int(obs_year) < 2010):
        if (print_offsets):
            LOG.warn('Does not work for VLA observations')
        return [1, '', []]
    [obs_hour,obs_minute,obs_second] = obs_time_string.split(':')
    obs_time = 10000*int(obs_year) + 100*int(obs_month) + int(obs_day) + \
               int(obs_hour)/24.0 + int(obs_minute)/1440.0 + \
               int(obs_second)/86400.0

    #
    # get antenna to station mappings
    #
    with casatools.TableReader(vis_name+'/ANTENNA') as table:
        #observation = tb.open(vis_name+'/ANTENNA')
        ant_names = table.getcol('NAME')
        ant_stations = table.getcol('STATION')
    ant_num_stas = []
    for ii in range(len(ant_names)):
        ant_num_stas.append([int(ant_names[ii][2:]), ant_names[ii], \
                            ant_stations[ii], 0.0, 0.0, 0.0, False])

    correction_lines = []
    current_year = datetime.datetime.now().year
    # first, see if the internet connection is possible
    try:
        response = urllib2.urlopen(URL_BASE + '2010')
    except URLError, err:
        if (print_offsets):
            LOG.warn('No internet connection to antenna position correction URL {}'.format(err.reason))
        return [2, '', []]
    response.close()
    for year in range(2010,current_year+1):
        response = urllib2.urlopen(URL_BASE + str(year))
        html = response.read()
        response.close()
        html_lines = html.split('\n')

        for correction_line in html_lines:
            if len(correction_line) and correction_line[0] != '<' and correction_line[0] != ';':
                for month in MONTHS:
                    if month in correction_line:
                        correction_lines.append(str(year)+' '+correction_line)
                        break

    corrections_list = []
    for correction_line in correction_lines:
        correction_line_fields = correction_line.split()
        if (len(correction_line_fields) > 9):
            [c_year, moved_date, obs_date, put_date, put_time_str, ant, pad, Bx, By, Bz] = correction_line_fields
            s_moved = moved_date[:3]
            i_month = 1
            for month in MONTHS:
                if (moved_date.find(month) >= 0):
                    break
                i_month = i_month + 1
            moved_time = 10000 * int(c_year) + 100 * i_month + \
                         int(moved_date[3:])
        else:
            [c_year, obs_date, put_date, put_time_str, ant, pad, Bx, By, Bz] = correction_line_fields
            moved_date = '     '
            moved_time = 0
        s_obs = obs_date[:3]
        i_month = 1
        for month in MONTHS:
            if (s_obs.find(month) >= 0):
                break
            i_month = i_month + 1
        obs_time_2 = 10000 * int(c_year) + 100 * i_month + int(obs_date[3:])
        s_put = put_date[:3]
        i_month = 1
        for month in MONTHS:
            if (s_put.find(month) >= 0):
                break
            i_month = i_month + 1
        put_time = 10000 * int(c_year) + 100 * i_month + int(put_date[3:])
        [put_hr, put_min] = put_time_str.split(':')
        put_time += (int(put_hr)/24.0 + int(put_min)/1440.0)
        corrections_list.append([c_year, moved_date, moved_time, obs_date, obs_time_2, put_date, put_time, int(ant), pad, float(Bx), float(By), float(Bz)])

    for correction_list in corrections_list:
        [c_year, moved_date, moved_time, obs_date, obs_time_2, put_date, put_time, ant, pad, Bx, By, Bz] = correction_list
        ant_ind = -1
        for ii in range(len(ant_num_stas)):
            ant_num_sta = ant_num_stas[ii]
            if (ant == ant_num_sta[0]):
                ant_ind = ii
                break
        if ((ant_ind == -1) or (ant_num_sta[6])):
            # the antenna in this correction isn't in the observation, or is done,
            # so skip it
            pass
        ant_num_sta = ant_num_stas[ant_ind]
        if (moved_time):
            # the antenna moved
            if (moved_time > obs_time):
                # we are done considering this antenna
                ant_num_sta[6] = True
            else:
                # otherwise, it moved, so the offsets should be reset
                ant_num_sta[3] = 0.0
                ant_num_sta[4] = 0.0
                ant_num_sta[5] = 0.0
        if ((put_time > obs_time) and (not ant_num_sta[6]) and \
            (pad == ant_num_sta[2])):
            # it's the right antenna/pad; add the offsets to those already accumulated
            ant_num_sta[3] += Bx
            ant_num_sta[4] += By
            ant_num_sta[5] += Bz

    ants = []
    parms = []
    for ii in range(len(ant_num_stas)):
        ant_num_sta = ant_num_stas[ii]
        if ((ant_num_sta[3] != 0.0) or (ant_num_sta[4] != 0.0) or \
            (ant_num_sta[3] != 0.0)):
            if (print_offsets):
                LOG.info("offsets for antenna %4s : %8.5f  %8.5f  %8.5f" % \
                      (ant_num_sta[1], ant_num_sta[3], ant_num_sta[4], ant_num_sta[5]))
            ants.append(ant_num_sta[1])
            parms.append(ant_num_sta[3])
            parms.append(ant_num_sta[4])
            parms.append(ant_num_sta[5])
    if ((len(parms) == 0) and print_offsets):
        LOG.info("No offsets found for this MS")
    ant_string = ','.join(["%s" % ii for ii in ants])
    return [ 0, ant_string, parms ]


class PriorcalsInputs(vdp.StandardInputs):
    swpow_spw = vdp.VisDependentProperty(default='')
    tecmaps = vdp.VisDependentProperty(default=False)

    def __init__(self, context, vis=None, tecmaps=None, swpow_spw=None):
        self.context = context
        self.vis = vis
        self.tecmaps = tecmaps
        self.swpow_spw = swpow_spw

    def to_casa_args(self):
        raise NotImplementedError


@task_registry.set_equivalent_casa_task('hifv_priorcals')
class Priorcals(basetask.StandardTaskTemplate):
    Inputs = PriorcalsInputs

    def prepare(self):

        callist = []
        
        gc_result = self._do_gaincurves()
        oc_result = self._do_opcal()
        rq_result = self._do_rqcal()
        sw_result = self._do_swpowcal()
        antpos_result, antcorrect = self._do_antpos()
        tecmaps_result = None
        if self.inputs.tecmaps:
            tecmaps_result = self._do_tecmaps()
        
        #try:
        #    antpos_result.merge_withcontext(self.inputs.context)
        #except:
        #    LOG.error('No antenna position corrections.')
            
        return resultobjects.PriorcalsResults(pool=callist, gc_result=gc_result,
                                              oc_result=oc_result, rq_result=rq_result,
                                              antpos_result=antpos_result, antcorrect=antcorrect,
                                              tecmaps_result=tecmaps_result, sw_result=sw_result)

    def analyse(self, results):
        return results

    def _do_gaincurves(self):
        """Run gaincurves task"""

        inputs = GainCurves.Inputs(self.inputs.context, vis=self.inputs.vis)
        task = GainCurves(inputs)
        return self._executor.execute(task)

    def _do_opcal(self):
        """Run opcal task"""

        inputs = Opcal.Inputs(self.inputs.context, vis=self.inputs.vis)
        task = Opcal(inputs)
        return self._executor.execute(task)

    def _do_rqcal(self):
        """Run requantizer gains task"""

        inputs = Rqcal.Inputs(self.inputs.context, vis=self.inputs.vis)
        task = Rqcal(inputs)
        return self._executor.execute(task)

    def _do_swpowcal(self):
        """Run switched power task"""

        inputs = Swpowcal.Inputs(self.inputs.context, vis=self.inputs.vis, spw=self.inputs.swpow_spw)
        task = Swpowcal(inputs)
        return self._executor.execute(task)

    def _do_antpos(self):
        """Run hif_antpos to correct for antenna positions"""
        inputs = Antpos.Inputs(self.inputs.context, vis=self.inputs.vis)
        task = Antpos(inputs)
        result = self._executor.execute(task)

        antcorrect = {}
        
        try:
            antpos_caltable = result.final[0].gaintable
            if os.path.exists(antpos_caltable):
                LOG.info("Start antenna position corrections")
                antparamlist = correct_ant_posns(inputs.vis, print_offsets=False)
                LOG.info("End antenna position corrections")

                self._check_tropdelay(antpos_caltable)

                antList = antparamlist[1].split(',')
                N=3
                subList = [antparamlist[2][n:n+N] for n in range(0, len(antparamlist[2]), N)]
                antcorrect = dict(zip(antList, subList))
        except:
            LOG.info("No offsets found. No caltable created.")

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

        fracantcorrect = len(antcorrect) / len(m.antennas)
        if fracantcorrect > 0.5:
            LOG.warn("{:5.2f} percent of antennas needed position corrections.".format(100.0 * fracantcorrect))

        return result, antcorrect

    def _do_tecmaps(self):
        """Run tec_maps function"""

        inputs = TecMaps.Inputs(self.inputs.context, vis=self.inputs.vis, output_dir='')
        task = TecMaps(inputs)
        return self._executor.execute(task)

    def _check_tropdelay(self, antpos_caltable):

        # Insert value if required for testing

        '''
        #print "ADDED TEST TROP VALUE"
        trdelscale = 1.23
        tb = casatools.casac.table()
        tb.open(antpos_caltable, nomodify=False)
        tb.putkeyword('VLATrDelCorr', trdelscale)
        tb.close()
        #print "END OF ADDING TEST TROP VALUE"
        '''

        # Detect EVLA 16B Trop Del Corr
        # (Silent if required keyword absent, or has value=0.0)
        # antpostable = 'cal.antpos'
        trdelkw = 'VLATrDelCorr'
        with casatools.TableReader(antpos_caltable) as tb:
            if tb.keywordnames().count(trdelkw) == 1:
                trdelscale = tb.getkeyword(trdelkw)
                if trdelscale != 0.0:
                    warning_message = "NB: This EVLA dataset appears to fall within the period of semester 16B " \
                                      "during which the online tropospheric delay model was mis-applied. " \
                                      "A correction for the online tropospheric delay model error WILL BE APPLIED!  " \
                                      "Tropospheric delay error correction coefficient="+str(-trdelscale/1000.0)+ " (ps/m) "
                    LOG.debug("EVLA 16B Online Trop Del Corr is ON, scale=" + str(trdelscale))
                    LOG.warn(warning_message)
