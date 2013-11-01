
# ------------------------------------------------------------------------------

# findrefant.py

# Description:
# ------------
# This file contains the reference antenna heuristics.

# The present heuristics are geometry and flagging.

# Classes:
# --------
# RefAntHeuristics - This class chooses the reference antenna heuristics.
# RefAntGeometry   - This class contains the geometry heuristics for the
#                    reference antenna.
# RefAntFlagging   - This class contains the flagging heuristics for the
#                    reference antenna.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

# Imports
# -------

import numpy

import casa
from casac import casac

import pipeline.infrastructure.api as api
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


# ------------------------------------------------------------------------------
# class RefAntHeuristics
# ------------------------------------------------------------------------------

# RefAntHeuristics
# ----------------

# Description:
# ------------
# This class chooses the reference antenna heuristics.

# Inherited classes:
# ------------------
# api.Heuristics - The base class common to all types of heuristics.

# Public member variables:
# ------------------------
# vis      - This python string contains the MS name.
#
# field    - This python string or list of strings contains the field numbers
#            or IDs.  Presently it is used only for the flagging heuristic.
# spw      - This python string or list of strings contains the spectral
#            window numbers of IDs.  Presently it is used only for the
#            flagging heuristic.
# intent   - This python string or list of strings contains the intent(s).
#            Presently it is used only for the flagging heuristic.
#
# geometry - This python boolean determines whether the geometry heuristic will
#            be used.
# flagging - This python boolean determines whether the flagging heuristic will
#            be used.

# Public member functions:
# ------------------------
# __init__  - This public member function constructs an instance of the
#             RefAntHeuristics() class.
# calculate - This public member function forms the reference antenna list
#             calculated from the selected heuristics.

# Private member functions:
# -------------------------
# _get_names - This private member function gets the antenna names from the MS.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version created with public member variables vis, field,
#               spw, intent, geometry, and flagging; public member functions
#               __init__() and calculate(); and private member function
#               _get_names().

# ------------------------------------------------------------------------------

class RefAntHeuristics( api.Heuristic ):

# ------------------------------------------------------------------------------

# RefAntHeuristics::__init__

# Description:
# ------------
# This public member function constructs an instance of the RefAntHeuristics()
# class.

# The primary purpose of this class is to initialize the public member
# variables.  The defaults for all parameters (except context) are None.

# Inputs:
# -------
# vis        - This python string contains the MS name.
#
# field      - This python string or list of strings contains the field numbers
#              or IDs.  Presently it is used only for the flagging heuristic.
# spw        - This python string or list of strings contains the spectral
#              window numbers of IDs.  Presently it is used only for the
#              flagging heuristic.
# intent     - This python string or list of strings contains the intent(s).
#              Presently it is used only for the flagging heuristic.
#
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

	def __init__( self, vis, field, spw, intent, geometry, flagging ):

		# Initialize the public member variables of this class

		self.vis = vis

		self.field = field
		self.spw = spw
		self.intent = intent

		self.geometry = geometry
		self.flagging = flagging

		# Return None

		return None

# ------------------------------------------------------------------------------

# RefAntHeuristics::calculate

# Description:
# ------------
# This public member function forms the reference antenna list calculated from
# the selected heuristics.

# NB: A total score is calculated from all heuristics.  The best antennas have
# the highest scores, so a reverse sort is performed to obtain the final list.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The numpy array of strings containing the ranked reference antenna list,
# returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def calculate( self ):

		# If no heuristics are specified, return no reference antennas

		if not ( self.geometry or self.flagging ): return []


		# Get the antenna names and initialize the score dictionary

		names = self._get_names()
		LOG.debug('Got antenna name list {0}'.format(names))

		score = dict()
		for n in names: score[n] = 0.0


		# For each selected heuristic, add the score for each antenna

		if self.geometry:
			geoClass = RefAntGeometry( self.vis )
			geoScore = geoClass.calc_score()
			#for n in names: score[n] += geoScore[n]
			for n in names:
			    if geoScore.has_key(n):
			        score[n] += geoScore[n]
		                LOG.debug('Antenna {0} geometry score {1}  total score {2}'.format(n, geoScore[n], score[n]))
				

		if self.flagging:
			flagClass = RefAntFlagging( self.vis, self.field,
			    self.spw, self.intent )
			flagScore = flagClass.calc_score()
			#for n in names: score[n] += flagScore[n]
			for n in names:
			    if flagScore.has_key(n):
			        score[n] += flagScore[n]
		                LOG.info('Antenna {0} flagging score {1} total score {2}'.format(n, flagScore[n], score[n]))


		# Calculate the final score and return the list of ranked
		# reference antennas.  NB: The best antennas have the highest
		# score, so a reverse sort is required.

		keys = numpy.array( score.keys() )
		values = numpy.array( score.values() )
		argSort = numpy.argsort( values )[::-1]

		refAnt = keys[argSort]

		# Return the list of ranked reference antennas

		return( refAnt )

