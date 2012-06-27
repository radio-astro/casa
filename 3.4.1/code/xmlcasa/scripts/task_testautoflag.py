##################################################################

import os, time
import numpy as np
import pylab as pl
from taskinit import *

import casac
from tasks import *

import commands;
import operator;
from matplotlib.font_manager import fontManager, FontProperties

#=====================================================================
# Task inputs

def testautoflag( vis, field, spw, 
              selectdata, antenna, uvrange, timerange, scan, feed, array,
              datacolumn, ntime,  corrs,
              tfcrop, timecutoff, freqcutoff, timefit, freqfit, maxnpieces, flagdimension, usewindowstats, halfwin,
              extendflags, extendpols, growtime, growfreq, growaround, flagneartime, flagnearfreq,
              datadisplay, plotsummary, showknownrfi, usepreflags,preflagzeros, writeflags, flagbackup):

    lfg = casac.homefinder.find_home_by_name('lightflaggerHome').create();
    flagstats={};
    msname=vis;

    # Check that the MS exists
    if(msname == ''):
        casalog.post(message="Please set the MS name. ",priority="WARN",origin='testautoflag');
        return False;
    
    #Check that the requested data column exists
    tb.open(msname);
    collist = tb.colnames();
    tb.close();
    if( ( (datacolumn == 'data') and ( ( 'DATA' in collist )==False) )  \
         or ( (datacolumn == 'corrected') and ( ( 'CORRECTED_DATA' in collist )==False) ) \
         or ( (datacolumn == 'model') and ( ( 'MODEL_DATA' in collist )==False) ) \
         or ( (datacolumn == 'residual') and ( ( 'CORRECTED_DATA' in collist )==False) ) \
         or ( (datacolumn == 'residual') and ( ( 'MODEL_DATA' in collist )==False) ) \
         or ( (datacolumn == 'residual_data') and ( ( 'MODEL_DATA' in collist )==False) ) ):
        casalog.post(message="Data column ["+ datacolumn+"] cannot be found. Please check that the MS has the required data columns.", priority='WARN',origin='testautoflag');
        return False;

    
    # Open the flagger tool.
    casalog.post(message="Opening the MS : "+msname, priority='INFO',origin='testautoflag');
    lfg.open(msname);
    
    # Select a subset of the data (or not)
    if(field==''): field='*';
    if(spw==''): spw='*';
    if(selectdata):
        lfg.setdata(field=field,spw=spw, baseline=antenna ,time=timerange,
                     scan=scan, feed=feed, array=array, uvrange=uvrange);
    else:
        lfg.setdata(field=field,spw=spw);
    
    # Get the detault parameters for TFCROP, and modify them with user-input
    if(tfcrop):
        par = lfg.getparameters('tfcrop');
        par['corrs']=np.array(corrs,'int');
        par['maxnpieces']=int(maxnpieces);
        par['freq_amp_cutoff']=float(freqcutoff);
        par['time_amp_cutoff']=float(timecutoff);
        par['freqfit']=str(freqfit);
        par['timefit']=str(timefit);
        par['flagdimension']=str(flagdimension);
        par['usewindowstats']=str(usewindowstats);
        par['halfwin']=int(halfwin);
        lfg.setparameters('tfcrop',par);
        casalog.post(message="tfcrop parameters : " +str(par), priority='INFO', origin='testautoflag');
 
    # Get the detault parameters for EXTENDFLAGS, and modify them with user-input
    if(extendflags):
        par = lfg.getparameters('extendflags');
        par['extend_across_pols']=bool(extendpols);
        par['grow_in_time']=float(growtime);
        par['grow_in_freq']=float(growfreq);
        par['grow_around']=bool(growaround);
        par['flag_prev_next_time']=bool(flagneartime);
        par['flag_prev_next_freq']=bool(flagnearfreq);
        lfg.setparameters('extendflags',par);
        casalog.post(message="extendflag parameters : " +str(par), priority='INFO', origin='testautoflag');
    
    # Set generic user-parameters
    par={};
    par['showplots']=bool(datadisplay);
    par['writeflags']=bool(writeflags);
    par['flagzeros']=bool(preflagzeros);
    par['usepreflags']=bool(usepreflags);
    par['ntime_sec']=int(ntime);
    par['column']=str(datacolumn);
    casalog.post(message="generic parameters : " +str(par), priority='INFO', origin='testautoflag');

    # Take a flag backup if requested.
    if(writeflags and flagbackup):
        #casalog.post(message="Taking a flag backup",priority='INFO',origin='testautoflag');
        #fvname = 'before_testautoflag_'+(time.asctime()).replace(' ', '_');
        #lfg.saveflagversion(versionname = fvname, comment='Save Flags before running testautoflag', merge='replace');
        backup_flags(lfg,"testautoflag_");

    # Run the flagger
    casalog.post(message="Starting Flagging", priority='INFO',origin='testautoflag');
    flagstats = lfg.run(par);

    # Close the lightflagger tool. 
    lfg.done();
    
    # Parse the output flagstats dictionary and make plots
    if(len(flagstats)>0 and plotsummary==True):
        flagsum = classFlagSummary();
        flagsum.plotflagstats(msname, flagstats, showknownrfi);

    return True;
