import os
from casac import casac
from taskinit import casalog
import asap as sd
from asap import _to_list
from asap.scantable import is_scantable
import numpy as np

qatl = casac.quanta()

def assert_infile_exists(infile=None):
    if (infile == ""):
        raise Exception, "infile is undefined"

    filename = os.path.expandvars(infile)
    filename = os.path.expanduser(filename)
    if not os.path.exists(filename):
        mesg = "File '%s' not found." % (filename)
        raise Exception, mesg


def get_default_outfile_name(infile=None, outfile=None, suffix=None):
    if (outfile == ""):
        res = infile.rstrip("/") + suffix
    else:
        res = outfile
    return res


def assert_outfile_canoverwrite_or_nonexistent(outfile=None, outform=None, overwrite=None):
    filename = os.path.expandvars(outfile)
    filename = os.path.expanduser(filename)
    if not overwrite and (outform.upper != "ASCII"):
        if os.path.exists(filename):
            mesg = "Output file '%s' exists." % (filename)
            raise Exception, mesg


def get_listvalue(value):
    return _to_list(value, int) or []

def get_selector(in_scans=None, in_ifs=None, in_pols=None, \
                 in_field=None, in_beams=None, in_rows=None):
    scans = get_listvalue(in_scans)
    ifs   = get_listvalue(in_ifs)
    pols  = get_listvalue(in_pols)
    beams = get_listvalue(in_beams)
    rows = get_listvalue(in_rows)
    selector = sd.selector(scans=scans, ifs=ifs, pols=pols, beams=beams,
                           rows=rows)

    if (in_field != ""):
        # NOTE: currently can only select one
        # set of names this way, will probably
        # need to do a set_query eventually
        selector.set_name(in_field)

    return selector


def get_restfreq_in_Hz(s_restfreq):
    if not qatl.isquantity(s_restfreq):
        mesg = "Input value is not a quantity: %s" % (str(s_restfreq))
        raise Exception, mesg
    if qatl.compare(s_restfreq,'Hz'):
        return qatl.convert(s_restfreq, 'Hz')['value']
    elif qatl.quantity(s_restfreq)['unit'] == '':
        return float(s_restfreq)
    else:
        mesg = "wrong unit of restfreq."
        raise Exception, mesg
###############################################################
# def get_restfreq_in_Hz(s_restfreq):
#     value = 0.0
#     unit = ""
#     s = s_restfreq.replace(" ","")
# 
#     for i in range(len(s))[::-1]:
#         if s[i].isalpha():
#             unit = s[i] + unit
#         else:
#             value = float(s[0:i+1])
#             break
# 
#     if (unit == "") or (unit.lower() == "hz"):
#         return value
#     elif (len(unit) == 3) and (unit[1:3].lower() == "hz"):
#         unitprefix = unit[0]
#         factor = 1.0
# 
#         if (unitprefix == "a"):
#             factor = 1.0e-18
#         elif (unitprefix == "f"):
#             factor = 1.0e-15
#         elif (unitprefix == "p"):
#             factor = 1.0e-12
#         elif (unitprefix == "n"):
#             factor = 1.0e-9
#         elif (unitprefix == "u"):
#             factor = 1.0e-6
#         elif (unitprefix == "m"):
#             factor = 1.0e-3
#         elif (unitprefix == "k"):
#             factor = 1.0e+3
#         elif (unitprefix == "M"):
#             factor = 1.0e+6
#         elif (unitprefix == "G"):
#             factor = 1.0e+9
#         elif (unitprefix == "T"):
#             factor = 1.0e+12
#         elif (unitprefix == "P"):
#             factor = 1.0e+15
#         elif (unitprefix == "E"):
#             factor = 1.0e+18
#         
#         return value*factor
#     else:
#         mesg = "wrong unit of restfreq."
#         raise Exception, mesg
###############################################################

