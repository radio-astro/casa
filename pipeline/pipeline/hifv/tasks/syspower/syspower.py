from __future__ import absolute_import
import os
import shutil
import numpy as np
from glob import glob
from copy import deepcopy
from math import factorial
import json
import re

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks, task_registry

LOG = infrastructure.get_logger(__name__)


# old
#  scipy.signal.savgol_filter(x, window_length, polyorder, deriv=0, delta=1.0, axis=-1, mode='interp', cval=0.0)

# http://scipy-cookbook.readthedocs.io/items/SavitzkyGolay.html
def savitzky_golay(y, window_size, order, deriv=0, rate=1):
    r"""Smooth (and optionally differentiate) data with a Savitzky-Golay filter.
    The Savitzky-Golay filter removes high frequency noise from data.
    It has the advantage of preserving the original shape and
    features of the signal better than other types of filtering
    approaches, such as moving averages techniques.
    Parameters
    ----------
    y : array_like, shape (N,)
        the values of the time history of the signal.
    window_size : int
        the length of the window. Must be an odd integer number.
    order : int
        the order of the polynomial used in the filtering.
        Must be less then `window_size` - 1.
    deriv: int
        the order of the derivative to compute (default = 0 means only smoothing)
    Returns
    -------
    ys : ndarray, shape (N)
        the smoothed signal (or it's n-th derivative).
    Notes
    -----
    The Savitzky-Golay is a type of low-pass filter, particularly
    suited for smoothing noisy data. The main idea behind this
    approach is to make for each point a least-square fit with a
    polynomial of high order over a odd-sized window centered at
    the point.
    Examples
    --------
    t = np.linspace(-4, 4, 500)
    y = np.exp( -t**2 ) + np.random.normal(0, 0.05, t.shape)
    ysg = savitzky_golay(y, window_size=31, order=4)
    import matplotlib.pyplot as plt
    plt.plot(t, y, label='Noisy signal')
    plt.plot(t, np.exp(-t**2), 'k', lw=1.5, label='Original signal')
    plt.plot(t, ysg, 'r', label='Filtered signal')
    plt.legend()
    plt.show()
    References
    ----------
    .. [1] A. Savitzky, M. J. E. Golay, Smoothing and Differentiation of
       Data by Simplified Least Squares Procedures. Analytical
       Chemistry, 1964, 36 (8), pp 1627-1639.
    .. [2] Numerical Recipes 3rd Edition: The Art of Scientific Computing
       W.H. Press, S.A. Teukolsky, W.T. Vetterling, B.P. Flannery
       Cambridge University Press ISBN-13: 9780521880688
    """

    try:
        window_size = np.abs(np.int(window_size))
        order = np.abs(np.int(order))
    except ValueError, msg:
        raise ValueError("window_size and order have to be of type int")
    if window_size % 2 != 1 or window_size < 1:
        raise TypeError("window_size size must be a positive odd number")
    if window_size < order + 2:
        raise TypeError("window_size is too small for the polynomials order")
    order_range = range(order+1)
    half_window = (window_size -1) // 2
    # precompute coefficients
    b = np.mat([[k**i for i in order_range] for k in range(-half_window, half_window+1)])
    m = np.linalg.pinv(b).A[deriv] * rate**deriv * factorial(deriv)
    # pad the signal at the extremes with
    # values taken from the signal itself
    firstvals = y[0] - np.abs( y[1:half_window+1][::-1] - y[0] )
    lastvals = y[-1] + np.abs(y[-half_window-1:-1][::-1] - y[-1])
    y = np.concatenate((firstvals, y, lastvals))

    return np.convolve( m[::-1], y, mode='valid')


class SyspowerResults(basetask.Results):
    def __init__(self, gaintable=None, spowerdict=None, dat_common=None,
                 clip_sp_template=None, template_table=None):

        if gaintable is None:
            gaintable = ''
        if spowerdict is None:
            spowerdict = {}
        if dat_common is None:
            dat_common = np.array([])
        if clip_sp_template is None:
            clip_sp_template = []
        if template_table is None:
            template_table = ''

        super(SyspowerResults, self).__init__()

        self.pipeline_casa_task = 'Syspower'
        self.gaintable = gaintable
        self.spowerdict = spowerdict
        self.dat_common = dat_common
        self.clip_sp_template = clip_sp_template
        self.template_table = template_table

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        return

    def __repr__(self):
        # return 'SyspowerResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'SyspowerResults:'


