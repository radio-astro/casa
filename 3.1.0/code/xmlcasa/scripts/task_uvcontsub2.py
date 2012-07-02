import os
import re
import shutil
import tempfile
from taskinit import *
from update_spw import *

def uvcontsub2(vis, field, fitspw, combine, solint, fitorder, spw, want_cont):
    try:
        casalog.origin('uvcontsub2')

        # Get this check done and out of the way.
        if not os.path.isdir(vis):
            raise Exception, 'Visibility data set not found - please verify the name'
        # cb will put the continuum-subtracted data in CORRECTED_DATA, so
        # protect vis by splitting out its relevant parts to a working copy.
        ms.open(vis, nomodify=True)
        csvis = vis.rstrip('/') + '.contsub'
        
        # The working copy will need all of the channels in fitspw + spw, so we
        # may or may not need an additional split at the end to filter it down
        # to spw.
        do_resplit = False
        myfitspw = fitspw
        myspw = spw

        tempspw = ''  # Conservatively default to keeping everything.
        if 'spw' not in combine:
            # We only need the spws (but potentially all the channels) listed
            # in fitspw.
            tempspw = re.sub(r':[^,]+', '', fitspw)
        if tempspw:
            # The split will reindex the spws.  Update spw and fitspw.
            # Do fitspw first because the spws in spw are supposed to be
            # a subset of the ones in fitspw.
            myfitspw, spwmap = update_spw(fitspw, None)
            myspw = update_spw(spw, spwmap)

        if spw and spw != tempspw:
            do_resplit = True
            final_csvis = csvis
            csvis = tempfile.mkdtemp(suffix=csvis)

        # tempspw is chan free, so averchan doesn't matter.
        ms.split(csvis, field=field, spw=tempspw, whichcol='CORRECTED_DATA')
        ms.close()       
        
        if (type(csvis) == str) and os.path.isdir(csvis):
            cb.open(csvis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'

        # for now, forbid requests for fitorder>0 
        if (fitorder>0):
            raise Exception, "Sorry, uvcontsub2 currently only supports fitorder=0."
        
        # select the data for continuum subtraction
        cb.reset()
        cb.selectvis(spw=myfitspw,field=field)

        # Arrange apply of existing other calibrations
        # First do the existing cal tables...
        #ngaintab = 0;
        #if (gaintable!=['']):
        #    ngaintab=len(gaintable)
        #ngainfld = len(gainfield)
        #nspwmap = len(spwmap)
        #ninterp = len(interp)

        # handle list of list issues with spwmap
        #if (nspwmap>0):
        #       if (type(spwmap[0])!=list):
        #           # first element not a list, only one spwmap specified
        #           # make it a list of list
        #           spwmap=[spwmap];
        #           nspwmap=1;

        #for igt in range(ngaintab):
        #    if (gaintable[igt]!=''):

        #        # field selection is null unless specified
        #        thisgainfield=''
        #        if (igt<ngainfld):
        #            thisgainfield=gainfield[igt]

        #           # spwmap is null unless specifed
        #           thisspwmap=[-1]
        #           if (igt<nspwmap):
        #               thisspwmap=spwmap[igt];

        #           # interp is 'linear' unless specified
        #           thisinterp='linear'
        #           if (igt<ninterp):
        #               if (interp[igt]==''):
        #                   interp[igt]=thisinterp
        #               thisinterp=interp[igt]
                    
        #           cb.setapply(t=0.0,table=gaintable[igt],field=thisgainfield,
        #               calwt=True,spwmap=thisspwmap,interp=thisinterp)

        # ...and now the specialized terms
        # (BTW, interp irrelevant for these, since they are evaluated)
        #if (opacity>0.0): cb.setapply(type='TOPAC',t=-1,opacity=opacity,calwt=True)
        #if gaincurve: cb.setapply(type='GAINCURVE',t=-1,calwt=True)

        # Apply parallactic angle, if requested
        #if parang: cb.setapply(type='P')

        # Set up the solve
        amuellertab = 'Temp_contsub.tab'

        cb.setsolve(type='A', t=solint, table=amuellertab, combine=combine)

        # solve for the continuum
        cb.solve()

        # subtract the continuum
        cb.selectvis(field=field, spw=myspw)
        aspwmap=-1
        # if we combined on spw in solve, fanout the result with spwmap=-999;
        if 'spw' in combine:
            aspwmap = -999
        cb.setapply(table=amuellertab, spwmap=aspwmap)

        # Generate CORRECTED_DATA without continuum
        cb.correct()

        if want_cont:
            # Generate MODEL_DATA with only the continuum model
            cb.corrupt()

        cb.close()

        # Delete the temporary caltable
        shutil.rmtree(amuellertab)

        if do_resplit:                   # Do final filtering by spw.
            ms.open(csvis)
            # Using ^ in spw is untested here!
            ms.split(final_csvis, spw=myspw, whichcol='all')
            ms.close()
            shutil.rmtree(csvis)
            csvis = final_csvis

        ms.open(csvis, nomodify=False)
        ms.writehistory('taskname = uvcontsub2', origin='uvcontsub2')
        ms.writehistory(message='vis       = ' + str(vis),
                        origin='uvcontsub2')
        ms.writehistory(message='field     = "' + str(field) + '"',
                        origin='uvcontsub2')
        ms.writehistory(message='fitspw    = "' + str(fitspw) + '"',
                        origin='uvcontsub2')
        ms.writehistory(message='solint    = ' + str(solint),
                        origin='uvcontsub2')
        ms.writehistory(message='fitorder  = ' + str(fitorder),
                        origin='uvcontsub2')
        ms.writehistory(message='spw       = "' + str(spw) + '"',
                        origin='uvcontsub2')
        ms.writehistory(message='want_cont = ' + str(want_cont),
                        origin='uvcontsub2')
        ms.close()

        if want_cont:
            ms.open(csvis)
            # No channel averaging (== skipping for fitorder == 0) is done
            # here, but it would be a reasonable option.  The user can always
            # do it by running split again.
            ms.split(csvis[:-3],             # .contsub -> .cont
                     whichcol='MODEL_DATA', averchan=False)
            ms.close()

    except Exception, instance:
        print '*** Error in ', instance
        cb.close()                        # Harmless if cb is closed.
        raise Exception, instance
