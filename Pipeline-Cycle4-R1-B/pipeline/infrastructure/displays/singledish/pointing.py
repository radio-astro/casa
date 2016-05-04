from __future__ import absolute_import

import os
import math
import pylab as pl
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from . import common
from . import drawpointing
from pipeline.domain.datatable import OnlineFlagIndex

LOG = infrastructure.get_logger(__name__)

class SDPointingDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8905
    AxesManager = drawpointing.PointingAxesManager

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        vis = parent_ms.basename
        # target_spws = [spwid for (spwid, spwobj) in st.spectral_window.items()
        #                if spwobj.is_target and spwobj.nchan != 4]
        target_spws = self.context.observing_run.get_spw_for_science(st.basename)
        spwid = target_spws[0]
        beam_size = casatools.quanta.convert(st.beam_size[spwid], 'deg')
        obs_pattern = st.pattern[spwid].values()[0]
        rows = self.datatable.get_row_index(idx, spwid)
        datatable = self.datatable
        
        plots = []
        
        #ROW = datatable.getcol('ROW')
        tRA = datatable.tb1.getcol('RA').take(rows)
        tDEC = datatable.tb1.getcol('DEC').take(rows)
        tNCHAN = datatable.tb1.getcol('NCHAN').take(rows)
        tSRCTYPE = datatable.tb1.getcol('SRCTYPE').take(rows)
        tFLAG = datatable.tb2.getcol('FLAG_PERMANENT').take(rows, axis=1)[OnlineFlagIndex]
        
        full_pointing_index = numpy.where(tNCHAN > 1)
        
        FLAG = tFLAG[full_pointing_index]
        RA = tRA[full_pointing_index]
        DEC = tDEC[full_pointing_index]
        plotfile = os.path.join(stage_dir, 'pointing_full_%s.png'%(st.basename))
        self.draw_radec(RA, DEC, FLAG, plotfile, circle=[0.5*beam_size['value']], ObsPattern=obs_pattern, plotpolicy='greyed')
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'full pointing'
        parameters['file'] = st.basename
        parameters['vis'] = vis
        plots.append(logger.Plot(plotfile,
                                 x_axis='R.A.', y_axis='Dec.',
                                 field=parent_ms.fields[0].name,
                                 parameters=parameters))
        
        srctype = st.calibration_strategy['srctype']
        onsource_pointing_index = numpy.where(numpy.logical_and(tNCHAN > 1, tSRCTYPE == srctype))
        
        FLAG = tFLAG[onsource_pointing_index]
        RA = tRA[onsource_pointing_index]
        DEC = tDEC[onsource_pointing_index]
        plotfile = os.path.join(stage_dir, 'pointing_onsource_%s.png'%(st.basename))
        self.draw_radec(RA, DEC, FLAG, plotfile, circle=[0.5*beam_size['value']], ObsPattern=obs_pattern, plotpolicy='greyed')
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'on source pointing'
        parameters['file'] = st.basename
        parameters['vis'] = vis
        plots.append(logger.Plot(plotfile,
                                 x_axis='R.A.', y_axis='Dec.',
                                 field=parent_ms.fields[0].name,
                                 parameters=parameters))
        return plots
        
    def draw_radec(self, RA, DEC, FLAG=None, plotfile=None, connect=True, circle=[], ObsPattern=False, plotpolicy='ignore'):
        """
        Draw loci of the telescope pointing
        RA: horizontal coordinate value
        DEC: vertical coordinate value
        FLAG: flag info. (1: Valid, 0: Invalid)
        xaxis: extension header keyword for RA
        yaxis: extension header keyword for DEC
        connect: connect points if True
        plotpolicy: plot policy for flagged data
                    'ignore' -- ignore flagged data
                    'plot' -- plot flagged data with same color as unflagged
                    'greyed' -- plot flagged data with grey color
        """
        drawpointing.draw_pointing(self.axes_manager, RA, DEC, FLAG, plotfile, connect, circle, ObsPattern, plotpolicy)
        

