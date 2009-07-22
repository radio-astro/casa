"""Pipeline Heuristics CheckMS class.
"""

# History:
# 16-Jul-2007 jfl First version.
#  9-Aug-2007 jfl facecolor parameter named in class to .fill()
# 30-Aug-2007 jfl Flux calibrating release.
#  6-Nov-2007 jfl Best bandpass release.
# 20-Mar-2008 jfl BookKeeper release.
# 25-Jun-2008 jfl regression release.
# 26-Jun-2008 jfl improve chart layout.
# 26-Sep-2008 jfl mosaic release.
# 12-Dec-2008 jfl 12-dec release.
# 12-Dec-2008 jfl 15-dec datetime.fromtimestamp fix.
# 19-Dec-2008 jfl times stored as epoch measures.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
# 15-Jun-2009 jfl add uv plots, use GHz and MHz.

# standard library modules

import datetime
import fnmatch
import math
import matplotlib.dates as mdates
import re
import string

# package modules

from numpy import *
import pylab

# alma modules

import util

###########################################################################
#################  checkMS  ############################################
#
# todo: do a separate chart and heuristics of each obs_id

class CheckMS:
    """Class to check the content of a MS."""

    def __init__(self, tools, tasks, htmlLogger, msfile):
        """Constructor.

        Keyword arguments:
        tools      -- set of casapy tools
        htmlLogger -- Conduit to html structure.
        msfile     -- the name of the MeasurementSet to be checked.

        Method:

        """
        tools.copy(self)
        self._tools = tools
        self._tasks = tasks
        self._htmlLogger = htmlLogger
        self._htmlLogger.timing_start('CheckMS.__init__')
	self._msfile = msfile
        self.read_fieldlist()
        self.read_observationlist()
        self.read_specwinlist()
        self.read_time_sequence()
        self._htmlLogger.timing_stop('CheckMS.__init__')


    def __del__(self):
        """Destructor.
        """


    def _plot_antenna_positions(self):
        """Plot the antenna positions.
        """
        self._htmlLogger.logHTML('<hr>')
        self._htmlLogger.logHTML('<h2 ALIGN="LEFT">  Antenna Positions: </h2>')
        self._htmlLogger.logHTML('''<img WIDTH="800" HEIGHT="600"
         ALIGN="CENTER" BORDER="0" SRC="antenna_positions.png">''')

        pylab.figure()
        pylab.clf()
        plotName = 'Antenna Positions'
        plotFile = 'html/antenna_positions.png'

# read the antenna positions. These seem to be in metres rel to centre of the
# Earth.

        self._table.open('%s/ANTENNA' % self._msfile)
        position = self._table.getcol('POSITION')
        flag_row = self._table.getcol('FLAG_ROW')
        position_keywords = self._table.getcolkeywords('POSITION')
        self._table.close()
 
# Make a position Measure for each antenna, this stores info in terms of long, lat
# and distance from centre of Earth

        antennas = {}
        for ant in arange(shape(position)[1]):
            if not(flag_row[ant]):
                ant_pos = self._measures.position(
                 rf=position_keywords['MEASINFO']['Ref'],
                 v0=self._quanta.quantity(position[0,ant],
                 position_keywords['QuantumUnits'][0]),
                 v1=self._quanta.quantity(position[1,ant],
                 position_keywords['QuantumUnits'][1]),
                 v2=self._quanta.quantity(position[2,ant],
                 position_keywords['QuantumUnits'][2]))

                antennas[ant] = ant_pos

# store the longs and lats of the antennas in lists - convert ot canonical 
# units

        longs = []
        lats = []
        radii = []
        names = []
        for ant in antennas.keys():
            position = antennas[ant]
            radius = position['m2']['value']
            radius_unit = position['m2']['unit']
            radius_quantum = self._quanta.quantity(radius, radius_unit)
            radius_quantum = self._quanta.convert(radius_quantum, 'm')
            radius = self._quanta.getvalue(radius_quantum)

            long = position['m0']['value']
            long_unit = position['m0']['unit']
            long_quantum = self._quanta.quantity(long, long_unit)
            long_quantum = self._quanta.convert(long_quantum, 'rad')
            long = self._quanta.getvalue(long_quantum)

            lat = position['m1']['value']
            lat_unit = position['m1']['unit']
            lat_quantum = self._quanta.quantity(lat, lat_unit)
            lat_quantum = self._quanta.convert(lat_quantum, 'rad')
            lat = self._quanta.getvalue(lat_quantum)

            longs.append(long)
            lats.append(lat)
            radii.append(radius)
            names.append(ant)

# calculate a mean longitude and get the antenna longs relative to that
# - otherwise get numerical errors

        longs = array(longs)
        longs -= mean(longs)
        lats = array(lats)
        radii = array(radii)

