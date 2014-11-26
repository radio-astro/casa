from __future__ import absolute_import
import collections
import itertools
import re
import os

import matplotlib
import matplotlib.pyplot as pyplot
import numpy

import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)

COLSHAPE_FORMAT = re.compile(r'\[(?P<num_pols>\d+), (?P<num_rows>\d+)\]')


class PlotbandpassDetailBase(object):
    def __init__(self, context, result, xaxis, yaxis, pol='', **kwargs):
        # identify the bandpass solution for the target
        calapps = [c for c in result.final
                   if (c.intent == '' or 'TARGET' in c.intent)]

        assert len(calapps) is 1, 'Target bandpass solutions != 1'
        calapp = calapps[0]

        self._vis = calapp.vis
        self._vis_basename = os.path.basename(self._vis)
        self._caltable = calapp.gaintable
        
        self._xaxis = xaxis
        self._yaxis = yaxis        
        self._kwargs = kwargs

        # we should only request plots for the antennas and spws in the
        # caltable, which may be a subset of those in the measurement set
        caltable_wrapper = CaltableWrapperFactory.from_caltable(self._caltable)
        antenna_ids = set(caltable_wrapper.antenna)
        spw_ids = set(caltable_wrapper.spw)
        
        # use antenna name rather than ID where possible
        antenna_arg = ','.join([str(i) for i in antenna_ids])
        ms = context.observing_run.get_ms(self._vis)
        antennas = ms.get_antenna(antenna_arg)
        self._antmap = dict((a.id, a.name) for a in antennas)

        # the number of polarisations for a spw may not be equal to the number
        # of shape of the column. For example, X403 has XX,YY for some spws 
        # but XX for the science data.
        num_pols = numpy.ma.shape(caltable_wrapper.data)[0]
        self._pols = {}
        for spw in spw_ids:
            dd = ms.get_data_description(spw=int(spw))
            num_pols = dd.num_polarizations
            pols = ','.join([dd.get_polarization_label(p)
                             for p in range(num_pols)])
            self._pols[spw] = pols
        
        overlay = self._kwargs.get('overlay', '')
        fileparts = {
            'caltable' : os.path.basename(calapp.gaintable),
            'x'        : self._xaxis,
            'y'        : self._yaxis,
            'overlay'  : '-%s' % overlay if overlay else ''
        }
        png = '{caltable}-{y}_vs_{x}{overlay}.png'.format(**fileparts)

        self._figroot = os.path.join(context.report_dir, 
                                     'stage%s' % result.stage_number,
                                     png)
        
        # plotbandpass injects spw ID and antenna name into every plot filename
        self._figfile = collections.defaultdict(dict)
        root, ext = os.path.splitext(self._figroot)
        time = '.t00' if 'time' not in overlay else ''
        for spw_id, ant_id in itertools.product(spw_ids, antenna_ids):
            ant_name = self._antmap[ant_id]
            real_figfile = '%s.%s.spw%0.2d%s%s' % (root, ant_name, spw_id, 
                                                   time, ext)
            self._figfile[spw_id][ant_id] = real_figfile

    def create_plot(self, spw_arg, antenna_arg):
        task_args = {'vis'         : self._vis,
                     'caltable'    : self._caltable,
                     'xaxis'       : self._xaxis,
                     'yaxis'       : self._yaxis,
                     'interactive' : False,
                     'spw'         : spw_arg,
                     'antenna'     : antenna_arg,
                     'subplot'     : 11,
                     'figfile'     : self._figroot}
        task_args.update(**self._kwargs)

        task = casa_tasks.plotbandpass(**task_args)
        task.execute(dry_run=False)

    def plot(self):
        pass


