from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary

import itertools

from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.hifv.heuristics import getCalFlaggedSoln, getBCalStatistics

LOG = infrastructure.get_logger(__name__)

class FlagBadDeformattersInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class FlagBadDeformattersResults(basetask.Results):
    def __init__(self, jobs=[]):
        super(FlagBadDeformattersResults, self).__init__()

        self.jobs=jobs
        
    def __repr__(self):
        s = 'Bad deformatter results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 

class FlagBadDeformatters(basetask.StandardTaskTemplate):
    Inputs = FlagBadDeformattersInputs
    
    def prepare(self):
        
        #Setting control parameters as method arguments

        method_args = {'testq' : 'amp',  # Which quantity to test? ['amp','phase','real','imag']
                       'tstat' : 'rat',  # Which stat to use? ['min','max','mean','var'] or 'rat'=min/max or 'diff'=max-min
                       'doprintall' : True,  # Print detailed flagging stats
                       'testlimit' : 0.15,   # Limit for test (flag values under/over this limit)
                       'testunder' : True,
                       'nspwlimit' : 4,      # Number of spw per baseband to trigger flagging entire baseband
                       'doflagundernspwlimit' : True, # Flag individual spws when below nspwlimit
                       'doflagemptyspws' : False, # Flag data for spws with no unflagged channel solutions in any poln?
                       'calBPtablename' : 'testBPcal.b', # Define the table to run this on
                       'flagreason' : 'bad_deformatters_amp or RFI'} # Define the REASON given for the flags
        
        bad_deformat_result = self._do_flag_baddeformatters(**method_args)
        
        method_args = {'testq' : 'phase',
                       'tstat' : 'diff',
                       'doprintall' : True,
                       'testlimit' : 50,
                       'testunder' : False,
                       'nspwlimit' : 4,
                       'doflagundernspwlimit' : True,
                       'doflagemptyspws' : False,
                       'calBPtablename' : 'testBPcal.b',
                       'flagreason' : 'bad_deformatters_phase or RFI'}
        
        bad_deformat_result = self._do_flag_baddeformatters(**method_args)
        
        return bad_deformat_result
        
    def _do_flag_baddeformatters(self, testq=None, tstat=None, doprintall=True,
                                 testlimit=None, testunder=True, nspwlimit=4,
                                 doflagundernspwlimit=True,
                                 doflagemptyspws=False,
                                 calBPtablename=None, flagreason=None):
        """Determine bad deformatters in the MS and flag them
           Looks for bandpass solutions that have small ratio of min/max amplitudes
        """
        
        # Which quantity to test? ['amp','phase','real','imag']
        ####testq = 'amp'
        
        # Which stat to use? ['min','max','mean','var'] or 'rat'=min/max or 'diff'=max-min
        ####tstat = 'rat'

        # Print detailed flagging stats
        ####doprintall = True

        # Limit for test (flag values under/over this limit)
        ####testlimit = 0.15
        ####testunder = True
        # Number of spw per baseband to trigger flagging entire baseband
        ####nspwlimit = 4
        # Flag individual spws when below nspwlimit
        ####doflagundernspwlimit = True
        # Flag data for spws with no unflagged channel solutions in any poln?
        ####doflagemptyspws = False
        
        with casatools.MSReader(self.inputs.vis) as ms:
            ms_summary = ms.summary()

        startdate = ms_summary['BeginTime']
        
        if startdate <= 56062.7:
            doflagdata = False
        else:
            doflagdata = True
        
        # Define the table to run this on
        ####calBPtablename ='testBPcal.b'
        # Define the REASON given for the flags
        ####flagreason = 'bad_deformatters_amp or RFI'
        
        LOG.info("Will test on quantity: "+testq)
        LOG.info("Will test using statistic: "+tstat)
        
        if testunder:
            LOG.info("Will flag values under limit = "+str(testlimit))
        else:
            LOG.info("Will flag values over limit = "+str(testlimit))
        
        LOG.info("Will identify basebands with more than "+str(nspwlimit)+" bad spw")
        
        if doflagundernspwlimit:
            LOG.info("Will identify individual spw when less than "+str(nspwlimit)+" bad spw")
            
        if doflagemptyspws:
            LOG.info("Will identify spw with no unflagged channels")
            
        LOG.info("Will use flag REASON = "+flagreason)
        
        if doflagdata:
            LOG.info("Will flag data based on what we found")
        else:
            LOG.info("Will NOT flag data based on what we found")
        
        calBPstatresult = getBCalStatistics(calBPtablename)
        flaglist = []
        extflaglist = []
        
        for iant in calBPstatresult['antband'].keys():
            antName = calBPstatresult['antDict'][iant]
            badspwlist = []
            flaggedspwlist = []
            for rrx in calBPstatresult['antband'][iant].keys():
                for bband in calBPstatresult['antband'][iant][rrx].keys():
                    # List of spw in this baseband
                    spwl = calBPstatresult['rxBasebandDict'][rrx][bband]
                    
                    nbadspws = 0
                    badspws = []
                    flaggedspws = []
                    if len(spwl)>0:
                        if doprintall:
                            LOG.info(' Ant %s (%s) %s %s processing spws=%s' % (str(iant),antName,rrx,bband,str(spwl)))
                        
                        for ispw in spwl:
                            testvalid = False
                            if calBPstatresult['antspw'][iant].has_key(ispw):
                                for poln in calBPstatresult['antspw'][iant][ispw].keys():
                                    # Get stats of this ant/spw/poln
                                    nbp = calBPstatresult['antspw'][iant][ispw][poln]['inner']['number']
                                    
                                    if nbp>0:
                                        if tstat=='rat':
                                            bpmax = calBPstatresult['antspw'][iant][ispw][poln]['inner'][testq]['max']
                                            bpmin = calBPstatresult['antspw'][iant][ispw][poln]['inner'][testq]['min']
                                            
                                            if bpmax==0.0:
                                                tval = 0.0
                                            else:
                                                tval = bpmin/bpmax
                                        elif tstat=='diff':
                                            bpmax = calBPstatresult['antspw'][iant][ispw][poln]['inner'][testq]['max']
                                            bpmin = calBPstatresult['antspw'][iant][ispw][poln]['inner'][testq]['min']
                                            
                                            tval = bpmax-bpmin
                                        else:
                                            # simple test on quantity
                                            tval = calBPstatresult['antspw'][iant][ispw][poln]['inner'][testq][tstat]
                                        if not testvalid:
                                            testval = tval
                                            testvalid = True
                                        elif testunder:
                                            if tval<testval:
                                                testval = tval
                                        else:
                                            if tval>testval:
                                                testval = tval
                                # Test on extrema of the polarizations for this ant/spw
                                if not testvalid:
                                    # these have no unflagged channels in any poln
                                    flaggedspws.append(ispw)
                                else:
                                    if (testunder and testval<testlimit) or (not testunder and testval>testlimit):
                                        nbadspws += 1
                                        badspws.append(ispw)
                                        if doprintall:
                                            LOG.info('  Found Ant %s (%s) %s %s spw=%s %s %s=%6.4f' % (str(iant),antName,rrx,bband,str(ispw),testq,tstat,testval))
                            
                            else:
                                # this spw is missing from this antenna/rx
                                if doprintall:
                                    LOG.info('  Ant %s (%s) %s %s spw=%s missing solution' % (str(iant),antName,rrx,bband,str(ispw)))
                    
                    # Test to see if this baseband should be entirely flagged
                    if nbadspws>0 and nbadspws>=nspwlimit:
                        # Flag all spw in this baseband
                        bbspws = calBPstatresult['rxBasebandDict'][rrx][bband]
                        badspwlist.extend(bbspws)
                        LOG.info('Ant %s (%s) %s %s bad baseband spws=%s' % (str(iant),antName,rrx,bband,str(bbspws)))
                    elif nbadspws>0 and doflagundernspwlimit:
                        # Flag spws individually
                        badspwlist.extend(badspws)
                        LOG.info('Ant %s (%s) %s %s bad spws=%s' % (str(iant),antName,rrx,bband,str(badspws)))
                    if len(flaggedspws)>0:
                        flaggedspwlist.extend(flaggedspws)
                        LOG.info('Ant %s (%s) %s %s no unflagged solutions spws=%s ' % (str(iant),antName,rrx,bband,str(flaggedspws)))
                                    
            if len(badspwlist)>0:
                spwstr = '' 
                for ispw in badspwlist:
                    if spwstr=='':
                        spwstr = str(ispw)
                    else:
                        spwstr+=','+str(ispw)
                #
                #reastr = 'bad_deformatters'
                reastr = flagreason
                # Add entry for this antenna
                #flagstr = "mode='manual' antenna='"+str(iant)+"' spw='"+spwstr+"' reason='"+reastr+"'"
                # Use name for flagging
                flagstr = "mode='manual' antenna='"+antName+"' spw='"+spwstr+"'"
                flaglist.append(flagstr)
                
            if doflagemptyspws and len(flaggedspwlist)>0:
                spwstr = '' 
                for ispw in flaggedspwlist:
                    if spwstr=='':
                        spwstr = str(ispw)
                    else:
                        spwstr+=','+str(ispw)
                #
                # Add entry for this antenna
                reastr = 'no_unflagged_solutions'
                #flagstr = "mode='manual' antenna='"+str(iant)+"' spw='"+spwstr+"' reason='"+reastr+"'"
                # Use name for flagging
                flagstr = "mode='manual' antenna='"+antName+"' spw='"+spwstr+"'"
                extflaglist.append(flagstr)
        
        nflagcmds = len(flaglist)+len(extflaglist)
        if nflagcmds<1:
            LOG.info("No bad basebands/spws found")
        else:
            LOG.info("Possible bad basebands/spws found:")
            
            for flagstr in flaglist:
                LOG.info("    "+flagstr)
            if len(extflaglist)>0:
                LOG.info("    ")
                for flagstr in extflaglist:
                    LOG.info("    "+flagstr)
                flaglist.extend(extflaglist)
            if doflagdata:
                LOG.info("Flagging bad deformatters in the ms...")
                
                task_args = {'vis'        : self.inputs.vis,
                             'mode'       : 'list',
                             'action'     : 'apply',                     
                             'inpfile'    : flaglist,
                             'savepars'   : True,
                             'flagbackup' : True}
                
                job = casa_tasks.flagdata(**task_args)
                
                self._executor.execute(job)
                
                return FlagBadDeformattersResults([job])
                
        #If the flag commands are not executed.
        return FlagBadDeformattersResults()
        
        
    def analyse(self, results):
	return results
