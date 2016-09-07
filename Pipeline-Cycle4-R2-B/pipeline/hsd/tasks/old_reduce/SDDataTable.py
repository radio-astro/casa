#
# ALMA - Atacama Large Milliiter Array (c) European Southern Observatory, 2002
# Copyright by ESO (in the framework of the ALMA collaboration), All rights
# reserved
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#
# $Revision: 1.1.2.8 $
# $Date: 2012/09/15 11:46:20 $
# $Author: tnakazat $
#

from taskinit import gentools
import os
import re
import string
import time
import numpy

def __coldesc( vtype, option, maxlen,
             ndim, comment, unit=None ):
    d={'dataManagerGroup': 'StandardStMan',
       'dataManagerType': 'StandardStMan'}
    d['valueType'] = vtype
    d['option'] = option
    if ndim > 0:
        d['ndim'] = ndim
    d['maxlen'] = maxlen
    d['comment'] = comment
    if unit is not None:
        d['keywords'] = {'UNIT': unit }
    return d

# List of table columns
TD_NAME = [
    #'ID',
    'ROW', 'SCAN', 'IF', 'POL', 'BEAM', 'DATE',
    'TIME', 'ELAPSED', 'EXPOSURE', 'RA', 'DEC',
    'AZ', 'EL', 'NCHAN', 'TSYS', 'TARGET', 'STATISTICS',
    'FLAG', 'FLAG_PERMANENT', 'FLAG_SUMMARY',
    'NMASK', 'MASKLIST', 'NOCHANGE', 'ANTENNA'
    ]

# Description for data table columns as dictionary.
# Each value is a tuple containing:
#
#    (valueType,option,ndim,maxlen,comment[,unit])
#
# dataManagerGroup and dataManagerType is always 'StandardStMan'.
## TD_DESC = [
## #    __coldesc('integer', 0, 0, -1, 'Primary key'),
##     __coldesc('integer', 0, 0, -1, 'Row number'),
##     __coldesc('integer', 0, 0, -1, 'Scan number'),
##     __coldesc('integer', 0, 0, -1, 'IF number'),
##     __coldesc('integer', 0, 0, -1, 'Pol number'),
##     __coldesc('integer', 0, 0, -1, 'Beam number'),
##     __coldesc('string',  0, 0, -1, 'Date'),
##     __coldesc('double',  0, 0, -1, 'Time in MJD', 'sec'),
##     __coldesc('double',  0, 0, -1, 'Elapsed time since first scan' 'sec'),
##     __coldesc('double',  0, 0, -1, 'Exposure time', 'sec'),
##     __coldesc('double',  0, 0, -1, 'Right Ascension', 'deg'),
##     __coldesc('double',  0, 0, -1, 'Declination', 'deg'),
##     __coldesc('double',  0, 0, -1, 'Azimuth', 'deg'),
##     __coldesc('double',  0, 0, -1, 'Elevation', 'deg'),
##     __coldesc('integer', 0, 0, -1, 'Number of channels'),
##     __coldesc('double',  0, 0, -1, 'Tsys', 'K'),
##     __coldesc('string',  0, 0, -1, 'Target name'),
##     __coldesc('double',  0, 0,  1, 'Statistics'),
##     __coldesc('integer', 0, 0,  1, 'Flgas'),
##     __coldesc('integer', 0, 0,  1, 'Permanent flags'),
##     __coldesc('integer', 0, 0, -1, 'Actual flag'),
##     __coldesc('integer', 0, 0, -1, 'Number of mask regions'),
##     __coldesc('integer', 0, 0,  2, 'List of mask ranges'),
##     #__coldesc('boolean', 0, 0, -1, 'Unchanged row or not'),
##     __coldesc('integer', 0, 0, -1, 'Unchanged row or not'),
##     __coldesc('integer', 0, 0, -1, 'Antenna index')
##     ]