# multiply longs by cos(lat) and radius to convert to metres

        x = longs * cos(lats) * radii
        y = lats * radii

# make x,y relative to 'centre' of array

        x -= mean(x)
        y -= mean(y)

# plot

        pylab.figure()
        pylab.clf()
        pylab.subplot(1,1,1)

        for ant in antennas.keys():
            pylab.scatter([x[ant]], [y[ant]], s=10, c='blue', marker='o')
            pylab.text(x[ant], y[ant], str(ant), ha='right', va='center')
        pylab.axis('equal')

        pylab.title(plotName)
        pylab.xlabel('X (m)')
        pylab.ylabel('Y (m)')

        pylab.savefig(plotFile)
        pylab.clf()
        pylab.close()

        pylab.ioff()


    def _plot_uv_coverage(self):
        """Plot the uv coverage for each field.
        """
        self._htmlLogger.logHTML('<hr>')
        if self._tasks.has_key('plotxy') and self._tasks.has_key('default'):
            self._htmlLogger.logHTML('<h2 ALIGN="LEFT">  U-V Coverage: </h2>')
            plotxy = self._tasks['plotxy']
            default = self._tasks['default']
            for field in range(self._nfields):
                pylab.figure()
                pylab.clf()
                plotName = 'Field %s (%s) - u,v coverage' % (
                 self._fieldlist[field]._name, self._fieldlist[field]._type)
                plotFile = 'html/uv_coverage_field_%s.png' % field
                default(plotxy)
                plotxy(vis=self._msfile, xaxis='u', yaxis='v', field=str(field),
                 interactive=False, figfile=plotFile, selectplot=True, 
                 title=plotName)
                pylab.clf()
                pylab.close()
                self._htmlLogger.appendNode(plotName, plotFile)
            pylab.ioff()
            self._htmlLogger.flushNode()

        else:
            self._htmlLogger.logHTML('<h2 ALIGN="LEFT">  U-V Coverage: </h2>')
            self._table.open(self._msfile)
            for field in range(self._nfields):
                pylab.figure()
                pylab.clf()
                plotName = 'Field %s (%s) - u,v coverage' % (
                 self._fieldlist[field]._name, self._fieldlist[field]._type)
                plotFile = 'html/uv_coverage_field_%s.png' % field

                s = self._table.query('FIELD_ID==%s && NOT(FLAG_ROW)' % field)
                uvw = s.getcol('UVW')

                pylab.figure()
                pylab.clf()
                pylab.subplot(1,1,1)

                print 'shape', shape(uvw)
                pylab.title(plotName)
                if len(uvw) > 0:
                    pylab.scatter(uvw[0,:], uvw[1,:], facecolor='blue', 
                     edgecolor='blue', s=1, marker='o')
                    pylab.axis('equal')

                    pylab.xlabel('U (m)')
                    pylab.ylabel('V (m)')
                else:
                    pylab.annotate('no data', (0.5,0.5), xycoords='axes fraction')

                pylab.savefig(plotFile)
                pylab.clf()
                pylab.close()
                self._htmlLogger.appendNode(plotName, plotFile)
            pylab.ioff()
            self._table.close()
            self._htmlLogger.flushNode()


    def writeHtmlSummary(self):
        """Methd to write a summary of the MS to HTML.
        """
#        print 'checkMSv2.htmlSummary called'

        self._htmlLogger.openNode('<li>MeasurementSet Summary', 'msSummary',
         True)

        self._htmlLogger.logHTML("<center><h1>Summary of %s</h1></center>" %
         self._msfile)    
        self._htmlLogger.logHTML('<hr><h2 ALIGN="LEFT">  OBSERVATIONS: </h2>')
        self._htmlLogger.logHTML(
         '<table CELLPADDING="5" BORDER="1">')
        align = ' ALIGN="CENTER" '
        colspan = ' COLSPAN="1" '
        self._htmlLogger.logHTML("<tr><td"+align+colspan+"> OBS_ID </td>") 
        self._htmlLogger.logHTML("<td"+align+colspan+"> ARRAY </td>")
        self._htmlLogger.logHTML("<td"+align+colspan+"> t_start </td>") 
        self._htmlLogger.logHTML("<td"+align+colspan+"> t_end </td></tr>")
        for iobs in range(len(self._observationlist)):
           self._observationlist[iobs]._info2html(self._htmlLogger)
        self._htmlLogger.logHTML("</table>")

        self._htmlLogger.logHTML('<hr><h2 ALIGN="LEFT">  CHART: </h2>')
        self._htmlLogger.logHTML('<img WIDTH="800" HEIGHT="600" ')
        self._htmlLogger.logHTML(' ALIGN="CENTER" BORDER="0" SRC="chart.png">')
        self.plot_chart('html/chart.png')
