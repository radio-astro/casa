"""
The exportdata module provides base classes for preparing data products
on disk for upload to the archive. 

To test these classes, register some data with the pipeline using ImportData,
then execute:

import pipeline
vis = [ '<MS name>' ]

# Create a pipeline context and register some data
context = pipeline.Pipeline().context
inputs = pipeline.tasks.ImportData.Inputs(context, vis=vis)
task = pipeline.tasks.ImportData(inputs)
results = task.execute(dry_run=False)
results.accept(context)

# Run some other pipeline tasks, e.g flagging, calibration,
# and imaging in a similar manner

# Execute the export data task. The details of
# what gets exported depends on what tasks were run
# previously but may include the following
# TBD
inputs = pipeline.tasks.exportdata.Exportdata.Inputs(context,
	  vis, output_dir, sessions, pprfile, products_dir)
task = pipeline.tasks.exportdata.ExportData (inputs)
  results = task.execute (dry_run = True)


"""
from __future__ import absolute_import
import os
import errno
import tarfile
import shutil
import fnmatch
import types
import StringIO
import copy
import string
import re
import collections

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.imagelibrary as imagelibrary

# the logger for this module
LOG = infrastructure.get_logger(__name__)


class ExportDataInputs(basetask.StandardInputs):
    """
    ExportDataInputs manages the inputs for the ExportData task.
	
    .. py:attribute:: context

	the (:class:`~pipeline.infrastructure.launcher.Context`) holding all
	pipeline state

    .. py:attribute:: output_dir
	
	the directory containing the output of the pipeline

    .. py:attribute:: session
	
	a string or list of strings containing the sessions(s) associated
	with each vis. Default to a single session containing all vis.
	Vis without a matching session are assigned to the last session
	in the list. 

    .. py:attribute:: vis

	a string or list of strings containing the MS name(s) on which to
	operate

    .. py:attribute:: pprfile

	the pipeline processing request. Defaults to a file matching the
	'PPR_*.xml' template

    .. py:attribute:: calintents

	the list of calintents defining the calibrator source images to be
	saved.  Defaults to all calibrator intents.

    .. py:attribute:: calimages

	the list of calibrator source images to be saved.  Defaults to all
	calibrator images matching calintents. If defined overrides
	calintents and the calibrator images in the context.

    .. py:attribute:: targetimages

	the list of target source images to be saved.  Defaults to all
	target images. If defined overrides the list of target images in
	the context.
		
    .. py:attribute:: products_dir
	
	the directory where the data productions will be written

     """	

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, session=None, vis=None,
        pprfile=None, calintents=None, calimages=None, targetimages=None,
	products_dir=None ):

	"""
	Initialise the Inputs, initialising any property values to those given
	here.
		
	:param context: the pipeline Context state object
	:type context: :class:`~pipeline.infrastructure.launcher.Context`
	:param output_dir: the working directory for pipeline data
	:type output_dir: string
	:param session: the  sessions for which data are to be exported
	:type session: a string or list of strings
	:param vis: the measurement set(s) for which products are to be exported
	:type vis: a string or list of strings
	:param pprfile: the pipeline processing request
	:type pprfile: a string
	:param calimages: the list of calibrator images to be saved
	:type calimages: a list
	:param targetimages: the list of target images to be saved
	:type targetimages: a list
	:param products_dir: the data products directory for pipeline data
	:type products_dir: string
	"""		

	# set the properties to the values given as input arguments
	self._init_properties(vars())

    # Code for handling sessions should be moved to the launcher.py, 
    # basetask.py, and importdata.py modules. Session information
    # may come from the user or the pipeline processing request.

    @property
    def session(self):
	if self._session is None:
	    self._session = []
	return self._session

    @session.setter
    def session (self, value):
        self._session = value

    @property
    def products_dir(self):
        if self._products_dir is None:
	    if self.context.products_dir is None:
	        self._products_dir = os.path.abspath('./')
	    else:
	        self._products_dir = self.context.products_dir
            try:
                LOG.trace('Creating products directory \'%s\'' % self._products_dir)
                os.makedirs(self.products_dir)
            except OSError as exc:
                if exc.errno == errno.EEXIST:
                    pass
                else: raise

            return self._products_dir
        return self._products_dir

    @products_dir.setter
    def products_dir(self, value):
        self._products_dir = value

    @property
    def pprfile(self):
        if self._pprfile is None:
	    self._pprfile = ''
        return self._pprfile

    @pprfile.setter
    def pprfile(self, value):
        self._pprfile = value

    @property
    def calintents(self):
        if self._calintents is None:
	    self._calintents = ''
        return self._calintents

    @calintents.setter
    def calintents(self, value):
        self._calintents = value

    @property
    def calimages(self):
        if self._calimages is None:
	    self._calimages = []
        return self._calimages

    @calimages.setter
    def calimages(self, value):
        self._calimages = value

    @property
    def targetimages(self):
        if self._targetimages is None:
	    self._targetimages = []
        return self._targetimages

    @targetimages.setter
    def targetimages(self, value):
        self._targetimages = value