TD_DESC_RO = [
#    __coldesc('integer', 0, 0, -1, 'Primary key'),
    __coldesc('integer', 0, 0, -1, 'Row number'),
    __coldesc('integer', 0, 0, -1, 'Scan number'),
    __coldesc('integer', 0, 0, -1, 'IF number'),
    __coldesc('integer', 0, 0, -1, 'Pol number'),
    __coldesc('integer', 0, 0, -1, 'Beam number'),
    __coldesc('string',  0, 0, -1, 'Date'),
    __coldesc('double',  0, 0, -1, 'Time in MJD', 'sec'),
    __coldesc('double',  0, 0, -1, 'Elapsed time since first scan' 'sec'),
    __coldesc('double',  0, 0, -1, 'Exposure time', 'sec'),
    __coldesc('double',  0, 0, -1, 'Right Ascension', 'deg'),
    __coldesc('double',  0, 0, -1, 'Declination', 'deg'),
    __coldesc('double',  0, 0, -1, 'Azimuth', 'deg'),
    __coldesc('double',  0, 0, -1, 'Elevation', 'deg'),
    __coldesc('integer', 0, 0, -1, 'Number of channels'),
    __coldesc('double',  0, 0, -1, 'Tsys', 'K'),
    __coldesc('string',  0, 0, -1, 'Target name'),
    __coldesc('integer', 0, 0, -1, 'Antenna index')
    ]

TD_DESC_RW = [
    __coldesc('double',  0, 0,  1, 'Statistics'),
    __coldesc('integer', 0, 0,  1, 'Flgas'),
    __coldesc('integer', 0, 0,  1, 'Permanent flags'),
    __coldesc('integer', 0, 0, -1, 'Actual flag'),
    __coldesc('integer', 0, 0, -1, 'Number of mask regions'),
    __coldesc('integer', 0, 0,  2, 'List of mask ranges'),
    __coldesc('integer', 0, 0, -1, 'Unchanged row or not')
    ]

#TD_INDEX = dict( zip(TD_NAME[1:], range(len(TD_NAME[1:]))) )
## TD_INDEX = dict( zip(TD_NAME, range(len(TD_NAME))) )

## def __tabledesc():
##     return dict( zip(TD_NAME, TD_DESC) )

def __tabledescro():
    name = [
        'ROW', 'SCAN', 'IF', 'POL', 'BEAM', 'DATE',
        'TIME', 'ELAPSED', 'EXPOSURE', 'RA', 'DEC',
        'AZ', 'EL', 'NCHAN', 'TSYS', 'TARGET', 'ANTENNA'
        ]
    return dict( zip(name,TD_DESC_RO) )

def __tabledescrw():
    name = [
        'STATISTICS', 'FLAG', 'FLAG_PERMANENT',
        'FLAG_SUMMARY', 'NMASK', 'MASKLIST', 'NOCHANGE'
        ]
    return dict( zip(name,TD_DESC_RW) )

## TABLEDESC = __tabledesc()
TABLEDESC_RO = __tabledescro()
TABLEDESC_RW = __tabledescrw()

def cast_type_get( v ):
    t = type(v)
    st = str(t)
    #print 'input type: %s'%(st)
    tmp = string.Template('<type \'numpy.${base}${n}\'>')
    if re.match(tmp.safe_substitute(base='int',n='[0-9]+'),st):
        val = int(v)
    elif re.match(tmp.safe_substitute(base='float',n='[0-9]+'),st) \
         or re.match(tmp.safe_substitute(base='double',n='.*'),st):
        val = float(v)
    elif re.match(tmp.safe_substitute(base='ndarray',n='.*'),st):
        val = v.tolist()
    else:
        val = v
    #print 'output type: %s'%(type(val))
    return val

def cast_type_put( v ):
    t = type(v)
    st = str(t)
    #print 'input type: %s'%(st)
    tmp = string.Template('<type \'numpy.${base}${n}\'>')
    if re.match(tmp.safe_substitute(base='int',n='[0-9]+'),st):
        val = int(v)
    elif re.match(tmp.safe_substitute(base='float',n='[0-9]+'),st) \
         or re.match(tmp.safe_substitute(base='double',n='.*'),st):
        val = float(v)
    elif re.match('<type \'list\'>',st):
        val = numpy.array(v)
    else:
        val = v
    #print 'output type: %s'%(type(val))
    return val