#        self._htmlLogger.logHTML("""Black boxes show the time limits of scans, or 
#         places where the exposures in a scan are not contiguous.""")

        self._plot_antenna_positions()

        self._plot_uv_coverage()

        for ifield in range(self._nfields):
            self._htmlLogger.logHTML(
             '<hr><h2 ALIGN="LEFT">  FIELD_ID = %s</h2>' % ifield)
            self._fieldlist[ifield]._info2html(self._htmlLogger)

        self._htmlLogger.logHTML(
         '<hr><h2 ALIGN="LEFT">  SPECTRAL WINDOWS: </h2>')
        self._htmlLogger.logHTML('<table CELLPADDING="5" BORDER="1">')
        align = ' ALIGN="CENTER" '
        colspan = ' COLSPAN="1" '
        self._htmlLogger.logHTML("<tr><td"+align+colspan+"> SPWID </td>") 
        self._htmlLogger.logHTML("<td"+align+colspan+"> REF_FREQ (GHz) </td>")
        self._htmlLogger.logHTML("<td"+align+colspan+"> BANDWIDTH (MHz) </td>") 
        self._htmlLogger.logHTML("<td"+align+colspan+"> nchannels </td></tr>")
        for ispecw in range(self._nspecwin):           
           self._specwinlist[ispecw]._info2html(self._htmlLogger)
        
        self._htmlLogger.closeNode()        
        

    def read_fieldlist(self):
        """Get the list of fields and their type.
        """
        self._fieldlist = []

        # open field table
        rtn = self._table.open(self._msfile + '/FIELD')

        # get field names
        field_names = list(self._table.getcol('NAME'))
        self._nfields = len(field_names)        

        # get field types (kind of calibrator or target)

        source_types = list(util.util.get_source_types(self._table))

        if (len(source_types)) == 0:
           source_types = ["UNKNOWN"]*self._nfields
        
        # get field coords (PHASE_DIR)
        phase_dir = self._table.getcol('PHASE_DIR')
        phase_dir_keywords = self._table.getcolkeywords('PHASE_DIR')

        field_dirs = []
        for field in range(self._nfields):
            dir_measure = self._measures.direction(
             phase_dir_keywords['MEASINFO']['Ref'],
             '%s%s' % (phase_dir[0,0,field],
             phase_dir_keywords['QuantumUnits'][0]),
             '%s%s' % (phase_dir[1,0,field],
              phase_dir_keywords['QuantumUnits'][1]))

            field_dirs.append(dir_measure)

        self._table.close()
        
        for ifield in range(self._nfields):
           st = source_types[ifield]
           st = st.strip()
           st = st.upper()

           tmpfield = Field(self._tools, ifield, field_names[ifield], st,
            field_dirs[ifield])
           self._fieldlist.append(tmpfield)
 
        

    def read_observationlist(self):
        """Get the list of telescope arrays.
        

        """
        self._observationlist = []

        # open observation table
        rtn = self._table.open(self._msfile + '/OBSERVATION')

        # get telescope names
        array_names = list(self._table.getcol('TELESCOPE_NAME'))
        self._nobs=len(array_names)
        # get  time ranges
# - column does not seem to be set correctly for PdB data
#        time_ranges =  list(self._table.getcol('TIME_RANGE'))
#        time_colkeywords = self._table.getcolkeywords('TIME_RANGE')
#        t_units = time_colkeywords['QuantumUnits'][0]
#        t_ref = time_colkeywords['MEASINFO']['Ref']
        
        # close observation table
        rtn = self._table.close()

#        self._start_time = self._measures.epoch(t_ref,
#         self._quanta.quantity(time_ranges[0][0], t_units))
#        print 'ranges', time_ranges
#        self._end_time = self._measures.epoch(t_ref,
#         self._quanta.quantity(time_ranges[1][0], t_units))

