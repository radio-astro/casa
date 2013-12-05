from __future__ import absolute_import

#import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

from pipeline.hif.tasks.importdata import importdata

class ALMAImportDataInputs( importdata.ImportDataInputs ):

	# These are ALMA specific settings and override the defaults in
	# the base class
        process_caldevice  = basetask.property_with_default('process_caldevice', False)
	asis = basetask.property_with_default('asis', 'Antenna Station Receiver CalAtmosphere')

	def __init__( self, context, vis=None, output_dir=None, asis=None,
	    process_caldevice=None, session=None, overwrite=None, save_flagonline=None ):

		# Initialize the public member variables of the inherited class
		# ImportDataInputs()

		super( ALMAImportDataInputs, self ).__init__( context, vis=vis,
		    output_dir=output_dir, asis=asis, process_caldevice=process_caldevice,
		    session=session, overwrite=overwrite, save_flagonline=save_flagonline)

class ALMAImportData( importdata.ImportData ):
	Inputs = ALMAImportDataInputs

#class ALMAImportDataResults( importdata.ImportDataResults ):
        pass