class DataTableImpl( object ):
    def __init__( self, nrow=0 ):
        """
        nrow -- number of rows
        """
        # unique memory table name
        timestamp = ('%f'%(time.time())).replace('.','')
        self.memtable1 = 'DataTableImplRO%s.MemoryTable'%(timestamp)
        self.memtable2 = 'DataTableImplRW%s.MemoryTable'%(timestamp)
        self.plaintable = ''
        self.cols = {}
        
        (self.tb1,self.tb2) = gentools( ['tb','tb'] )
        self.isopened = False
        self.nrow = nrow
        self._create()

    def __del__( self ):
        # make sure that table is closed
        #print '__del__ close CASA table...'
        self.cols.clear()
        self._close()

    def __len__( self ):
        return self.tb1.nrows() + len(self.tb1.getkeywords())

    def nrows( self ):
        return self.tb1.nrows()

    def addrows( self, nrow ):
        self.tb1.addrows( nrow )
        self.tb2.addrows( nrow )

        # reset self.plaintable since memory table and corresponding
        # plain table have different nrows
        self.plaintable == ''

    def colnames( self ):
        return self.cols.keys()

    def getcol( self, name, startrow=0, nrow=-1, rowincr=1 ):
        return self.cols[name].getcol(startrow,nrow,rowincr)

    def putcol( self, name, val, startrow=0, nrow=-1, rowincr=1  ):
        self.cols[name].putcol(val,startrow,nrow,rowincr)

    def getcell( self, name, idx ):
        return self.cols[name].getcell(idx)

    def putcell( self, name, idx, val ):
        """
        name -- column name
        idx -- row index
        val -- value to be put
        """
        self.cols[name].putcell(idx,val)

    def putkeyword( self, name, val ):
        """
        name -- keyword name
        val -- keyword value
        """
        self.tb1.putkeyword( name, val )

    def getkeyword( self, name ):
        """
        name -- keyword name
        """
        return self.tb1.getkeyword( name )

    def importdata( self, name, minimal=True ):
        """
        name -- name of DataTable to be imported
        """
        print 'Importing DataTable from %s...'%(name)

        # copy input table to memory
        self._copyfrom( name, minimal )
        self.plaintable = os.path.abspath( os.path.expandvars(name) )
        self._initcols2()

    def exportdata( self, name, minimal=True, overwrite=False ):
        """
        name -- name of exported DataTable
        overwrite -- overwrite existing DataTable
        """
        print 'Exporting DataTable to %s...'%(name)
        # overwrite check
        abspath = os.path.abspath( os.path.expandvars(name) )
        basename = os.path.basename(abspath)
        if not os.path.exists(abspath):
            os.mkdir(abspath)
        elif overwrite:
            print 'Overwrite existing DataTable %s...'%(name)
            #os.system( 'rm -rf %s/*'%(abspath) )
        else:
            raise IOError('The file %s exists.'%(name))

        # save
        if not minimal or not os.path.exists( os.path.join(abspath,'RO') ):
            #print 'Exporting RO table'
            tbloc = self.tb1.copy( os.path.join(abspath,'RO'), deep=True,
                                   valuecopy=True, returnobject=True )
            tbloc.close()
        #print 'Exporting RW table'
        tbloc = self.tb2.copy( os.path.join(abspath,'RW'), deep=True,
                               valuecopy=True, returnobject=True )
        tbloc.close()
        self.plaintable = abspath

    def _create( self ):
        self._close()
        self.tb1.create( tablename=self.memtable1,
                         tabledesc=TABLEDESC_RO,
                         memtype='memory',
                         nrow=self.nrow )
        self.tb2.create( tablename=self.memtable2,
                         tabledesc=TABLEDESC_RW,
                         memtype='memory',
                         nrow=self.nrow )
        self.isopened = True
        self._initcols()

    def _initcols( self ):
        self.cols.clear()
        self.cols = {
            'ROW': RWDataTableColumn(self.tb1,'ROW'),
            'SCAN': RWDataTableColumn(self.tb1,'SCAN'),
            'IF': RWDataTableColumn(self.tb1,'IF'),
            'POL': RWDataTableColumn(self.tb1,'POL'),
            'BEAM': RWDataTableColumn(self.tb1,'BEAM'),
            'DATE': RWDataTableColumn(self.tb1,'DATE'),
            'TIME': RWDataTableColumn(self.tb1,'TIME'),
            'ELAPSED': RWDataTableColumn(self.tb1,'ELAPSED'),
            'EXPOSURE': RWDataTableColumn(self.tb1,'EXPOSURE'),
            'RA': RWDataTableColumn(self.tb1,'RA'),
            'DEC': RWDataTableColumn(self.tb1,'DEC'),
            'AZ': RWDataTableColumn(self.tb1,'AZ'),
            'EL': RWDataTableColumn(self.tb1,'EL'),
            'NCHAN': RWDataTableColumn(self.tb1,'NCHAN'),
            'TSYS': RWDataTableColumn(self.tb1,'TSYS'),
            'TARGET': RWDataTableColumn(self.tb1,'TARGET'),
            'STATISTICS': RWDataTableColumn(self.tb2,'STATISTICS'),
            'FLAG': RWDataTableColumn(self.tb2,'FLAG'),
            'FLAG_PERMANENT': RWDataTableColumn(self.tb2,'FLAG_PERMANENT'),
            'FLAG_SUMMARY': RWDataTableColumn(self.tb2,'FLAG_SUMMARY'),
            'NMASK': RWDataTableColumn(self.tb2,'NMASK'),
            'MASKLIST': DataTableColumnMaskList(self.tb2),
            'NOCHANGE': DataTableColumnNoChange(self.tb2),
            'ANTENNA': RWDataTableColumn(self.tb1,'ANTENNA')
            }

    def _initcols2( self ):
        self.cols.clear()
        self.cols = {
            'ROW': RODataTableColumn(self.tb1,'ROW'),
            'SCAN': RODataTableColumn(self.tb1,'SCAN'),
            'IF': RODataTableColumn(self.tb1,'IF'),
            'POL': RODataTableColumn(self.tb1,'POL'),
            'BEAM': RODataTableColumn(self.tb1,'BEAM'),
            'DATE': RODataTableColumn(self.tb1,'DATE'),
            'TIME': RODataTableColumn(self.tb1,'TIME'),
            'ELAPSED': RODataTableColumn(self.tb1,'ELAPSED'),
            'EXPOSURE': RODataTableColumn(self.tb1,'EXPOSURE'),
            'RA': RODataTableColumn(self.tb1,'RA'),
            'DEC': RODataTableColumn(self.tb1,'DEC'),
            'AZ': RODataTableColumn(self.tb1,'AZ'),
            'EL': RODataTableColumn(self.tb1,'EL'),
            'NCHAN': RODataTableColumn(self.tb1,'NCHAN'),
            'TSYS': RODataTableColumn(self.tb1,'TSYS'),
            'TARGET': RODataTableColumn(self.tb1,'TARGET'),
            'STATISTICS': RWDataTableColumn(self.tb2,'STATISTICS'),
            'FLAG': RWDataTableColumn(self.tb2,'FLAG'),
            'FLAG_PERMANENT': RWDataTableColumn(self.tb2,'FLAG_PERMANENT'),
            'FLAG_SUMMARY': RWDataTableColumn(self.tb2,'FLAG_SUMMARY'),
            'NMASK': RWDataTableColumn(self.tb2,'NMASK'),
            'MASKLIST': DataTableColumnMaskList(self.tb2),
            'NOCHANGE': DataTableColumnNoChange(self.tb2),
            'ANTENNA': RODataTableColumn(self.tb1,'ANTENNA')
            }

    def _close( self ):
        if self.isopened:
            self.tb1.close()
            self.tb2.close()
            self.isopened = False

    def _copyfrom( self, name, minimal=True ):
        self._close()
        abspath = os.path.abspath( os.path.expandvars( name ) )
        tbloc = gentools( ['tb'] )[0]
        if not minimal or abspath != self.plaintable:
            tbloc.open( os.path.join(name,'RO') )
            self.tb1 = tbloc.copy( self.memtable1, deep=True,
                                   valuecopy=True, memorytable=True,
                                   returnobject=True )
            tbloc.close()
        tbloc.open( os.path.join(name,'RW') )
        self.tb2 = tbloc.copy( self.memtable2, deep=True,
                              valuecopy=True, memorytable=True,
                              returnobject=True )

        tbloc.close()
        del tbloc
        self.isopened = True
        self.nrow = self.tb1.nrows()