#       TIME_RANGES not set correctly for PdB data, so get time range for
#       each observation directly

        self._table.open(self._msfile)
        time_colkeywords = self._table.getcolkeywords('TIME')
        t_units = time_colkeywords['QuantumUnits'][0]
        t_ref = time_colkeywords['MEASINFO']['Ref']
 
        for iobs in range(self._nobs):
           subTable = self._table.query('OBSERVATION_ID==%s' % iobs)
           t = subTable.getcol('TIME')
           interval = subTable.getcol('INTERVAL')
           t0 = min(t - interval/2.0)
           t1 = max(t + interval/2.0)   
           t0 = self._measures.epoch(t_ref, self._quanta.quantity(t0, t_units))
           t1 = self._measures.epoch(t_ref, self._quanta.quantity(t1, t_units))

           tmpobs = Observation(self._tools, iobs, array_names[iobs], t0, t1)
           self._observationlist.append(tmpobs)

        self._table.close() 



    def read_specwinlist(self):
        """Get the list of spectral windows.
        

        """
        self._specwinlist = []

        # open observation table
        rtn = self._table.open(self._msfile + '/SPECTRAL_WINDOW')

        # get reference frequencies
        ref_freqs = self._table.getcol('REF_FREQUENCY')
        ref_freq_unit = self._table.getcolkeyword('REF_FREQUENCY',
         'QuantumUnits')[0]
        ref_freqs_ghz = []
        for freq in ref_freqs:
            ref_freq_quantum = self._quanta.quantity(freq, ref_freq_unit)
            ref_freq_quantum = self._quanta.convert(ref_freq_quantum, 'GHz')
            ref_freqs_ghz.append(self._quanta.getvalue(ref_freq_quantum))
        self._nspecwin=len(ref_freqs)

        # get  band widths
        bandwidths = list(self._table.getcol('TOTAL_BANDWIDTH'))
        bandwidth_unit = self._table.getcolkeyword('TOTAL_BANDWIDTH',
         'QuantumUnits')[0]
        bandwidths_mhz = []
        for band in bandwidths:
            bandwidth_quantum = self._quanta.quantity(band, bandwidth_unit)
            bandwidth_quantum = self._quanta.convert(bandwidth_quantum, 'MHz')
            bandwidths_mhz.append(self._quanta.getvalue(bandwidth_quantum))

        # get  number of channels
        nchannels = list(self._table.getcol('NUM_CHAN'))
        
        self._table.close()
             
        for ispecw in range(self._nspecwin):
           tmpspecwin = Specwin(ispecw, ref_freqs_ghz[ispecw],
            bandwidths_mhz[ispecw], nchannels[ispecw])
           self._specwinlist.append(tmpspecwin)
 


    def read_time_sequence(self) :
        """Get time sequence of the observations.
           For each field find the times of each chunk 
           of continuous observation.
        
        """

        # open data_description table
        rtn = self._table.open(self._msfile + '/DATA_DESCRIPTION')

        # get specwin_ids
        specwin_ids = list(self._table.getcol('SPECTRAL_WINDOW_ID'))
        # close data_description table
        rtn = self._table.close()

        self._start_time = None
        self._end_time = None
        self._time_intervals = []
        self._scan_intervals = []
        rtn = self._table.open(self._msfile)
        time_colkeywords = self._table.getcolkeywords('TIME')
        time_unit = time_colkeywords['QuantumUnits'][0]
        time_ref = time_colkeywords['MEASINFO']['Ref']
         
        for ifield in range(self._nfields) : 
          #for iobs in range(self._nobs) :
           queryexpr = string.join (['FIELD_ID in ', str(ifield)], '')
           data = self._table.queryC( queryexpr,'','TIME','' )
           time = list(data.getcol('TIME'))
           ant1= list(data.getcol('ANTENNA1'))
           ant2= list(data.getcol('ANTENNA2'))
           obs_id= list(data.getcol('OBSERVATION_ID'))
           dd_id = list(data.getcol('DATA_DESC_ID'))
           expo = list(data.getcol('EXPOSURE'))

           expomed = pylab.median(array(expo))
           scan= list(data.getcol('SCAN_NUMBER'))
           dt = []
           timeinterv = []
           tmptimeint =  [time[0]-expo[0]/2.0]
           if self._start_time == None:
               self._start_time = tmptimeint[0]
               self._end_time = tmptimeint[0]
           else:
               self._start_time = min(self._start_time, tmptimeint[0])
               self._end_time = max(self._end_time, tmptimeint[0])

           antlist = []
           spwidlist = []
           for i in range(len(time)-1):
             if not(ant1[i] in antlist):
                antlist.append(ant1[i])
             if not(ant2[i] in antlist):
                antlist.append(ant2[i])
             if not(specwin_ids[dd_id[i]] in spwidlist):
                spwidlist.append(specwin_ids[dd_id[i]])
             if ((time[i+1]-expo[i+1]/2.)-(time[i]+expo[i]/2.) > expomed/100.) or (scan[i+1] != scan[i]):
		 dt.append(time[i+1]-expo[i+1]/2.-time[i]-expo[i]/2.)
	         tmptimeint.append(time[i]+expo[i]/2.0)

                 if self._start_time == None:
                    self._start_time = tmptimeint[1]
                    self._end_time = tmptimeint[1]
                 else:
                    self._start_time = min(self._start_time, tmptimeint[1])
                    self._end_time = max(self._end_time, tmptimeint[1])

                 timeinterv.append(tmptimeint)
		 tmptimeint = [time[i+1]-expo[i+1]/2.0]
                 if scan[i+1] != scan[i]:
                    tmpscan = Scan(self._tools, scan[i])
                    tmpscan.set_time_intervals(timeinterv, time_ref, time_unit)
                    tmpscan.set_specwin_ids(spwidlist)
                    tmpscan.set_observation(obs_id[i],self._observationlist[obs_id[i]]._start_time)
                    tmpscan.set_antlist(antlist)
                    tmpscan.set_array_name(self._observationlist[obs_id[i]]._telescope)
                    self._fieldlist[ifield].append_scan(tmpscan)	   
                    timeinterv = []
                    antlist = []
                    spwidlist = []

           ilast = len(time)-1
	   tmptimeint.append(time[ilast]+expo[ilast]/2.0)
           timeinterv.append(tmptimeint)
           tmpscan = Scan(self._tools, scan[ilast])
           tmpscan.set_time_intervals(timeinterv, time_ref, time_unit)
           tmpscan.set_specwin_ids(spwidlist)
           tmpscan.set_observation(obs_id[ilast],self._observationlist[obs_id[ilast]]._start_time)
           tmpscan.set_antlist(antlist)
           tmpscan.set_array_name(self._observationlist[obs_id[ilast]]._telescope)
           self._fieldlist[ifield].append_scan(tmpscan)
           timeinterv = []
           antlist = []
           spwidlist = []

        rtn = self._table.close()
        self._start_time = self._measures.epoch(time_ref, 
         self._quanta.quantity(self._start_time, time_unit))
        self._end_time = self._measures.epoch(time_ref, 
         self._quanta.quantity(self._end_time, time_unit))


    def plot_chart(self, filename) :
        """Plot time sequence of the observations on a chart.
           
        Keyword arguments:
        """

        pylab.ioff()
        f = pylab.figure()
        pylab.clf()
        pylab.axes([0.1,0.15,0.8,0.7])
        time_ref = self._measures.getref(self._start_time)
        dateString = self._quanta.time(self._measures.getvalue(
         self._start_time)['m0'], form=['dmy','no_time','clean'])