# ------------------------------------------------------------------------------

# RefAntHeuristics::_get_names

# Description:
# ------------
# This private member function gets the antenna names from the MS.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The numpy array of strings containing the antenna names, returned via the
# function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def _get_names( self ):

		# Create the local instance of the table tool and open the MS

		#tbLoc = casa.__tablehome__.create()
		tbLoc = casac.table()
		#tbLoc.open( self.vis[0]+'/ANTENNA' ) # Take zeroth element
		tbLoc.open( self.vis+'/ANTENNA' ) # Take zeroth element


		# Get the antenna names and capitalize them (unfortunately,
		# some CASA tools capitalize them and others don't)
		# This should no longer be necessary. Clean up code
		# later.

		names = tbLoc.getcol( 'NAME' ).tolist()

		rNames = range( len(names) )
		#for n in rNames: names[n] = names[n].upper()
		for n in rNames: names[n] = names[n]


		# Close the local instance of the table tool and delete it

		tbLoc.close()
		del tbLoc


		# Return the antenna names

		return names

# ------------------------------------------------------------------------------
# class RefAntGeometry
# ------------------------------------------------------------------------------

# RefAntGeometry
# --------------

# Description:
# ------------
# This class contains the geometry heuristics for the reference antenna.

# Algorithm:
# ----------
# * Calculate the antenna distances from the array center.
# * Normalize the distances by the maximum distance.
# * Calculate the score for each antenna, which is one minus the normalized
#   distance.  The best antennas have the highest score.
# * Sort according to score.

# Public member variables:
# ------------------------
# vis - This python string contains the MS name.

# Public member functions:
# ------------------------
# __init__   - This public member function constructs an instance of the
#              RefAntGeometry() class.
# calc_score - This public member function calculates the geometry score for
#              each antenna.

# Private member functions:
# -------------------------
# _get_info       - This private member function gets the information from the
#                   antenna table of the MS.
# _get_measures   - This private member function gets the measures from the
#                   antenna table of the MS.
# _get_latlongrad - This private member function gets the latitude, longitude
#                   and radius (from the center of the earth) for each antenna.
# _calc_distance  - This private member function calculates the antenna
#                   distances from the array reference from the radii,
#                   longitudes, and latitudes.
# _calc_score     - This private member function calculates the geometry score
#                   for each antenna.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

class RefAntGeometry:

# ------------------------------------------------------------------------------

# RefAntGeometry::__init__

# Description:
# ------------
# This public member function constructs an instance of the RefAntGeometry()
# class.

# Inputs:
# -------
# vis - This python string contains the MS name.

# Outputs:
# --------
# None, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def __init__( self, vis ):

		# Set the public variables

		self.vis = vis


		# Return None

		return None

# ------------------------------------------------------------------------------

# RefAntGeometry::calc_score

# Description:
# ------------
# This public member function calculates the geometry score for each antenna.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the score for each antenna, returned via the
# function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def calc_score( self ):

		# Get the antenna information, measures, and locations

		info = self._get_info()
		measures = self._get_measures( info )
		radii, longs, lats = self._get_latlongrad( info, measures )


		# Calculate the antenna distances and scores

		distance = self._calc_distance( radii, longs, lats )
		score = self._calc_score( distance )


		# Return the scores

		return score

# ------------------------------------------------------------------------------

