"""
The exportdata for SD module provides base classes for preparing data products
on disk for upload to the archive. 

To test these classes, register some data with the pipeline using ImportData,
then execute:

import pipeline
# Create a pipeline context and register some data
context = pipeline.Pipeline().context
output_dir = "."
products_dir = "./products"
inputs = pipeline.tasks.singledish.SDExportData.Inputs(context,output_dir,products_dir)
task = pipeline.tasks.singledish.SDExportData (inputs)
results = task.execute (dry_run = True)
results = task.execute (dry_run = False)

"""
from __future__ import absolute_import
import os
import re
import copy
import errno
import StringIO
import tarfile
import fnmatch
import shutil
import string
import collections
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.sdfilenamer as filenamer
import pipeline.hif.tasks.exportdata.exportdata as hif_exportdata
import pipeline.hif.tasks.exportdata.manifest as manifest

# the logger for this module
LOG = infrastructure.get_logger(__name__)


class SDExportDataInputs(basetask.StandardInputs):
    """
    ExportDataInputs manages the inputs for the ExportData task.
    
    .. py:attribute:: context
    
    the (:class:`~pipeline.infrastructure.launcher.Context`) holding all
    pipeline state
    
    .. py:attribute:: output_dir
    
    the directory containing the output of the pipeline
    
    .. py:attribute:: products_dir
    
    the directory where the data productions will be written
    
    .. py:attribute:: targetimages
    
    the list of target source images to be saved.  Defaults to all
    target images. If defined overrides the list of target images in
    the context.
    
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None,pprfile=None,
                 targetimages=None,products_dir=None):
        """
        Initialise the Inputs, initialising any property values to those given
        here.
            
        :param context: the pipeline Context state object
        :type context: :class:`~pipeline.infrastructure.launcher.Context`
        :param output_dir: the working directory for pipeline data
        :type output_dir: string
        :param pprfile: the pipeline processing request
        :type pprfile: a string
        :param targetimages: the list of target images to be saved
        :type targetimages: a list
        :param products_dir: the data products directory for pipeline data
        :type products_dir: string
        """        
        # set the properties to the values given as input arguments
        self._init_properties(vars())
     
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
                    else: 
                        raise
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
    def targetimages(self):
        if self._targetimages is None:
            self._targetimages = []
        return self._targetimages
     
    @targetimages.setter
    def targetimages(self, value):
        self._targetimages = value
        
class SDExportDataResults(hif_exportdata.ExportDataResults):
    def __repr__(self):
        s = 'SDExportData results:\n'
        return s 

class SDExportData(hif_exportdata.ExportData):
    """
    SDExportData is the base class for exporting data to the products
    subdirectory. It performs the following operations:
    
    - Saves the pipeline processing request in an XML file
    - Saves the images in FITS cubes one per target and spectral window
    - Saves the final flags and bl coefficient per ASDM in a compressed / tarred CASA flag
      versions file
    - Saves the final web log in a compressed / tarred file
    - Saves the text formatted list of contents of products directory
    """

    
    # link the accompanying inputs to this task 
    Inputs = SDExportDataInputs
    
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
        
        # Get the parent ous ousstatus name. This is the sanitized ous
        # status uid
        ps = inputs.context.project_structure
        if ps is None:
            oussid = 'unknown'
        elif ps.ousstatus_entity_id == 'unknown':
            oussid = 'unknown'
        else:
            oussid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__'))
    
        # Initialize the manifest document and the top level ous status.
        pipemanifest = self._init_pipemanifest(oussid)
        ouss = pipemanifest.set_ous(oussid)

        self.init_visdict()
        
        # Get the session list and the visibility files associated with
        # each session.
        input_vislist = map(lambda x: x.basename, inputs.context.observing_run.measurement_sets)
        input_vislist2 = map(lambda x: x.name, inputs.context.observing_run.measurement_sets)
        session_list, session_names, session_vislists \
            = self._get_sessions (inputs.context, [], input_vislist2)
        LOG.info('session_list, session_names, session_vislists=%s\n\t%s\n\t%s'%(session_list,session_names,session_vislists))
            
        # Loop over the measurements sets in the working directory and 
        # save the final flags using the flag manager. 
        vislist = self._generate_vislist()
        flag_version_name = 'Pipeline_Final'
        visdict2 = collections.defaultdict(list)
        for visfile in vislist:
            self._save_final_flagversion (visfile, flag_version_name)
            basevis = os.path.basename(visfile)
            if self.visdict.has_key(basevis):
                visdict2[self.visdict[basevis]].append(basevis)
            else:
                visdict2[basevis].append(basevis)
    
        # Copy the final flag versions to the data products directory
        # and tar them up.
        flag_version_list = []
        flagversionsdict= {}
        for basevis, myvislist in visdict2.items():
            flag_version_file = self._export_final_flagversion(inputs.context, basevis, myvislist, flag_version_name, inputs.products_dir)
            flag_version_list.append(flag_version_file)
            flagversionsdict[basevis] = os.path.basename(flag_version_file)
        LOG.info('flagversiondict=%s'%(flagversionsdict))
        
        # create the calibration apply file(s) in the products directory. 
        apply_file_list = []
        applydict = {}
        for visfile in vislist:
            apply_file =  self._export_final_applylist (inputs.context, \
                                                        visfile, inputs.products_dir)
            if len(apply_file) > 0:
                apply_file_list.append (apply_file)
                applydict[os.path.basename(visfile)] = apply_file
        LOG.info('applydict=%s'%(applydict))

        # Export a tar file of skycal , tsyscal and bl-subtracted which is created by asap
        caltabledict = {}
        for session_name, session_vislist in zip(session_names, session_vislists):
            caltabledict[session_name] = self._export_final_calfiles(inputs.context, 
                                                                     oussid, 
                                                                     session_name, 
                                                                     session_vislist, 
                                                                     inputs.products_dir)
        LOG.info('caltabledict=%s'%(caltabledict))
        caltable_file_list = caltabledict.values()
        
        # Create the ordered session dictionary
        #    The keys are the session names
        #    The values are a tuple containing the vislist and the caltables 
        for session_name, session_vislist in zip(session_names, session_vislists):
            session = pipemanifest.set_session(ouss, session_name)
            pipemanifest.add_caltables(session, caltabledict[session_name])
            for vis_name in session_vislist:
                basename = os.path.basename(vis_name)
                pipemanifest.add_asdm (session, vis_name, flagversionsdict[basename],
                                       applydict[basename])                
            
        # Create fits files from CASA images
        imagelist, targetimages_fitslist = self._export_images (inputs.context, inputs.products_dir, inputs.targetimages)

        # Export a tar file of the web log
        weblog_file = self._export_weblog (inputs.context, inputs.products_dir)
        
        # Locate and copy the pipeline processing request.
        ppr_file = self._export_pprfile (inputs.context,inputs.products_dir, inputs.pprfile)
        
        # Export the processing log independently of the web log
        casa_commands_file = self._export_casa_commands_log(inputs.context,
                inputs.context.logs['casa_commands'], inputs.products_dir)

        # Export the processing script independently of the web log
        casa_pipescript = self._export_casa_script (inputs.context,
                                                    inputs.context.logs['pipeline_script'], inputs.products_dir)

        # Export the restore script
        #casa_restore_script = self._export_casa_restore_script (inputs.context,
        #                                                        inputs.context.logs['pipeline_restore_script'],
        #                                                        inputs.products_dir, vislist, session_list)

        sessiondict = None
        visdict = None
        casa_restore_script = None
        
        # Export a XML format list of files within a products directory
        self._fill_manifest(pipemanifest, ouss, ppr_file, sessiondict, visdict, weblog_file, casa_commands_file, 
                            casa_pipescript, casa_restore_script, targetimages_fitslist)

#         # Export a text format list of files whithin a products directory
#         self._export_list_txt(inputs.products_dir, fitsfiles=targetimages_fitslist, weblog=weblog_file, pprfile=ppr_file,
#                               flagversions=flag_version_list, calapply=apply_file_list, caltables=caltable_file_list,
#                               casa_commands=casa_commands_file)
        #LOG.info('contents of product direoctory is %s' % os.listdir(inputs.products_dir))

        # Create the ordered session dictionary
        #    The keys are the session names
        #    The values are a tuple containing the vislist and the caltables 
        sessiondict = collections.OrderedDict()
        for i in xrange(len(session_names)):
            sessiondict[session_names[i]] = \
                ([os.path.basename(visfile) for visfile in session_vislists[i]], \
                 os.path.basename(caltabledict[session_names[i]])) 
                
        visdict = collections.OrderedDict()
        for (vis, flagversion) in flagversionsdict.items():
            visdict[vis] = (flagversion, applydict[vis])
                
        # Export the pipeline manifest file
        casa_pipe_manifest = self._export_pipe_manifest(inputs.context, oussid,
                                                        'pipeline_manifest.xml', inputs.products_dir, pipemanifest)
         
        return SDExportDataResults(pprequest=ppr_file, weblog=weblog_file, 
                                   sessiondict=sessiondict, visdict=visdict,
                                   targetimages=(imagelist, targetimages_fitslist),
                                   restorescript='',
                                   pipescript=casa_pipescript, commandslog=casa_commands_file)
     
    def init_visdict(self):
        self.visdict = {}
        for ms in self.inputs.context.observing_run.measurement_sets:
            self.visdict[ms.basename] = ms.basename
        for scantable in self.inputs.context.observing_run:
            original_vis = scantable.ms.basename
            if hasattr(scantable, 'exported_ms') and scantable.exported_ms is not None:
                self.visdict[os.path.basename(scantable.exported_ms)] = original_vis
     
    def _fill_manifest(self, pipemanifest, ouss, ppr_file, sessiondict, visdict, weblog_file, 
                       casa_commands_file, casa_pipescript, casa_restorescript, target_images):
        # PPR
        pipemanifest.add_pprfile (ouss, os.path.basename(ppr_file))
        
        # session dependent products
        
        # weblog
        pipemanifest.add_weblog (ouss, os.path.basename(weblog_file))
        
        # casa_commands_log
        pipemanifest.add_casa_cmdlog (ouss,
                                      os.path.basename(casa_commands_file))
        
        # casa_pipescript
        pipemanifest.add_pipescript (ouss, os.path.basename(casa_pipescript))
        
        # restore script (N/A)
        #pipemanifest.add_restorescript (ouss,
        #                                os.path.basename(casa_restore_script))
        
        # target images
        pipemanifest.add_images (ouss,
                                 map(os.path.basename, target_images),#[os.path.basename(image) for image in targetimages_fitslist],
                                 'target')

    def _export_pprfile (self, context, products_dir, pprfile):
        """
        Export the pipeline processing request to the output products directory as is.
        """
        os.chdir(context.output_dir)
        # Prepare the search template for the pipeline processing request file.
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
        for _f in os.listdir(context.output_dir):
            if fnmatch.fnmatch (_f, pprtemplate):
                LOG.debug('Located pipeline processing request(PPR) xmlfile %s' % (_f))
                pprmatches.append (os.path.join(context.output_dir, _f))
        
        # Copy the pipeline processing request files.
        for _f in pprmatches: 
            LOG.info('Copying pipeline processing request(PPR) xmlfile %s to %s' % \
                (os.path.basename(_f), products_dir))
            if not self._executor._dry_run:
                shutil.copy (_f, products_dir)
        return os.path.basename(pprmatches[0] if len(pprmatches) > 0 else '')

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
        
        #return os.path.basename(out_casalog_file)
        return os.path.basename(out_casalog_file)
    
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

    def _export_final_applylist (self, context, vis, products_dir):
        """
	    Save the final calibration list to a file. For now this is
    	a text file. Eventually it will be the CASA callibrary file.
    	"""
        # return '' if vis is not in callibrary applied list
        if not context.callibrary.applied.has_key(vis):
            return ''

        applyfile_name = os.path.basename(vis) + '.calapply.txt'
        LOG.info('Storing calibration apply list for %s in  %s' % \
                 (os.path.basename(vis), applyfile_name))

        if not self._executor._dry_run:
            # Get the applied calibration state for the visibility file and
            # convert it into a dictionary of apply instructions.
            callib = callibrary.SDCalState()
            callib[vis] = context.callibrary.applied[vis]

            # Log the list in human readable form. Better way to do this ?
            callib_merged = callib.merged()
            for calto, calfrom in callib_merged.iteritems():
                LOG.info('Apply to:  Field: %s  Spw: %s  Antenna: %s',
                         calto.field, calto.spw, calto.antenna)
                for item in calfrom:
                    LOG.info ('    Gaintable: %s  Caltype: %s  Spwmap: %s  Interp: %s',
                              os.path.basename(item.gaintable),
                              item.caltype,
                              item.spwmap,
                              item.interp)

            # Open the file.
            with open(os.path.join(products_dir, applyfile_name), 'w') as applyfile:

                # Write
                applyfile.write ('# Apply file for %s\n' % (os.path.basename(vis)))
                applyfile.write (callib.as_applycal())

        return applyfile_name

    def _export_images (self, context, products_dir, images):
        #cwd = os.getcwd()
        os.chdir(context.output_dir)
        producted_filename = '*.im'
        
        if len(images) == 0:
            xx_dot_im = [fname for fname in os.listdir(context.output_dir)
                if fnmatch.fnmatch(fname, producted_filename)]
        else:
            xx_dot_im = images
        
        fits_list = []
        if len(xx_dot_im) != 0:
            # split by dot
            splitwith_dot = [name.split('.') for name in xx_dot_im]
            
            # pop by 'line'
            tmplate_line = "line*"
            num_selected_im = [ i for i in range(len(splitwith_dot))
                for words in splitwith_dot[i]
                    if fnmatch.fnmatch(words,tmplate_line)]
            
            xx_dot_im_next = [xx_dot_im[i] for i in range(len(xx_dot_im))
                if not (i in num_selected_im)]
            
            # Export combined images only
            #antenna_names = set([a.name for ms in context.observing_run.measurement_sets for a in ms.antennas])
            antenna_names = set([st.antenna.name for st in context.observing_run])
            #LOG.info('antenna_names=%s'%(antenna_names))
            
            pattern_nomatch = '.*\.(DV|DA|PM|CM)[0-9][0-9]\.spw.*\.sd\.im$'
            pattern_match = '.*\.spw.*\.sd\.im$'
            #LOG.info('pattren_string=%s'%(pattern_nomatch))
            
            combined_images = [image for image in xx_dot_im_next 
                               if (re.match(pattern_nomatch, image) is None) and re.match(pattern_match, image)]
            #LOG.info('combined_images=%s'%(combined_images))
            
            # If no combined images available, export all the images detected 
            if len(combined_images) > 0:
                resulting_images = combined_images
            else:
                resulting_images = xx_dot_im_next
            
            splitted_path = []
            for fname in resulting_images:
                root, ext = os.path.splitext(fname)
                splitted_path.append(root)
             
            images_list = splitted_path
             
            # showing target CASA image
            if self._executor._dry_run:
                for imfile in xx_dot_im_next:
                    LOG.info('FITS: Target CASA image is %s' % imfile)
            else:
                pass
             
            # Convert to FITS 
            fits_list = []
            for image in images_list:
                fitsfile = os.path.join (products_dir, image + '.fits')
                if not self._executor._dry_run:
                    task = casa_tasks.exportfits (imagename=image + '.im',
                    fitsimage = fitsfile,  velocity=False, optical=False,
                    bitpix=-32, minpix=0, maxpix=-1, overwrite=True,
                    dropstokes=False, stokeslast=True)
                    self._executor.execute (task)
                elif self._executor._dry_run:
                    LOG.info('FITS: Saving final FITS file is %s' % fitsfile)
                fits_list.append(fitsfile)
        else:
            LOG.info('FITS: There are no CASA image files here')
            
        # generate image description from FITS file name
        # FITS name should be <source name>.<spw id>.<pol>.sd.fits 
        imlibrary = imagelibrary.ImageLibrary()
        for filename in fits_list:
            image = os.path.basename(filename)
            match = re.search('\.spw[0-9]+\.', image)
            if match is None:
                # create dummy description
                sourcename = 'UNKNOWN'
                spwlist = 'UNKNOWN'
            else:
                sourcename = image[:match.start()]
                spwlist = image[match.start()+4:match.end()-1]
            imageitem = imagelibrary.ImageItem(imagename=image,
                                               sourcename=sourcename,
                                               spwlist=spwlist,
                                               sourcetype='TARGET')
            imlibrary.add_item(imageitem)
        imagelist = copy.deepcopy(imlibrary.get_imlist())
            
        return imagelist, fits_list
     
    def _export_final_calfiles(self, context, oussid, session, vislist, products_dir):
        """
        Save the final calibration tables in a tarfile one file
        per session.
        """

        # Save the current working directory and move to the pipeline
        # working directory. This is required for tarfile IO
        cwd = os.getcwd()
        os.chdir (context.output_dir)
    
        ousid = oussid + '.'
    
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
                        LOG.info('Adding %s'%(item.gaintable))
                        caltable_list.append(os.path.basename(item.gaintable))
    
                # Merge the per vis list into the session list
                caltable_master_list = list(set(caltable_master_list + \
                                                caltable_list))
                
                # baseline table
                for st in context.observing_run:
                    if st.ms.basename == os.path.basename(visfile):
                        spw_list = [spwid for (spwid,spw) in st.spectral_window.items()
                                    if spw.num_channels > 1 and spw.type != 'WVR' and spw.is_target]
                        namer = filenamer.CalibrationTable()
                        prefix = os.path.basename(visfile).replace('.ms','')
                        antenna = st.antenna.name
                        namer.asdm(prefix)
                        namer.antenna_name(antenna)
                        namer.bl_cal()
                        for spw in spw_list:
                            namer.spectral_window(spw)
                            name = namer.get_filename()
                            LOG.debug('baseline table name: %s'%(name))
                            if os.path.exists(name):
                                caltable_master_list.append(name)
            LOG.debug('caltable_master_list=%s'%(caltable_master_list))
    
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
        
        product_tar_list = []
        if os.path.exists(context.report_dir):
            # Define the name of the output tarfile
            ps = context.project_structure
            if ps is None or ps.ousstatus_entity_id == 'unknown':
                product_tarfilename = 'weblog.tar.gz'
            else:
                ousid = ps.ousstatus_entity_id.translate(string.maketrans(':/', '__')) 
                product_tarfilename = ousid + '.weblog.tar.gz'
            # Create tar file
            product_tar_list = []
            if not self._executor._dry_run:
                LOG.info('WEBLOG: Copying final weblog of SD in %s' % os.path.join(products_dir,product_tarfilename))
                tar = tarfile.open (os.path.join(products_dir, product_tarfilename), "w:gz")
                tar.add (os.path.join(os.path.basename(os.path.dirname(context.report_dir)), 'html'))
                tar.close()
                product_tar_list = [product_tarfilename]
            elif self._executor._dry_run:
                LOG.info('WEBLOG: Target path is  %s' % context.report_dir)
                if os.path.isdir(context.report_dir):
                    LOG.info('WEBLOG: Located html directory is %s' % os.path.relpath(context.report_dir,context.output_dir))
                else:
                    LOG.info('WEBLOG: no html directory is found')
                LOG.info('WEBLOG: Saving final weblog of SD in %s' % os.path.join (products_dir,product_tarfilename))
        else:
            LOG.info('WEBLOG: There is no html directory in report_dir')
        # Restore the original current working directory
        os.chdir(cwd)
        return product_tar_list[0]
                                          
#     def _export_list_txt(self, products_dir, fitsfiles=None, caltables=None, weblog=None, pprfile=None, 
#                          flagversions=None, calapply=None, casa_commands=None):
#         if not self._executor._dry_run:
#             def get_text(title, item):
#                 title_text = lambda x: '{0}: \n--------\n'.format(x)
#                 item_text = lambda x: '{0}\n'.format(os.path.basename(x))
#                 next_content = lambda: '\n'
#                 if item is not None:
#                     yield title_text(title)
#                     for i in item:
#                         yield item_text(i)
#                     yield next_content()
#                      
#             with open(os.path.join(products_dir, 'list_of_exported_files.txt'), 'w') as f:
#                 def dowrite(lines):
#                     for line in lines:
#                         f.write(line)
#                         
#                 # PPR
#                 dowrite(get_text('PPR file', [pprfile]))
#                 
#                 # Weblog
#                 dowrite(get_text('Weblog', [weblog]))
#                 
#                 # Images
#                 dowrite(get_text('Target Images', fitsfiles))
#                     
#                 # flagversions
#                 dowrite(get_text('Flagversions', flagversions))
#                 
#                 # caltables
#                 dowrite(get_text('Caltables', caltables))
#                     
#                 # calapply list
#                 dowrite(get_text('Calapply list', calapply))
#                     
#                 # casa_commands.log
#                 dowrite(get_text('Casa Commands Log', [casa_commands]))
#                                     
    def _save_final_flagversion(self, vis, flag_version_name):
        """
        Save the final flags to a final flag version.
        """
        LOG.info('Saving final flags for %s in flag version %s' % \
                 (os.path.basename(vis), flag_version_name))
        if not self._executor._dry_run:
            task = casa_tasks.flagmanager (vis=vis,
                                           mode='save', versionname=flag_version_name)
            self._executor.execute (task)

    def _export_final_flagversion(self, context, basevis, vislist, flag_version_name,
                                  products_dir):
        """
        Save the final flags version to a compressed tarfile in products.
        """
        # Save the current working directory and move to the pipeline
        # working directory. This is required for tarfile IO
        cwd = os.getcwd()
        #os.chdir (os.path.dirname(vis))
        os.chdir(context.output_dir)
    
        # Define the name of the output tarfile
        basevisname = os.path.basename(basevis)
        tarfilename = basevisname + '.flagversions.tar.gz'
        LOG.info('Storing final flags for %s in %s' % (basevisname, tarfilename))
    
        export_dir_list = []
        export_file_list = []
        for visname in vislist:
            # Define the directory to be saved
            flagsname = os.path.join (visname + '.flagversions',
                                      'flags.' + flag_version_name) 
            LOG.info('Saving flag version %s' % (flag_version_name))
            export_dir_list.append(flagsname)
    
            # Define the versions list file to be saved
            flag_version_list = os.path.join (visname + '.flagversions',
                                              'FLAG_VERSION_LIST')
            ti = tarfile.TarInfo(flag_version_list)
            #line = "Pipeline_Final : Final pipeline flags\n"
            line = "%s : Final pipeline flags\n" % flag_version_name
            ti.size = len (line)
            LOG.info('Saving flag version list')
            export_file_list.append((ti, line))
    
        # Create the tar file
        if not self._executor._dry_run:
            tar = tarfile.open (os.path.join(products_dir, tarfilename), "w:gz")
            for flagsname in export_dir_list:
                tar.add (flagsname)
            for (ti, line) in export_file_list:
                tar.addfile (ti, StringIO.StringIO(line))
            tar.close()
    
        # Restore the original current working directory
        os.chdir(cwd)
    
        return tarfilename
    
    def _generate_vislist(self):
        return list(self.__generate_vislist())
    
    def __generate_vislist(self):
        context = self.inputs.context
        
        # input_vislist is a list of MSs that are registered to pipeline
        input_vislist = context.observing_run.measurement_sets
        for vis in input_vislist:
            if os.path.exists(vis.name):
                yield vis.name
                
        # imager_vislist is a list of MSs that are generated for sd imaging
        for st in context.observing_run:
            if hasattr(st, 'exported_ms') and st.exported_ms is not None:
                if os.path.exists(st.exported_ms):
                    yield st.exported_ms