# base_time is the number of seconds up to the start of the day.

        base_time = self._measures.getvalue(self._start_time)['m0']
        base_time = self._quanta.convert(base_time, 'd')
        base_time = self._quanta.floor(base_time)
        pylab.ylabel('FIELD_ID')

        datemin = None
        datemax = None

        for ifield in range(self._nfields):
           colours = []
           if (self._fieldlist[ifield].isoftype("*BANDPASS*")):
             colours.append('red')
           if  (self._fieldlist[ifield].isoftype("*GAIN*")):
             colours.append('green')
           if (self._fieldlist[ifield].isoftype("*FLUX*")):
             colours.append('blue')
           if  (self._fieldlist[ifield].isoftype("*SOURCE*")):
             colours.append('cyan')
           if len(colours) == 0:
               colours = ['magenta']

# all 'datetime' objects are in UTC.

           intervals = self._fieldlist[ifield].get_scantimes()
           for iinterv in range(len(intervals)):
               x0 = self._measures.getvalue(intervals[iinterv][0])['m0']
               x0 = self._quanta.sub(x0, base_time)
               x0 = self._quanta.convert(x0, 's')
               x0 = datetime.datetime.utcfromtimestamp(
                self._quanta.getvalue(x0))
               x1 = self._measures.getvalue(intervals[iinterv][1])['m0']
               x1 = self._quanta.sub(x1, base_time)
               x1 = self._quanta.convert(x1, 's')
               x1 = datetime.datetime.utcfromtimestamp(
                self._quanta.getvalue(x1))
               y0 = ifield-0.5
               y1 = ifield+0.5

               height = (y1 - y0) / float(len(colours))
               ys = y0
               ye = y0 + height
               for colour in colours:
                   pylab.gca().fill([x0,x1,x1,x0],[ys,ys,ye,ye],
                    facecolor=colour, edgecolor=colour)
                   ys += height
                   ye += height
               if datemin == None:
                   datemin = x0
               else:
                   datemin = min(datemin,x0)
               if datemax == None:
                   datemax = x0
               else:
                   datemax = max(datemax,x1)

        datemin = datemin.replace(minute=0, second=0, microsecond=0)
        datemax = datemax.replace(hour=datemax.hour + 1, minute=0, second=0,
         microsecond=0)

# next bit cribbed from date_demo pylab example
# do not FIDDLE with this as it took ages to get it working. If it stops
# working try putting a 'print' of the the xticklabels in to force it to 
# calculate something.

        pylab.gca().set_xlim(datemin, datemax)
        hours = mdates.HourLocator(interval=1)
        minutes = mdates.MinuteLocator(interval=10)
        pylab.gca().xaxis.set_major_locator(hours)
        pylab.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Hh%Mm'))
        pylab.gca().xaxis.set_minor_locator(minutes)
        pylab.gca().set_xlabel('Time (after start %s)' % dateString)        
        f.autofmt_xdate()

