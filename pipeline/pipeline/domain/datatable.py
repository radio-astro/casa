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
# $Revision: 1.1.2.4 $
# $Date: 2012/12/04 14:51:19 $
# $Author: tnakazat $
#

from taskinit import gentools
import os
import re
import string
import time
import numpy

import pipeline.infrastructure.logging as logging
LOG = logging.get_logger(__name__)

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
    'NMASK', 'MASKLIST', 'NOCHANGE', 'ANTENNA', 'SRCTYPE',
    'POSGRP', 'TIMEGRP_S', 'TIMEGRP_L'
    ]

# Description for data table columns as dictionary.
# Each value is a tuple containing:
#
#    (valueType,option,ndim,maxlen,comment[,unit])
#
# dataManagerGroup and dataManagerType is always 'StandardStMan'.
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
    __coldesc('integer', 0, 0, -1, 'Antenna index'),
    __coldesc('integer', 0, 0, -1, 'Source type enum')
    ]

TD_DESC_RW = [
    __coldesc('double',  0, 0,  1, 'Statistics'),
    __coldesc('integer', 0, 0,  1, 'Flgas'),
    __coldesc('integer', 0, 0,  1, 'Permanent flags'),
    __coldesc('integer', 0, 0, -1, 'Actual flag'),
    __coldesc('integer', 0, 0, -1, 'Number of mask regions'),
    __coldesc('integer', 0, 0,  2, 'List of mask ranges'),
    __coldesc('integer', 0, 0, -1, 'Unchanged row or not'),
    __coldesc('integer', 0, 0, -1, 'Position group id'),
    __coldesc('integer', 0, 0, -1, 'Time group id for large gap'),
    __coldesc('integer', 0, 0, -1, 'Time group id for small gap')
    ]

def __tabledescro():
    name = [
        'ROW', 'SCAN', 'IF', 'POL', 'BEAM', 'DATE',
        'TIME', 'ELAPSED', 'EXPOSURE', 'RA', 'DEC',
        'AZ', 'EL', 'NCHAN', 'TSYS', 'TARGET', 'ANTENNA',
        'SRCTYPE'
        ]
    return dict( zip(name,TD_DESC_RO) )

def __tabledescrw():
    name = [
        'STATISTICS', 'FLAG', 'FLAG_PERMANENT',
        'FLAG_SUMMARY', 'NMASK', 'MASKLIST', 'NOCHANGE',
        'POSGRP', 'TIMEGRP_S', 'TIMEGRP_L'
        ]
    return dict( zip(name,TD_DESC_RW) )

TABLEDESC_RO = __tabledescro()
TABLEDESC_RW = __tabledescrw()

