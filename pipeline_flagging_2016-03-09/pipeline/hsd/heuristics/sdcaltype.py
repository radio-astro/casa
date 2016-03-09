from __future__ import absolute_import

import numpy

import pipeline.infrastructure.api as api
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure
from pipeline.hsd.heuristics import DataTypeHeuristics
LOG = infrastructure.get_logger(__name__)


class CalibrationTypeHeuristics(api.Heuristic):
    """
    """
    def calculate(self, filename):
        h = self._heuristics_factory(filename)
        return h(filename)

    def _heuristics_factory(self, filename):
        h = DataTypeHeuristics()
        datatype = h(filename).lower()
        return getattr(self,'_generate_%s_heuristics'%(datatype))()

    def _generate_asap_heuristics(self):
        return AsapCalibrationTypeHeuristics()

    def _generate_ms2_heuristics(self):
        return MsCalibrationTypeHeuristics()

    def _generate_asdm_heuristics(self):
        return AsdmCalibrationTypeHeuristics()

    def _generate_fits_heuristics(self):
        return DefaultCalibrationTypeHeuristics()

    def _generate_nro_heuristics(self):
        return DefaultCalibrationTypeHeuristics()

    def _generate_unknown_heuristics(self):
        return DefaultCalibrationTypeHeuristics()


class DefaultCalibrationTypeHeuristics(api.Heuristic):
    """
    Always return calibration mode 'none'.
    """
    def calculate(self, filename):
        """
        Return calibration type. Always return 'none'.
        """
        return 'none'


class AsapCalibrationTypeHeuristics(api.Heuristic):
    """
    Return appropriate calibration type by examining
    SRCTYPE column in MAIN table.
    """
    def calculate(self, filename):
        """
        Return calibration type, which is determined
        by examining SRCTYPE column in MAIN table.
        """
        import re
        import numpy
        from asap import srctype as st
        caltype = 'none'

        with casatools.TableReader(filename) as tb:
            #antenna_name = tb.getkeyword('AntennaName').upper()
            srctypes = numpy.unique(tb.getcol('SRCTYPE'))
        if st.fson in srctypes or st.fsoff in srctypes:
            caltype = 'fs'
        elif st.nod in srctypes:
            caltype = 'nod'
        elif st.psoff in srctypes:
            caltype = 'ps'
        elif st.pson in srctypes:
            # 'otf', 'otfraster', or already calibrated data.
            LOG.warn('It seems that the data doesn\'t have reference scans. The data would be either fast scan without explicit reference scans or already calibrated data. Here, we assume the data is fast scan without reference. Try to analyse whether observing pattern is raster or not.')
            
            LOG.todo('implement an algorithm to distinguish between calibrated data (\'none\') and uncalibrated data without OFF scans (\'otf\' or \'otfraster\')')

            if _israster(filename):
                caltype = 'otfraster'
            else:
                caltype = 'otf'
        else:
            # TODO: we have to find a way to distinguish calibrated
            #       data (caltype should be 'none') and uncalibrated
            #       data that doesn't have explicit OFF scans
            #       (such as 'otf' or 'otfraster')
            LOG.warn('Unrecognized calibration type. Set to \'none\'')
            caltype = 'none'

        return caltype


def _gap_analysis(timestamps, intervals, threshold_factor=3.0):
    separation = timestamps[1:] - timestamps[:-1]
    real_separation = separation.take(separation.nonzero()[0])
    average_interval = numpy.median(intervals)
    threshold = average_interval * threshold_factor
    #threshold = numpy.median(real_separation) * threshold_factor
    LOG.debug('threshold = %s'%(threshold))
    gaplist = numpy.where(separation > threshold)[0]
    return gaplist
        