# RefAntGeometry::_get_info

# Description:
# ------------
# This private member function gets the information from the antenna table of
# the MS.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the antenna information, returned via the
# function value.  The dictionary format is:
# 'position'          - This numpy array contains the antenna positions.
# 'flag_row'          - This numpy array of booleans contains the flag row
#                       booleans.  NB: This element is of limited use now and
#                       may be eliminated.
# 'name'              - This numpy array of strings contains the antenna names.
# 'position_keywords' - This python dictionary contains the antenna information.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def _get_info( self ):

		# Create the local instance of the table tool and open it with
		# the antenna subtable of the MS

		#tbLoc = casa.__tablehome__.create()
		tbLoc = casac.table()
		#tbLoc.open( self.vis[0]+'/ANTENNA' ) # Take zeroth element
		tbLoc.open( self.vis+'/ANTENNA' ) # Take zeroth element


		# Get the antenna information from the antenna table

		info = dict()

		info['position'] = tbLoc.getcol( 'POSITION' )
		info['flag_row'] = tbLoc.getcol( 'FLAG_ROW' )
		info['name'] = tbLoc.getcol( 'NAME' )
		info['position_keywords'] = tbLoc.getcolkeywords( 'POSITION' )


		# Close the table tool and delete the local instance

		tbLoc.close()
		del tbLoc


		# The flag tool appears to return antenna names as upper case,
		# which seems to be different from the antenna names stored in
		# MSes.  Therefore, these names will be capitalized here.

		rRow = range( len( info['name'] ) )
		#for r in rRow: info['name'][r] = info['name'][r].upper()


		# Return the antenna information

		return info

# ------------------------------------------------------------------------------

# RefAntGeometry::_get_measures

# Description:
# ------------
# This private member function gets the measures from the antenna table of the
# MS.

# Inputs:
# -------
# info - This python dictionary contains the antenna information from private
#        member function _get_info().

# Outputs:
# --------
# The python dictionary containing the antenna measures, returned via the
# function value.  The dictionary format is:
# '<antenna name>' - The python dictionary containing the antenna measures.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def _get_measures( self, info ):

		# Create the local instances of the measures and quanta tools

		#meLoc = casa.__measureshome__.create()
		meLoc = casac.measures()
		#qaLoc = casa.__quantahome__.create()
		qaLoc = casac.quanta()


		# Initialize the measures dictionary and the position and
		# position_keywords variables

		measures = dict()

		position = info['position']
		position_keywords = info['position_keywords']

		rf = position_keywords['MEASINFO']['Ref']

		for row,ant in enumerate( info['name'] ):

			if not info['flag_row'][row]:

				p = position[0,row]
				pk = position_keywords['QuantumUnits'][0]
				v0 = qaLoc.quantity( p, pk )

				p = position[1,row]
				pk = position_keywords['QuantumUnits'][1]
				v1 = qaLoc.quantity( p, pk )

				p = position[2,row]
				pk = position_keywords['QuantumUnits'][2]
				v2 = qaLoc.quantity( p, pk )

				measures[ant] = meLoc.position( rf=rf, v0=v0,
				    v1=v1, v2=v2 )


		# Delete the local instances of the measures and quanta tools

		del qaLoc
		del meLoc


		# Return the measures

		return measures

# ------------------------------------------------------------------------------

# RefAntGeometry::_get_latlongrad

# Description:
# ------------
# This private member function gets the latitude, longitude and radius (from the
# center of the earth) for each antenna.

# Inputs:
# -------
# info     - This python dictionary contains the antenna information from
#            private member function _get_info().
# measures - This python dictionary contains the antenna measures from private
#            member function _get_measures().