def normalise_restfreq(in_restfreq):
    if isinstance(in_restfreq, float):
        return in_restfreq
    elif isinstance(in_restfreq, int) or isinstance(in_restfreq, long):
        return float(in_restfreq)
    elif isinstance(in_restfreq, str):
        return get_restfreq_in_Hz(in_restfreq)
    elif isinstance(in_restfreq, list) or isinstance(in_restfreq, np.ndarray):
        if isinstance(in_restfreq, np.ndarray):
            if len(in_restfreq.shape) > 1:
                mesg = "given in numpy.ndarray, in_restfreq must be 1-D."
                raise Exception, mesg
        
        res = []
        for i in xrange(len(in_restfreq)):
            elem = in_restfreq[i]
            if isinstance(elem, float):
                res.append(elem)
            elif isinstance(elem, int) or isinstance(elem, long):
                res.append(float(elem))
            elif isinstance(elem, str):
                res.append(get_restfreq_in_Hz(elem))
            elif isinstance(elem, dict):
                if isinstance(elem["value"], float):
                    res.append(elem)
                elif isinstance(elem["value"], int):
                    dictelem = {}
                    dictelem["name"]  = elem["name"]
                    dictelem["value"] = float(elem["value"])
                    res.append(dictelem)
                elif isinstance(elem["value"], str):
                    dictelem = {}
                    dictelem["name"]  = elem["name"]
                    dictelem["value"] = get_restfreq_in_Hz(elem["value"])
                    res.append(dictelem)
            else:
                mesg = "restfreq elements must be float, int, or string."
                raise Exception, mesg
        return res
    else:
        mesg = "wrong type of restfreq given."
        raise Exception, mesg

def set_restfreq(s, restfreq):
    rfset = (restfreq != '') and (restfreq != [])
    if rfset:
        molids = s._getmolidcol_list()
        s.set_restfreqs(normalise_restfreq(restfreq))

def set_spectral_unit(s, specunit):
    if (specunit != ''):
        s.set_unit(specunit)

def set_doppler(s, doppler):
    if (doppler != ''):
        if (doppler in ['radio', 'optical', 'z']):
            ddopler = doppler.upper()
        else:
            ddopler = doppler
        s.set_doppler(ddoppler)
    else:
        casalog.post('Using current doppler conversion')

def set_freqframe(s, frame):
    if (frame != ''):
        s.set_freqframe(frame)
    else:
        casalog.post('Using current frequency frame')

def set_fluxunit(s, fluxunit, telescopeparm, insitu=True):
    ret = None
    
    # check current fluxunit
    # for GBT if not set, set assumed fluxunit, Kelvin
    antennaname = s.get_antennaname()
    fluxunit_now = s.get_fluxunit()
    if (antennaname == 'GBT'):
        if (fluxunit_now == ''):
            casalog.post('No fluxunit in the data. Set to Kelvin.')
            s.set_fluxunit('K')
            fluxunit_now = s.get_fluxunit()
    casalog.post('Current fluxunit = %s'%(fluxunit_now))

    # convert flux
    # set flux unit string (be more permissive than ASAP)
    if (fluxunit == 'k'):
        fluxunit_local = 'K'
    elif (fluxunit.upper() == 'JY'):
        fluxunit_local = 'Jy'
    else:
        fluxunit_local = fluxunit

        
    # fix the fluxunit if necessary
    if ( telescopeparm == 'FIX' or telescopeparm == 'fix' ):
        if ( fluxunit_local != '' ):
            if ( fluxunit_local == fluxunit_now ):
                #print "No need to change default fluxunits"
                casalog.post( "No need to change default fluxunits" )
            else:
                s.set_fluxunit(fluxunit_local)
                #print "Reset default fluxunit to "+fluxunit
                casalog.post( "Reset default fluxunit to "+fluxunit_local )
                fluxunit_now = s.get_fluxunit()
        else:
            #print "Warning - no fluxunit for set_fluxunit"
            casalog.post( "no fluxunit for set_fluxunit", priority = 'WARN' )


    elif ( fluxunit_local=='' or fluxunit_local==fluxunit_now ):
        if ( fluxunit_local==fluxunit_now ):
            #print "No need to convert fluxunits"
            casalog.post( "No need to convert fluxunits" )

    elif ( type(telescopeparm) == list ):
        # User input telescope params
        if ( len(telescopeparm) > 1 ):
            D = telescopeparm[0]
            eta = telescopeparm[1]
            #print "Use phys.diam D = %5.1f m" % (D)
            #print "Use ap.eff. eta = %5.3f " % (eta)
            casalog.post( "Use phys.diam D = %5.1f m" % (D) )
            casalog.post( "Use ap.eff. eta = %5.3f " % (eta) )
            ret = s.convert_flux(eta=eta,d=D,insitu=insitu)
        elif ( len(telescopeparm) > 0 ):
            jypk = telescopeparm[0]
            #print "Use gain = %6.4f Jy/K " % (jypk)
            casalog.post( "Use gain = %6.4f Jy/K " % (jypk) )
            ret = s.convert_flux(jyperk=jypk,insitu=insitu)
        else:
            #print "Empty telescope list"
            casalog.post( "Empty telescope list" )

    elif ( telescopeparm=='' ):
        if ( antennaname == 'GBT'):
            # needs eventually to be in ASAP source code
            #print "Convert fluxunit to "+fluxunit
            casalog.post( "Convert fluxunit to "+fluxunit_local )
            # THIS IS THE CHEESY PART
            # Calculate ap.eff eta at rest freq
            # Use Ruze law
            #   eta=eta_0*exp(-(4pi*eps/lambda)**2)
            # with
            #print "Using GBT parameters"
            casalog.post( "Using GBT parameters" )
            eps = 0.390  # mm
            eta_0 = 0.71 # at infinite wavelength
            # Ideally would use a freq in center of
            # band, but rest freq is what I have
            rf = s.get_restfreqs()[0][0]*1.0e-9 # GHz
            eta = eta_0*np.exp(-0.001757*(eps*rf)**2)
            #print "Calculated ap.eff. eta = %5.3f " % (eta)
            #print "At rest frequency %5.3f GHz" % (rf)
            casalog.post( "Calculated ap.eff. eta = %5.3f " % (eta) )
            casalog.post( "At rest frequency %5.3f GHz" % (rf) )
            D = 104.9 # 100m x 110m
            #print "Assume phys.diam D = %5.1f m" % (D)
            casalog.post( "Assume phys.diam D = %5.1f m" % (D) )
            ret = s.convert_flux(eta=eta,d=D,insitu=insitu)
            
            #print "Successfully converted fluxunit to "+fluxunit
            casalog.post( "Successfully converted fluxunit to "+fluxunit_local )
        elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
            ret = s.convert_flux(insitu=insitu)
            
        else:
            # Unknown telescope type
            #print "Unknown telescope - cannot convert"
            casalog.post( "Unknown telescope - cannot convert", priority = 'WARN' )

    return ret
    