#####################################

#####################################
# Make names for flag versions (copied from flagdata2.py)
#####################################
def backup_flags(fglocal, modes):
    existing = fglocal.getflagversionlist(printflags=False)
    # remove comments from strings
    existing = [x[0:x.find(' : ')] for x in existing]
    i = 1
    while True:
        versionname = modes + str(i)
        if not versionname in existing:
            break
        else:
            i = i + 1
    time_string = str(time.strftime('%Y-%m-%d %H:%M:%S'))
    casalog.post(message = "Saving current flags to " + versionname + " before applying new flags", 
                 priority='INFO',origin='testautoflag');
    fglocal.saveflagversion(versionname=versionname,
                           comment='flagdata autosave before ' + modes + ' on ' + time_string,
                           merge='replace')
    return;


######################################
##  Class to make plots of flag counts and statistics
######################################
class classFlagSummary:
    """
    A class that parses the output dictionary from the flagger
    and makes diagnostic plots.
    """
    ###################################
    #### Init
    ###################################
    def __init__(self):
          self.name = 'tfcrop';
          self.lfg = casac.homefinder.find_home_by_name('lightflaggerHome').create();
          self.flagstats={};
          self.msname="";
          self.nfld=1;
          #self.bslist=[];

    ##################################
    # Make plots from the dictionary of flag counts
    ##################################
    def plotflagstats(self,msname,flagstats,showknownrfi):
        if( len(flagstats) > 0 and os.path.exists(msname)):
            #self.bslist = [];
            pl.clf();
            
  #Get freq lists for all spws
            tb.open(msname+'/SPECTRAL_WINDOW');
            allfreqlist = tb.getcol('CHAN_FREQ'); 
            tb.close();
            cshape = allfreqlist.shape;
            Nchan = cshape[0];
            Nspw = cshape[1];
            
  # Get field information
            tb.open(msname+'/FIELD');
            allfieldlist = tb.getcol('NAME'); 
            tb.close();
            Nfld = len(allfieldlist);
            
  # Get antenna information
            tb.open(msname+'/ANTENNA');
            allantlist = tb.getcol('NAME'); 
            tb.close();
            Nant = len(allantlist);
            
  # Make empty arrays of correct shapes.
            freqvals = {};
            flagvals = {};
            ant1vals = {};
            ant2vals = {};
            baselinevals = {};
            baseflagvals = {};
            for fld in range(0,Nfld):
                freqvals[str(fld)] = {};
                flagvals[str(fld)] = {};
                ant1vals[str(fld)] = {};
                ant2vals[str(fld)] = {};
                baseflagvals[str(fld)] = {};
                baselinevals[str(fld)] = {};
                for spw in range(0,Nspw):
                    freqvals[str(fld)][str(spw)] = []
                    flagvals[str(fld)][str(spw)] = [];
                    ant1vals[str(fld)][str(spw)] = [];
                    ant2vals[str(fld)][str(spw)] = [];
                    baseflagvals[str(fld)][str(spw)] = [];
                    baselinevals[str(fld)][str(spw)] = [];
                    
  #Fill in spectrum of flag percentage
            for chans in flagstats["channel"]:
                splitchan = chans.split(':');
                fld = splitchan[0];
                spw = splitchan[1];
                ch = splitchan[2];
                if( (flagstats["channel"][chans]["total"]) > 0.0 ):
                    freqvals[str(fld)][str(spw)].append( allfreqlist[ch,spw]/1e+06 );
                    flagvals[str(fld)][str(spw)].append( 100 * float(flagstats["channel"][chans]["flagged"]) / float(flagstats["channel"][chans]["total"])  );
                    
                   
  # Plot separately for each field and spw.
            casalog.post(message="\nSummary of Flag Counts",priority='INFO',origin='testautoflag');
            minfreq = 50000.0;
            maxfreq = 900.0;
            self.nfld=Nfld;
            for fld in range(0,Nfld):
                for spw in range(0,Nspw):
                    # sort and plot flag fraction as a function of frequency
                    sortedchans = sorted(enumerate(freqvals[str(fld)][str(spw)]), key=operator.itemgetter(1));
                    newflags=[];
                    newfreqs=[];
                    for cc in range(0,len(freqvals[str(fld)][str(spw)])):
                        newfreqs.append(sortedchans[cc][1]);
                        newflags.append(flagvals[str(fld)][str(spw)][ sortedchans[cc][0]  ]);
                    if( len(newfreqs) > 0 ):
                        pl.subplot(111);
                        pl.plot(newfreqs,newflags,color=self.__getplotcolour__(fld,spw),label='fid:'+str(fld) + ' ' + allfieldlist[fld] +'  spw:'+str(spw));
                        minfreq = min( minfreq , np.min(newfreqs) );
                        maxfreq = max( maxfreq, np.max(newfreqs) );
                        #print counts to logger
                        labstring = 'Field['+str(fld) + ']:' + allfieldlist[fld] +'  Spw['+str(spw)+']:'+'%6.2fMHz-%6.2fMHz'%(np.min(newfreqs),np.max(newfreqs)) + ' --> Flagged %3.2f'%(np.sum(newflags)/len(newflags)) + '%.'
                        casalog.post(message=labstring, priority='INFO', origin='testautoflag');
                                
            pl.subplot(111);
            pl.xlabel('Frequency (MHz)');
            pl.ylabel('Percentage flagged');
            pl.title('Spectrum of percentage of RFI-affected data');
            #pl.legend(borderpad=0.1,prop=FontProperties(size='smaller'));

            if(showknownrfi):
                self.plotknownrfi(minfreq, maxfreq);