def _israster(filename):
    from asap import srctype as st
    # filter out WVR data
    with casatools.TableReader(filename) as tb:
        ifnos = numpy.unique(tb.getcol('IFNO'))
        wvr_list = [ifno for ifno in ifnos
                    if len(tb.query('IFNO==%s'%(ifno)).getcell('FLAGTRA',0)) == 4]
    LOG.debug('wvr_list=%s'%(wvr_list))

    # retrieve direction and timestamp
    query_string = 'SRCTYPE == %s'%(int(st.pson))
    if len(wvr_list) > 0:
        query_string += ' && IFNO NOT IN %s'%(wvr_list)
    with casatools.TableReader(filename) as tb:
        tsel = tb.query(query_string)
        direction = tsel.getcol('DIRECTION')
        timestamp = tsel.getcol('TIME') * 86400.0
        interval = tsel.getcol('INTERVAL')

    # gap analysis
    gap_list = _gap_analysis(timestamp, interval, threshold_factor=5.0)
    LOG.debug('gap_list=%s'%(gap_list))

    # inspect scan direction
    dir0 = direction[:,:(gap_list[0]+1)]
    ddir0 = dir0[:,1:] - dir0[:,:-1]
    ddir0_nonzero = ddir0[:,ddir0[0].nonzero()[0]]
    slope = ddir0_nonzero[1] / ddir0_nonzero[0]
    LOG.debug('slope=%s'%(slope))
    slope_mean = slope.mean()
    slope_std = slope.std()
    LOG.debug('slope_mean=%s'%(slope_mean))
    LOG.debug('slope_std=%s'%(slope_std))

    # compare extent of one direction segment with total map extent
    dx_total = direction[0].max() - direction[0].min()
    dy_total = direction[1].max() - direction[1].min()
    LOG.debug('dx_total, dy_total = %s, %s'%(dx_total, dy_total))
    dx = dir0[0].max() - dir0[0].min()
    dy = dir0[1].max() - dir0[1].min()
    LOG.debug('dx, dy = %s, %s'%(dx, dy))
    dx_fraction = dx / dx_total
    dy_fraction = dy / dy_total
    LOG.debug('fraction = %s, %s'%(dx_fraction, dy_fraction))

    # Heuristics of observing pattern
    # Here, slope and extent of directions in the first segment 
    # are examined. If slope is almost same in the segment (i.e.,
    # straight line) and extent of the segment is significant
    # with respect to total map extent, the observing pattern
    # is regarded as 'raster'.
    israster = (slope_std / abs(slope_mean) < 0.01) \
               and (max(dx_fraction, dy_fraction) > 0.34)
    
    return israster
    

class MsCalibrationTypeHeuristics(api.Heuristic):
    """
    Return appropriate calibration type by examining
    STATE subtable.
    """
    def calculate(self, filename):
        """
        Return calibration type, which is determined
        by examining STATE subtable.
        """
        caltype = 'none'
        
        with casatools.TableReader(filename) as tb:
            statetable = tb.getkeyword('STATE').lstrip('Table: ')
        if os.path.exists(statetable):
            with casatools.TableReader(statetable) as tb:
                obsmodes = tb.getcol( 'OBS_MODE' )
                obsmode = obsmodes[0]
            if ( obsmode.find( 'PSWITCH' ) != -1 ):
                caltype = 'ps'
            elif ( obsmode.find( 'FSWITCH' ) != -1 ):
                caltype = 'fs'
            else:
                import re
                regex = '^OBSERVE_TARGET[#,_](ON|OFF)_SOURCE'
                found = False
                for obsmode in obsmodes:
                    if re.match(regex,obsmode) is not None:
                        found = True
                        break
                if found:
                    caltype = 'ps' #'otf'

        return caltype


class AsdmCalibrationTypeHeuristics(api.Heuristic):
    """
    """
    def calculate(self, filename):
        caltype = 'none'
        
        import xml.dom.minidom as DOM
        scan_table = os.path.join(filename,'Scan.xml')
        dom3 = DOM.parse(scan_table)
        rootnode = dom3.getElementsByTagName('scanIntent')[0]
        intelm = rootnode
        intstr = intelm.childNodes[0].data.encode( 'UTF-8' )
        if ( intstr.find( 'OBSERVE_TARGET' ) != -1 ):
            caltype = 'ps' #'otf'
        dom3.unlink()
        
        return caltype
    
