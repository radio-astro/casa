# ------------------------------------------------------------------------------

# referenceantenna.py

# Description:
# ------------
# This file contains the classes to return a ranked list of reference antennas.

# NB: The heuristics for forming the ranked list of reference antennas are
# located in findrefant.py.
# NB: The supported heuristics are geometry and flagging.

# Classes:
# --------
# RefAnt        - This class calls the heuristics to rank antennas as reference
#                 antennas.
# RefAntInputs  - This class manages the inputs for the RefAnt() class.
# RefAntResults - This class manages the results from the RefAnt() class.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

# Imports
# -------
import os
import string

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from ...heuristics import findrefant

# ------------------------------------------------------------------------------

# Initialize a logger
# -------------------

LOG = infrastructure.get_logger(__name__)


# ------------------------------------------------------------------------------
# class RefAntInputs
# ------------------------------------------------------------------------------

# RefAntInputs:
# -------------

# Description:
# ------------
# This class calls the heuristics to rank antennas as reference antennas.

# Inherited classes:
# ------------------
# api.Inputs                 - The base class common to all types of inputs.
# common.PipelineInputsMixin - The class to min in arguments to the pipeline
#                              class.

# Public member variables:
# ------------------------
# context    - This python dictionary contains the pipeline context (state).
#
# vis        - This python string contains the MS name.
#
# output_dir - This python string contains the output directory name.
#
# field      - This python string or list of strings contains the field numbers
#              or IDs.  Presently it is used only for the flagging heuristic.
# spw        - This python string or list of strings contains the spectral
#              window numbers of IDs.  Presently it is used only for the
#              flagging heuristic.
# intent     - This python string or list of strings contains the intent(s).
#              Presently it is used only for the flagging heuristic.
#
# hm_refant   - This python string contains the heuristics method.  It must be either 'manual'
#              or 'automatic'.
#
# refant     - This python list of strings contains an ordered list of reference
#              antennas used for manual mode (i.e., no heuristics).
# geometry   - This python boolean determines whether the geometry heuristic
#              will be used in automatic mode.
# flagging   - This python boolean determines whether the flagging heuristic
#              will be used in flagging mode.

# Public member functions:
# ------------------------
# __init__     - This public member function constructs an instance of the
#                RefAntInputs() class.
# clone        - This public member function creates a cloned instance of an
#                existing instance.
# to_casa_args - This public member function translates the input parameters of
#                this class.  It is not used presently.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version created with public member variables context,
#               vis, output_dir, field, spw, intent, hm_refant, refant, geometry, and
#               flagging; public member functions __init__() and clone().

# ------------------------------------------------------------------------------

class RefAntInputs(basetask.StandardInputs):

# ------------------------------------------------------------------------------

# RefAnt::__init__

# Description:
# ------------
# This public member function constructs an instance of the RefAntInputs()
# class.

# The primary purpose of this class is to initialize the public member
# variables.  The defaults for all parameters (except context) are None.

# Inputs:
# -------
# context    - This python dictionary contains the pipeline context (state).
#
# vis        - This python string contains the MS name.
#
# output_dir - This python string contains the output directory name.
#
# field      - This python string or list of strings contains the field numbers
#              or IDs.  Presently it is used only for the flagging heuristic.
# spw        - This python string or list of strings contains the spectral
#              window numbers of IDs.  Presently it is used only for the
#              flagging heuristic.
# intent     - This python string or list of strings contains the intent(s).
#              Presently it is used only for the flagging heuristic.
#
# hm_refant    - This python string contains the heuristics method or mode.  It must be either 'manual'
#              or 'automatic'.
#
# refant     - This python list of strings contains an ordered list of reference
#              antennas used for manual mode (i.e., no heuristics).
# geometry   - This python boolean determines whether the geometry heuristic
#              will be used in automatic mode.
# flagging   - This python boolean determines whether the flagging heuristic
#              will be used in automatic mode.

# Outputs:
# --------
# None, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def __init__( self, context, vis=None, output_dir=None, field=None,
                      spw=None, intent=None, hm_refant=None, refant=None,
		      geometry=None, flagging=None ):

                # set MandatoryInputsMixin parameters
		#self.context = context
		#self.output_dir = output_dir
		#self.vis = vis 

		# Other parameters
		#self.field = field
		#self.intent = intent
		#self.spw = spw
		#self.hm_refant = hm_refant
		#self.refant = refant
		#self.geometry = geometry
		#self.flagging = flagging
		self._init_properties(vars())

        @property
        def field(self):
                return self._field

        @field.setter
        def field(self, value):
                if value is None:
                        value = ''
                self._field = value

        @property
        def intent(self):
                return self._intent

        @intent.setter
        def intent(self, value):
                if value is None:
                        value = ''
                self._intent = value

        @property
        def spw(self):
                return self._spw

        @spw.setter
        def spw(self, value):
                if value is None:
                        value = ''
                self._spw = value

        @property
        def hm_refant(self):
                return self._hm_refant

        @hm_refant.setter
        def hm_refant(self, value):
                if value is None:
                        value = 'automatic'
                if value in 'automatic | manual':
                    self._hm_refant = value
                else:
                    self._hm_refant = 'automatic'
        @property
        def refant(self):
                return self._refant

        @refant.setter
        def refant(self, value):
                if value is None:
                        value = ''
                self._refant = value

        @property
        def geometry(self):
                return self._geometry

        @geometry.setter
        def geometry(self, value):
                if value is None:
                        value = True
                self._geometry = value

        @property
        def flagging(self):
                return self._flagging

        @flagging.setter
        def flagging(self, value):
                if value is None:
                        value = True
                self._flagging = value



