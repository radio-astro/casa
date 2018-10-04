from __future__ import absolute_import

import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.hifv.heuristics import getBCalStatistics
from pipeline.hifv.heuristics import getCalFlaggedSoln
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class FlagBadDeformattersInputs(vdp.StandardInputs):
    doflagundernspwlimit = vdp.VisDependentProperty(default=True)

    def __init__(self, context, vis=None, doflagundernspwlimit=None):
        super(FlagBadDeformattersInputs, self).__init__()
        self.context = context
        self.vis = vis
        self.doflagundernspwlimit = doflagundernspwlimit


class FlagBadDeformattersResults(basetask.Results):
    def __init__(self, jobs=None, result_amp=None, result_phase=None,
                 amp_collection=None, phase_collection=None,
                 num_antennas=None):

        if amp_collection is None:
            amp_collection = collections.defaultdict(list)
        if phase_collection is None:
            phase_collection = collections.defaultdict(list)
        if jobs is None:
            jobs = []
        if result_amp is None:
            result_amp = []
        if result_phase is None:
            result_phase = []

        super(FlagBadDeformattersResults, self).__init__()

        self.jobs = jobs
        self.result_amp = result_amp
        self.result_phase = result_phase
        self.amp_collection = amp_collection
        self.phase_collection = phase_collection
        self.num_antennas = num_antennas

    def __repr__(self):
        s = 'Bad deformatter results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 