def absolute_path(name):
    return os.path.abspath(os.path.expanduser(os.path.expandvars(name)))

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
    def __init__( self, name=None, nrow=0 ):
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
        self.isselected = False
        self.nrow = nrow
        if name is None or len(name) == 0:
            self._create()
        elif not os.path.exists(name):
            self._create()
            self.plaintable = absolute_path(name)
        else:
            self.importdata(name=name, minimal=False)

    def __del__( self ):
        # make sure that table is closed
        #print '__del__ close CASA table...'
        self.cols.clear()
        self._close()

    def __len__( self ):
        return self.tb1.nrows()

    def nrows( self ):
        return self.tb1.nrows()

    def has_key(self, name):
        return (name in self.tb2.keywordnames())

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
        self.tb2.putkeyword( name, val )

    def getkeyword( self, name ):
        """
        name -- keyword name
        """
        return self.tb2.getkeyword( name )

    def importdata( self, name, minimal=True ):
        """
        name -- name of DataTable to be imported
        """
        print 'Importing DataTable from %s...'%(name)

        # copy input table to memory
        self._copyfrom( name, minimal )
        self.plaintable = absolute_path(name)
        self._initcols2()

    def exportdata( self, name=None, minimal=True, overwrite=False ):
        """
        name -- name of exported DataTable
        overwrite -- overwrite existing DataTable
        """
        if name is None or len(name) == 0:
            if len(self.plaintable) == 0:
                raise IOError('You have to specify name of export table')
            else:
                name = self.plaintable
                overwrite = True
        
        print 'Exporting DataTable to %s...'%(name)
        # overwrite check
        abspath = absolute_path(name)
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
            'ANTENNA': RWDataTableColumn(self.tb1,'ANTENNA'),
            'SRCTYPE': RWDataTableColumn(self.tb1,'SRCTYPE'),
            'POSGRP': RWDataTableColumn(self.tb2, 'POSGRP'),
            'TIMEGRP_S': RWDataTableColumn(self.tb2, 'TIMEGRP_S'),
            'TIMEGRP_L': RWDataTableColumn(self.tb2, 'TIMEGRP_L')
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
            'ANTENNA': RODataTableColumn(self.tb1,'ANTENNA'),
            'SRCTYPE': RODataTableColumn(self.tb1,'SRCTYPE'),
            'POSGRP': RWDataTableColumn(self.tb2, 'POSGRP'),
            'TIMEGRP_S': RWDataTableColumn(self.tb2, 'TIMEGRP_S'),
            'TIMEGRP_L': RWDataTableColumn(self.tb2, 'TIMEGRP_L')
            }

    def _close( self ):
        if self.isopened:
            self.tb1.close()
            self.tb2.close()
            self.isopened = False

    def _copyfrom( self, name, minimal=True ):
        self._close()
        abspath = absolute_path(name)
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

    def get_posdict(self, ant, spw, pol):
        posgrp_list = self.getkeyword('POSGRP_LIST')
        try:
            mygrp = posgrp_list[str(ant)][str(spw)][str(pol)]
        except KeyError, e:
            raise KeyError('ant %s spw %s pol %s not in reduction group list'%(ant,spw,pol))
        except Exception, e:
            raise e
        
        posgrp_rep = self.getkeyword('POSGRP_REP')
        rows = self.getcol('ROW')
        posgrp = self.getcol('POSGRP')
        posdict = {}
        for (k,v) in posgrp_rep.items():
            if int(k) not in mygrp:
                continue
            key = rows[v]
            posdict[key] = [[],[]]
            
        for idx in xrange(len(posgrp)):
            grp = posgrp[idx]
            if grp not in mygrp:
                continue
            row = rows[idx]
            rep = posgrp_rep[str(grp)]
            key = rows[rep]
            posdict[key][0].append(row)
            posdict[key][1].append(idx)
            if row != key:
                posdict[row] = [[-1,key],[rep]]

        return posdict

    def get_timetable(self, ant, spw, pol):
        timegrp_list = self.getkeyword('TIMEGRP_LIST')
        try:
            mygrp = timegrp_list[str(ant)][str(spw)][str(pol)]
        except KeyError, e:
            raise KeyError('ant %s spw %s pol %s not in reduction group list'%(ant,spw,pol))
        except Exception, e:
            raise e

        mygrp_s = mygrp['small']
        mygrp_l = mygrp['large']
        timegrp_s = self.getcol('TIMEGRP_S')
        timegrp_l = self.getcol('TIMEGRP_L')
        rows = self.getcol('ROW')

        timetable = [[],[]]
        timedic_s = {}
        timedic_l = {}
        for idx in xrange(len(rows)):
            grp_s = timegrp_s[idx]
            if grp_s not in mygrp_s:
                continue
            if not timedic_s.has_key(grp_s):
                timedic_s[grp_s] = [[],[]]
            timedic_s[grp_s][0].append(rows[idx])
            timedic_s[grp_s][1].append(idx)
        for idx in xrange(len(rows)):
            grp_l = timegrp_l[idx]
            if grp_l not in mygrp_l:
                continue
            if not timedic_l.has_key(grp_l):
                timedic_l[grp_l] = [[],[]]
            timedic_l[grp_l][0].append(rows[idx])
            timedic_l[grp_l][1].append(idx)
        for idx in xrange(len(mygrp_s)):
            grp = timedic_s[mygrp_s[idx]]
            timetable[0].append(grp)
        for idx in xrange(len(mygrp_l)):
            grp = timedic_l[mygrp_l[idx]]
            timetable[1].append(grp)

        return timetable
            
    def get_timegap(self, ant, spw, pol):
        timegap_s = self.getkeyword('TIMEGAP_S')
        timegap_l = self.getkeyword('TIMEGAP_L')
        try:
            mygap_s = timegap_s[str(ant)][str(spw)][str(pol)]
            mygap_l = timegap_l[str(ant)][str(spw)][str(pol)]
        except KeyError, e:
            raise KeyError('ant %s spw %s pol %s not in reduction group list'%(ant,spw,pol))
        except Exception, e:
            raise e

        rows = self.getcol('ROW')
        timegap = [[],[]]
        for idx in mygap_s:
            timegap[0].append(rows[idx])
        for idx in mygap_l:
            timegap[1].append(rows[idx])

        return timegap
    

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

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
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