# ------------------------------------------------------------------------------
# RefAntInputs::to_casa_args

# Description:
# ------------
# This public member function translates the input parameters of this class.  It
# is not used presently.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The empty python dictionary, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial stub version.

# ------------------------------------------------------------------------------

	def to_casa_args( self ): return {}

# ------------------------------------------------------------------------------
# class RefAntResults
# ------------------------------------------------------------------------------

# RefAntResults
# -------------

# Description:
# ------------
# This class manages the results from the RefAnt() class.

# Inherited classes:
# ------------------
# api.Results - The base class common to all types of results.

# Public member functions:
# ------------------------
# __init__           - This public member function constructs an instance of the
#                      RefAntResults() class.
# merge_with_context - This public member function merges the flagging results
#                      with the pipeline context.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version created with public member functions __init__()
#               and merge_with_context().

# ------------------------------------------------------------------------------

class RefAntResults( basetask.Results ):

# ------------------------------------------------------------------------------

# RefAntResults::__init__

# Description:
# ------------
# This public member function constructs an instance of the RefAntResults()
# class.

# Inputs:
# -------
# vis    - This python string contains the MS name.
# refant - This python list of strings contains an ordered list of reference
#          antennas used for manual mode (i.e., no heuristics).

# Outputs:
# --------
# None, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

    def __init__( self, vis, refant ):
        super(RefAntResults, self).__init__()

        # Initialize the private member variables
        self._vis = vis
        self._refant = ','.join([str(ant) for ant in refant])


# ------------------------------------------------------------------------------

# RefAntResults::merge_with_context

# Description:
# ------------
# This public member function merges the reference antenna list with the
# pipeline context.

# Inputs:
# -------
# context - This python dictionary contains the context.

# Outputs:
# --------
# None, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

    def merge_with_context( self, context ):
        if self._vis is None or self._refant is None:
            LOG.error ( 'No results to merge' )
            return

        # Do we also need to locate the antenna in the measurement set?
        # This might become necessary when using different sessions

        ms = context.observing_run.get_ms( name=self._vis )

        if ms:
            LOG.debug('Setting refant for {0} to \'{1}\''
                ''.format(ms.basename, self._refant))
                #''.format(self._refant.identifier, ms.name))
            ms.reference_antenna = self._refant

# ------------------------------------------------------------------------------

# RefAntResults::__repr__

# ------------------------------------------------------------------------------

    def __repr__( self ):
        if self._vis is None or self._refant is None:
            return ('Reference antenna results:\n'
                '\tNo reference antenna selected')
        else:
            return ('Reference antenna results:\n'
                '{ms}: refant=\'{refant}\''
                ''.format(ms=os.path.basename(self._vis),
                refant=self._refant))
                #refant=self._refant.identifier))

# ------------------------------------------------------------------------------
# class RefAnt
# ------------------------------------------------------------------------------

# RefAnt
# ------

# Description:
# ------------
# This class represents the pipeline interface to the reference antenna
# heuristics.

# Inherited classes:
# ------------------
# basetask.StandardTaskTemplate - The simple task class.

# Public member functions:
# ------------------------
# All public member functions from the RefAntInputs() class.
# prepare - This public member function organizes preparatory jobs.
# analyse - This public member function invokes the reference antenna
#           heuristics.

# Private member functions:
# -------------------------
# All private member functions from the RefAntInputs() class.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version created with public member functions analyse()
#               and prepare().

# ------------------------------------------------------------------------------

class RefAnt( basetask.StandardTaskTemplate ):

# ------------------------------------------------------------------------------

	# Make the member functions of the RefAntInputs() class member functions
	# of this class

	Inputs = RefAntInputs

# ------------------------------------------------------------------------------

# RefAnt::prepare

# Description:
# ------------
# This public member function organizes preparatory jobs.

# NB: This public member function is trivial in this particular class.

# Inputs:
# -------
# ** parameters - This python dictionary contains the parameters.

# Outputs:
# --------
# An empty python list, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def prepare( self, ** parameters):

	        inputs = self.inputs

		# Get the reference antenna list
		if inputs.hm_refant == 'manual':
			refant = string.split(inputs.refant, ',')
		elif inputs.hm_refant == 'automatic':
			heuristics = findrefant.RefAntHeuristics(
			    vis=inputs.vis, field=inputs.field,
			    spw=inputs.spw, intent=inputs.intent,
			    geometry=inputs.geometry,
			    flagging=inputs.flagging )
			refant = heuristics.calculate()
		else:
		    pass

		return RefAntResults( inputs.vis, refant )

# ------------------------------------------------------------------------------

# RefAnt::analyse

# Description:
# ------------
# This public member function invokes the reference antenna heuristics.

# Inputs:
# -------
# jobs - This python list contains the jobs.  It is unused now.

# Outputs:
# --------
# The python list containing the list of jobs to execute, returned via the
# function value.  It is empty now.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def analyse( self, results):

		return results
