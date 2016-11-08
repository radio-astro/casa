try:
    from casac import casac
except ImportError, e:
    print "failed to load casa:\n", e
    sys.exit(1)

imager = casac.imager
imtool=imager
calibrater = casac.calibrater
cbtool=calibrater
mstool = casac.ms
tptool = casac.tableplot
tp = tptool()
mptool = casac.msplot
mp = mptool()
pmtool = casac.plotms
pm = pmtool()
cptool = casac.calplot
cp = cptool()
qatool = casac.quanta
qa = casac.qa =  qatool()
tbtool = casac.table
#fgtool = casac.flagger
aftool = casac.agentflagger
af = aftool()
metool = casac.measures
iatool = casac.image
potool = casac.imagepol
lmtool= casac.linearmosaic
smtool = casac.simulator
cltool = casac.componentlist
coordsystool = casac.coordsys
cstool = casac.coordsys
rgtool = casac.regionmanager
sltool = casac.spectralline
dctool = casac.deconvolver
vptool = casac.vpmanager
msmdtool = casac.msmetadata
fitool = casac.fitter
fntool = casac.functional
imdtool = casac.imagemetadata

utilstool = casac.utils
cu = casac.cu = utilstool()
vftask = casac.vlafillertask()
vlafiller=vftask.fill
at = casac.atmosphere()
ca = casac.calanalysis()
mttool = casac.mstransformer
mt = mttool()
sdmstool = casac.singledishms
sdms = sdmstool()
parallelimager = casac.parallelimager()

def gentools(tools=None):
	"""
	Generate a fresh set of tools; only the ones who have
	states..so globally sharing the same one can be unpredicatable 
	im,cb,ms,tb,me,ia,po,sm,cl,cs,rg,sl,dc,vp,msmd,fi,fn,imd,sdms,lm=gentools() 
	or if you want specific set of tools
	im, ia, cb=gentools(['im', 'ia', 'cb'])

	"""
	tooldic={'im':'imager()', 'cb' :'calibrater()', 'ms':'mstool()',
		 'tb':'tbtool()',  'me' :'metool()', 
		 'ia': 'iatool()', 'po':'potool()', 'sm' :'smtool()', 
		 'cl': 'cltool()', 'cs' :'cstool()', 'rg':'rgtool()',
		 'sl':'sltool()', 'dc':'dctool()', 'vp':'vptool()',
		 'msmd':'msmdtool()','fi':'fitool()','fn':'fntool()', 
		 'imd':'imdtool()','sdms':'sdmstool()', 'lm':'lmtool()'}
	reqtools=[]
        if (not tools) or not hasattr(tools, '__iter__'):
		reqtools=['im', 'cb', 'ms','tb', 'me', 'ia', 'po',
                          'sm', 'cl', 'cs', 'rg','sl', 'dc', 'vp', 
			  'msmd', 'fi', 'fn', 'imd', 'sdms', 'lm']
	else:
		reqtools=tools
	return tuple([eval(tooldic[reqtool]) for reqtool in reqtools])

im,cb,ms,tb,me,ia,po,sm,cl,cs,rg,sl,dc,vp,msmd,fi,fn,imd,sdms,lm=gentools()

def write_history(myms, vis, tname, param_names, param_vals, myclog=None, debug=False):
        """
        Update vis with the parameters that task tname was called with.

        myms - an ms tool instance
        vis  - the MS to write to.
        tname - name of the calling task.
        param_names - list of parameter names.
        param_vals - list of parameter values (in the same order as param_names).
        myclog - a casalog instance (optional)
        debug - Turns on debugging print statements on errors if True.

        Example:
        The end of split does
        param_names = split.func_code.co_varnames[:split.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]  # Must be done in the task.
        write_history(myms, outputvis, 'split', param_names, param_vals,
                      casalog),
        which appends, e.g.,
        
        vis = 'TWHydra_CO3_2.ms'
        outputvis   = 'scan9.ms'
        datacolumn  = 'data'
        field       = ''
        spw         = ''
        width       = 1
        antenna     = ''
        timebin     = '0s'
        timerange   = ''
        scan        = '9'
        intent      = ''
        array       = ''
        uvrange     = ''
        correlation = ''
        keepflags   = True
        async       = False

        to the HISTORY of outputvis.
        """
        if not hasattr(myms, 'writehistory'):
                if debug:
                        print "write_history(myms, %s, %s): myms is not an ms tool" % (vis, tname)
                return False
        retval = True
        isopen = False
        try:
                if not myclog and hasattr(casalog, 'post'):
                        myclog = casalog
        except Exception, instance:
                # There's no logger to complain to, and I don't want to exit
                # just because of that.
                pass
        try:
                myms.open(vis, nomodify=False)
                isopen = True
                myms.writehistory(message='taskname=%s' % tname, origin=tname)
                vestr = 'version: '
                try:
                        # Don't use myclog.version(); it also prints to the
                        # logger, which is confusing.
                        vestr += casa['build']['version'] + ' '
                        vestr += casa['source']['url'].split('/')[-2]
                        vestr += ' rev. ' + casa['source']['revision']
                        vestr += ' ' + casa['build']['time']
                except Exception, instance:
                        if hasattr(myclog, 'version'):
                                # Now give it a try.
                                vestr += myclog.version()
                        else:
                                vestr += ' could not be determined' # We tried.
                myms.writehistory(message=vestr, origin=tname)

                # Write the arguments.
                for argnum in xrange(len(param_names)):
                        msg = "%-11s = " % param_names[argnum]
                        val = param_vals[argnum]
                        if type(val) == str:
                                msg += '"'
                        msg += str(val)
                        if type(val) == str:
                                msg += '"'
                        myms.writehistory(message=msg, origin=tname)
        except Exception, instance:
                if hasattr(myclog, 'post'):
                        myclog.post("*** Error \"%s\" updating HISTORY of %s" % (instance, vis),
                                    'SEVERE')
                retval = False
        finally:
                if isopen:
                        myms.close()
        return retval        