def save(s, outfile, outform, overwrite):
    assert_outfile_canoverwrite_or_nonexistent(outfile,
                                               outform,
                                               overwrite)
    if ( (outform == 'ASCII') or (outform == 'ascii') ):
            outform_local = 'ASCII'
            outfile_local = outfile #+ '_'
    elif ( (outform == 'ASAP') or (outform == 'asap') ):
            outform_local = 'ASAP'
            outfile_local = outfile
    elif ( (outform == 'SDFITS') or (outform == 'sdfits') ):
            outform_local = 'SDFITS'
            outfile_local = outfile
    elif ( (outform == 'MS') or (outform == 'ms') or (outform == 'MS2') or (outform == 'ms2') ):
            outform_local = 'MS2'
            outfile_local = outfile
    else:
            outform_local = 'ASAP'
            outfile_local = outfile

    outfilename = os.path.expandvars(outfile_local)
    outfilename = os.path.expanduser(outfilename)
    if overwrite and os.path.exists(outfilename):
        os.system('rm -rf %s' % outfilename)

    s.save(outfile_local, outform_local, overwrite)
    
    if outform_local!='ASCII':
        casalog.post('Wrote output %s file %s'%(outform_local,outfile_local))

def doopacity(s, tau):
    antennaname = s.get_antennaname()
    if (tau > 0.0):
        if (antennaname != 'GBT'):
            s.recalc_azel()
        s.opacity(tau, insitu=True)

def dochannelrange(s, channelrange):
    # channel splitting
    if ( channelrange != [] ):
        if ( len(channelrange) == 1 ):
            #print "Split spectrum in the range [%d, %d]" % (0, channelrange[0])
            casalog.post( "Split spectrum in the range [%d, %d]" % (0, channelrange[0]) )
            s._reshape( 0, int(channelrange[0]) )
        else:
            #print "Split spectrum in the range [%d, %d]" % (channelrange[0], channelrange[1])
            casalog.post( "Split spectrum in the range [%d, %d]" % (channelrange[0], channelrange[1]) )
            s._reshape( int(channelrange[0]), int(channelrange[1]) )


