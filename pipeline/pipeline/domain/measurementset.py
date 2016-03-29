from __future__ import absolute_import
import collections
import contextlib
import itertools
import os
import string
import types

import numpy
import pylab

from . import spectralwindow
from . import measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


class MeasurementSet(object):    
    def __init__(self, name, session=None):
        self.name = name
        self.antenna_array = None
        self.data_descriptions = []
        self.spectral_windows = []
        self.fields = []
        self.states = []
        self.reference_antenna = None
        self.reference_spwmap= None
        self.phaseup_spwmap= None
        self.combine_spwmap= None
        self.flagcmds = []
        self.session = session
        self.filesize = self._calc_filesize() 
        self.parent_ms = None

    def _calc_filesize(self):
        """
        Calculate the disk usage of this measurement set.
        """
        total_bytes = 0
        for dirpath, _, filenames in os.walk(self.name):
            for f in filenames:
                fp = os.path.join(dirpath, f)
                total_bytes += os.path.getsize(fp)
    
        return measures.FileSize(total_bytes, 
                                 measures.FileSizeUnits.BYTES)
    
    def __str__(self):
        return 'MeasurementSet({0})'.format(self.name)
        
    @property
    def intents(self):
        intents = set()
        # we look to field rather than state as VLA datasets don't have state
        # entries
        for field in self.fields:
            intents.update(field.intents)
        return intents
    
    @property
    def antennas(self):
        # return a copy rather than the underlying list
        return list(self.antenna_array.antennas)

    @property
    def basename(self):
        return os.path.basename(self.name)
    
    def get_antenna(self, search_term=''):
        if search_term == '':
            return self.antennas

        return [a for a in self.antennas
                if a.id in utils.ant_arg_to_id(self.name, search_term)]

    def get_state(self, state_id=None):
        match = [state for state in self.states if state.id == state_id]
        if match:
            return match[0]
        else:
            return None

    def get_scans(self, scan_id=None, scan_intent=None):
        pool = self.scans

        if scan_id is not None:
            # encase raw numbers in a tuple
            if not isinstance(scan_id, collections.Sequence):
                scan_id = (scan_id,)
            pool = [s for s in pool if s.id in scan_id]

        if scan_intent is not None:
            if type(scan_intent) in types.StringTypes:
                if scan_intent in ('', '*'):
                    # empty string equals all intents for CASA
                    scan_intent = ','.join(self.intents)
                scan_intent = scan_intent.split(',')
            scan_intent = set(scan_intent) 
            pool = [s for s in pool if not s.intents.isdisjoint(scan_intent)]

        return pool

    def get_data_description(self, spw=None, id=None):
        match = None
        if spw is not None:
            if isinstance(spw, spectralwindow.SpectralWindow):
                match = [dd for dd in self.data_descriptions
                         if dd.spw is spw]
            elif type(spw) is types.IntType:
                match = [dd for dd in self.data_descriptions
                         if dd.spw.id is spw]        
        if id is not None:
            match = [dd for dd in self.data_descriptions if dd.id == id]
            
        if match:
            return match[0]
        else:
            return None
    
    def get_fields(self, task_arg=None, field_id=None, name=None, intent=None):
        """
        Get Fields from this MeasurementSet matching the given criteria. If no
        criteria are given, all Fields in the MeasurementSet will be returned.

        Arguments can be given as either single items of the expected type,
        sequences of the expected type, or in the case of name or intent, as
        comma separated strings. For instance, name could be 'HOIX', 
        'HOIX,0841+708' or ('HOIX','0841+708').
        
        :param field_id: field ID(s) to match
        :param name: field name(s) to match
        :param intent: observing intent(s) to match
        :rtype: a (potentially empty) list of :class:`~pipeline.domain.field.Field` \
             objects
        """
        pool = self.fields
        #print pool

        if task_arg not in (None, ''):
            pool = [f for f in pool
                    if f.id in utils.field_arg_to_id(self.name, task_arg)]
        
        if field_id is not None:
            # encase raw numbers in a tuple
            if not isinstance(field_id, collections.Sequence):
                field_id = (field_id,)
            pool = [f for f in pool if f.id in field_id]

        if name is not None:
            if type(name) in types.StringTypes:
                name = string.split(name, ',')
            name = set(name) 
            pool = [f for f in pool if f.name in name]
        
        if intent is not None:
            if type(intent) in types.StringTypes:
                if intent in ('', '*'):
                    # empty string equals all intents for CASA
                    intent = ','.join(self.intents)
                intent = string.split(intent, ',')
            intent = set(intent) 
            pool = [f for f in pool if not f.intents.isdisjoint(intent)]

        return pool

    def get_spectral_window(self, spw_id):
        if spw_id is not None:
            spw_id = int(spw_id)
            match = [spw for spw in self.spectral_windows 
                     if spw.id is spw_id]
            if match:
                return match[0]
            else:
                raise KeyError('No spectral window with ID \'{0}\' found in '
                               '{1}'.format(spw_id, self.basename))

    def get_spectral_windows(self, task_arg='', with_channels=False,
                             num_channels=(), science_windows_only=True):
        """
        Return the spectral windows corresponding to the given CASA-style spw
        argument, filtering out windows that may not be science spectral 
        windows (WVR windows, channel average windows etc.).
        """
        spws = self.get_all_spectral_windows(task_arg, with_channels)

        # if requested, filter spws by number of channels
        if num_channels:
            spws = [w for w in spws if w.num_channels in num_channels] 
            
        if not science_windows_only:
            return spws
        
        if self.antenna_array.name == 'ALMA':
            science_intents = set(['TARGET','PHASE','BANDPASS','AMPLITUDE', 'POLARIZATION', 'POLANGLE', 'POLLEAKAGE', 'CHECK'])
            return [w for w in spws if w.num_channels not in (1,4)
                    and not science_intents.isdisjoint(w.intents)]

        return spws

    def get_all_spectral_windows(self, task_arg='', with_channels=False):
        """Return the spectral windows corresponding to the given CASA-style
        spw argument.
        """
        # we may have more spectral windows in our MeasurementSet than have
        # data in the measurement set on disk. Ask for all 
        if task_arg in (None, ''):
            task_arg = '*'

        # expand spw tuples into a range per spw, eg. spw9 : 1,2,3,4,5
        selected = collections.defaultdict(set)
        for (spw, start, end, step) in utils.spw_arg_to_id(self.name, task_arg):
            selected[spw].update(set(range(start, end+1, step))) 
            
        if not with_channels:
            return [spw for spw in self.spectral_windows if spw.id in selected]

        spws = []
        for spw_id, channels in selected.items():
            spw_obj = self.get_spectral_window(spw_id)
            proxy = spectralwindow.SpectralWindowWithChannelSelection(spw_obj, 
                                                                      channels)
            spws.append(proxy)
        return spws

    def get_original_intent(self, intent=None):
        """
        Get the original obs_modes that correspond to the given pipeline
        observing intents.
        """
        obs_modes = [state.get_obs_mode_for_intent(intent)
                     for state in self.states]
        return set(itertools.chain(*obs_modes))
    
    @property
    def start_time(self):
        qt = casatools.quanta
        s = sorted(self.scans, 
                   key=lambda scan: scan.start_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[0].start_time

    @property
    def end_time(self):
        qt = casatools.quanta
        s = sorted(self.scans, 
                   key=lambda scan: scan.end_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[-1].start_time
        
    
    def get_vla_max_integration_time(self):
        """Get the integration time used by the original VLA scripts
       
           Returns -- The max integration time used
        """
        
        vis = self.name


        #with casatools.TableReader(vis + '/FIELD') as table:
        #    numFields = table.nrows()
        #    field_positions = table.getcol('PHASE_DIR')
        #    field_ids = range(numFields)
        #    field_names = table.getcol('NAME')
        
        #with casatools.TableReader(vis) as table:
        #    scanNums = sorted(numpy.unique(table.getcol('SCAN_NUMBER')))
        #    field_scans = []
        #    for ii in range(0,numFields):
        #        subtable = table.query('FIELD_ID==%s'%ii)
        #        field_scans.append(list(numpy.unique(subtable.getcol('SCAN_NUMBER'))))
        #        subtable.close()
        
        ## field_scans is now a list of lists containing the scans for each field.
        ## so, to access all the scans for the fields, you'd:
        #
        #for ii in range(0,len(field_scans)):
        #   for jj in range(0,len(field_scans[ii]))
        #
        ## the jj'th scan of the ii'th field is in field_scans[ii][jj]
        
        # Identify intents
        
        with casatools.TableReader(vis + '/STATE') as table:
            #casatools.table.open(vis+'/STATE')
            intents = table.getcol('OBS_MODE')
            #casatools.table.close()
        
        """Figure out integration time used"""
        
        
        with casatools.MSReader(vis) as ms:
            scan_summary = ms.getscansummary()
            #ms_summary = ms.summary()
        #startdate=float(ms_summary['BeginTime'])

    
        integ_scan_list = []
        for scan in scan_summary:
            integ_scan_list.append(int(scan))
        sorted_scan_list = sorted(integ_scan_list)
        
        # find max and median integration times
        #
        integration_times = []
        for ii in sorted_scan_list:
            integration_times.append(scan_summary[str(ii)]['0']['IntegrationTime'])
            
        maximum_integration_time = max(integration_times)
        median_integration_time = numpy.median(integration_times)
        
        int_time = maximum_integration_time
        
        return int_time
    
    def get_vla_datadesc(self):
        '''Generate VLA data description index'''
        
        vis = self.name
        
        cordesclist = ['Undefined','I','Q','U','V',
                       'RR','RL','LR','LL',
                       'XX','XY','YX','YY',
                       'RX','RY','LX','LY',
                       'XR','XL','YR','YL',
                       'PP','PQ','QP','QQ',
                       'RCircular','LCircular',
                       'Linear','Ptotal',
                       'Plinear','PFtotal',
                       'PFlinear','Pangle' ]
        
        #From Steve Myers buildscans function
        with casatools.TableReader(vis + '/DATA_DESCRIPTION') as table:
            #tb.open(msfile+"/DATA_DESCRIPTION")
            ddspwarr=table.getcol("SPECTRAL_WINDOW_ID")
            ddpolarr=table.getcol("POLARIZATION_ID")
            #tb.close()
        ddspwlist = ddspwarr.tolist()
        ddpollist = ddpolarr.tolist()
        ndd = len(ddspwlist)
        
        with casatools.TableReader(vis + '/SPECTRAL_WINDOW') as table:
            #tb.open(msfile+"/SPECTRAL_WINDOW")
            nchanarr=table.getcol("NUM_CHAN")
            spwnamearr=table.getcol("NAME")
            reffreqarr=table.getcol("REF_FREQUENCY")
            #tb.close()
        nspw = len(nchanarr)
        spwlookup = {}
        for isp in range(nspw):
            spwlookup[isp] = {}
            spwlookup[isp]['nchan'] = nchanarr[isp]
            spwlookup[isp]['name'] = str( spwnamearr[isp] )
            spwlookup[isp]['reffreq'] = reffreqarr[isp]
        
        
        with casatools.TableReader(vis + '/POLARIZATION') as table:
            #tb.open(msfile+"/POLARIZATION")
            ncorarr=table.getcol("NUM_CORR")
            npols = len(ncorarr)
	    polindex = {}
	    poldescr = {}
	    for ip in range(npols):
		cort=table.getcol("CORR_TYPE",startrow=ip,nrow=1)
		(nct,nr) = cort.shape
		cortypes = []
		cordescs = []
		for ict in range(nct):
		    cct = cort[ict][0]
		    cde = cordesclist[cct]
		    cortypes.append(cct)
		    cordescs.append(cde)
		polindex[ip] = cortypes
		poldescr[ip] = cordescs
            
            
        ddindex = {}
        ncorlist=ncorarr.tolist()
        for idd in range(ndd):
            ddindex[idd] = {}
            isp = ddspwlist[idd]
            ddindex[idd]['spw'] = isp
            ddindex[idd]['spwname'] = spwlookup[isp]['name']
            ddindex[idd]['nchan'] = spwlookup[isp]['nchan']
            ddindex[idd]['reffreq'] = spwlookup[isp]['reffreq']
            #
            ipol = ddpollist[idd]
            ddindex[idd]['ipol'] = ipol
            ddindex[idd]['npol'] = ncorlist[ipol]
            ddindex[idd]['corrtype'] = polindex[ipol]
            ddindex[idd]['corrdesc'] = poldescr[ipol]
            
        return ddindex
    
    
    def get_vla_corrstring(self):
        '''Get correlation string for VLA'''
        
        """
        Prep string listing of correlations from dictionary created by method buildscans
        For now, only use the parallel hands.  Cross hands will be implemented later.
        """
        
        ddindex = self.get_vla_datadesc()
        
        corrstring_list = ddindex[0]['corrdesc']
        removal_list = ['RL', 'LR', 'XY', 'YX']
        corrstring_list = list(set(corrstring_list).difference(set(removal_list)))
        corrstring = string.join(corrstring_list,',')
        
        return corrstring
        
    def get_alma_corrstring(self):
        '''Get correlation string for ALMA for the science windows'''
        
        sci_spwlist = self.get_spectral_windows(science_windows_only=True)
        sci_spwids = [spw.id for spw in sci_spwlist]
        
        datadescs = [dd for dd in self.data_descriptions if dd.spw.id in sci_spwids]
        
        numpols = len(datadescs[0].polarizations)
        
        if numpols == 1:
            corrstring= 'XX'
        else:
            corrstring = 'XX,YY'
        
        return corrstring
        
    def get_vla_spw2band(self):
    
        ddindex = self.get_vla_datadesc()
        
        spw2band = {}
    
        for spw in ddindex:

            strelems =  list(ddindex[spw]['spwname'])
            #print strelems
            bandname = strelems[5]
            if bandname in '4PLSCXUKAQ':
                spw2band[spw] = strelems[5]
            #Check for U / KU
            if strelems[5] == 'K' and strelems[6] == 'U':
                spw2band[spw] = 'U'
            if strelems[5] == 'K' and strelems[6] == 'A':
                spw2band[spw] = 'A'

        return spw2band
        
        
    
    
    def vla_spws_for_field(self,field):
        '''VLA spws for field'''
    
        vis = self.name
    
        # get observed DDIDs for specified field from MAIN
        with casatools.TableReader(vis) as table:
            st = table.query('FIELD_ID=='+str(field))
            ddids=pylab.unique(st.getcol('DATA_DESC_ID'))
            st.close()
    
        # get SPW_IDs corresponding to those DDIDs
        with casatools.TableReader(vis+'/DATA_DESCRIPTION') as table:
            spws=table.getcol('SPECTRAL_WINDOW_ID')[ddids]
        
        # return as a list
        return list(spws)
    
    def get_vla_field_ids(self):
        ''' Find field ids for VLA'''
        
        vis = self.name
        
        with casatools.TableReader(vis+'/FIELD') as table:
            numFields = table.nrows()
            field_ids = range(numFields)
        
        return field_ids
        
    def get_vla_field_names(self):
        ''' Find field names  for VLA'''
        
        vis = self.name
        
        with casatools.TableReader(vis+'/FIELD') as table:
            field_names = table.getcol('NAME')
        
        return field_names
        
    def get_vla_field_spws(self):
        ''' Find field spws for VLA  '''
        
        vis = self.name
        
        with casatools.TableReader(vis+'/FIELD') as table:
            numFields = table.nrows()
            
        #Map field IDs to spws
        field_spws = []
        for ii in range(numFields):
            field_spws.append(self.vla_spws_for_field(ii))
       
        return field_spws
    
    
    def get_vla_numchan(self):
        ''' Get number of channels for VLA'''
        
        vis = self.name
        
        with casatools.TableReader(vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            
        return channels
    
    
    def get_vla_tst_bpass_spw(self):
        ''' Get VLA test bandpass spws'''
        
        vis = self.name
        tst_delay_spw=''
        
        with casatools.TableReader(vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
        
        numSpws = len(channels)
        
        for ispw in range(numSpws):
            endch1=int(channels[ispw]/3.0)
            endch2=int(2.0*channels[ispw]/3.0)+1
            if (ispw<max(range(numSpws))):
                tst_delay_spw=tst_delay_spw+str(ispw)+':'+str(endch1)+'~'+str(endch2)+','
                #all_spw=all_spw+str(ispw)+','
            else:
                tst_delay_spw=tst_delay_spw+str(ispw)+':'+str(endch1)+'~'+str(endch2)
                #all_spw=all_spw+str(ispw)
        
        tst_bpass_spw=tst_delay_spw
        
        return tst_bpass_spw
    
    
    def get_vla_tst_delay_spw(self):
        ''' Get VLA test bandpass spws'''
        
        vis = self.name
        tst_delay_spw=''
        
        with casatools.TableReader(vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
        
        numSpws = len(channels)
        
        for ispw in range(numSpws):
            endch1=int(channels[ispw]/3.0)
            endch2=int(2.0*channels[ispw]/3.0)+1
            if (ispw<max(range(numSpws))):
                tst_delay_spw=tst_delay_spw+str(ispw)+':'+str(endch1)+'~'+str(endch2)+','
                #all_spw=all_spw+str(ispw)+','
            else:
                tst_delay_spw=tst_delay_spw+str(ispw)+':'+str(endch1)+'~'+str(endch2)
                #all_spw=all_spw+str(ispw)
       
        
        return tst_delay_spw
    
    def get_vla_quackingscans(self):
        '''Find VLA scans for quacking.  Quack! :) '''
        
        vis = self.name
        with casatools.MSReader(vis) as ms:
            scan_summary = ms.getscansummary()
        
        integ_scan_list = []
        for scan in scan_summary:
            integ_scan_list.append(int(scan))
        sorted_scan_list = sorted(integ_scan_list)
        
        scan_list = [1]
        old_scan = scan_summary[str(sorted_scan_list[0])]['0']
        
        old_field = old_scan['FieldId']
        old_spws = old_scan['SpwIds']
        for ii in range(1,len(sorted_scan_list)):
            new_scan = scan_summary[str(sorted_scan_list[ii])]['0']
            new_field = new_scan['FieldId']
            new_spws = new_scan['SpwIds']
            if ((new_field != old_field) or (set(new_spws) != set(old_spws))):
                scan_list.append(sorted_scan_list[ii])
                old_field = new_field
                old_spws = new_spws
        quack_scan_string = ','.join(["%s" % ii for ii in scan_list])
        
        return quack_scan_string
    
    
    def get_vla_critfrac(self):
        """ Identify bands/basebands/spws
        """
        
        vis = self.name
        
        with casatools.TableReader(vis+'/SPECTRAL_WINDOW') as table:
            spw_names = table.getcol('NAME')
        
        
        # If the dataset is too old to have the bandname in it, assume that
        # either there are 8 spws per baseband (and allow for one or two for
        # pointing), or that this is a dataset with one spw per baseband

        if (len(spw_names)>=8):
            critfrac=0.9/int(len(spw_names)/8.0)
        else:
            critfrac=0.9/float(len(spw_names))
        
        
        if '#' in spw_names[0]:
        #
        # i assume that if any of the spw_names have '#', they all do...
        #
            bands_basebands_subbands = []
            for spw_name in spw_names:
                receiver_name, baseband, subband = spw_name.split('#')
                receiver_band = (receiver_name.split('_'))[1]
                bands_basebands_subbands.append([receiver_band, baseband, int(subband)])
            spws_info = [[bands_basebands_subbands[0][0], bands_basebands_subbands[0][1], [], []]]
            bands = [bands_basebands_subbands[0][0]]
            for ii in range(len(bands_basebands_subbands)):
                band,baseband,subband = bands_basebands_subbands[ii]
                found = -1
                for jj in range(len(spws_info)):
                    oband,obaseband,osubband,ospw_list = spws_info[jj]
                    if band==oband and baseband==obaseband:
                        osubband.append(subband)
                        ospw_list.append(ii)
                        found = jj
                        break
                if found >= 0:
                    spws_info[found] = [oband,obaseband,osubband,ospw_list]
                else:
                    spws_info.append([band,baseband,[subband],[ii]])
                    bands.append(band)
            #logprint("Bands/basebands/spws are:", logfileout='logs/msinfo.log')
            for spw_info in spws_info:
                spw_info_string = spw_info[0] + '   ' + spw_info[1] + '   [' + ','.join(["%d" % ii for ii in spw_info[2]]) + ']   [' + ','.join(["%d" % ii for ii in spw_info[3]]) + ']'
                #logprint(spw_info_string, logfileout='logs/msinfo.log')
                
        # Critical fraction of flagged solutions in delay cal to avoid an
        # entire baseband being flagged on all antennas
            critfrac=0.9/float(len(spws_info))
        elif ':' in spw_names[0]:
            print("old spw names with :")
            #logprint("old spw names with :", logfileout='logs/msinfo.log')
        else:
            print("unknown spw names")
    
        return critfrac
        
    
    def get_median_integration_time(self, intent=None):
        """Get the median integration time used to get data for the given
        intent.
    
        Keyword arguments:
        intent  -- The intent of the data of interest.
      
        Returns -- The median integration time used.
        """
        LOG.debug('inefficiency - MSFlagger reading file to get integration '
                  'time')
    
        # get the field IDs and state IDs for fields in the measurement set,
        # filtering by intent if necessary
        if intent:    
            field_ids = [field.id for field in self.fields 
                         if intent in field.intents]
            state_ids = [state.id for state in self.states
                         if intent in state.intents]
#        if intent:
#            re_intent = intent.replace('*', '.*')
#            re_intent = re.compile(re_intent)
#            field_ids = [field.id for field in self.fields 
#                         if re_intent.match(str(field.intents))]
#            state_ids = [state.id for state in self.states
#                         if re_intent.match(str(state.intents))]
        else:
            field_ids = [field.id for field in self.fields]
            state_ids = [state.id for state in self.states]
            
        # VLA datasets have an empty STATE table; in the main table such rows
        # have a state ID of -1.
        if not state_ids:
            state_ids = [-1] 
        
        with casatools.TableReader(self.name) as table:
            taql = '(STATE_ID IN %s AND FIELD_ID IN %s)' % (state_ids, field_ids)
            with contextlib.closing(table.query(taql)) as subtable:
                integration = subtable.getcol('INTERVAL')          
            return numpy.median(integration)
    
    
    def get_median_science_integration_time(self, intent=None, spw=None):
        """Get the median integration time for science targets used to get data for the given
        intent.
    
        Keyword arguments:
        intent  -- The intent of the data of interest.
        spw     -- spw string list - '1,7,11,18'
      
        Returns -- The median integration time used.
        """
        LOG.debug('inefficiency - MSFlagger reading file to get median integration '
                  'time for science targets')
        
        if (spw == None):
            spws = self.spectral_windows
        else: 
            
            try:
                #Put csv string of spws into a list
                spw_string_list = spw.split(',')
                
                #Get all spw objects
                all_spws = self.spectral_windows
        
                #Filter out the science spw objects
                spws = [ispw for ispw in all_spws if str(ispw.id) in spw_string_list]
            except:
                LOG.error("Incorrect spw string format.")
        
            
        
        # now get the science spws, those used for scientific intent
        science_spws = [ispw for ispw in spws if 
          ispw.num_channels not in [1,4] and not ispw.intents.isdisjoint(
          ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]
        LOG.debug('science spws are: %s' % [ispw.id for ispw in science_spws])
    
        # and the science fields/states
        science_field_ids = [field.id for field in self.fields
          if not set(field.intents).isdisjoint(
            ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]
        science_state_ids = [state.id for state in self.states
          if not set(state.intents).isdisjoint(
            ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]
            
        
        science_spw_dd_ids = [self.get_data_description(spw).id for spw in science_spws]
        
        with casatools.TableReader(self.name) as table:
            taql = '(STATE_ID IN %s AND FIELD_ID IN %s AND DATA_DESC_ID in %s)' % (science_state_ids, science_field_ids, science_spw_dd_ids)
            with contextlib.closing(table.query(taql)) as subtable:
                integration = subtable.getcol('INTERVAL')          
            return numpy.median(integration)
    
    @property
    def session(self):
        return self._session
    
    @session.setter
    def session(self, value):
        if value is None:
            value = 'session_1'
        self._session = value