class ExportDataResults(basetask.Results):
    def __init__(self, pprequest='', sessiondict=collections.OrderedDict(),
        visdict=collections.OrderedDict(), calimages=(), targetimages=(),
	weblog='', pipescript='', restorescript='', commandslog=''):
	"""
	Initialise the results object with the given list of JobRequests.
	"""
        super(ExportDataResults, self).__init__()
        self.pprequest = pprequest
	self.sessiondict = sessiondict
	self.visdict = visdict
	self.calimages = calimages
	self.targetimages = targetimages
	self.weblog = weblog
	self.pipescript = pipescript
	self.restorescript = restorescript
	self.commandslog = commandslog

    def __repr__(self):
	s = 'ExportData results:\n'
	return s 


class ExportData(basetask.StandardTaskTemplate):
    """
    ExportData is the base class for exporting data to the products
    subdirectory. It performs the following operations:
	
    - Saves the pipeline processing request in an XML file
    - Saves the final flags per ASDM in a compressed / tarred CASA flag
      versions file
    - Saves the final calibration apply list per ASDM in a text file
    - Saves the final set of caltables per session in a compressed /
      tarred file containing CASA tables
    - Saves the final web log in a compressed / tarred file
    - Saves the final processing log in a text file
    - Saves the images in FITS cubes one per target and spectral window
    """

    # link the accompanying inputs to this task 
    Inputs = ExportDataInputs

    # Override the default behavior for multi-vis tasks
    def is_multi_vis_task(self):
        return True

    def prepare(self):
        """
        Prepare and execute an export data job appropriate to the
        task inputs.
        """

	# Create a local alias for inputs, so we're not saying
	# 'self.inputs' everywhere
	inputs = self.inputs

	# Force inputs.vis to be a list.
	vislist = inputs.vis
	if type(vislist) is types.StringType:
	    vislist = [vislist,]

	# Locate and copy the pipeline processing request.
        ppr_files = self._export_pprfile (inputs.context, inputs.output_dir,
	    inputs.products_dir, inputs.pprfile)
	# There should normally be at most one pipeline processing request.
        # In interactive mode there is no PPR.
        if (ppr_files != []):
            ppr_file = os.path.basename(ppr_files[0])
        else:
            ppr_file = None

	# Save list of ppr files in a file.
	#     TBD if necessary

	# Loop over the measurements sets in the working directory and 
	# save the final flags using the flag manager. 
	flag_version_name = 'Pipeline_Final'
	for visfile in vislist:
	    self._save_final_flagversion (visfile, flag_version_name)

	# Copy the final flag versions to the data products directory
	# and tar them up.
	flag_version_list = []
	for visfile in vislist:
	    flag_version_file = self._export_final_flagversion ( \
	        inputs.context, visfile, flag_version_name, \
		inputs.products_dir)
	    flag_version_list.append(flag_version_file)

	 # Save the list of flag versions in a file.
	 #     TBD if necessary

        # Loop over the measurements sets in the working directory, and
	# create the calibration apply file(s) in the products directory. 
	apply_file_list = []
	for visfile in vislist:
	    apply_file =  self._export_final_applylist (inputs.context, \
	        visfile, inputs.products_dir)
	    apply_file_list.append (apply_file)

	# Save the list of calibration apply files in a file.
	#     TBD if necessary

	# Create the ordered vis dictionary
	#    The keys are the base vis names
	#    The values are a tuple containing the flags and applycal files
	visdict = collections.OrderedDict()
	for i in range(len(vislist)):
	    visdict[os.path.basename(vislist[i])] = \
	        (os.path.basename(flag_version_list[i]), \
		os.path.basename(apply_file_list[i])) 

	# Get the session list and the visibility files associated with
	# each session.
	session_list, session_names, session_vislists= self._get_sessions ( \
	    inputs.context, inputs.session, vislist)

	# Export tar files of the calibration tables one per session
	caltable_file_list = []
	for i in range(len(session_names)):
	    caltable_file = self._export_final_calfiles (inputs.context,
	        session_names[i], session_vislists[i], inputs.products_dir)
	    caltable_file_list.append (caltable_file)

	# Save the list of session calibration tar files
	#     TBD if necessary

	# Create the ordered session dictionary
	#    The keys are the session names
	#    The values are a tuple containing the 
	sessiondict = collections.OrderedDict()
	for i in range(len(session_names)):
	    sessiondict[session_names[i]] = \
	        ([os.path.basename(visfile) for visfile in session_vislists[i]], \
		os.path.basename(caltable_file_list[i])) 

	# Export a tar file of the web log
	weblog_file = self._export_weblog (inputs.context, inputs.products_dir)

	# Export the processing log independently of the web log
	casa_commands_file = self._export_casa_commands_log (inputs.context,
	    inputs.context.logs['casa_commands'], inputs.products_dir)

	# Export the processing script independently of the web log
	casa_pipescript = self._export_casa_script (inputs.context,
	    inputs.context.logs['pipeline_script'], inputs.products_dir)
	    #'casa_pipescript.py', inputs.products_dir)

	# Export the restore script
	casa_restore_script = self._export_casa_restore_script (inputs.context,
	    inputs.context.logs['pipeline_restore_script'],
	    inputs.products_dir, vislist, session_list)

	# Export calibrator images to FITS
	LOG.info ('Exporting calibrator source images')
	if inputs.calintents == '':
	    calintents_list = ['PHASE', 'BANDPASS', 'AMPLITUDE']
	else:
	    calintents_list = inputs.calintents.split(',')
	calimages_list, calimages_fitslist = self._export_images ( \
	    inputs.context, True, calintents_list, inputs.calimages, \
	    inputs.products_dir)

	# Export science target images to FITS
	LOG.info ('Exporting target source images')
	targetimages_list, targetimages_fitslist = self._export_images (inputs.context, False,
	    ['TARGET'], inputs.targetimages, inputs.products_dir)

	# Return the results object, which will be used for the weblog
	return ExportDataResults(pprequest=ppr_file, \
	    sessiondict=sessiondict, \
	    visdict=visdict,
	    calimages=(calimages_list, calimages_fitslist),
	    targetimages=(targetimages_list, targetimages_fitslist),
	    weblog=os.path.basename(weblog_file), \
	    pipescript=os.path.basename(casa_pipescript), \
	    restorescript=os.path.basename(casa_restore_script), \
	    commandslog=os.path.basename(casa_commands_file))

    def analyse(self, results):
	"""
	Analyse the results of the export data operation.
		
	This method does not perform any analysis, so the results object is
	returned exactly as-is, with no data massaging or results items
	added.
		
	:rtype: :class:~`ExportDataResults`		
	"""
	return results

    def _export_pprfile (self, context, output_dir, products_dir, pprfile):

	"""
	Export the pipeline processing request to the output products
	directory as is.
	"""
        # Prepare the search template for the pipeline processing
	# request file.
        if pprfile == '':
	    ps = context.project_structure
	    if ps is None:
	        pprtemplate = 'PPR_*.xml'
	    elif ps.ppr_file == '':
	        pprtemplate = 'PPR_*.xml'
	    else:
	        pprtemplate = os.path.basename(ps.ppr_file)
        else:
	    pprtemplate = os.path.basename(pprfile)

	# Locate the pipeline processing request(s) and  generate a list
	# to be copied to the data products directory. Normally there
	# should be only one match but if there are more copy them all.
	pprmatches = []
	for file in os.listdir(output_dir):
	    if fnmatch.fnmatch (file, pprtemplate):
	        LOG.debug('Located pipeline processing request %s' % (file))
	        pprmatches.append (os.path.join(output_dir, file))

	# Copy the pipeline processing request files.
	for file in pprmatches: 
	    LOG.info('Copying pipeline processing file %s to %s' % \
	        (os.path.basename(file), products_dir))
	    if not self._executor._dry_run:
	        shutil.copy (file, products_dir)

	return pprmatches

    def _save_final_flagversion (self, vis, flag_version_name):

	"""
	Save the final flags to a final flag version.
	"""

	LOG.info('Saving final flags for %s in flag version %s' % \
		     (os.path.basename(vis), flag_version_name))
	if not self._executor._dry_run:
            task = casa_tasks.flagmanager (vis=vis,
	        mode='save', versionname=flag_version_name)
	    self._executor.execute (task)

    def _export_final_flagversion (self, context, vis, flag_version_name,
        products_dir):

	"""
	Save the final flags version to a compressed tarfile in products.
	"""

	# Save the current working directory and move to the pipeline
	# working directory. This is required for tarfile IO
	cwd = os.getcwd()
	#os.chdir (os.path.dirname(vis))
	os.chdir (context.output_dir)

	# Define the name of the output tarfile
        visname = os.path.basename(vis)
        tarfilename = visname + '.flagversions.tar.gz'
	LOG.info('Storing final flags for %s in %s' % (visname, tarfilename))

	# Define the directory to be saved
	flagsname = os.path.join (visname + '.flagversions',
	    'flags.' + flag_version_name) 
	LOG.info('Saving flag version %s' % (flag_version_name))

	# Define the versions list file to be saved
	flag_version_list = os.path.join (visname + '.flagversions',
	    'FLAG_VERSION_LIST')
	ti = tarfile.TarInfo(flag_version_list)
	#line = "Pipeline_Final : Final pipeline flags\n"
	line = "%s : Final pipeline flags\n" % flag_version_name
	ti.size = len (line)
	LOG.info('Saving flag version list')

	# Create the tar file
	if not self._executor._dry_run:
	    tar = tarfile.open (os.path.join(products_dir, tarfilename), "w:gz")
	    tar.add (flagsname)
	    tar.addfile (ti, StringIO.StringIO(line))
	    tar.close()

	# Restore the original current working directory
	os.chdir(cwd)

	return tarfilename

    def _export_final_applylist (self, context, vis, products_dir):

        """
	Save the final calibration list to a file. For now this is
	a text file. Eventually it will be the CASA callibrary file.
	"""

	applyfile_name = os.path.basename(vis) + '.calapply.txt'
	LOG.info('Storing calibration apply list for %s in  %s' % \
	    (os.path.basename(vis), applyfile_name))

	if not self._executor._dry_run:

	    # Get the applied calibration state for the visibility file and
	    # convert it into a dictionary of apply instructions.
	    callib = callibrary.CalState()
	    callib[vis] = context.callibrary.applied[vis]

	    # Log the list in human readable form. Better way to do this ?
	    callib_merged = callib.merged()
	    for calto, calfrom in callib_merged.iteritems():
	        LOG.info('Apply to:  Field: %s  Spw: %s  Antenna: %s',
		    calto.field, calto.spw, calto.antenna)
		for item in calfrom:
		    LOG.info ('    Gaintable: %s  Caltype: %s  Gainfield: %s  Spwmap: %s  Interp: %s',
		        os.path.basename(item.gaintable),
			item.caltype,
		        item.gainfield,
		        item.spwmap,
		        item.interp)

	    # Open the file.
	    applyfile = open (os.path.join(products_dir, applyfile_name), "w")

	    # Write
	    applyfile.write ('# Apply file for %s\n' % (os.path.basename(vis)))
	    applyfile.write (callib.as_applycal())

	    # Close file
	    applyfile.close()

	return applyfile_name

    def _get_sessions (self, context, sessions, vis):

        """
	Return a list of sessions where each element of the list contains
	the  vis files associated with that session. In future this routine
	will be driven by the context but for now use the user defined sessions
	"""

	# If the input session list is empty put all the visibility files
	# in the same session.
	if len(sessions) == 0:
	    wksessions = [] 
	    for visname in vis:
	        session = context.observing_run.get_ms(name=visname).session
		wksessions.append(session)
	    #session_names = ['Session_1']
	    #session_vis_list = [vis]
	    #LOG.info('Visibility list for session %s is %s' % \
	        #(session_names[0], session_vis_list[0]))
	    #return session_names, session_vis_list
	else:
	    wksessions = sessions

	# Determine the number of unique sessions.
	session_seqno = 0; session_dict = {}
	for i in range(len(wksessions)): 
	    if wksessions[i] not in session_dict:
	        session_dict[wksessions[i]] = session_seqno
		session_seqno = session_seqno + 1

	# Initialize the output session names and visibility file lists
	session_names = []
	session_vis_list = []
	for key, value in sorted(session_dict.iteritems(), \
	    key=lambda(k,v): (v,k)):
	    session_names.append(key)
	    session_vis_list.append([])

	# Assign the visibility files to the correct session
	for j in range(len(vis)): 
	    # Match the session names if possible 
	    if j < len(wksessions):
		for i in range(len(session_names)):
		    if wksessions[j] == session_names[i]:
		         session_vis_list[i].append(vis[j])
	    # Assign to the last session
	    else:
		session_vis_list[len(session_names)-1].append(vis[j])

	# Log the sessions
	for i in range(len(session_vis_list)):
	    LOG.info('Visibility list for session %s is %s' % \
	    (session_names[i], session_vis_list[i]))
	        
	return wksessions, session_names, session_vis_list

    def _export_final_calfiles (self, context, session, vislist, products_dir):

	"""
	Save the final calibration tables in a tarfile one file
	per session.
	"""

	# Save the current working directory and move to the pipeline
	# working directory. This is required for tarfile IO
	cwd = os.getcwd()
	os.chdir (context.output_dir)

	ps = context.project_structure
	if ps is None:
	    ousid = ''
	elif ps.ousstatus_entity_id == 'unknown':
	    ousid = ''
	else:
	    ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__')) + '.'

	# Define the name of the output tarfile
        tarfilename = ousid + session + '.caltables.tar.gz'
	LOG.info('Saving final caltables for %s in %s' % (session, tarfilename))

	# Create the tar file
	if not self._executor._dry_run:
	    tar = tarfile.open (os.path.join(products_dir, tarfilename), "w:gz")
	    caltable_master_list = []
	    for visfile in vislist:

	        LOG.info('Collecting final caltables for %s in %s' % \
		    (os.path.basename(visfile), tarfilename))

		# Create the list of applied caltables for that vis
	        caltable_list = []
	        callib = callibrary.CalState()
	        callib[visfile] = context.callibrary.applied[visfile]
	        callib_merged = callib.merged()
		for calto, calfrom in callib_merged.iteritems():
		    for item in calfrom:
		        caltable_list.append(os.path.basename(item.gaintable))

		# Merge the per vis list into the session list
		caltable_master_list = list(set(caltable_master_list + \
		    caltable_list))

	    # Tar the session list.
	    for table in caltable_master_list:
		tar.add(table)
	    tar.close()

	# Restore the original current working directory
	os.chdir(cwd)

	return tarfilename

    def _export_weblog (self, context, products_dir):

	"""
	Save the processing web log to a tarfile
	"""

	# Save the current working directory and move to the pipeline
	# working directory. This is required for tarfile IO
	cwd = os.getcwd()
	os.chdir (context.output_dir)

	# Define the name of the output tarfile
	ps = context.project_structure
	if ps is None:
            tarfilename = 'weblog.tar.gz'
	elif ps.ousstatus_entity_id == 'unknown':
            tarfilename = 'weblog.tar.gz'
	else:
	    ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__')) 
            tarfilename = ousid + '.weblog.tar.gz'

	LOG.info('Saving final weblog in %s' % (tarfilename))

	# Create the tar file
	if not self._executor._dry_run:
	    tar = tarfile.open (os.path.join(products_dir, tarfilename), "w:gz")
	    tar.add (os.path.join(os.path.basename(os.path.dirname(context.report_dir)), 'html'))
	    tar.close()

	# Restore the original current working directory
	os.chdir(cwd)

	return tarfilename

    def _export_casa_commands_log (self, context, casalog_name, products_dir):

	"""
	Save the CASA commands file.
	"""

	ps = context.project_structure
	if ps is None:
	    casalog_file = os.path.join (context.report_dir, casalog_name)
	    out_casalog_file = os.path.join (products_dir, casalog_name) 
	elif ps.ousstatus_entity_id == 'unknown':
	    casalog_file = os.path.join (context.report_dir, casalog_name)
	    out_casalog_file = os.path.join (products_dir, casalog_name) 
	else:
	    ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__'))
	    casalog_file = os.path.join (context.report_dir, casalog_name)
	    out_casalog_file = os.path.join (products_dir, ousid + '.' + casalog_name)

	LOG.info('Copying casa commands log %s to %s' % \
	        (casalog_file, out_casalog_file))
	if not self._executor._dry_run:
	    shutil.copy (casalog_file, out_casalog_file)

	return os.path.basename(out_casalog_file)

    def _export_casa_restore_script (self, context, script_name, products_dir, vislist, session_list):

	"""
	Save the CASA restore scropt.
	"""

	# Get the output file name
	ps = context.project_structure
	if ps is None:
	    script_file = os.path.join (context.report_dir, script_name)
	    out_script_file = os.path.join (products_dir, script_name) 
	elif ps.ousstatus_entity_id == 'unknown':
	    script_file = os.path.join (context.report_dir, script_name)
	    out_script_file = os.path.join (products_dir, script_name) 
	else:
	    ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__'))
	    script_file = os.path.join (context.report_dir, script_name)
	    out_script_file = os.path.join (products_dir, ousid + '.' + script_name)

	LOG.info('Creating casa restore script %s' %  (script_file))

	# This is hardcoded.
	tmpvislist=[]
	for vis in vislist:
	    filename = os.path.basename(vis)
	    if filename.endswith('.ms'):
	        filename, filext = os.path.splitext(filename)
	    tmpvislist.append(filename)
	task_string = '    hif_restoredata (vis=%s, session=%s)' % (tmpvislist, session_list) 

        template = '''__rethrow_casa_exceptions = True
h_init()
try:
%s
finally:
    h_save()
''' % task_string

	with open (script_file, 'w') as casa_restore_file:
	    casa_restore_file.write(template)

	LOG.info('Copying casa restore script %s to %s' % \
	        (script_file, out_script_file))
	if not self._executor._dry_run:
	    shutil.copy (script_file, out_script_file)

	return os.path.basename (out_script_file)

    def _export_casa_script (self, context, casascript_name, products_dir):

	"""
	Save the CASA script.
	"""

	ps = context.project_structure
	if ps is None:
	    casascript_file = os.path.join (context.report_dir, casascript_name)
	    out_casascript_file = os.path.join (products_dir, casascript_name) 
	elif ps.ousstatus_entity_id == 'unknown':
	    casascript_file = os.path.join (context.report_dir, casascript_name)
	    out_casascript_file = os.path.join (products_dir, casascript_name) 
	else:
	    ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__'))
	    casascript_file = os.path.join (context.report_dir, casascript_name)
	    out_casascript_file = os.path.join (products_dir, ousid + '.' + casascript_name)

	LOG.info('Copying casa script file %s to %s' % \
	        (casascript_file, out_casascript_file))
	if not self._executor._dry_run:
	    shutil.copy (casascript_file, out_casascript_file)

	return os.path.basename(out_casascript_file)


    def _export_images (self, context, calimages, intents, images,
        products_dir):

	"""
	Export the images to FITS files.
	"""

        # Create the image list
	images_list = []
	if len(images) == 0:
	    # Get the image library
	    if calimages:
	        cleanlist = context.calimlist.get_imlist()
	    else:
	        cleanlist = context.sciimlist.get_imlist()
	    for image in cleanlist:
		# Image name probably includes path
		if image['sourcetype'] in intents:
	            images_list.append(image['imagename'])
	else:
	    # Assume only the root image name was given.
	    cleanlib = imagelibrary.ImageLibrary()
	    for image in images:
		if caliimages:
		    imageitem = imagelibrary.ImageItem(imagename=image,
		        sourcename='UNKNOWN',
		        spwlist='UNKNOWN',
		        sourcetype='CALIBRATOR')
		else:
		    imageitem = imagelibrary.ImageItem(imagename=image,
		        sourcename='UNKNOWN',
		        spwlist='UNKNOWN',
		        sourcetype='TARGET')
		cleanlib.add_item(imageitem)
	        if os.path.basename(image) == '':
	            images_list.append(os.path.join(context.output_dir, image))
		else:
	            images_list.append(image)
	    cleanlist = cleanlib.get_imlist()

	# Convert to FITS.
        fits_list = []
	for image in images_list:
	    # Need to remove stage / iter information
	    #fitsname = re.sub('\.s\d+.*\.iter.*\.', '.', image)
	    fitsname = re.sub('\.s\d+[_]\d+\.', '.', image)
	    fitsname = re.sub('\.iter\d+\.image', '', fitsname)
	    fitsfile = os.path.join (products_dir,
	        os.path.basename(fitsname) + '.fits')
	    LOG.info('Saving final image %s to FITS file %s' % \
		(os.path.basename(image), os.path.basename(fitsfile)))
	    if not self._executor._dry_run:
                task = casa_tasks.exportfits (imagename=image,
		    fitsimage=fitsfile,  velocity=False, optical=False,
		    bitpix=-32, minpix=0, maxpix=-1, overwrite=True,
		    dropstokes=False, stokeslast=True)
	        self._executor.execute (task)
	        fits_list.append(fitsfile)


	new_cleanlist = copy.deepcopy(cleanlist)

	return new_cleanlist, fits_list