class RODataTableColumn( object ):
    def __init__( self, table, name ):
        self.tb = table
        self.name = name

    def getcell( self, idx ):
        return cast_type_get(self.tb.getcell(self.name, idx))

    def getcol( self, startrow=0, nrow=-1, rowincr=1 ):
        return cast_type_get(self.tb.getcol(self.name, startrow, nrow, rowincr))

    def putcell( self, idx, val ):
        self.__raise()

    def putcol( self, val ):
        self.__raise()

    def __raise( self ):
        raise NotImplementedError( 'column %s is read-only'%(self.name) )
    
class RWDataTableColumn( RODataTableColumn ):
    def __init__( self, table, name ):
        super(RWDataTableColumn,self).__init__(table, name)

    def putcell( self, idx, val ):
        self.tb.putcell(self.name,int(idx),cast_type_put(val))

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        self.tb.putcol(self.name,cast_type_put(val),int(startrow),int(nrow),int(rowincr))
        
class DataTableColumnNoChange( RWDataTableColumn ):
    def __init__( self, table ):
        super(RWDataTableColumn,self).__init__(table, "NOCHANGE")

    def getcell( self, idx ):
        v = self.tb.getcell(self.name, int(idx))
        if v < 0:
            return False
        else:
            return cast_type_get(v)

    def getcol( self, startrow=0, nrow=-1, rowincr=1 ):
        if nrow < 0:
            nrow = self.tb.nrows()
        ret = [0 for i in xrange(startrow,nrow,rowincr)]
        idx = 0
        for i in xrange(startrow,nrow,rowincr):
            tNOCHANGE = self.tb.getcell(self.name,i)
            ret[idx] = False if tNOCHANGE < 0 else tNOCHANGE
            idx += 1
        return ret

    def putcell( self, idx, val ):
        if type(val)==bool:
            v = -1
        else:
            v = val
        self.tb.putcell(self.name,int(idx),cast_type_put(v))

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        if nrow < 0:
            nrow = min(startrow+len(val)*rowincr,self.tb.nrows())
        idx = 0
        for i in xrange(startrow,nrow,rowincr):
            self.putcell(i,val[idx])
            idx += 1