# Outputs:
# --------
# The python tuple containing containing radius, longitude, and latitude python
# dictionaries, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def _get_latlongrad( self, info, measures ):

		# Create the local instance of the quanta tool

		#qaLoc = casa.__quantahome__.create()
		qaLoc = casac.quanta()


		# Get the radii, longitudes, and latitudes

		radii = dict()
		longs = dict()
		lats = dict()

		for ant in info['name']:

			value = measures[ant]['m2']['value']
			unit = measures[ant]['m2']['unit']
			quantity = qaLoc.quantity( value, unit )
			convert = qaLoc.convert( quantity, 'm' )
			radii[ant] = qaLoc.getvalue( convert )

			value = measures[ant]['m0']['value']
			unit = measures[ant]['m0']['unit']
			quantity = qaLoc.quantity( value, unit )
			convert = qaLoc.convert( quantity, 'rad' )
			longs[ant] = qaLoc.getvalue( convert )

			value = measures[ant]['m1']['value']
			unit = measures[ant]['m1']['unit']
			quantity = qaLoc.quantity( value, unit )
			convert = qaLoc.convert( quantity, 'rad' )
			lats[ant] = qaLoc.getvalue( convert )


		# Delete the local instance of the quanta tool

		del qaLoc


		# Return the tuple containing the radius, longitude, and
		# latitude python dictionaries

		return radii, longs, lats

# ------------------------------------------------------------------------------

# RefAntGeometry::_calc_distance

# Description:
# ------------
# This private member function calculates the antenna distances from the array
# reference from the radii, longitudes, and latitudes.

# NB: The array reference is the median location.

# Inputs:
# -------
# radii - This python dictionary contains the radius (from the center of the
#         earth) for each antenna.
# longs - This python dictionary contains the longitude for each antenna.
# lats  - This python dictionary contains the latitude for each antenna.

# Outputs:
# --------
# The python dictionary containing the antenna distances from the array
# reference, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def _calc_distance( self, radii, longs, lats ):

		# Convert the dictionaries to numpy float arrays.  The median
		# longitude is subtracted.

		radiusValues = numpy.array( radii.values() )

		longValues = numpy.array( longs.values() )
		longValues -= numpy.median( longValues )

		latValues = numpy.array( lats.values() )


		# Calculate the x and y antenna locations.  The medians are
		# subtracted.

		x = longValues * numpy.cos(latValues) * radiusValues
		x -= numpy.median( x )

		y = latValues * radiusValues
		y -= numpy.median( y )


		# Calculate the antenna distances from the array reference and
		# return them

		distance = dict()
		names = radii.keys()

		for i,ant in enumerate(names):
			distance[ant] = numpy.sqrt( pow(x[i],2) + pow(y[i],2) )

		return distance

# ------------------------------------------------------------------------------

# RefAntGeometry::_calc_score

# Description:
# ------------
# This private member function calculates the geometry score for each antenna.

# Algorithm:
# ----------
# * Calculate the antenna distances from the array center.
# * Normalize the distances by the maximum distance.
# * Calculate the score for each antenna, which is one minus the normalized
#   distance.  The best antennas have the highest score.
# * Sort according to score.

# Inputs:
# -------
# distance - This python dictionary contains the antenna distances from the
#            array reference.  They are calculated in private member function
#            _calc_distance().

# Outputs:
# --------
# The python dictionary containing the score for each antenna, returned via the
# function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def _calc_score( self, distance ):

		# Get the number of good data, calculate the fraction of good
		# data, and calculate the good and bad weights

		far = numpy.array( distance.values(), numpy.float )
		fFar = far / float( numpy.max(far) )

		wFar = fFar * len(far)
		wClose = ( 1.0 - fFar ) * len(far)


		# Calculate the score for each antenna and return them

		score = dict()

		names = distance.keys()
		rName = range( len(wClose) )

		#for n in rName: score[names[n]] = wClose[n]
		for n in rName: score[names[n]] = wClose[n][0]

		return score

# ------------------------------------------------------------------------------

# RefAntFlagging
# --------------

# Description:
# ------------
# This class contains the flagging heuristics for the reference antenna.

# Algorithm:
# ----------
# * Get the number of unflagged (good) data for each antenna.
# * Normalize the good data by the maximum good data.
# * Calculate the score for each antenna, which is one minus the normalized
#   number of good data.  The best antennas have the highest score.
# * Sort according to score.

# Public member variables:
# ------------------------
# vis    - This python string contains the MS name.
#
# field  - This python string or list of strings contains the field numbers or
#          or IDs.
# spw    - This python string or list of strings contains the spectral window
#          numbers of IDs.
# intent - This python string or list of strings contains the intent(s).