#            ax=pl.subplot(111);
#            for ant1 in range(0,Nant):
#                for ant2 in range(0,Nant):
#                    self.bslist.append( allantlist[ant1]+'-'+ allantlist[ant2]  );
#            pl.xticks(range(minfreq,maxfreq));
#            ax.xaxis.set_major_formatter(pl.FuncFormatter(self.__printspwsummary__));
        else:
            print "Empty statistics dictionary";
######################################   

    def __getplotcolour__(self,fld,spw):
        #allcols = [(1,0,0), (1,0.3,0), (1,0.8,0), (0.5,1,0), (0,1,0), (0,1,1), (0,0.5,1), (0,0,1), (0.5,0,1), (1,0,1)]
        allcols = [(0,0,0), (0,0,1)];
        compcols = [(1,1,1), (1,1,0)]
        ncols = len(allcols);
        if(spw < ncols):
            thecol = allcols[spw] ;
            thecompcol = compcols[spw];
        else:
            thecol = allcols[(spw)%ncols];
            thecompcol = compcols[(spw)%ncols];
        fac = 0.75*float(fld)/float(self.nfld);
        #print 'fld:', fld, '  nfld:', self.nfld, '  fac:', fac ,'  col:', thecol[0]+fac*thecompcol[0] ,thecol[1]+fac*thecompcol[1], thecol[2]+fac*thecompcol[2] ;
        return ( thecol[0]+fac*thecompcol[0] ,thecol[1]+fac*thecompcol[1], thecol[2]+fac*thecompcol[2] ) ;
######################################
######################################
    def __printspwsummary__(self,x,pos=0):
            return str(x)+' MHz';

######################################

#    def __makebaselinelabel__(self,x,pos=0):
#        if(int(x) >= 0 and int(x) < len(self.bslist)):
#            return self.bslist[int(x)];
#        else:
#            return '';

######################################
######################################
    def plotknownrfi(self,minfreq,maxfreq):
        knownrfi = self.getknownrfi(minfreq,maxfreq);
        for rfi in knownrfi:
            minf = float(rfi['freq'][0]);
            maxf = float(rfi['freq'][1]);
            tcol='black';
            if(rfi['type'].find('Continuous') > -1):
                tcol='purple';
            if(rfi['type'].find('Intermittent') > -1):
                tcol='green';
            pl.vlines([ minf ],0.0,105.0,linestyles='dashed',color=tcol);
            if( maxf - minf > 0.1 ):
                pl.bar(minf,105.0,maxf-minf,0.0,alpha=0.2,color=tcol);
                pl.vlines([ maxf ],0.0,105.0,linestyles='dashed',color=tcol);
        return;