class DataTableColumnMaskList( RWDataTableColumn ):
    def __init__( self, table ):
        super(RWDataTableColumn,self).__init__(table, "MASKLIST")

    def getcell( self, idx ):
        v = self.tb.getcell(self.name, int(idx))
        if len(v)==1 and v[0][0]==0 and v[0][1]==0:
            return []
        else:
            return cast_type_get(v)

    def getcol( self, startrow=0, nrow=-1, rowincr=1 ):
        """
        Note: returned array has shape (nrow,nmask), in  
              contrast to (nmask,nrow) for return value of 
              tb.getcol().
        """
        if nrow < 0:
            nrow = self.tb.nrows()
        ret = [[] for i in xrange(startrow,nrow,rowincr)]
        idx=0
        for i in xrange(startrow,nrow,rowincr):
            tMASKLIST = list(self.getcell(self.name,i))
            if len(tMASKLIST)==1 and tMASKLIST[0][0]==0 and \
                   tMASKLIST[0][1]==0:
                ret[idx] = tMASKLIST
            idx += 1
        return ret

    def putcell( self, idx, val ):
        if len(val)==0:
            v = [[0,0]]
        else:
            v = val
        self.tb.putcell(self.name,int(idx),cast_type_put(v))

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        """
        Note: input array should have shape (nrow,nmask), in  
              contrast to (nmask,nrow) for tb.putcol()
        """
        if nrow < 0:
            nrow = min(startrow+len(val)*rowincr,self.tb.nrows())
        idx = 0
        for i in xrange(startrow,nrow,rowincr):
            self.putcell(i,val[idx])
            idx += 1