# plot key

        pylab.axes([0.1,0.8,0.8,0.2])
        lims = pylab.axis()
        pylab.axis('off')

        pylab.gca().fill([0.0,0.05,0.05,0.0], [0.4,0.4,0.6,0.6],
         facecolor='red', edgecolor='red')
        pylab.text(0.06, 0.5, 'BANDPASS', va='center')

        pylab.gca().fill([0.25,0.30,0.30,0.25], [0.4,0.4,0.6,0.6],
         facecolor='green', edgecolor='green')
        pylab.text(0.31, 0.5, 'GAIN', va='center')

        pylab.gca().fill([0.45,0.50,0.50,0.45], [0.4,0.4,0.6,0.6],
         facecolor='blue', edgecolor='blue')
        pylab.text(0.51, 0.5, 'FLUX', va='center')

        pylab.gca().fill([0.65,0.7,0.7,0.65], [0.4,0.4,0.6,0.6],
         facecolor='cyan', edgecolor='cyan')
        pylab.text(0.71, 0.5, 'SOURCE', va='center')

        pylab.gca().fill([0.85,0.9,0.9,0.85], [0.4,0.4,0.6,0.6],
         facecolor='magenta', edgecolor='magenta')
        pylab.text(0.91, 0.5, 'OTHER', va='center')

        pylab.axis(lims)

        pylab.savefig(filename)
        pylab.clf()
        pylab.close()


###########################################################################
#################  field  ############################################
#
# todo:

class Field:
    """Class containing info about a single field."""

    def __init__(self, tools, id, name, type, direction):
        """Constructor.

        Keyword arguments:
        tools -- set of casapy tools
        id -- integer corresponding to the id of the field.
        name -- string containing the name of the field.
        type -- string containing the type of the field 
                (e.g. BANDPASS, FLUX, GAIN).
        direction -- direction measure holding the 
                     PHASE_DIR of the field.


        """

        tools.copy(self)
        self._id = id
	self._name = name
        self._type = type
        self._direction = direction

# initializations 

	self._scanlist = []


    def __str__(self):
        outstr="field: "+str(self._name)+" type: "+str(self._type)+"\n"
        outstr=outstr+"-----------"+"\n"
        for iscan in range(len(self._scanlist)):
           outstr=outstr+str(self._scanlist[iscan])+"\n"
        return outstr

    def _info2html(self, htmlLogger):
        """ append info to the html summary
                
        Keyword arguments:

        """
        htmlLogger.logHTML("<h3>  NAME           : "+str(self._name)+"</h3>")
        htmlLogger.logHTML("<h3>  TYPE           : "+str(self._type)+"</h3>")
        measRef= self._measures.getref(self._direction)
        if measRef=='J2000' or measRef=='B1950':
            dir = self._measures.getvalue(self._direction)
            ra = dir['m0']
            dec = dir['m1']
            htmlLogger.logHTML("<h3>  PHASE DIRECTION: "+
             self._quanta.angle(ra, form=['time', 'clean']) +
             self._quanta.angle(dec, form=['clean']) + ' ' + measRef)
        else:
            htmlLogger.logHTML("<h3>  PHASE DIRECTION: "+
             self._measures.dirshow(self._direction)+"</h3>")
        htmlLogger.logHTML('<table CELLPADDING="5" BORDER="1">')
        align = ' ALIGN="CENTER" '
        colspan1 = ' COLSPAN="1" '
        colspan2 = ' COLSPAN="9" '

        tmpobsid = -1

        for iscan in range(len(self._scanlist)):
            if self._scanlist[iscan]._observation_id != tmpobsid:
                tmpobsid = self._scanlist[iscan]._observation_id
                obsDate = self._quanta.time(self._measures.getvalue(
                 self._scanlist[iscan]._observation_start_time)['m0'],
                 form=['dmy', 'no_time', 'clean']) + ' ' +\
                 self._measures.getref(
                 self._scanlist[iscan]._observation_start_time)

                htmlLogger.logHTML("<tr><td"+align+colspan2+
                 "> observation date: "+obsDate+
                 "</td></tr>")
                htmlLogger.logHTML("<tr>")
                htmlLogger.logHTML("<td"+align+colspan1+"> SCAN </td>") 
                htmlLogger.logHTML("<td"+align+colspan1+"> ARRAY </td>")
                htmlLogger.logHTML("<td"+align+colspan1+"> Nant </td>") 
                htmlLogger.logHTML("<td"+align+colspan1+"> SPWID </td>")
                htmlLogger.logHTML("<td"+align+colspan1+"> t_start </td>") 
                htmlLogger.logHTML("<td"+align+colspan1+"> t_end </td>") 
                htmlLogger.logHTML("<td"+align+colspan1+"> t_on (sec) </td>") 
                htmlLogger.logHTML("</tr>")

            self._scanlist[iscan]._info2html(htmlLogger)

        htmlLogger.logHTML("</table>")


    def isoftype(self,source_type):
        """ append a scan to the scan list
                
        Keyword arguments:
        source_type -- string giving the type of source to be compared against


        """
        pattern = fnmatch.translate(source_type)
	return (re.match(pattern, self._type))


    def append_scan(self,scan):
        """ append a scan to the scan list
                
        Keyword arguments:
        name -- the scan instance


        """
	self._scanlist.append(scan)


    def get_scantimes(self):
        """ return the starting and ending time of each
            scan in a list of lists of float
                
        Keyword arguments:


        """
        
        timeslist = []
        tmplist = []
	for iscan in range(len(self._scanlist)):
           tmplist.append(self._scanlist[iscan]._start_time)
           tmplist.append(self._scanlist[iscan]._end_time)
           timeslist.append(tmplist)
           tmplist = []

        return timeslist


    def get_spwids(self):
        """ return the list of spwids for the field
                
        Keyword arguments:


        """
        
        spwidlist = []
        #print self._scanlist[0]._specwin_ids
        #print range(len(self._scanlist))
	for iscan in range(len(self._scanlist)):
            tmpspwlist = self._scanlist[iscan]._specwin_ids
            for ispw in range(len(tmpspwlist)):
                if not(tmpspwlist[ispw] in spwidlist):
                    spwidlist.append(tmpspwlist[ispw])
            
        #print 'spwidlist=',spwidlist
        return spwidlist

