"""Module to keep track of calibrations applied to MS."""

# History:
# 10-Sep-2008 jfl Original version.
#  3-Nov-2008 jfl amalgamated stage release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import __builtin__
from numpy import *
import os.path
import types

# alma modules


class MSCalibrater:
    """
    Class that keeps track of the calibration state of the wrapped
    Measurement Set. 

    To do this the class has its own setapply, solve and correct methods
    and keeps an internal record of the calibrations to be applied/that have
    been applied.

    No information is stored in the ms itself. On startup this class assumes
    that the state of the ms is unknown. 
    """

    def __init__(self, tools, htmlLogger, msName, verbose=False):
        """Constructor.

        Keyword arguments:
        tools      -- BaseTools object, giving access to the casa tools.
        htmlLogger -- The HTMLLogger object that is writing the HTML
                      log.
        msName     -- The name of the Measurement Set whose calibration
                      state is being kept.
        verbose    -- True if verbose reports from the object are to
                      be written to the terminal. 
        """
#        print 'MSCalibrater constructor called'

        htmlLogger.timingStageStart('MSCalibration')
        tools.copy(self)
        self._html = htmlLogger
        self._msName = msName
        self._verbose = verbose

# open the ms in the calibrater tool

        self._calibrater.open(self._msName)
        self._imager.open(self._msName)

# get some basic info about the MS.

        rtn = self._ms.open(self._msName)
        msInfo = self._ms.range(['data_desc_id','field_id'])
        self._data_desc_ids = msInfo['data_desc_id']
        self._field_ids = msInfo['field_id']
        self._ms.close()

# initialise internal variables.

        self._calibrationApplied = {}
        for field_id in self._field_ids:
            for data_desc_id in self._data_desc_ids:
                self._calibrationApplied[(field_id,data_desc_id)] = None

        self._calibrationToApply = []


    def _rmall(self, dirPath):  
        """Method to remove the specified file or. if it's a directory
        all its contents and then the directory itself.

        Keyword arguments:
        dirPath  -- the name of the file/directory to be removed.
        """

# do nothing if directory does not exist

        if not os.path.exists(dirPath):
            return 

        namesHere = os.listdir(dirPath)
        for name in namesHere:                  # remove all contents first
            path = os.path.join(dirPath, name)
            if not os.path.isdir(path):         # remove simple files
                os.remove(path)
            else:                               # recur to remove subdirs
                self._rmall(path)
        os.rmdir(dirPath)                       # remove now-empty dirPath


    def correct(self, field, spw, commands):
        """Method to apply the stored corrections to the specified field(s)
        and spw of the MS.

        Keyword arguments:
        field -- The field of list of field ids to which the correction is to 
                 be applied.
        spw   -- The spectral window to which the calibration is to be
                 applied.
        """
        if self._verbose:
            print 'MSCalibrater.correct called'

        if len(self._calibrationToApply) < 1:
            raise RuntimeError, 'no calibrations to apply'

        if type(field) != types.ListType:
            fieldList = [field]
        else:
            fieldList = field