class DataTableRowImpl( object ):
    def __init__( self, table, irow=0 ):
        """
        table -- DataTableImpl instance
        irow -- row index
        """
        #self.tb = table.stb
        #self.tb = table.tb
        self.tb = table
        #print 'DataTableRowImpl:', self.tb.nrows()
        self.irow = 0
        self.moveto( irow )
        self.ncol = len(self.tb.colnames())

    def __getitem__( self, idx ):
        return self.getbyindex( idx )

    def __setitem__( self, idx, val ):
        self.setbyindex( idx, val )

    def moveto( self, irow ):
        self.irow = irow
        if self.irow >= self.tb.nrows():
            raise IndexError('row index out of range: nrow=%s, irow=%s'%(self.tb.nrows(),self.irow))
        
    def colname( self, idx ):
        """
        idx -- column index
        """
        if idx >= self.ncol:
            raise IndexError('column index out of range')
        return TD_NAME[idx]

    def getbyindex( self, idx ):
        """
        idx -- column index
        """
        col = self.colname( idx )
        return self.getbyname( col )

    def getbyname( self, col ):
        """
        col -- column name
        """
        #print 'RowImpl.getbyname: irow=%s'%(self.irow)
        return cast_type_get(self.tb.getcell( col, self.irow ))

    def setbyindex( self, idx, val ):
        """
        idx -- column index
        """
        col = self.colname( idx )
        self.setbyname( col, val )

    def setbyname( self, name, val ):
        """
        name -- column name
        """
        self.tb.putcell( name, self.irow, val )

    def set( self, vals ):
        """
        vals -- list of values
        """
        for icol in xrange(self.ncol):
            self.setbyname( TD_NAME[icol], vals[icol] )

class DataTableAdapter( DataTableImpl ):
    def __init__( self, nrow=0 ):
        super(DataTableAdapter,self).__init__( nrow )
        self.row = None
        self.irow = 0
        #print 'DataTableAdapter:',self.tb.nrows()

    def __getitem__( self, key ):
        if type(key) is not str:
            return self.getrow( key )
        elif key == 'FileName':
            return self.getkeyword( key )
        else:
            raise KeyError( 'keyword %s not found'%(key) )

    def __setitem__( self, key, val ):
        if type(key) is not str:
            self.setrow( key, val )
        else:
            self.putkeyword( key, val )

    def __len__( self ):
        return (self.tb.nrows()+len(self.tb.getkeywords()))

    def has_key( self, key ):
        return (key in self.tb.keywordnames())

    def appendrow( self, vals ):
        """
        vals -- list of values
        """
        self.addrows( 1 )
        self.setrow( self.nrow-1, vals )

    def addrows( self, nrow ):
        """
        nrow -- number of rows to be added
        """
        super(DataTableAdapter,self).addrows( nrow )
        if self.nrow == 0:
            #print 'empty table, create row'
            self.row = DataTableRowImpl( self, 0 )
            self.irow = 0
        self.nrow = self.nrows()

    def getrow( self, irow ):
        """
        irow -- row index
        """
        if self.row is None:
            self.row = DataTableRowImpl( self, irow )
            self.irow = irow
        if irow != self.irow:
            self.row.moveto( irow )
            self.irow = irow
        return self.row

    def setrow( self, irow, val ):
        """
        irow -- row index
        val -- list of values
        """
        #print 'irow=%s'%(irow)
        row = self.getrow( irow )
        row.set( val )