class PlotcalLeaf(object):
    """
    Class to execute plotcal and return a plot wrapper. It passes the spw and
    ant arguments through to plotcal without further manipulation, creating
    exactly one plot. 
    """
    def __init__(self, context, result, calapp, xaxis, yaxis, spw='', ant='',
                 pol='', plotrange=[]):
        self._context = context
        self._result = result

        self._calapp = calapp
        self._caltable = calapp.gaintable   
        self._vis = calapp.vis

        self._xaxis = xaxis
        self._yaxis = yaxis

        self._spw = spw
        self._intent = calapp.intent

        # use antenna name rather than ID if possible
        if ant != '':
            ms = self._context.observing_run.get_ms(self._vis)
            domain_antennas = ms.get_antenna(ant)
            idents = [a.name if a.name else a.id for a in domain_antennas]
            ant = ','.join(idents)
        self._ant = ant

        self._figfile = self._get_figfile()
        self._plotrange = plotrange

    def plot(self):
        try:
            plots = [self._get_plot_wrapper()]
            return [p for p in plots if p is not None]
        finally:
            # plotcal with time as x axis seems to leave matplotlib
            # in an open state. Work around this by closing pyplot
            # after each call.
            pyplot.close()
            
    def _get_figfile(self):
        fileparts = {
            'caltable' : os.path.basename(self._calapp.gaintable),
            'x'        : self._xaxis,
            'y'        : self._yaxis,
            'spw'      : '' if self._spw == '' else 'spw%0.2d-' % int(self._spw),
            'ant'      : '' if self._ant == '' else 'ant%s-' % self._ant.replace(',','_'),
            'intent'   : '' if self._intent == '' else '%s-' % self._intent.replace(',','_')
        }
        png = '{caltable}-{spw}{ant}{intent}{y}_vs_{x}.png'.format(**fileparts)

        return os.path.join(self._context.report_dir, 
                            'stage%s' % self._result.stage_number,
                            png)

    def _get_plot_wrapper(self):
        if not os.path.exists(self._figfile):
            LOG.trace('Creating new plot: %s' % self._figfile)
            try:
                self._create_plot()
            except Exception as ex:
                LOG.error('Could not create plot %s' % self._figfile)
                LOG.exception(ex)
                return None

        parameters={'vis'      : self._vis,
                    'caltable' : self._caltable}        

        for attr in ['spw', 'ant', 'intent']:
            val = getattr(self, '_%s' % attr)
            if val != '':
                parameters[attr] = val 
            
        wrapper = logger.Plot(self._figfile,
                              x_axis=self._xaxis,
                              y_axis=self._yaxis,
                              parameters=parameters)
            
        return wrapper

    def _create_plot(self):
        task_args = {'caltable'  : self._caltable,
                     'xaxis'     : self._xaxis,
                     'yaxis'     : self._yaxis,
                     'showgui'   : False,
                     'spw'       : str(self._spw),
                     'antenna'   : self._ant,
                     'figfile'   : self._figfile,
                     'plotrange' : self._plotrange}

        task = casa_tasks.plotcal(**task_args)
        task.execute(dry_run=False)


class PlotbandpassLeaf(object):
    """
    Class to execute plotbandpass and return a plot wrapper. It passes the spw
    and ant arguments through to plotbandpass without further manipulation. More
    than one plot may be created though not necessarily returned, as 
    plotbandpass may create many plots depending on the input arguments. 
    """
    def __init__(self, context, result, calapp, xaxis, yaxis, spw='', ant='', pol='',
                 overlay='', showatm=True):
        self._context = context
        self._result = result

        self._calapp = calapp
        self._caltable = calapp.gaintable   
        self._vis = calapp.vis
        ms = self._context.observing_run.get_ms(self._vis)

        self._xaxis = xaxis
        self._yaxis = yaxis

        self._spw = spw
        self._intent = calapp.intent

        # use antenna name rather than ID if possible
        if ant != '':
            domain_antennas = ms.get_antenna(ant)
            idents = [a.name if a.name else a.id for a in domain_antennas]
            ant = ','.join(idents)
        self._ant = ant

        # convert pol ID from integer to string, eg. 0 to XX
        if pol != '':
            dd = ms.get_data_description(spw=int(spw))
            pol = dd.get_polarization_label(pol)
        self._pol = pol

        self._figfile = self._get_figfile()

        # plotbandpass injects antenna name, spw ID and t0 into every plot filename
        root, ext = os.path.splitext(self._figfile)
        # if spw is '', the spw component will be set to the first spw 
        if spw == '':
            with casatools.TableReader(calapp.gaintable) as tb:
                caltable_spws = set(tb.getcol('SPECTRAL_WINDOW_ID'))
            spw = min(caltable_spws) 
            
        self._pb_figfile = '%s%s%s.t00%s' % (root, 
                                             '.%s' % ant if ant else '',
                                             '.spw%0.2d' % spw if spw else '',
                                             ext)
                
        self._overlay = overlay
        self._showatm = showatm
        
    def plot(self):
        plots = [self._get_plot_wrapper()]
        return [p for p in plots 
                if p is not None
                and os.path.exists(p.abspath)]

    def _get_figfile(self):
        fileparts = {
            'caltable' : os.path.basename(self._calapp.gaintable),
            'x'        : self._xaxis,
            'y'        : self._yaxis,
            'spw'      : '' if self._spw == '' else 'spw%s-' % self._spw,
            'ant'      : '' if self._ant == '' else 'ant%s-' % self._ant.replace(',','_'),
            'intent'   : '' if self._intent == '' else '%s-' % self._intent.replace(',','_'),
            'pol'      : '' if self._pol == '' else '%s-' % self._pol
        }
        png = '{caltable}-{spw}{pol}{ant}{intent}{y}_vs_{x}.png'.format(**fileparts)

        return os.path.join(self._context.report_dir, 
                            'stage%s' % self._result.stage_number,
                            png)

    def _get_plot_wrapper(self):
        if not os.path.exists(self._pb_figfile):
            LOG.trace('Creating new plot: %s' % self._pb_figfile)
            try:
                self._create_plot()
            except Exception as ex:
                LOG.error('Could not create plot %s' % self._pb_figfile)
                LOG.exception(ex)
                return None

        parameters={'vis'      : self._vis,
                    'caltable' : self._caltable}        

        for attr in ['spw', 'ant', 'intent', 'pol']:
            val = getattr(self, '_%s' % attr)
            if val != '':
                parameters[attr] = val 
            
        wrapper = logger.Plot(self._pb_figfile,
                              x_axis=self._xaxis,
                              y_axis=self._yaxis,
                              parameters=parameters)
            
        return wrapper

    def _create_plot(self):
        task_args = {'vis'         : self._vis,
                     'caltable'    : self._caltable,
                     'xaxis'       : self._xaxis,
                     'yaxis'       : self._yaxis,
                     'antenna'     : self._ant,
                     'spw'         : self._spw,
                     'poln'        : self._pol,
                     'overlay'     : self._overlay,
                     'figfile'     : self._figfile,
                     'showatm'     : self._showatm,
                     'interactive' : False,
                     'subplot'     : 11}

        task = casa_tasks.plotbandpass(**task_args)
        task.execute(dry_run=False)