# Public member functions:
# ------------------------
# __init__   - This public member function constructs an instance of the
#              RefAntFlagging() class.
# calc_score - This public member function calculates the flagging score for
#              each antenna.

# Private member functions:
# -------------------------
# _get_good   - This private member function gets the number of unflagged (good)
#               data from the MS.
# _calc_score - This private member function calculates the flagging score for
#               each antenna.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

class RefAntFlagging:

# ------------------------------------------------------------------------------

# RefAntFlagging::__init__

# Description:
# ------------
# This public member function constructs an instance of the RefAntFlagging()
# class.

# Inputs:
# -------
# vis    - This python string contains the MS name.
#
# field  - This python string or list of strings contains the field numbers or
#          or IDs.
# spw    - This python string or list of strings contains the spectral window
#          numbers of IDs.
# intent - This python string or list of strings contains the intent(s).

# Outputs:
# --------
# None, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def __init__( self, vis, field, spw, intent ):

		# Set the public member functions

		self.vis = vis

		self.field = field
		self.spw = spw
		self.intent = intent


		# Return None

		return None

# ------------------------------------------------------------------------------

# RefAntFlagging::calc_score

# Description:
# ------------
# This public member function calculates the flagging score for each antenna.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the score for each antenna, returned via the
# function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def calc_score( self ):

		# Calculate the number of unflagged (good) measurements for each
		# antenna, determine the score, and return them

		good = self._get_good()
		LOG.info('Get good antennas {0}'.format(good))
		score = self._calc_score( good )
		LOG.info('Get good antenna score {0}'.format(score))

		return( score )

# ------------------------------------------------------------------------------

# RefAntFlagging::_get_good

# Description:
# ------------
# This private member function gets the number of unflagged (good) data from the
# MS.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The dictionary containing the number of unflagged (good) data from the MS,
# returned via the function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def _get_good( self ):

		# Create the local version of the flag tool and open the MS

		#fgLoc = casac.flagger()
		fgLoc = casac.agentflagger()
		fgLoc.open( self.vis )


		# Get the flag statistics from the MS

		#fgLoc.setdata( field=self.field, spw=self.spw,
		    #intent=self.intent )
		fgLoc.selectdata( field=self.field, spw=self.spw,
		    intent=self.intent )

		agents = {}
		agents['mode'] = 'summary'
		fgLoc.parseagentparameters(agents)
		#fgLoc.setflagsummary()

		fgLoc.init()
		d = fgLoc.run()
		fgLoc.done()


		# Delete the local version of the flag tool

		del fgLoc

		# Calculate the number of good data for each antenna and return
		# them

		antenna = d['report0']['antenna']
		good = dict()

		for a in antenna.keys():
			good[a] = antenna[a]['total'] - antenna[a]['flagged']

		return( good )

# ------------------------------------------------------------------------------

# RefAntFlagging::_calc_score

# Description:
# ------------
# This private member function calculates the flagging score for each antenna.

# Algorithm:
# ----------
# * Get the number of unflagged (good) data for each antenna.
# * Normalize the good data by the maximum good data.
# * Calculate the score for each antenna, which is one minus the normalized
#   number of good data.  The best antennas have the highest score.
# * Sort according to score.

# Inputs:
# -------
# good - This python dictionary contains the number of unflagged (good) data
#        from the MS.  They are obtained in private member function _get_good().

# Outputs:
# --------
# The python dictionary containing the score for each antenna, returned via the
# function value.

# Modification history:
# ---------------------
# 2012 May 21 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def _calc_score( self, good ):

		# Get the number of good data, calculate the fraction of good
		# data, and calculate the good and bad weights

		nGood = numpy.array( good.values(), numpy.float )
		fGood = nGood / float( numpy.max(nGood) )

		wGood = fGood * len(nGood)
		wBad = ( 1.0 - fGood ) * len(nGood)


		# Calculate the score for each antenna and return them

		score = dict()

		names = good.keys()
		rName = range( len(wGood) )

		for n in rName: score[names[n]] = wGood[n]

		return score