@task_registry.set_equivalent_casa_task('hifv_flagbaddef')
class FlagBadDeformatters(basetask.StandardTaskTemplate):
    Inputs = FlagBadDeformattersInputs
    
    def prepare(self):
        
        # Setting control parameters as method arguments

        method_args = {'testq': 'amp',  # Which quantity to test? ['amp','phase','real','imag']
                       'tstat': 'rat',  # Which stat to use?['min','max','mean','var']or'rat'=min/max or 'diff'=max-min
                       'doprintall': True,  # Print detailed flagging stats
                       'testlimit': 0.15,   # Limit for test (flag values under/over this limit)
                       'testunder': True,
                       'nspwlimit': 4,      # Number of spw per baseband to trigger flagging entire baseband
                       'doflagundernspwlimit': self.inputs.doflagundernspwlimit,  # Flag individual spws when below nspwlimit
                       'doflagemptyspws': False,  # Flag data for spws with no unflagged channel solutions in any poln?
                       'calBPtablename': self.inputs.context.results[-1].read()[0].bpcaltable,  # Define the table
                       'flagreason': 'bad_deformatters_amp or RFI'}  # Define the REASON given for the flags
        
        (result_amp, amp_collection, num_antennas) = self._do_flag_baddeformatters(**method_args)
        
        method_args = {'testq': 'phase',
                       'tstat': 'diff',
                       'doprintall': True,
                       'testlimit': 50,
                       'testunder': False,
                       'nspwlimit': 4,
                       'doflagundernspwlimit': self.inputs.doflagundernspwlimit,
                       'doflagemptyspws': False,
                       'calBPtablename': self.inputs.context.results[-1].read()[0].bpcaltable,
                       'flagreason': 'bad_deformatters_phase or RFI'}
        
        (result_phase, phase_collection, num_antennas) = self._do_flag_baddeformatters(**method_args)
        
        return FlagBadDeformattersResults(result_amp=result_amp, result_phase=result_phase, 
                                          amp_collection=amp_collection, phase_collection=phase_collection,
                                          num_antennas=num_antennas)
        
    def _do_flag_baddeformatters(self, testq=None, tstat=None, doprintall=True,
                                 testlimit=None, testunder=True, nspwlimit=4,
                                 doflagundernspwlimit=True,
                                 doflagemptyspws=False,
                                 calBPtablename=None, flagreason=None):
        """Determine bad deformatters in the MS and flag them
           Looks for bandpass solutions that have small ratio of min/max amplitudes
        """
        
        # Which quantity to test? ['amp','phase','real','imag']
        # testq = 'amp'
        
        # Which stat to use? ['min','max','mean','var'] or 'rat'=min/max or 'diff'=max-min
        # tstat = 'rat'

        # Print detailed flagging stats
        # doprintall = True

        # From original script...
        # Limit for test (flag values under/over this limit)
        # testlimit = 0.15
        # testunder = True
        # Number of spw per baseband to trigger flagging entire baseband
        # nspwlimit = 4
        # Flag individual spws when below nspwlimit
        # doflagundernspwlimit = True
        # Flag data for spws with no unflagged channel solutions in any poln?
        # doflagemptyspws = False

        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        num_antennas = len(m.antennas)
        startdate = m.start_time['m0']['value']
        
        LOG.info("Start date for flag bad deformatters is: " + str(startdate))
        
        if startdate <= 56062.7:
            doflagdata = False
        else:
            doflagdata = True
        
        # Define the table to run this on
        # calBPtablename ='testBPcal.b'
        # Define the REASON given for the flags
        # flagreason = 'bad_deformatters_amp or RFI'
        
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
        weblogflagdict = collections.defaultdict(list)
        
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
                    if len(spwl) > 0:
                        if doprintall:
                            LOG.info(' Ant %s (%s) %s %s processing spws=%s' %
                                     (str(iant), antName, rrx, bband, str(spwl)))
                        
                        for ispw in spwl:
                            testvalid = False
                            if ispw in calBPstatresult['antspw'][iant]:
                                for poln in calBPstatresult['antspw'][iant][ispw].keys():
                                    # Get stats of this ant/spw/poln
                                    nbp = calBPstatresult['antspw'][iant][ispw][poln]['inner']['number']
                                    
                                    if nbp > 0:
                                        if tstat == 'rat':
                                            bpmax = calBPstatresult['antspw'][iant][ispw][poln]['inner'][testq]['max']
                                            bpmin = calBPstatresult['antspw'][iant][ispw][poln]['inner'][testq]['min']
                                            
                                            if bpmax == 0.0:
                                                tval = 0.0
                                            else:
                                                tval = bpmin/bpmax
                                        elif tstat == 'diff':
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
                                            if tval < testval:
                                                testval = tval
                                        else:
                                            if tval > testval:
                                                testval = tval
                                # Test on extrema of the polarizations for this ant/spw
                                if not testvalid:
                                    # these have no unflagged channels in any poln
                                    flaggedspws.append(ispw)
                                else:
                                    if (testunder and testval < testlimit) or (not testunder and testval > testlimit):
                                        nbadspws += 1
                                        badspws.append(ispw)
                                        if doprintall:
                                            LOG.info('  Found Ant %s (%s) %s %s spw=%s %s %s=%6.4f' %
                                                     (str(iant), antName, rrx, bband, str(ispw), testq, tstat, testval))
                            
                            else:
                                # this spw is missing from this antenna/rx
                                if doprintall:
                                    LOG.info('  Ant %s (%s) %s %s spw=%s missing solution' %
                                             (str(iant), antName, rrx, bband, str(ispw)))
                    
                    # Test to see if this baseband should be entirely flagged
                    if nbadspws > 0 and nbadspws >= nspwlimit:
                        # Flag all spw in this baseband
                        bbspws = calBPstatresult['rxBasebandDict'][rrx][bband]
                        badspwlist.extend(bbspws)
                        LOG.info('Ant %s (%s) %s %s bad baseband spws=%s' %
                                 (str(iant), antName, rrx, bband, str(bbspws)))
                    elif nbadspws > 0 and doflagundernspwlimit:
                        # Flag spws individually
                        badspwlist.extend(badspws)
                        LOG.info('Ant %s (%s) %s %s bad spws=%s' % (str(iant), antName, rrx, bband, str(badspws)))
                    if len(flaggedspws) > 0:
                        flaggedspwlist.extend(flaggedspws)
                        LOG.info('Ant %s (%s) %s %s no unflagged solutions spws=%s ' %
                                 (str(iant), antName, rrx, bband, str(flaggedspws)))
                                    
            if len(badspwlist) > 0:
                spwstr = '' 
                for ispw in badspwlist:
                    if spwstr == '':
                        spwstr = str(ispw)
                    else:
                        spwstr += ','+str(ispw)
                #
                # reastr = 'bad_deformatters'
                reastr = flagreason
                # Add entry for this antenna
                # flagstr = "mode='manual' antenna='"+str(iant)+"' spw='"+spwstr+"' reason='"+reastr+"'"
                # Use name for flagging
                flagstr = "mode='manual' antenna='"+antName+"' spw='"+spwstr+"'"
                flaglist.append(flagstr)
                weblogflagdict[antName].append(spwstr)
                
            if doflagemptyspws and len(flaggedspwlist) > 0:
                spwstr = '' 
                for ispw in flaggedspwlist:
                    if spwstr == '':
                        spwstr = str(ispw)
                    else:
                        spwstr += ','+str(ispw)
                #
                # Add entry for this antenna
                reastr = 'no_unflagged_solutions'
                # flagstr = "mode='manual' antenna='"+str(iant)+"' spw='"+spwstr+"' reason='"+reastr+"'"
                # Use name for flagging
                flagstr = "mode='manual' antenna='"+antName+"' spw='"+spwstr+"'"
                extflaglist.append(flagstr)
                weblogflagdict[antName].append(spwstr)

        # Get basebands matched with spws.  spws is a single element list with a single csv string
        tempDict = {}
        for antNamekey, spws in weblogflagdict.iteritems():
            basebands = []
            for spwstr in spws[0].split(','):
                spw = m.get_spectral_window(spwstr)
                basebands.append(spw.name.split('#')[0] + '  ' + spw.name.split('#')[1])
            basebands = list(set(basebands))  # Unique basebands
            tempDict[antNamekey] = {'spws': spws, 'basebands': basebands}

        weblogflagdict = tempDict

        nflagcmds = len(flaglist) + len(extflaglist)
        if nflagcmds < 1:
            LOG.info("No bad basebands/spws found")
        else:
            LOG.info("Possible bad basebands/spws found:")
            
            for flagstr in flaglist:
                LOG.info("    "+flagstr)
            if len(extflaglist) > 0:
                LOG.info("    ")
                for flagstr in extflaglist:
                    LOG.info("    "+flagstr)
                flaglist.extend(extflaglist)
            if doflagdata:
                LOG.info("Flagging bad deformatters in the ms...")
                
                task_args = {'vis': self.inputs.vis,
                             'mode': 'list',
                             'action': 'apply',
                             'inpfile': flaglist,
                             'savepars': True,
                             'flagbackup': True}
                
                job = casa_tasks.flagdata(**task_args)
                
                self._executor.execute(job)

                # get the total fraction of data flagged for all antennas
                # flagging_stats = getCalFlaggedSoln(calBPtablename)
                # total = 0
                # flagged = 0
                # for antenna in flagging_stats['ant']:
                #     for pol in flagging_stats['ant'][antenna]:
                #         flagged += flagging_stats['ant'][antenna][pol]['flagged']
                #         total += flagging_stats['ant'][antenna][pol]['total']
                # fraction_flagged = flagged / total

                # LOG.info('Flagged ({}) Total ({}) Fraction ({})'.format(flagged, total, fraction_flagged))
                return flaglist, weblogflagdict, num_antennas

        # If the flag commands are not executed.
        return [], collections.defaultdict(list), num_antennas

    def analyse(self, results):
        return results