###########################################################################
#################  scan  ############################################
#
# todo:

class Scan:
    """Class containing info about a single scan."""

    def __init__(self, tools, number):
        """Constructor.

        Keyword arguments:
        number -- integer corresponding to the scan number.
        tools      -- set of casapy tools
        """

        tools.copy(self)
	self._number = number

# initializations 

        self._time_intervals = []



    def __str__(self):
        outstr = "scan nb: "+str(self._number)+"\n"
        outstr = outstr + "  starts:"+str(self._start_time)+"\n"
        outstr = outstr + "  ends:"+str(self._end_time)+"\n"
        outstr = outstr + "  intervals:"+str(self._time_intervals)+"\n"
        return outstr

    def _info2html(self, htmlLogger):
        """ append info to the html summary
                
        Keyword arguments:

        """
        align1 = ' ALIGN="CENTER" VALIGN="TOP" '
        align2 = ' ALIGN="LEFT" VALIGN="TOP" '
        w1 = 'WIDTH="50" '
        w2 = 'WIDTH="100" '

        start_time = self._quanta.time(self._measures.getvalue(
         self._start_time)['m0'], form=['clean'])
        end_time = self._quanta.time(self._measures.getvalue(
         self._end_time)['m0'], form=['clean'])
        htmlLogger.logHTML("<tr>")
        htmlLogger.logHTML("<td"+align1+w1+">"+str(self._number)+"</td>") 
        htmlLogger.logHTML("<td"+align1+w1+">"+self._array_name+"</td>") 
        htmlLogger.logHTML("<td"+align1+w1+">"+str(len(self._antlist))+"</td>") 
        htmlLogger.logHTML("<td"+align1+w1+">"+str(self._specwin_ids)+"</td>") 
        htmlLogger.logHTML("<td"+align2+w2+">%s</td>"%start_time)
        htmlLogger.logHTML("<td"+align2+w2+">%s</td>"%end_time)
        htmlLogger.logHTML("<td"+align2+w2+">%2.f</td>"%self._time_onsource)
        htmlLogger.logHTML("</tr>")


    def set_time_intervals(self, timeinterv, time_ref, time_unit):
        """ set the time intervals of the scan
                
        Keyword arguments:
        timeinterv -- list of float lists giving the time intervals
        time_ref   -- String defining time reference
        time_unit  -- string giving time unit
        """
        #print str(timeinterv)
	self._time_intervals = timeinterv
        self._ngaps = len(timeinterv)-1
        if self._ngaps > 0:
           tmplist = []
           for igap in range(self._ngaps):
	      tmplist.append(timeinterv[igap+1][0]-timeinterv[igap][1])
           self._gap_median =  pylab.median(array(tmplist))
        else :
           self._gap_median = -1
        self._time_onsource = 0.0
        for iinterv in range(len(timeinterv)):
           self._time_onsource += (timeinterv[iinterv][1]-timeinterv[iinterv][0])