# loop through the fields

        for field in fieldList:
            k = (field, spw)

            if self._verbose:
                print '(field/spw) for calibration', field, spw
                print 'requested calibration', self._calibrationToApply
                print 'current calibration  ', self._calibrationApplied[k]

            if self._calibrationApplied[k] == self._calibrationToApply:
                self._calibrationToApply = []
                if self._verbose:
                    print 'calibration already applied'
            else:
                if self._verbose:
                    print 'calibration will be applied'

                try:
                    self._calibrater.reset(apply=True, solve=True)
                    self._calibrater.selectvis(spw=spw, field=field)
                    commands.append('calibrater.reset(apply=True, solve=True)')
                    commands.append('calibrater.selectvis(spw=%s, field=%s)' % 
                     (spw, field))

                    for cal in self._calibrationToApply:
                        calType = cal[0]
                        calTable = cal[1]
                        if calTable == None:
                            self._calibrater.setapply(type=calType)
                            commands.append('calibrater.setapply(type=%s)' %
                             calType)
                        else:
                            self._calibrater.setapply(type=calType,
                             table=calTable)
                            commands.append(
                             'calibrater.setapply(type=%s, table=%s)' % 
                             (calType, calTable))

                    self._calibrater.correct()
                    commands.append('calibrater.correct()')
                    self._calibrationApplied[k] = self._calibrationToApply
                except:
                    self._calibrationApplied[k] = 'failed'
                    commands.append('...exception')
                    raise
                finally:
                    self._applyField = None
                    self._applySpw = None
                    self._calibrationToApply = []
        if self._verbose:
            for command in commands:
                print command


    def fluxscale(self, tablein, reference, tableout, transfer, append):
        """Wrapper for fluxscale method in calibrater tool.
        """
        if self._verbose:
            print 'MSCalibrater.fluxscale'

        fluxd = self._calibrater.fluxscale(tablein=tablein,
         reference=reference, tableout=tableout, transfer=transfer,
         append=append)
        return fluxd


    def resetCalibrationApplied(self):
        """Called to clear the 'memory' of the calibration applied.
           This might occur if something has been done to the MS behind
           the back of MSCalibrater, e.g. MSFlagger changing the flag state.
        """
        if self._verbose:
            print 'MSCalibrater.resetCalibrationApplied'

        for field_id in self._field_ids:
            for data_desc_id in self._data_desc_ids:
                self._calibrationApplied[(field_id,data_desc_id)] = None


    def setapply(self, type, table=None):
        """Request that a calibration be applied to the MS.

        Keyword arguments:
        type     -- type of calibration.
        table    -- Name of table containing calibration.
        """
        if self._verbose:
            print 'MSCalibrater.setApply called', type, table

        self._calibrationToApply.append((type,table))


    def setjy(self, field, spw, fluxdensity, standard):
        """Use imager to compute the model visibility for a specified
        source flux density and insert in the MODEL_DATA column of the MS.

        Keyword arguments:
        field       -- The field_id to be calibrated.
        spw         -- The spectral window to be calibrated.
        fluxdensity -- Specified flux density (I,Q,U,V) in Jy.
        standard    -- Flux density standard.
        """
        if self._verbose:
            print 'setjy', (field,spw), fluxdensity, standard

        self._imager.setjy(field=field, spw=spw, fluxdensity=fluxdensity,
         standard=standard)
        commands = '''imager.setjy(field=%s, spw=%s, fluxdensity=%s,
         standard=%s)''' % (field, spw, fluxdensity, standard)

        if spw == -1:
            for dd in self._data_desc_ids:
                self._calibrationApplied[(field,dd)] = 'setjy'
        else:
            self._calibrationApplied[(field,spw)] = 'setjy'

        if self._verbose:
            print commands
        return commands


    def solve(self, field, spw, type, t, combine, apmode, table, append,
     solnorm=False, minsnr=0.0, commands=[]):
        """Apply stored calibrations then calculate a calibration solution.
            
        Keyword arguments:
        field             -- The field of list of field ids to which the
                             correction is to be applied.
        spw               -- The spectral window to which the calibration is
                             to be applied.
        type              -- parameter for setsolve.
        t                 -- parameter for setsolve.
        combine           -- parameter for setsolve.
        apmode            -- parameter for setsolve.
        table             -- parameter for setsolve.
        append            -- parameter for setsolve.
        solnorm           -- parameter for setsolve.
        minsnr            -- parameter for setsolve.
        commands          -- list of casapy commands issued.
        """

        if self._verbose:
            print ''
            print 'MSCalibrater.solve called'

# __builtin__ to prevent name clash with parameter

        if __builtin__.type(field) != types.ListType:
            fieldList = [field]
        else:
            fieldList = field

# do we need to initalise the CORRECTED_DATA and MODEL_DATA columns?

        initcal = False
        for f in fieldList:
            k = (f, spw)
            if not(self._calibrationApplied.has_key(k)):
                initcal = True
                break
            elif (self._calibrationApplied[k] != 'initcal') and \
             (self._calibrationApplied[k] != 'setjy'):
                initcal = True
                break

        if initcal:

# yes, initialise the calibration columns

            self._calibrater.initcalset()
            commands.append('calibrater.initcalset()')

            for f in self._field_ids:
               for s in self._data_desc_ids:
                   self._calibrationApplied[(f,s)] = 'initcal'

# OK, columns ready. Arrange to pre-apply calibrations before the solve 

        self._calibrater.reset(apply=True, solve=True)
        self._calibrater.selectvis(spw=spw, field=fieldList)
        commands.append('calibrater.reset(apply=True, solve=True)')
        commands.append('calibrater.selectvis(spw=%s, field=%s)' % (
         spw, fieldList))

        for cal in self._calibrationToApply:
            calType = cal[0]
            calTable = cal[1]
            if calTable == None:
                self._calibrater.setapply(type=calType)
                commands.append('calibrater.setapply(type=%s)' % calType)
            else:
                commands.append('calibrater.setapply(type=%s, table=%s)' % 
                 (calType, calTable))
                try:
                    self._calibrater.setapply(type=calType, table=calTable)
                except:
                    commands.append('...exception')
                    self._calibrationToApply = []
                    raise