class LeafComposite(object):
    """
    Base class to hold multiple PlotLeafs, thus generating multiple plots when
    plot() is called.
    """
    def __init__(self, children):
        self._children = children
        
    def plot(self):
        plots = []
        for child in self._children:
            plots.extend(child.plot())        
        return [p for p in plots if p is not None]


class PolComposite(LeafComposite):
    """
    Create a PlotLeaf for each polarization in the caltable.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None
    
    def __init__(self, context, result, calapp, xaxis, yaxis, ant='', spw='',
                 **kwargs):
        # the number of polarisations for a spw may not be equal to the number
        # of shape of the column. For example, X403 has XX,YY for some spws 
        # but XX for the science data. If we're given a spw argument we can
        # bypass the calls for the missing polarisation.
        if spw != '':
            vis = calapp.vis
            ms = context.observing_run.get_ms(vis)

            dd = ms.get_data_description(spw=int(spw))
            num_pols = dd.num_polarizations

        else:
            num_pols = utils.get_num_caltable_polarizations(calapp.gaintable)
        
        pol_range = range(num_pols)
        children = [self.leaf_class(context, result, calapp, xaxis, yaxis,
                                    spw=spw, ant=ant, pol=pol, **kwargs)
                                    for pol in pol_range]
        super(PolComposite, self).__init__(children)

    
class SpwComposite(LeafComposite):
    """
    Create a PlotLeaf for each spw in the caltable.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None
    
    def __init__(self, context, result, calapp, xaxis, yaxis, ant='', pol='',
                 **kwargs):
        with casatools.TableReader(calapp.gaintable) as tb:
            table_spws = set(tb.getcol('SPECTRAL_WINDOW_ID'))
        
        caltable_spws = [int(spw) for spw in table_spws]
        children = [self.leaf_class(context, result, calapp, xaxis, yaxis,
                                    spw=spw, ant=ant, pol=pol, **kwargs)
                                    for spw in caltable_spws]
        super(SpwComposite, self).__init__(children)


