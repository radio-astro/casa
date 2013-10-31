from __future__ import absolute_import

import matplotlib
import matplotlib.pyplot as pyplot
import numpy

import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class PlotcalXVsYSummaryChart(object):
    xaxis = ''
    yaxis = ''
    iteration = ''
    
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])

    def plot(self):
        for calapp in self.result.final:
            self.caltable = calapp.gaintable   
                     
            wrapper = CaltableWrapper.from_caltable(calapp.gaintable)
            caltable_spws = ','.join([str(spw) for spw in set(wrapper.spw)])
            domain_spws = self.ms.get_spectral_windows(caltable_spws)
            plots = [self.get_plot_wrapper(spw, calapp.intent)
                     for spw in domain_spws]
        return [p for p in plots if p is not None]

    def create_plot(self, figfile, spw):
        task_args = {'caltable'  : self.caltable,
                     'xaxis'     : self.xaxis,
                     'yaxis'     : self.yaxis,
                     'iteration' : self.iteration,
                     'showgui'   : False,
                     'spw'       : str(spw.id),
                     'figfile'   : figfile}

        task = casa_tasks.plotcal(**task_args)
        task.execute(dry_run=False)

    def get_figfile(self, spw, intent):
        if intent != '':
            intent = intent.replace(',','_')
            intent += '-'
        
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            '%s-spw%s-%s_vs_%s-%ssummary.png' % (self.caltable, 
                                                                 spw.id,
                                                                 self.yaxis, 
                                                                 self.xaxis,
                                                                 intent))

    def get_plot_wrapper(self, spw, intent):
        figfile = self.get_figfile(spw, intent)

        wrapper = logger.Plot(figfile,
                              x_axis=self.xaxis,
                              y_axis=self.yaxis,
                              parameters={'vis'      : self.ms.basename,
                                          'caltable' : self.caltable,
                                          'spw'      : spw.id,
                                          'intent'   : intent})

        if not os.path.exists(figfile):
            LOG.trace('Summary plot for spw %s not found. Creating new '
                      'plot: %s' % (spw.id, figfile))
            try:
                self.create_plot(figfile, spw)
            except Exception as ex:
                LOG.error('Could not create summary plot for spw %s'
                          '' % spw.id)
                LOG.exception(ex)
                return None
            
        return wrapper


class CaltableWrapper(object):
    @staticmethod
    def from_caltable(filename):
        with casatools.TableReader(filename) as tb:
            time_mjd = tb.getcol('TIME')
            antenna1 = tb.getcol('ANTENNA1')
            gain = tb.getcol('CPARAM')
            spw = tb.getcol('SPECTRAL_WINDOW_ID')
            scan = tb.getcol('SCAN_NUMBER')
            flag = tb.getcol('FLAG')
            phase = numpy.arctan2(numpy.imag(gain),
                                  numpy.real(gain)) * 180.0 / numpy.pi

            # shape of phase and flag arrays is number of corrs, 1, number of
            # rows. Remove the middle dimension.
            phases = phase[:,0]
            flags = flag[:,0]
            data = numpy.ma.MaskedArray(phases, mask=flags)

            # convert MJD times stored in caltable to matplotlib equivalent
            time_unix = utils.mjd_seconds_to_datetime(time_mjd)
            time_matplotlib = matplotlib.dates.date2num(time_unix)

            return CaltableWrapper(filename, data, time_matplotlib, antenna1, spw, scan)

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
        data = self.data[:,mask]
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
        return unwrapped_degs
        
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
            return numpy.ma.std(self.offsets_from_median)
        except FloatingPointError:
            def err_handler(t, flag):
                ant = set(self.data.antenna).pop()
                spw = set(self.data.spw).pop()
                scan = set(self.data.scan).pop()                
                LOG.warn('Floating point error (%s) calculating RMS offset for'
                         ' Scan %s Spw %s Ant %s.' % (t, scan, spw, ant))
            saved_handler = numpy.seterrcall(err_handler)
            saved_err = numpy.seterr(all='call')
            return numpy.ma.std(self.offsets_from_median)
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
            return numpy.ma.std(self.unwrapped_data)
        except FloatingPointError:
            def err_handler(t, flag):
                ant = set(self.data.antenna).pop()
                spw = set(self.data.spw).pop()
                scan = set(self.data.scan).pop()                
                LOG.warn('Floating point error (%s) calculating unwrapped RMS for'
                         ' Scan %s Spw %s Ant %s.' % (t, scan, spw, ant))
            saved_handler = numpy.seterrcall(err_handler)
            saved_err = numpy.seterr(all='call')
            return numpy.ma.std(self.unwrapped_data)
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

    def _get_qa2_intents(self):
        return set(self.result.inputs['qa2_intent'].split(','))
    
    def _get_qa2_scans(self):
        qa2_intents = self._get_qa2_intents()
        return [scan for scan in self.ms.scans 
                if not qa2_intents.isdisjoint(scan.intents)]
    
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
