import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import casac
import unittest

import numpy

class calanalysis_tsys1( unittest.TestCase ):

	"""This is a very simple unit test for introspective methods.  A Tsys
	   calibration table is used."""

	ca = casac.casac.calanalysis()

	calName = 'uid___A002_X30a93d_X43e.ms.tsys.s3.tbl'
	msName = 'uid___A002_X30a93d_X43e.ms'
	parType = 'Float'
	polBasis = 'U'
	visCal = 'B TSYS'

	fieldName = ['J2253+161; 3c454.3', 'Callisto',
	    'B0007+106; J0010+109', 'GRB021004']
	fieldNumber = ['0', '1', '2', '3']
	numField = len( fieldNumber )

	antennaName = ['DA41', 'DA42', 'DA43', 'DV02', 'DV03', 'DV05',
	    'DV07', 'DV10', 'DV11', 'DV12', 'DV13', 'DV14', 'PM02', 'PM03']
	antennaNumber = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	    '10', '11', '12', '13']
	numAntenna = len( antennaNumber )

	antenna1Name = antennaName
	antenna1Number = antennaNumber
	numAntenna1 = numAntenna

	antenna2Name = ['NoName']
	antenna2Number = ['-1']
	numAntenna2 = 1

	feed = ['1', '2']
	numFeed = len( feed )

	spwName = ['', '', '', '', '', '', '', '', '', '', '', '', '', '',
	    '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '']
	spwNumber = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	    '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20',
	    '21', '22', '23', '24', '25', '26', '27', '28', '29', '30']
	numSPW = len( spwNumber )

	numChannel = numpy.array([4, 128, 1, 128, 1, 128, 1, 128, 1, 128, 1,
	    128, 1, 128, 1, 128, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	    4, 4, 4, 4, 4], dtype=numpy.int32)

	time = numpy.array([4827167647.3920002, 4827167780.5439997,
	    4827168227.2320004, 4827168353.7600002, 4827168887.7600002,
	    4827169009.632, 4827169543.6800003, 4827169665.6960001,
	    4827170204.3520002, 4827170332.4160004, 4827170866.5600004,
	    4827170989.5360003, 4827171523.776, 4827171647.2799997])
	numTime = len( time )

	def setUp( self ):
		datapath = os.environ.get( 'CASAPATH' ).split()[0]
		datapath += '/data/regression/unittest/calanalysis/'
		return self.ca.open( datapath+self.calName )

	def tearDown( self ):
		return self.ca.close()

	def test_introspective( self ):

		"""Test of introspective member functions"""

		self.assertEqual( os.path.split(self.ca.calname())[1],
		    self.calName )
		self.assertEqual( os.path.split(self.ca.msname())[1],
		    self.msName )
		self.assertEqual( self.ca.partype(), self.parType )
		self.assertEqual( self.ca.polbasis(), self.polBasis )
		self.assertEqual( self.ca.viscal(), self.visCal )

		self.assertEqual( self.ca.numfield(), self.numField )
		self.assertEqual( self.ca.field(name=True), self.fieldName )
		self.assertEqual( self.ca.field(name=False), self.fieldNumber )

		self.assertEqual( self.ca.numantenna(), self.numAntenna )
		self.assertEqual( self.ca.antenna(name=True), self.antennaName )
		self.assertEqual( self.ca.antenna(name=False),
		    self.antennaNumber )

		self.assertEqual( self.ca.numantenna1(), self.numAntenna1 )
		self.assertEqual( self.ca.antenna1(name=True),
		    self.antenna1Name )
		self.assertEqual( self.ca.antenna1(name=False),
		    self.antenna1Number )

		self.assertEqual( self.ca.numantenna2(), self.numAntenna2 )
		self.assertEqual( self.ca.antenna2(name=True),
		    self.antenna2Name )
		self.assertEqual( self.ca.antenna2(name=False),
		    self.antenna2Number )

		self.assertEqual( self.ca.numfeed(), self.numFeed )
		self.assertEqual( self.ca.feed(), self.feed )

		self.assertEqual( self.ca.numspw(), self.numSPW )
		self.assertEqual( self.ca.spw(name=True), self.spwName )
		self.assertEqual( self.ca.spw(name=False), self.spwNumber )

		self.assertTrue( numpy.array_equal(self.ca.numchannel(), self.numChannel))
		self.assertEqual( self.ca.numtime(), self.numTime )
		self.assertTrue( numpy.allclose(self.ca.time(), self.time))

		return None

def suite():
	return [calanalysis_tsys1]