class AntComposite(LeafComposite):
    """
    Create a PlotLeaf for each antenna in the caltable.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None

    def __init__(self, context, result, calapp, xaxis, yaxis, spw='', pol='',
                 **kwargs):
        with casatools.TableReader(calapp.gaintable) as tb:
            table_ants = set(tb.getcol('ANTENNA1'))
        
        caltable_antennas = [int(ant) for ant in table_ants]
        children = [self.leaf_class(context, result, calapp, xaxis, yaxis,
                                    ant=ant, spw=spw, pol=pol, **kwargs)
                                    for ant in caltable_antennas]
        super(AntComposite, self).__init__(children)


class AntSpwComposite(LeafComposite):
    """
    Create a PlotLeaf for each spw and antenna in the caltable.
    """
    leaf_class = None

    def __init__(self, context, result, calapp, xaxis, yaxis, pol='', **kwargs):
        with casatools.TableReader(calapp.gaintable) as tb:
            table_ants = set(tb.getcol('ANTENNA1'))
        
        caltable_antennas = [int(ant) for ant in table_ants]
        children = [self.leaf_class(context, result, calapp, xaxis, yaxis,
                                    ant=ant, pol=pol, **kwargs)
                                    for ant in caltable_antennas]
        super(AntSpwComposite, self).__init__(children)


class SpwPolComposite(LeafComposite):
    """
    Create a PlotLeaf for each spw and polarization in the caltable.
    """
    leaf_class = None

    def __init__(self, context, result, calapp, xaxis, yaxis, ant='', **kwargs):
        with casatools.TableReader(calapp.gaintable) as tb:
            table_spws = set(tb.getcol('SPECTRAL_WINDOW_ID'))
        
        caltable_spws = [int(spw) for spw in table_spws]
        children = [self.leaf_class(context, result, calapp, xaxis, yaxis,
                                    spw=spw, ant=ant, **kwargs)
                                    for spw in caltable_spws]
        super(SpwPolComposite, self).__init__(children)


class AntSpwPolComposite(LeafComposite):
    """
    Create a PlotLeaf for each antenna, spw and polarization in the caltable.
    """
    leaf_class = None

    def __init__(self, context, result, calapp, xaxis, yaxis, **kwargs):
        with casatools.TableReader(calapp.gaintable) as tb:
            table_ants = set(tb.getcol('ANTENNA1'))
        
        caltable_antennas = [int(ant) for ant in table_ants]
        children = [self.leaf_class(context, result, calapp, xaxis, yaxis,
                                    ant=ant, **kwargs)
                                    for ant in caltable_antennas]
        super(AntSpwPolComposite, self).__init__(children)


class PlotcalAntComposite(AntComposite):
    leaf_class = PlotcalLeaf


class PlotcalSpwComposite(SpwComposite):
    leaf_class = PlotcalLeaf


class PlotcalAntSpwComposite(AntSpwComposite):
    leaf_class = PlotcalSpwComposite


class PlotbandpassAntComposite(AntComposite):
    leaf_class = PlotbandpassLeaf


class PlotbandpassSpwComposite(SpwComposite):
    leaf_class = PlotbandpassLeaf


class PlotbandpassPolComposite(PolComposite):
    leaf_class = PlotbandpassLeaf
    

class PlotbandpassAntSpwComposite(AntSpwComposite):
    leaf_class = PlotbandpassSpwComposite


class PlotbandpassSpwPolComposite(SpwPolComposite):
    leaf_class = PlotbandpassPolComposite


class PlotbandpassAntSpwPolComposite(AntSpwPolComposite):
    leaf_class = PlotbandpassSpwPolComposite


class CaltableWrapperFactory(object):
    @staticmethod
    def from_caltable(filename):
        LOG.trace('CaltableWrapperFactory.from_caltable(%r)', filename)
        with utils.open_table(filename) as tb:
            viscal = tb.getkeyword('VisCal')            
            caltype = callibrary.CalFrom.get_caltype_for_viscal(viscal) 
        if caltype == 'gaincal':
            return CaltableWrapperFactory.create_gaincal_wrapper(filename)
        if caltype == 'tsys':
            return CaltableWrapperFactory.create_fparam_wrapper(filename)            
        if caltype == 'bandpass':
            return CaltableWrapperFactory.create_bandpass_wrapper(filename)            
        raise NotImplementedError('Unhandled caltype: %s', caltype)
    
    @staticmethod    
    def create_gaincal_wrapper(path):
        with utils.open_table(path) as tb:
            time_mjd = tb.getcol('TIME')
            antenna1 = tb.getcol('ANTENNA1')
            spw = tb.getcol('SPECTRAL_WINDOW_ID')
            scan = tb.getcol('SCAN_NUMBER')
            flag = tb.getcol('FLAG')
                
            # convert MJD times stored in caltable to matplotlib equivalent
            time_unix = utils.mjd_seconds_to_datetime(time_mjd)
            time_matplotlib = matplotlib.dates.date2num(time_unix)

            gain = tb.getcol('CPARAM')            
            phase = numpy.arctan2(numpy.imag(gain),
                                  numpy.real(gain)) * 180.0 / numpy.pi
            data = numpy.ma.MaskedArray(phase, mask=flag)

            return CaltableWrapper(path, data, time_matplotlib, antenna1, spw,
                                   scan)

    @staticmethod    
    def create_bandpass_wrapper(path):
        with utils.open_table(path) as tb:
            time_mjd = tb.getcol('TIME')
            antenna1 = tb.getcol('ANTENNA1')
            spw = tb.getcol('SPECTRAL_WINDOW_ID')
            scan = tb.getcol('SCAN_NUMBER')
                
            # convert MJD times stored in caltable to matplotlib equivalent
            time_unix = utils.mjd_seconds_to_datetime(time_mjd)
            time_matplotlib = matplotlib.dates.date2num(time_unix)

            gain = tb.getvarcol('CPARAM')
            temp = [gain['r%s' % (k+1)] for k in range(len(gain))]
            gain = zip(*itertools.chain(*temp))

            flag = tb.getvarcol('FLAG')
            temp = [flag['r%s' % (k+1)] for k in range(len(flag))]
            flag = zip(*itertools.chain(*temp))

            data = numpy.ma.MaskedArray(gain, mask=flag)

            return CaltableWrapper(path, data, time_matplotlib, antenna1, spw,
                                   scan)

    @staticmethod    
    def create_fparam_wrapper(path):
        with utils.open_table(path) as tb:
            time_mjd = tb.getcol('TIME')
            antenna1 = tb.getcol('ANTENNA1')
            spw = tb.getcol('SPECTRAL_WINDOW_ID')
            scan = tb.getcol('SCAN_NUMBER')
            flag = tb.getcol('FLAG')

            # convert MJD times stored in caltable to matplotlib equivalent
            time_unix = utils.mjd_seconds_to_datetime(time_mjd)
            time_matplotlib = matplotlib.dates.date2num(time_unix)
            
            tsys = tb.getcol('FPARAM')            

            # shape of tsys and flag arrays is number of corrs, number of
            # channels, number of rows. Remove the middle dimension.
            data = numpy.ma.MaskedArray(tsys, mask=flag)

            return CaltableWrapper(path, data, time_matplotlib, antenna1, spw,
                                   scan)
        

class CaltableWrapper(object):
    @staticmethod
    def from_caltable(filename):
        return CaltableWrapperFactory.from_caltable(filename)            

    def __init__(self, filename, data, time, antenna, spw, scan):
        # tag the extra metadata columns onto our data array 
        self.filename = filename
        self.data = data
        self.time = time
        self.antenna = antenna
        self.spw = spw
        self.scan = scan
    
        # get list of which spws, antennas and scans we have data for
        self._spws = frozenset(spw)
        self._antennas = frozenset(antenna)
        self._scans = frozenset(scan)

    def _get_mask(self, allowed, data):
        mask = numpy.zeros_like(data)
        for a in allowed:
            if a not in data:
                raise KeyError, '%s is not in caltable data' % a
            mask = (mask==1) | (data==a)
        return mask

    def filter(self, spw=None, antenna=None, scan=None):
        if spw is None:
            spw = self._spws
        if antenna is None:
            antenna = self._antennas
        if scan is None:
            scan = self._scans
            
        # get data selection mask for each selection parameter
        antenna_mask = self._get_mask(antenna, self.antenna)
        spw_mask = self._get_mask(spw, self.spw)
        scan_mask = self._get_mask(scan, self.scan)
        
        # combine masks to create final data selection mask
        mask = (antenna_mask==1) & (spw_mask==1) & (scan_mask==1)

        # find data for the selection mask 
        data = self.data[:,:,mask]
        time = self.time[mask]
        antenna = self.antenna[mask]
        spw = self.spw[mask]
        scan = self.scan[mask]
        
        # create new object for the filtered data
        return CaltableWrapper(self.filename, data, time, antenna, spw, scan)


class PhaseVsBaselineData(object):
    def __init__(self, data, ms, corr_id, refant_id):
        # While it is possible to do so, we shouldn't calculate statistics for
        # mixed antennas/spws/scans.
        if len(set(data.antenna)) is 0:
            raise ValueError('No antennas defined in data selection')
        if len(set(data.spw)) is 0:
            raise ValueError('No spw defined in data selection')
        if len(set(data.antenna)) > 1:
            raise ValueError('Data slice contains multiple antennas. Got %s' % data.antenna)
        if len(set(data.spw)) > 1:
            raise ValueError('Data slice contains multiple spws. Got %s' % data.spw) 
#        assert len(set(data.scan)) is 1, 'Data slice contains multiple scans'

        self.data = data
        self.ms = ms
        self.corr = corr_id
        self.data_for_corr = self.data.data[corr_id]
        self.__refant_id = refant_id

        if len(self.data_for_corr) is 0:
            raise ValueError('No data for spw %s ant %s scan %s' % (data.spw[0],
                                                                    data.antenna[0],
                                                                    data.scan))

    @property
    def antenna(self):
        return self.data.antenna
    
    @property
    def scan(self):
        return self.data.scan
    
    @property
    def spw(self):
        return self.data.spw[0]

    @property
    def num_corr_axes(self):
        return len(self.data.data)

    @property
    def refant(self):
        return self.__refant_id

    @property
    @utils.memoized
    def baselines(self):
        """
        Get the baselines for the antenna in this data selection in metres.
        """
        antenna_ids = set(self.data.antenna)
        baselines = [float(b.length.to_units(measures.DistanceUnits.METRE))
                     for b in self.ms.antenna_array.baselines
                     if b.antenna1.id in antenna_ids 
                     or b.antenna2.id in antenna_ids]                
        return baselines

    @property
    @utils.memoized
    def distance_to_refant(self):
        """
        Return the distance between this antenna and the reference antenna in 
        metres.
        """
        antenna_id = int(self.data.antenna[0])
        if antenna_id == self.refant:
            return 0.0

        baseline = self.ms.antenna_array.get_baseline(self.refant, antenna_id)
        return float(baseline.length.to_units(measures.DistanceUnits.METRE))

    @property
    @utils.memoized
    def median_baseline(self):
        """
        Return the median baseline for this antenna in metres.
        """
        return numpy.median(self.baselines)

    @property          
    @utils.memoized  
    def mean_baseline(self):
        """
        Return the mean baseline for this antenna in metres.
        """
        return numpy.mean(self.baselines)

    @property
    @utils.memoized
    def unwrapped_data(self):
        rads = numpy.deg2rad(self.data_for_corr)
        unwrapped_rads = numpy.unwrap(rads)
        unwrapped_degs = numpy.rad2deg(unwrapped_rads)
        # the operation above removed the mask, so add it back.
        remasked = numpy.ma.MaskedArray((unwrapped_degs), 
                                        mask=self.data_for_corr.mask)
        return remasked
        
    @property
    @utils.memoized
    def offsets_from_median(self):
        try:
            unwrapped_degs = self.unwrapped_data
            deg_offsets = unwrapped_degs - numpy.ma.median(unwrapped_degs)
            # the operation above removed the mask, so add it back.
            remasked = numpy.ma.MaskedArray((deg_offsets), 
                                            mask=self.data_for_corr.mask)
            return remasked
        except:
            raise
        
    @property
    @utils.memoized
    def rms_offset(self):
        saved_handler = None
        saved_err = None
        try:
            return numpy.ma.sqrt(numpy.ma.mean(self.offsets_from_median**2))
        except FloatingPointError:
            def err_handler(t, flag):
                ant = set(self.data.antenna).pop()
                spw = set(self.data.spw).pop()
                scan = set(self.data.scan).pop()                
                LOG.warn('Floating point error (%s) calculating RMS offset for'
                         ' Scan %s Spw %s Ant %s.' % (t, scan, spw, ant))
            saved_handler = numpy.seterrcall(err_handler)
            saved_err = numpy.seterr(all='call')
            return numpy.ma.sqrt(numpy.ma.mean(self.offsets_from_median**2))
        finally:
            if saved_handler:
                numpy.seterrcall(saved_handler)
                numpy.seterr(**saved_err)

    @property
    @utils.memoized
    def unwrapped_rms(self):
        saved_handler = None
        saved_err = None
        try:
            return numpy.ma.sqrt(numpy.ma.mean(self.unwrapped_data**2))
        except FloatingPointError:
            def err_handler(t, flag):
                ant = set(self.data.antenna).pop()
                spw = set(self.data.spw).pop()
                scan = set(self.data.scan).pop()                
                LOG.warn('Floating point error (%s) calculating unwrapped RMS for'
                         ' Scan %s Spw %s Ant %s.' % (t, scan, spw, ant))
            saved_handler = numpy.seterrcall(err_handler)
            saved_err = numpy.seterr(all='call')
            return numpy.ma.sqrt(numpy.ma.mean(self.unwrapped_data**2))
        finally:
            if saved_handler:
                numpy.seterrcall(saved_handler)
                numpy.seterr(**saved_err)
        
    @property
    @utils.memoized
    def median_offset(self):            
        abs_offset = numpy.ma.abs(self.offsets_from_median)
        return numpy.ma.median(abs_offset)


class XYData(object):
    def __init__(self, delegate, x_axis, y_axis):
        self.__delegate = delegate
        self.__x_axis = x_axis
        self.__y_axis = y_axis

    @property
    def antenna(self):
        return self.__delegate.antenna

    @property
    def corr(self):
        return self.__delegate.corr

    @property
    def data(self):
        return self.__delegate.data

    @property
    def ratio(self):
        return self.__delegate.ratio

    @property
    def scan(self):
        return self.__delegate.scan
    
    @property
    def spw(self):
        return self.__delegate.spw

    @property
    def x(self):
        return getattr(self.__delegate, self.__x_axis)

    @property
    def y(self):
        return getattr(self.__delegate, self.__y_axis)


class DataRatio(object):
    def __init__(self, before, after):
        # test symmetric differences to find data selection errors
        if set(before.antenna) ^ set(after.antenna):
            raise ValueError('Data slices are for different antennas')
        if set(before.scan) ^ set(after.scan):
            raise ValueError('Data slices are for different scans')
        if before.spw != after.spw:
            raise ValueError('Data slices are for different spws')
        if before.corr != after.corr:
            raise ValueError('Data slices are for different correlations')

        self.__before = before
        self.__after = after

        self.__antennas = frozenset(before.data.antenna).union(set(after.data.antenna))
        self.__spws = frozenset(before.data.spw).union(set(after.data.spw))
        self.__scans = frozenset(before.data.scan).union(set(after.data.scan))
        self.__corr = frozenset((before.corr, after.corr))

    @property
    def after(self):
        return self.__after
    
    @property
    def antennas(self):
        return self.__antennas

    @property
    def before(self):
        return self.__before
        
    @property
    def corr(self):
        return self.__corr
        
    @property
    def scans(self):
        return self.__scans
    
    @property
    def spws(self):
        return self.__spws

    @property            
    def x(self):
        assert(self.__before.x == self.__after.x)
        return self.__before.x

    @property
    def num_corr_axes(self):
        # having ensured the before/after data are for the same scan and spw,
        # they should have the same number of correlations
        return len(self.__before.data)

    @property
    @utils.memoized
    def y(self):
        before = self.__before.y
        after = self.__after.y
        
        if None in (before, after):
            return None
        # avoid divide by zero
        if after == 0:
            return None
        return before / after


class NullScoreFinder(object):
    def get_score(self, *args, **kwargs):
        return None


class PlotBase(object):
    def get_symbol_and_colour(self, pol, state='BEFORE'):
        """
        Get the plot symbol and colour for this polarization and bandtype.
        """
        d = {'BEFORE' : {'L' : ('-', 'orange', 0.6),
                         'R' : ('--', 'sandybrown', 0.6),
                         'X' : ('-', 'lightslategray', 0.6),
                         'Y' : ('--', 'lightslategray', 0.6),
                         'XX' : ('-', 'lightslategray', 0.6),
                         'YY' : ('--', 'lightslategray', 0.6)},
             'AFTER' : {'L' : ('-', 'green', 0.6),
                        'R' : ('-', 'red', 0.6),
                        'X' : ('-', 'green', 0.6),
                        'Y' : ('-', 'red', 0.6),
                        'XX' : ('-', 'green', 0.6),
                        'YY' : ('-', 'red', 0.6)}}
    
        return d.get(state, {}).get(pol, ('x', 'grey'))

    def _load_caltables(self, before, after=None):
        if self._caltables_loaded:
            return
        
        # Get phases before and after
        data_before = CaltableWrapper.from_caltable(before)
        if after:
            data_after = CaltableWrapper.from_caltable(after)
        else:
            data_after = data_before
        
        # some sanity checks, as unequal caltables have bit me before
        assert utils.areEqual(data_before.time, data_after.time), 'Time columns are not equal'
        assert utils.areEqual(data_before.antenna, data_after.antenna), 'Antenna columns are not equal'
        assert utils.areEqual(data_before.spw, data_after.spw), 'Spw columns are not equal'
        assert utils.areEqual(data_before.scan, data_after.scan), 'Scan columns are not equal'

        self._data_before = data_before
        self._data_after = data_after
        self._caltables_loaded = True

    def _get_qa_intents(self):
        return set(self.result.inputs['qa_intent'].split(','))
    
    def _get_qa_scans(self):
        qa_intents = self._get_qa_intents()
        return [scan for scan in self.ms.scans 
                if not qa_intents.isdisjoint(scan.intents)]
    
# linux matplotlib is stuck on v0.99.3, which is missing the subplots function    
def subplots(nrows=1, ncols=1, sharex=False, sharey=False, squeeze=True,
                subplot_kw=None, **fig_kw):
    """Create a figure with a set of subplots already made.

    This utility wrapper makes it convenient to create common layouts of
    subplots, including the enclosing figure object, in a single call.

    Keyword arguments:

    nrows : int
      Number of rows of the subplot grid.  Defaults to 1.

    ncols : int
      Number of columns of the subplot grid.  Defaults to 1.

    sharex : bool
      If True, the X axis will be shared amongst all subplots.  If
      True and you have multiple rows, the x tick labels on all but
      the last row of plots will have visible set to False

    sharey : bool
      If True, the Y axis will be shared amongst all subplots. If
      True and you have multiple columns, the y tick labels on all but
      the first column of plots will have visible set to False

    squeeze : bool
      If True, extra dimensions are squeezed out from the returned axis object:

        - if only one subplot is constructed (nrows=ncols=1), the
          resulting single Axis object is returned as a scalar.

        - for Nx1 or 1xN subplots, the returned object is a 1-d numpy
          object array of Axis objects are returned as numpy 1-d
          arrays.

        - for NxM subplots with N>1 and M>1 are returned as a 2d
          array.

      If False, no squeezing at all is done: the returned axis object is always
      a 2-d array contaning Axis instances, even if it ends up being 1x1.

    subplot_kw : dict
      Dict with keywords passed to the add_subplot() call used to create each
      subplots.

    fig_kw : dict
      Dict with keywords passed to the figure() call.  Note that all keywords
      not recognized above will be automatically included here.

    Returns:

    fig, ax : tuple

      - fig is the Matplotlib Figure object

      - ax can be either a single axis object or an array of axis
        objects if more than one supblot was created.  The dimensions
        of the resulting array can be controlled with the squeeze
        keyword, see above.

    **Examples:**

    x = np.linspace(0, 2*np.pi, 400)
    y = np.sin(x**2)

    # Just a figure and one subplot
    f, ax = plt.subplots()
    ax.plot(x, y)
    ax.set_title('Simple plot')

    # Two subplots, unpack the output array immediately
    f, (ax1, ax2) = plt.subplots(1, 2, sharey=True)
    ax1.plot(x, y)
    ax1.set_title('Sharing Y axis')
    ax2.scatter(x, y)

    # Four polar axes
    plt.subplots(2, 2, subplot_kw=dict(polar=True))
    """

    if subplot_kw is None:
        subplot_kw = {}

    fig = pyplot.figure(**fig_kw)

    # Create empty object array to hold all axes.  It's easiest to make it 1-d
    # so we can just append subplots upon creation, and then
    nplots = nrows*ncols
    axarr = numpy.empty(nplots, dtype=object)

    # Create first subplot separately, so we can share it if requested
    ax0 = fig.add_subplot(nrows, ncols, 1, **subplot_kw)
    if sharex:
        subplot_kw['sharex'] = ax0
    if sharey:
        subplot_kw['sharey'] = ax0
    axarr[0] = ax0

    # Note off-by-one counting because add_subplot uses the MATLAB 1-based
    # convention.
    for i in range(1, nplots):
        axarr[i] = fig.add_subplot(nrows, ncols, i+1, **subplot_kw)



    # returned axis array will be always 2-d, even if nrows=ncols=1
    axarr = axarr.reshape(nrows, ncols)


    # turn off redundant tick labeling
    if sharex and nrows>1:
        # turn off all but the bottom row
        for ax in axarr[:-1,:].flat:
            for label in ax.get_xticklabels():
                label.set_visible(False)


    if sharey and ncols>1:
        # turn off all but the first column
        for ax in axarr[:,1:].flat:
            for label in ax.get_yticklabels():
                label.set_visible(False)

    if squeeze:
        # Reshape the array to have the final desired dimension (nrow,ncol),
        # though discarding unneeded dimensions that equal 1.  If we only have
        # one subplot, just return it instead of a 1-element array.
        if nplots==1:
            ret = fig, axarr[0,0]
        else:
            ret = fig, axarr.squeeze()
    else:
        # returned axis array will be always 2-d, even if nrows=ncols=1
        ret = fig, axarr.reshape(nrows, ncols)

    return ret