def doaverage(s, scanaverage, timeaverage, tweight, polaverage, pweight,
              averageall=False):
    # Average in time if desired
    sret = None
    if ( timeaverage ):
        if tweight=='none':
            errmsg = "Please specify weight type of time averaging"
            raise Exception,errmsg
        stave=sd.average_time(s,weight=tweight,scanav=scanaverage,compel=averageall)
        # Now average over polarizations;
        if ( polaverage ):
            if pweight=='none':
                errmsg = "Please specify weight type of polarization averaging"
                raise Exception,errmsg
            np = len(stave.getpolnos())
            if ( np > 1 ):
                sret=stave.average_pol(weight=pweight)
            else:
                # only single polarization
                #print "Single polarization data - no need to average"
                casalog.post( "Single polarization data - no need to average" )
                sret = stave
        else:
            sret = stave
        #    spave=stave.copy()
        
    else:
        #if ( scanaverage ):
        #        # scan average if the input is a scantable
        #        spave=sd.average_time(scal,weight=pweight,scanav=True)
        #        scal=spave.copy()
        if ( polaverage ):
            if pweight=='none':
                errmsg = "Please specify weight type of polarization averaging"
                raise Exception,errmsg
            np = s.npol()
            if ( np > 1 ):
                if not scanaverage:
                    sret = sd.average_time(s,weight=pweight)
                else:
                    sret = s
                sret=sret.average_pol(weight=pweight)
            else:
                # only single polarization
                #print "Single polarization data - no need to average"
                casalog.post( "Single polarization data - no need to average" )
                #spave=scal.copy()
                sret = s
        else:
            if scanaverage:
                sret=sd.average_time(s,scanav=True)
            else:
                #spave=scal.copy()
                sret = s
    return sret

def plot_scantable(s, pltfile, plotlevel, comment=None):
    # reset plotter
    if sd.plotter._plotter:
        sd.plotter._plotter.quit()
    visible = sd.plotter._visible
    sd.plotter.__init__(visible=visible)
    # each IF is separate panel, pols stacked
    sd.plotter.set_mode(stacking='p',panelling='i',refresh=False)
    sd.plotter.set_histogram(hist=True,linewidth=1,refresh=False)
    sd.plotter.plot(s)
    if comment is not None:
        # somehow I need to put text() twice in order to the second
        # text() actually displays on the plot...
        sd.plotter.text(0.0, 1.0,'',coords='relative')
        #sd.plotter.text(0.0, 1.0,'Raw spectra', coords='relative')
        sd.plotter.text(0.0, 1.0,comment, coords='relative')
    #sd.plotter.axhline(color='r',linewidth=2)
    if ( plotlevel < 0 ):
        # Hardcopy - currently no way w/o screen display first
        #pltfile=project+'_'+suffix+'.eps'
        sd.plotter.save(pltfile)

def scantable_restore_factory(s, infile, fluxunit, specunit, frame, doppler, restfreq=''):
    storage = sd.rcParams['scantable.storage']
    isscantable = is_scantable(infile)
    if storage == 'memory' or isscantable == False:
        return scantable_restore_null(s, fluxunit, specunit, frame, doppler, restfreq)
    else:
        return scantable_restore_impl(s, fluxunit, specunit, frame, doppler, restfreq)

class scantable_restore_interface(object):
    def __init__(self, s=None, fluxunit=None, specunit=None, frame=None, doppler=None, restfreq=None):
        pass

    def restore(self):
        raise NotImplementedError('scantable_restore.restore() is not implemented')

class scantable_restore_null(scantable_restore_interface):
    def __init__(self, s, fluxunit, specunit, frame, doppler, restfreq=''):
        super(scantable_restore_null,self).__init__()

    def restore(self):
        pass
    
        
class scantable_restore_impl(scantable_restore_interface):
    def __init__(self, s, fluxunit, specunit, frame, doppler, restfreq=''):
        super(scantable_restore_impl,self).__init__()
        self.scntab = s
        self.fluxunit = s.get_fluxunit()
        self.specunit = s.get_unit()
        self.coord = s._getcoordinfo()
        self.frame = self.coord[1]
        self.doppler = self.coord[2]
        self.molids = s._getmolidcol_list()
        self.rfset = ((restfreq != '') and (restfreq != []))
        self.dorestore = (self.rfset) and \
                         ((frame != '' and frame != self.frame) or \
                          (doppler != '' and doppler != self.doppler) or \
                          (fluxunit != '' and fluxunit != self.fluxunit) or \
                          (specunit != '' and specunit != self.specunit))
        #casalog.post('dorestore=%s'%(self.dorestore))

    def restore(self):
        self.scntab.set_selection()
        
        if self.dorestore:
            casalog.post('Restoreing header information in input scantable')
            self._restore()
                         
    def _restore(self):
        self.scntab.set_fluxunit(self.fluxunit)
        self.scntab.set_unit(self.specunit)
        self.scntab.set_doppler(self.doppler)
        self.scntab.set_freqframe(self.frame)
        if self.rfset:
            self.scntab._setmolidcol_list(self.molids)
                     

    
