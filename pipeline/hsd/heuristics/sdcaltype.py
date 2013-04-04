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
        else:
            # TODO: we have to find a way to distinguish calibrated
            #       data (caltype should be 'none') and uncalibrated
            #       data that doesn't have explicit OFF scans
            #       (such as 'otf' or 'otfraster')
            caltype = 'none'

            LOG.todo('implement an algorithm to distinguish between calibrated data (\'none\') and uncalibrated data without OFF scans (\'otf\' or \'otfraster\')')
            
            # all spectra seems to be ON, so try to analyze
            # observing pattern
            #caltype = self.__observing_pattern_analysis(filename)
        return caltype

    def __observing_pattern_analysis(self, filename):
        """
        Return calibration type 'otf' or 'otfraster' depending
        on observing pattern.
        """
        import numpy
        caltype = 'otf'

        with casatools.TableReader(filename) as tb:
            tb2 = tb.query('',sortlist='TIME')
            times = tb.getcol('TIME') * 86400.0
            intervals = tb.getcol('INTERVAL')
            tb2.close()
            del tb2
        # this is preliminary
        # if there are any clear time gap, it may be a raster scan
        separation = times[1:] - times[:-1]
        real_separation = separation.take(separation.nonzero()[0])
        average_interval = numpy.median(intervals)
        gaplist = real_separation.take(numpy.where(real_separation > 3.0 * average_interval)[0])
        if len(gaplist) > 0:
            caltype = 'otfraster'

        return caltype


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
    