class DataTableDictHandler( DataTableAdapter ):
    key = 'FileName'
    def __init__( self, table={} ):
        """
        table -- DataTable
        """
        nrow = len(table)
        if table.has_key(self.key):
            nrow -= 1
        super(DataTableDictHandler,self).__init__( nrow )
        self.table = table
        if self.nrow > 0:
            self.ascasatable()

    def importdata( self, name ):
        """
        name -- name of DataTable to be imported
        """
        super(DataTableDictHandler,self).importdata( name )

        # create dictionary
        self.asdictionary()

    def exportdata( self, name, overwrite=False ):
        """
        name -- name of exported DataTable
        overwrite -- overwrite existing DataTable
        """
        # DataTable -> CASA table
        self.ascasatable()

        # save to disk
        super(DataTableDictHandler,self).exportdata( name, overwrite )
        
    def asdictionary( self ):
        self.table.clear()
        self.nrow = self.tb.nrows()
        colnames = self.tb.colnames()
        #colnames.pop(colnames.index('ID'))
        #row = [None] * (len(TD_NAME)-1)
        row = [None] * len(TD_NAME)
        for irow in xrange(self.nrow):
            #print 'processing row',irow
            #idx = self.tb.getcell('ID',irow)
            idx = irow
            #print 'idx =',idx
            for col in colnames:
                #print 'processing column', col
                row[TD_INDEX[col]] = self.tb.getcell(col,irow)
            self.table[idx] = row
        if self.key in self.tb.keywordnames():
            self.table[self.key] = self.getkeyword( self.key )
            
    def ascasatable( self ):
        #print 'tabledesc=',TABLEDESC
        self._create()
        colnames = self.tb.colnames()
        #colnames.pop(colnames.index('ID'))
        colnames.pop(colnames.index('MASKLIST'))
        #print 'colnames=',colnames
        if self.table.has_key( self.key ):
            self.putkeyword( self.key, self.table[self.key] )
        for irow in xrange(self.nrow):
            #print 'processing row',irow
            row = self.table[irow]
            # store ID
            #self.tb.putcell('ID',irow,irow)
            # store data except MASKLIST
            for col in colnames:
                #print 'processing column',col
                val=row[TD_INDEX[col]]
                #print 'value is %s (type %s)'%(val,type(val))
                self.putcell(col,irow,val)
            # special care is needed for MASKLIST
            col='MASKLIST'
            val=row[TD_INDEX[col]]
            #print 'value is',val
            if len(val)==0:
                self.putcell(col,irow,[[0,0]])    


def testexport():
    dummytable = {
        'FileName': ['blabla','bob'],
        0: [0,0,0,0,0,'today',1.0,-1.0,0.5,3.14,-3.14,1.2,2.1,3840,420.0,'earth',[-1.0,-1.0],[1,1],[1,1,1],1,0,[],False,0],
        1: [1,0,0,1,0,'today',1.0,-1.0,0.5,3.14,-3.14,1.2,2.1,3840,420.0,'earth',[-1.0,-1.0],[1,1],[1,1,1],1,0,[],False,0]
        }
    DataTable = DataTableDictHandler( dummytable )
    DataTable.exportdata( name='test.tbl', overwrite=True )
    return DataTable

def testimport():
    DataTable = DataTableDictHandler()
    DataTable.importdata( name='test.tbl' )
    return DataTable

def testrow():
    t = DataTableAdapter()
    t.importdata('test.tbl')
    print 'nrow=%s'%(t.nrow)
    r0 = t[0]
    print 't[0][0]=%s'%(r0[0])
    print 't[0][0]=%s'%(t[0][0])
    print 't[1][0]=%s'%(t[1][0])
    t[0][0]=100
    print 't[\'FileName\']=%s'%(t['FileName'])
    dummydata = [5,2,3,8,6,'today',1.0,-1.0,0.5,3.14,-3.14,1.2,2.1,3840,420.0,'earth',[-1.0,-1.0],[1,1],[1,1,1],1,0,[[0,900],[450,1324]],False,0]
    t[0]=dummydata
    t.appendrow( dummydata )
    t.exportdata('test2.tbl',overwrite=True)