# now delete any previous result file and solve

        if self._verbose:
            print 'solving for %s' % type

        self._rmall(table)
        self._calibrater.setsolve(type=type, t=t, combine=combine,
         apmode=apmode, table=table, append=append, solnorm=solnorm,
         minsnr=minsnr)
        commands.append('''calibrater.setsolve(type=%s, t=%s, combine=%s,
         apmode=%s, table=%s, append=%s, solnorm=%s, minsnr=%s)''' % (type,
         t, combine, apmode, table, append, solnorm, minsnr))

        try:
            self._calibrater.solve()
            commands.append('calibrater.solve()')
        except:
            commands.append('calibrater.solve()')
            commands.append('...exception')
        finally:

# reset list of calibrations to apply

            self._calibrationToApply = []

        if self._verbose:
            for command in commands:
                print command


    def solvebandpoly(self, field, spw, table, append, degamp, degphase, 
     visnorm, solnorm, maskcenter, maskedge, commands):
        """Calculate a polynomial bandpass calibration solution.
            
        Keyword arguments:
        field             -- The field of list of field ids to which the
                             correction is to be applied.
        spw               -- The spectral window to which the calibration is
                             to be applied.
        table             -- parameter for setsolvebandpoly.
        append            -- parameter for setsolvebandpoly.
        degamp            -- parameter for setsolvebandpoly.
        degphase          -- parameter for setsolvebandpoly.
        visnorm           -- parameter for setsolvebandpoly.
        solnorm           -- parameter for setsolvebandpoly.
        maskcenter        -- parameter for setsolvebandpoly.
        maskedge          -- parameter for setsolvebandpoly.
        commands          -- list of casapy commands issued.
        """

        if self._verbose:
            print 'MSCalibrater.solveBandPoly called'

# __builtin__ to prevent name clash with parameter

        if __builtin__.type(field) != types.ListType:
            fieldList = [field]
        else:
            fieldList = field

# do we need to initalise the CORRECTED_DATA and MODEL_DATA columns?

        initcal = False
        for f in fieldList:
            k = (f, spw)
            if not(self._calibrationApplied.has_key(k)):
                initcal = True
                break
            elif (self._calibrationApplied[k] != 'initcal') and \
             (self._calibrationApplied[k] != 'setjy'):
                initcal = True
                break

        if initcal:

# yes, initialise the calibration columns

            self._calibrater.initcalset()
            commands.append('calibrater.initcalset()')

            for f in self._field_ids:
               for s in self._data_desc_ids:
                   self._calibrationApplied[(f,s)] = 'initcal'

# OK, columns ready, arrange to pre-apply calibrations before the solve 

        self._calibrater.selectvis(spw=spw, field=fieldList)
        commands.append('calibrater.selectvis(spw=%s, field=%s)' % (
         spw, fieldList))

        for cal in self._calibrationToApply:
            calType = cal[0]
            calTable = cal[1]
            if calTable == None:
                self._calibrater.setapply(type=calType)
                commands.append('calibrater.setapply(type=%s)' % calType)
            else:
                self._calibrater.setapply(type=calType, table=calTable)
                commands.append('calibrater.setapply(type=%s, table=%s)' % (
                 calType, calTable))

# now delete any previous result file and solve

        if self._verbose:
            print ''
            print 'solving'

        self._rmall(table)
        self._calibrater.setsolvebandpoly(table=table, append=append,
         degamp=degamp, degphase=degphase, visnorm=visnorm, solnorm=solnorm,
         maskcenter=maskcenter, maskedge=maskedge)
        commands.append('''calibrater.setsolvebandpoly(table=%s, append=%s,
         degamp=%s, degphase=%s, visnorm=%s, solnorm=%s, maskcenter=%s,
         maskedge=%s)''' % (table, append, degamp, degphase, visnorm,
         solnorm, maskcenter, maskedge))

        try:
            self._calibrater.solve()
            commands.append('calibrater.solve()')
        except:
            commands.append('...exception')
            raise
        finally:

# reset list of calibrations to be applied.

            self._calibrationToApply = []

        if self._verbose:
            for command in commands:
                print command