######################################
#    def __makeRFIlabel__(self,x,pos=0):
        
######################################
    def getknownrfi(self,minfreq,maxfreq):
        rfilist =   [{'desc': 'RATSCAT Low FQ',  'freq': ['1000.0', '1000.0'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'Aeronautical IFF, gnd2air<br />',  'freq': ['1030', '1030'],  'src': 'OHD,Everywhere',  'type': 'Intermittent'},
                     {'desc': 'Aircraft navigation DME<br />',  'freq': ['1025', '1150'],  'src': 'DME/VOR sites',  'type': 'intermittent'},
                     {'desc': 'Aeronautical IFF, air2gnd',  'freq': ['1090', '1090'],  'src': 'OHD,Everywhere',  'type': 'Intermittent'},
                     {'desc': 'GPS,Galileo,GLONASS L5',  'freq': ['1166', '1186'],  'src': 'OHD,MEO',  'type': 'Continuous'},
                     {'desc': 'VLA modem',  'freq': ['1200', '1200'],  'src': 'VLA Site',  'type': 'Continuous'},
                     {'desc': 'GPS L2',  'freq': ['1217', '1237'],  'src': 'OHD, MEO',  'type': 'Continuous'},
                     {'desc': 'AEROSTAT',  'freq': ['1231', '1231'],  'src': 'TX, FT Bliss',  'type': 'Malfunction'},
                     {'desc': 'FAA ARSR4 radar',  'freq': ['1233.71', '1233.71'], 'src': '',  'type': 'Continuous'},
                     {'desc': 'AEROSTAT',  'freq': ['1235', '1235'],  'src': '',  'type': 'Malfunction'},
                     {'desc': 'GLONASS L2',  'freq': ['1243', '1251'],  'src': 'OHD, MEO',  'type': 'Continuous'},
                     {'desc': 'AEROSTAT',  'freq': ['1252', '1252'],  'src': 'NM, Deming',  'type': 'Malfunction'},
                     {'desc': 'FAA ARSR4 radar',  'freq': ['1254.42', '1254.42'],  'src': 'NM, Deming',  'type': 'Continuous'},
                     {'desc': 'FAA ARSR4 radar',  'freq': ['1262.19', '1262.19'],  'src': 'NM, Eagle Peak',  'type': 'Continuous'},
                     {'desc': 'COMPASS E6', 'freq': ['1268', '1268'], 'src': '', 'type': ''}, 
                     {'desc': 'AEROSTAT',  'freq': ['1291', '1291'],  'src': 'TX, FT Bliss',  'type': 'Malfunction'}, 
                     {'desc': 'AEROSTAT',  'freq': ['1295', '1295'],  'src': 'TX, FT Bliss',  'type': 'Malfunction'}, 
                     {'desc': 'FAA ASR radar',  'freq': ['1310', '1310'],  'src': 'NM, Albuquerque',  'type': 'Continuous'},
                     {'desc': 'AEROSTAT',  'freq': ['1312', '1312'],  'src': 'NM, Deming',  'type': 'Malfunction'},
                     {'desc': 'FAA ARSR4 radar',  'freq': ['1317', '1317'],  'src': '',  'type': 'Continuous'},
                     {'desc': 'FAA ASR radar',  'freq': ['1330', '1330'],  'src': 'NM, Albuquerqu',  'type': ''},
                     {'desc': 'FAA ARSR4 radar',  'freq': ['1337.28', '1337.28'],  'src': 'NM, Deming',  'type': 'Continuous'},
                     {'desc': 'FAA ARSR4 radar',  'freq': ['1345.04', '1345.04'],  'src': 'NM, Eagle Peakk',  'type': 'Continuous'},
                     {'desc': 'WSMR RAJPO transponder',  'freq': ['1357', '1357'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'WSMR RAJPO transponder',  'freq': ['1365', '1365'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'WSMR RAJPO transponder',  'freq': ['1369', '1369'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'GPS L3 NUDET',  'freq': ['1376', '1386'],  'src': 'OHD',  'type': 'Intermittent'},
                     {'desc': 'WSMR RAJPO transponder',  'freq': ['1385', '1385'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'WSMR RAJPO transponder',  'freq': ['1393', '1393'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'WSMR RAJPO transponder',  'freq': ['1432.5', '1432.5'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'WSMR telemetry',  'freq': ['1437', '1453'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'High alt balloon',  'freq': ['1444.5', '1444.5'],  'src': 'NM, FT Sumner',  'type': 'Intermittent'},
                     {'desc': 'Test telemetry',  'freq': ['1448.5', '1448.5'],  'src': 'NM, FT Bliss',  'type': 'Intermittent'},
                     {'desc': 'Test telemetry',  'freq': ['1452.5', '1452.5'],  'src': 'NM, FT Bliss',  'type': 'Intermittent'},
                     {'desc': 'High alt baloon',  'freq': ['1453.5', '1453.5'],  'src': 'NM, FT Sumner',  'type': 'Intermittent'},
                     {'desc': 'Test telemetry',  'freq': ['1460.5', '1460.5'],  'src': 'NM, FT Bliss',  'type': 'Intermittent'},
                     {'desc': 'High alt baloon',  'freq': ['1501.5', '1501.5'],  'src': 'NM, FT Sumner',  'type': 'Intermittent'},
                     {'desc': 'WSMR jamming telemetry',  'freq': ['1504.5', '1504.5'],  'src': 'NM, WSMR',  'type': 'Intermittent'},
                     {'desc': 'High alt baloon',  'freq': ['1515.5', '1515.5'],  'src': 'NM, FT Sumner',  'type': 'Intermittent'},
                     {'desc': 'INMARSAT satellites',  'freq': ['1525', '1564'],  'src': '',  'type': ''},
                     {'desc': 'High alt baloon',  'freq': ['1525.5', '1525.5'],  'src': 'NM, FT Sumner',  'type': 'Intermittent'},
                     {'desc': 'Test telemetry',  'freq': ['1529.5', '1529.5'],  'src': 'NM, FT Bliss',  'type': 'Intermittent'}, 
                     {'desc': 'WSMR jamming telemetry',  'freq': ['1530.5', '1530.5'],  'src': 'NM, WSMR',  'type': 'Intermittent'}, 
                     {'desc': 'GPS L1 jamming',  'freq': ['1565.42', '1565.42'],  'src': 'NM, WSMR',  'type': 'Continuous'}, 
                     {'desc': 'GPS L1',  'freq': ['1564', '1586'],  'src': 'OHD',  'type': 'Continuous'}, 
                     {'desc': 'GPS L1 jamming',  'freq': ['1585.42', '1585.42'],  'src': 'NM, WSMR',  'type': 'Continuous'}, 
                     {'desc': 'GLONASS L1',  'freq': ['1606.0', '1606.0'],  'src': 'OHD',  'type': 'Continuous'}, 
                     {'desc': 'GLONASS L1',  'freq': ['1616.0', '1616.0'],  'src': 'OHD',  'type': 'Continuous'}, 
                     {'desc': 'IRIDIUM satellites',  'freq': ['1618', '1627'],  'src': 'OHD',  'type': 'Continuous'}, 
                     {'desc': '2nd harmonic VLA radios',  'freq': ['1642', '1642'],  'src': '',  'type': 'Sporadic'},
                     {'desc': 'RADIOSONDES- WX baloons',  'freq': ['1668', '1680'],  'src': '',  'type': 'Intermittent'},
                     {'desc': 'GOES weather satellite',  'freq': ['1683', '1687'],  'src': '',  'type': ''},
                     {'desc': 'GOES weather satellite',  'freq': ['1689', '1693'],  'src': '',  'type': ''},
                     {'desc': 'NOAA weather satellite',  'freq': ['1700', '1702'],  'src': '',  'type': ''},
                     {'desc': 'NOAA weather satellite',  'freq': ['1705', '1709'],  'src': '',  'type': ''},
                     {'desc': 'High alt baloon',  'freq': ['1771.0', '1771.0'],  'src': 'NM, FT Sumner',  'type': 'Intermittent'},
                     {'desc': 'PCS cell phone base stations',  'freq': ['1850', '1990'],  'src': 'Everywhere',  'type': 'Continuous'},
                     {'desc': '??',  'freq': ['2178.0', '2195.0'],  'src': '',  'type': 'Intermittent'},
                     {'desc': 'High alt baloon',  'freq': ['2106.4', '2106.4'],  'src': 'NM, FT Sunmer',  'type': 'Intermittent'},
                     {'desc': 'Test telemetry',  'freq': ['2204.5', '2204.5'],  'src': 'NM, FT Bliss',  'type': 'Intermittent'},
                     {'desc': 'Satellite downlink',  'freq': ['2180', '2290'],  'src': 'OHD?GSO/MEO',  'type': 'Continuous'},
                     {'desc': 'Sirius DARS',  'freq': ['2320.0', '2332.5'],  'src': 'OHD/GSO-Tundra',  'type': 'Continous'},
                     {'desc': '??',  'freq': ['2227.5', '2231.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': '??',  'freq': ['2246.5', '2252.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'}, 
                     {'desc': '??',  'freq': ['2268.5', '2274.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'}, 
                     {'desc': '??',  'freq': ['2282.5', '2288.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': '??',  'freq': ['2314.5', '2320.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': '??',  'freq': ['2324.5', '2330.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': 'XM DARS',  'freq': ['2332.5', '2345.0'],  'src': 'OHD/GSO-Equatoral',  'type': 'Continuous'},
                     {'desc': '??',  'freq': ['2334.5', '2340.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': 'High alt baloon',  'freq': ['2387.5', '2387.5'],  'src': 'NM, FT Sunmer',  'type': 'Intermittent'},
                     {'desc': 'Microwave Ovens',  'freq': ['2400.0', '2483.5'],  'src': 'Everywhere',  'type': 'Continuous'},
                     {'desc': 'WiFi(802.11b) Wireless',  'freq': ['2400.0', '2483.5'],  'src': 'Everywhere',  'type': 'Continuous'},
                     {'desc': 'Satellite downlinks',  'freq': ['3700.0', '4200.0'],  'src': 'OHD',  'type': 'Continuous'},
                     {'desc': '??',  'freq': ['5648.5', '5663.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': '??',  'freq': ['5659.5', '5670.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': '??',  'freq': ['5695.5', '5704.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': '??',  'freq': ['5742.5', '5757.5'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': 'ISM, Wireless Phone?',  'freq': ['5765', '5769'],  'src': '<br />',  'type': 'Intermittent'},
                     {'desc': '??',  'freq': ['5796.0', '5804.0'],  'src': 'NM, WSMR?',  'type': 'Intermittent'},
                     {'desc': 'Microwave Link',  'freq': ['5994.0', '6020.0'],  'src': 'Mangus Mtn, NM',  'type': 'Continuous'},
                     {'desc': 'Microwave Link',  'freq': ['6242.0', '6272.0'],  'src': 'Gray Hill, NM',  'type': 'Continuous'},
                     {'desc': 'Microwave Link',  'freq': ['6610.0', '6622.0'],  'src': 'Gray Hill, NM',  'type': 'Continuous'},
                     {'desc': 'Microwave Link',  'freq': ['6772.0', '6778.0'],  'src': 'Datil/Bagley, NM',  'type': 'Continuous'},
                     {'desc': 'Mil Sat &amp; Met downlinks: GSO &amp; MSS',  'freq': ['7250.0', '7850.0'],  'src': 'OHD',  'type': 'Continuous'}, 
                     {'desc': 'TerraSAR-X/TanDEM-X mapping satellites<br />',  'freq': ['9500.0', '9800.0'],  'src': 'OHD',  'type': 'Intermittent'}, 
                     {'desc': 'strong RFI all along the geostationary orbit (GSO)           path',  'freq': ['12000', '12700'],  'src': 'Satellites',  'type': 'Continuous'}, 
                     {'desc': 'Satellite downlink',  'freq': ['17800', '20200'],  'src': 'Clarke Belt',  'type': 'continuous'}, 
                     {'desc': 'local Wildblue VSAT',  'freq': ['29500', '30000'],  'src': 'Local residences',  'type': 'Intermittent'},
                     {'desc': 'Internal (June 2 to Oct. 8, 2010)',  'freq': ['34875', '34875'],  'src': 'Antenna EA10',  'type': 'Continuous'},
                     {'desc': 'Internal (June 2 to Oct. 8, 2010)',  'freq': ['36286', '36286'],  'src': 'Antenna EA10',  'type': 'Continuous'}]

        chosenrfi = [];
        for rfi in rfilist:
            if( float(rfi['freq'][0]) >= minfreq and float(rfi['freq'][1]) <= maxfreq ):
                chosenrfi.append(rfi);
            
        return chosenrfi;

######################################

######################################
######################################