# convert start end times to measures

        for k in range(len(self._time_intervals)):
            time_interval = self._time_intervals[k]
            t0 = self._measures.epoch(time_ref, 
             self._quanta.quantity(time_interval[0], time_unit))
            t1 = self._measures.epoch(time_ref, 
             self._quanta.quantity(time_interval[1], time_unit))
            self._time_intervals[k] = [t0,t1]

        self._start_time = self._time_intervals[0][0]
        self._end_time = self._time_intervals[-1][1]


    def set_array_name(self,array_name):
        """ set the array name of the scan
                
        Keyword arguments:
        array_name -- string corresponding to the array name 


        """
        self._array_name = array_name


    def set_specwin_ids(self,specwin_ids):
        """ set the list of spectral ids of the scan
                
        Keyword arguments:
        specwin_ids -- list of spectral window ids


        """
        self._specwin_ids = specwin_ids

    def set_observation(self,observation_id, observation_start_time):
        """ set the spectral array id of the scan
                
        Keyword arguments:
        observation_id -- integer corresponding to the 
                          scan observation id
         observation_start_time -- starting time of 
                                   observation block (not of scan!)

        """
        self._observation_id = observation_id
        self._observation_start_time = observation_start_time


    def set_antlist(self,antlist):
        """ set the spectral array id of the scan
                
        Keyword arguments:
        antlist -- list of antenna ids corresponding to the scan


        """
        self._antlist = antlist


###########################################################################
#################  observation  ############################################
#
# todo:

class Observation:
    """Class containing info about a single observation."""

    def __init__(self, tools, id, telescope, start_time, end_time):
        """Constructor.

        Keyword arguments:
        tools      -- set of casapy tools
        id -- integer corresponding to the observation id
        telescope -- string corresponding to the telescope name.
        start_time -- epoch Measure giving starting time.
        end_time  -- epoch Measure giving ending time.


        """
        tools.copy(self)
        self._id = id
	self._telescope = telescope
	self._start_time = start_time
	self._end_time = end_time




    def __str__(self):
        outstr = "telescope: "+self._telescope+"\n"
        outstr = outstr + "  starts:"+str(self._start_time)+"\n"
        outstr = outstr + "  ends:"+str(self._end_time)+"\n"
        return outstr

    def _info2html(self, htmlLogger):
        """ append info to the html summary
                
        Keyword arguments:

        """
        align1 = ' ALIGN="CENTER" VALIGN="TOP" '
        align2 = ' ALIGN="LEFT" VALIGN="TOP" '
        w1 = 'WIDTH="50" '
        w2 = 'WIDTH="100" '

        start_time = self._quanta.time(self._measures.getvalue(
         self._start_time)['m0'], form=['dmy', 'clean']) + ' ' +\
         self._measures.getref(self._start_time)
        end_time = self._quanta.time(self._measures.getvalue(
         self._end_time)['m0'], form=['dmy', 'clean']) + ' ' + \
         self._measures.getref(self._end_time)
        htmlLogger.logHTML("<tr>")
        htmlLogger.logHTML("<td"+align1+w1+">"+str(self._id)+"</td>") 
        htmlLogger.logHTML("<td"+align1+w1+">"+self._telescope+"</td>") 
        htmlLogger.logHTML("<td"+align2+w2+">"+start_time+"</td>")
        htmlLogger.logHTML("<td"+align2+w2+">"+end_time+"</td>")
        htmlLogger.logHTML("</tr>")


    def set_largestgaingap(self,dt):
        """ set the largest gap between to gain calibrator scans
                
        Keyword arguments:
        dt -- largest gap between two gain scans


        """
        self._largestgaingap = dt

###########################################################################
#################  specwin  ############################################
#
# todo:

class Specwin:
    """Class containing info about a single spectral window."""

    def __init__(self, id, ref_freq, band_width, nchannels):
        """Constructor.

        Keyword arguments:
        id -- integer corresponding to the spectral window id
        ref_freq -- reference frequency GHz
        band_width -- bandwidth GHz
        nchannels  -- number of channels


        """

	self._id = id
	self._ref_freq = ref_freq
	self._band_width = band_width
	self._nchannels = nchannels




    def __str__(self):
        outstr = "ref_freq: "+str(self._ref_freq)+"\n"
        outstr = outstr + "  band width:"+str(self._band_width)+"\n"
        outstr = outstr + "  nchannels:"+str(self._nchannels)+"\n"
        return outstr

    def _info2html(self, htmlLogger):
        """ append info to the html summary
                
        Keyword arguments:

        """
        align1 = ' ALIGN="CENTER" VALIGN="TOP" '
        align2 = ' ALIGN="LEFT" VALIGN="TOP" '
        w1 = 'WIDTH="50" '
        w2 = 'WIDTH="100" '

# convert frequencies to GHz

        htmlLogger.logHTML("<tr>")
        htmlLogger.logHTML("<td"+align1+w1+">"+str(self._id)+"</td>") 
        htmlLogger.logHTML("<td"+align1+w1+">"+str(self._ref_freq)+"</td>") 
        htmlLogger.logHTML("<td"+align2+w2+">"+str(self._band_width)+"</td>")
        htmlLogger.logHTML("<td"+align2+w2+">"+str(self._nchannels)+"</td>")
        htmlLogger.logHTML("</tr>")