class SyspowerInputs(vdp.StandardInputs):

    @vdp.VisDependentProperty
    def clip_sp_template(self):
        return [0.7, 1.2]

    def __init__(self, context, vis=None, clip_sp_template=None):
        self.context = context
        self.vis = vis
        self.clip_sp_template = clip_sp_template


@task_registry.set_equivalent_casa_task('hifv_syspower')
@task_registry.set_casa_commands_comment('Sys power fix compression')
class Syspower(basetask.StandardTaskTemplate):
    Inputs = SyspowerInputs

    def prepare(self):
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)

        # flag normalized p_diff outside this range
        clip_sp_template = self.inputs.clip_sp_template

        try:
            rq_table = self.inputs.context.results[4].read()[0].rq_result[0].final[0].gaintable
        except Exception as ex:
            rq_table = self.inputs.context.results[4].read()[0].rq_result.final[0].gaintable
            LOG.debug(ex)

        template_table = 'pdiff.tbl'

        fields = m.get_fields(intent='AMPLITUDE')
        field = fields[0]
        flux_field = field.id
        flux_times = field.time
        antenna_ids = np.array([a.id for a in m.antennas])
        antenna_names = [a.name for a in m.antennas]
        spws = [spw.id for spw in m.get_spectral_windows(science_windows_only=True)]
        LOG.info("Using flux field: {0}  (ID: {1})".format(field.name, flux_field))

        # get switched power from MS
        with casatools.TableReader(self.inputs.vis + '/SYSPOWER') as tb:
            stb = tb.query('SPECTRAL_WINDOW_ID > '+str(min(spws)-1))  # VLASS specific?
            sp_time = stb.getcol('TIME')
            sp_ant = stb.getcol('ANTENNA_ID')
            sp_spw = stb.getcol('SPECTRAL_WINDOW_ID')
            p_diff = stb.getcol('SWITCHED_DIFF')
            rq = stb.getcol('REQUANTIZER_GAIN')
            stb.done()

        # setup arrays
        sorted_time      = np.unique(sp_time)
        dat_raw          = np.zeros((len(antenna_ids), len(spws), 2, len(sorted_time)))
        dat_rq           = np.zeros((len(antenna_ids), len(spws), 2, len(sorted_time)))
        dat_flux         = np.zeros((len(antenna_ids), len(spws), 2))
        dat_scaled       = np.zeros((len(antenna_ids), len(spws), 2, len(sorted_time)))
        dat_filtered     = np.zeros((len(antenna_ids), len(spws), 2, len(sorted_time)))
        dat_common       = np.ma.zeros((len(antenna_ids), 2, 2, len(sorted_time)))
        dat_online_flags = np.zeros((len(antenna_ids), len(sorted_time)), dtype='bool')
        dat_sum          = np.zeros((len(antenna_ids), len(spws), 2, len(sorted_time)))
        dat_sum_flux     = np.zeros((len(antenna_ids), len(spws), 2, len(sorted_time)))

        # Obtain online flagging commands from flagdata result
        flagresult = self.inputs.context.results[2]
        result = flagresult.read()
        result = result[0]
        onlineflagslist = result._flagcmds

        # get online flags from .flagonline.txt
        flag_file_name = self.inputs.vis.replace('.ms', '.flagonline.txt')
        if os.path.isfile(flag_file_name):
            with open(flag_file_name, 'r') as flag_file:
                for line in flag_file:
                    try:
                        r = re.search("antenna='ea(\d*)&&\*' timerange='(.*)' reason", line)
                    except:
                        r = False
                    if r:
                        this_ant = 'ea' + r.groups()[0]
                        start_time = r.groups()[1].split('~')[0]
                        end_time = r.groups()[1].split('~')[1]
                        start_time_sec = casatools.quanta.convert(casatools.quanta.quantity(start_time), 's')['value']
                        end_time_sec = casatools.quanta.convert(casatools.quanta.quantity(end_time), 's')['value']
                        indices_to_flag = np.where((sorted_time >= start_time_sec) & (sorted_time <= end_time_sec))[0]
                        dat_online_flags[antenna_names.index(this_ant), indices_to_flag] = True

        # remove requantizer changes from p_diff
        pdrq = p_diff / (rq ** 2)

        # read tables into arrays
        for i, this_ant in enumerate(antenna_ids):
            LOG.info('reading antenna {0}'.format(this_ant))
            for j, this_spw in enumerate(spws):
                hits = np.where((sp_ant == this_ant) & (sp_spw == this_spw))[0]
                times = sp_time[hits]
                hits2 = np.where(np.in1d(sorted_time, times))[0]
                flux_hits = np.where((times >= np.min(flux_times)) & (times <= np.max(flux_times)))[0]

                for pol in [0, 1]:
                    LOG.debug(str(i) + ' ' + str(j) + ' ' + str(pol) + ' ' + str(hits2))
                    dat_raw[i, j, pol, hits2] = p_diff[pol, hits]
                    dat_flux[i, j, pol] = np.median(pdrq[pol, hits][flux_hits])
                    dat_rq[i, j, pol, hits2] = rq[pol, hits]
                    dat_scaled[i, j, pol, hits2] = pdrq[pol, hits] / dat_flux[i, j, pol]
                    dat_filtered[i, j, pol, hits2] = deepcopy(dat_scaled[i, j, pol, hits2])

        # common baseband template
        for i, this_ant in enumerate(antenna_ids):
            LOG.info('Creating template for antenna {0}'.format(antenna_names[this_ant]))
            for bband in [0,1]:
                common_indices = range(0, 8) if bband == 0 else range(8, 16)
                for pol in [0, 1]:
                    LOG.info('  processing baseband {0},  polarization {1}'.format(bband,pol))

                    # create initial template
                    sp_data = dat_filtered[i, common_indices, pol, :]
                    sp_data = np.ma.array(sp_data)
                    sp_data.mask = np.ma.getmaskarray(sp_data)
                    sp_data.mask = dat_online_flags[i]

                    sp_data, flag_percent = self.flag_with_medfilt(sp_data, sp_data, flag_median=True,
                                                                   k=9, threshold=8,do_shift=True)
                    LOG.info('    total flagged data: {0:.2f}% in first pass'.format(flag_percent))

                    sp_data, flag_percent = self.flag_with_medfilt(sp_data, sp_data, flag_rms=True,
                                                                   k=5, threshold=8, do_shift=True)
                    LOG.info('    total flagged data: {0:.2f}% in second pass'.format(flag_percent))

                    sp_template = np.ma.median(sp_data, axis=0)

                    # flag residuals and recalculate template
                    sp_data, flag_percent = self.flag_with_medfilt(sp_data, sp_template, flag_median=True,
                                                                   k=11, threshold=7, do_shift=False)
                    LOG.info('    total flagged data: {0:.2f}% in third pass'.format(flag_percent))

                    sp_data, flag_percent = self.flag_with_medfilt(sp_data, sp_template, flag_rms=True,
                                                                   k=5, threshold=7, do_shift=False)
                    LOG.info('    total flagged data: {0:.2f}% in fourth pass'.format(flag_percent))

                    sp_median_data = np.ma.median(sp_data, axis=0)
                    sp_median_mask = deepcopy(sp_median_data.mask)
                    # scipy.signal.savgol_filter(x, window_length, polyorder, deriv=0, delta=1.0, axis=-1, mode='interp', cval=0.0) # OLD
                    # savitzky_golay(y, window_size, order, deriv=0, rate=1):  NEW
                    # sp_template = savgol_filter(self.interp_with_medfilt(sp_median_data), 7, 3)
                    sp_template = savitzky_golay(self.interp_with_medfilt(sp_median_data), 7, 3)
                    sp_template = np.ma.array(sp_template)
                    sp_template.mask = np.ma.getmaskarray(sp_template)
                    sp_template.mask = sp_median_mask
                    LOG.info('    restored {0:.2f}% template flags after interpolation'.format(
                             100.0 * np.sum(sp_median_mask) / sp_median_mask.size))

                    # repeat after square root
                    if type(sp_data.mask) == type(False): sp_data.mask = np.ma.getmaskarray(sp_data)
                    sp_data.mask[sp_data < 0] = True
                    sp_data = sp_data ** .5
                    sp_template = sp_template ** .5
                    sp_data.mask[sp_data != sp_data] = True
                    sp_data, flag_percent = self.flag_with_medfilt(sp_data, sp_template, flag_rms=True,
                                                                   flag_median=True,
                                                                   k=5, threshold=6, do_shift=False)
                    LOG.info('    total flagged data: {0:.2f}% in fifth pass'.format(flag_percent))
                    sp_median_data = np.ma.median(sp_data, axis=0)
                    # sp_median_mask = deepcopy(sp_median_data.mask)
                    sp_template = savitzky_golay(self.interp_with_medfilt(sp_median_data), 7, 3)

                    dat_common[i, bband, pol, :] = sp_template

        spowerdict = {}
        spowerdict['spower_raw']          = dat_raw
        spowerdict['spower_flux_levels']  = dat_flux
        spowerdict['spower_rq']           = dat_rq
        spowerdict['spower_scaled']       = dat_scaled
        spowerdict['spower_filtered']     = dat_filtered
        spowerdict['spower_common']       = np.ma.filled(dat_common, 0)
        spowerdict['spower_online_flags'] = dat_online_flags
        spowerdict['spower_sum']          = dat_sum
        spowerdict['spower_sum_flux']     = dat_sum_flux

        # flag template using clip values
        final_template = np.ma.array(dat_common)
        final_template.mask = np.ma.getmaskarray(final_template)

        final_template.mask[final_template < clip_sp_template[0]] = True
        final_template.mask[final_template > clip_sp_template[1]] = True

        with casatools.TableReader(rq_table, nomodify=False) as tb:
            rq_time = tb.getcol('TIME')
            rq_spw  = tb.getcol('SPECTRAL_WINDOW_ID')
            rq_par  = tb.getcol('FPARAM')
            rq_ant  = tb.getcol('ANTENNA1')
            rq_flag = tb.getcol('FLAG')

            LOG.info('Starting RQ table')
            spw_offset = 2  # Hardwired for VLASS
            for i, this_ant in enumerate(antenna_ids):
                LOG.info('  writing RQ table for antenna {0}'.format(this_ant))

                for j, this_spw in enumerate(range(len(spws))):
                    hits = np.where((rq_ant == i) & (rq_spw == j + spw_offset))[0]
                    # hits = np.where((rq_ant == i) & (rq_spw == j))[0]
                    bband = 0 if (j < 8) else 1

                    hits2 = np.where(np.in1d(sorted_time, rq_time[hits]))[0]

                    for pol in [0, 1]:
                        try:
                            rq_par[2 * pol, 0, hits] *= final_template[i, bband, pol, hits2].data
                            rq_flag[2 * pol, 0, hits] = np.logical_or(rq_flag[2 * pol, 0, hits],
                                                                  final_template[i, bband, pol, hits2].mask)
                            if j in [0, 8]:
                                message = '  {2}% of solutions flagged in baseband {0},  polarization {1}'
                                LOG.info(message.format(bband, pol, 100. * np.sum(rq_flag[2 * pol, 0, hits]) /
                                                                                  rq_flag[2 * pol, 0, hits].size))
                        except:
                            LOG.warn('Error preparing final RQ table')
                            raise  # SystemExit('shape mismatch writing final RQ table')

            try:
                tb.putcol('FPARAM', rq_par)
                tb.putcol('FLAG', rq_flag)
            except Exception as ex:
                LOG.warn('Error writing final RQ table - switched power will not be applied' + str(ex))

        # create new table to plot pdiff template_table
        if os.path.isdir(template_table):
            shutil.rmtree(template_table)
        shutil.copytree(rq_table, template_table)

        with casatools.TableReader(template_table, nomodify=False) as tb:
            for i, this_ant in enumerate(antenna_ids):
                for j, this_spw in enumerate(range(len(spws))):
                    hits = np.where((rq_ant == i) & (rq_spw == j + spw_offset))[0]
                    bband = 0 if (j < 8) else 1
                    hits2 = np.where(np.in1d(sorted_time, rq_time[hits]))[0]

                    for pol in [0, 1]:
                        try:
                            rq_par[2 * pol, 0, hits] = final_template[i, bband, pol, hits2].data
                            rq_flag[2 * pol, 0, hits] = final_template[i, bband, pol, hits2].mask
                        except Exception as ex:
                            LOG.error('Shape mismatch writing final template table')

            tb.putcol('FPARAM', rq_par)
            tb.putcol('FLAG', rq_flag)

        return SyspowerResults(gaintable=rq_table, spowerdict=spowerdict, dat_common=dat_common,
                               clip_sp_template=clip_sp_template, template_table=template_table)

    def analyse(self, results):
        return results

    # function for smoothing and statistical flagging
    # adapted from https://gist.github.com/bhawkins/3535131
    def medfilt(self, x, k, threshold=6.0, flag_rms=False, flag_median=False, flag_only=False, fill_gaps=False):

        k2 = (k - 1) // 2
        y = np.ma.zeros((len(x), k))
        y.mask = np.ma.resize(x.mask, (len(x), k))
        y[:, k2] = x

        for i in range(k2):
            j = k2 - i
            y[j:, i] = x[:-j]
            y[:j, i] = x[0]
            y.mask[:j, i] = True
            y[:-j, -(i + 1)] = x[j:]
            y[-j:, -(i + 1)] = x[-1]
            y.mask[-j:, -(i + 1)] = True
        medians = np.ma.median(y, axis=1)

        if type(medians.mask) == np.bool_:
            medians.mask = np.ma.getmaskarray(medians)

        if np.ma.all(medians.mask): return medians

        if fill_gaps:
            x[x.mask] = medians[x.mask]
            return x

        if flag_median:
            rms = np.ma.std(y, axis=1)
            dev = np.ma.median(rms[rms != 0])
            medians.mask[abs(x - medians) > (dev * threshold)] = True
            medians.mask[rms == 0] = True
            medians.mask[rms != rms] = True

        if flag_rms:
            rms = np.ma.std(y, axis=1)
            dev = np.ma.median(rms[rms != 0])
            medians.mask[rms > (dev * threshold)] = True
            medians.mask[rms == 0] = True
            medians.mask[rms != rms] = True

        if not flag_only:
            return medians
        else:
            x.mask = np.logical_or(x.mask, medians.mask)
            return x

    # combine SPWs and flag based on moving window statistics
    def flag_with_medfilt(self, x, temp, k=21, threshold=6, do_shift=False, **kwargs):
        if do_shift:
            resid = x.ravel() - np.roll(x.ravel(), -1)
        else:
            resid = (x - temp[np.newaxis, :]).ravel()
        new_flags = self.medfilt(resid, k, threshold=threshold, flag_only=True, **kwargs)
        x.mask = np.reshape(new_flags.mask, newshape=x.shape)
        flag_percent = 100.0 * np.sum(x.mask) / x.size
        x.mask[x == 0] = True
        return x, flag_percent

    # use median filter to interpolate flagged values
    def interp_with_medfilt(self, x, k=21, threshold=99, max_interp=10):
        x.mask = np.ma.getmaskarray(x)
        this_interp = 0
        while np.any(x.mask == True):
            flag_percent = 100.0 * np.sum(x.mask) / x.size
            message = '    will attempt to interpolate {0:.2f}% of data in iteration {1}'.format(flag_percent,
                                                                                                 this_interp + 1)
            if this_interp == 0:
                LOG.info(message)
            else:
                LOG.debug(message)

            x = self.medfilt(x, k, threshold, fill_gaps=True)
            this_interp += 1
            if this_interp > max_interp: break
        flag_percent2 = 100.0 * np.sum(x.mask) / x.size
        LOG.info('    finished interpolation with {0:.2f}% of data flagged'.format(flag_percent2))
        x.mask[x == 0] = True
        return x
