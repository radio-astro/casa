#*******************************************************************************
# ALMA - Atacama Large Millimeter Array
# Copyright (c) NAOJ - National Astronomical Observatory of Japan, 2011
# (in the framework of the ALMA collaboration).
# All rights reserved.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#*******************************************************************************
#
# $Revision: 1.14.2.2.2.3 $
# $Date: 2012/09/03 19:05:14 $
# $Author: tnakazat $
#
import numpy
import os
import string
#import casac
from SDTool import is_scantable, is_ms
import asap as sd
from taskinit import gentools

# product identifier
def productIdentifier():
    identifier = 'product'
    postfix = 'tbl'
    return string.join([identifier,postfix],'.')

class SDDataProductNameList:
    def __init__( self, rawdir, rawfile, workdir=None, format=None, source=None, antenna=None ):
        """
        constructor
        """
        self.dir = workdir
        files = rawfile
        if type(files) == str:
            files = [files]
        self.nelem = len(files)
        self.objlist = [-1 for i in xrange(self.nelem)]
        for i in xrange(self.nelem):
            path = rawdir + '/' + files[i]
            self.objlist[i] = SDDataProductName( path, workdir, format, source, antenna )

    def __getitem__( self, idx ):
        return self.getProductName( idx )

    def getProductName( self, idx ):
        """
        get SDDataProductName instance
        """
        return self.objlist[idx]

    def getGlobalHtmlName( self, dataid=None ):
        """
        get data product name for root directory of HTML front page.

        dataid
        """
        # In context class, default root directory name
        # for weblog is 'html'
        # At the moment, single dish web log is located at
        # subdirectory 'html/sd'
        #return self.dir + '/result_html'
        return self.dir + '/html/sd'

    def setFormat( self, dataformat ):
        """
        set data format for all instances
        """
        if type(dataformat) == str:
            for obj in self.objlist:
                obj.setFormat( dataformat.lower() )
        else:
            if len(dataformat) != self.nelem:
                raise Exception('SDDataProductNameList.setFormat: length differ')
            else:
                for i in xrange(self.nelem):
                    self.objlist[i].setFormat( dataformat[i].lower() )

    def setSource( self ):
        """
        set source
        """
        for obj in self.objlist:
            obj.setSource()
            
            
        
class SDDataProductName:
    def __init__( self, rawfile, workdir=None, format=None, source=None, antenna=None ):
        """
        constructor

        rawfile  -- filename. full path is recommended.
        workdir  -- working directory (optional)
        format   -- data format (optional)
                    options: 'asap','asdm','ms2','fits'
                    default: None (automatically set)
        source   -- source name (optional)
                    default: None (retrieve from data)
                    it is safe to set source name explicitly
                    since the data can include several sources.
        antenna  -- list of antennas available (optional)
        """
        # dictionary for polarization mapping
        self.polMap = { 'stokes':   {0:'I',  1:'Q',  2:'U',  3:'V'},
                        'linear':   {0:'XX', 1:'YY', 2:'XY', 3:'YX'},
                        'circular': {0:'RR', 1:'LL', 2:'RL', 3:'LR'} }
        
        # local tool
        #self._tbtool = casac.homefinder.find_home_by_name('tableHome')
        #self._tb = self._tbtool.create()
        self._tb = gentools(['tb'])[0]
        
        abspath=os.path.expanduser( os.path.expandvars( rawfile ) )
        if abspath.find('/') != -1:
            pieces = abspath.split( '/' )
            self.dirname = string.join( pieces[:-1], '/' )
            self.filename = pieces[-1]
        else:
            self.dirname = './'
            self.filename = abspath
        self.workdir = workdir
        if self.workdir is None:
            self.workdir = self.dirname
        pieces = self.filename.split('.')
        if len(pieces) == 1:
            self.prefix = pieces[0]
        else:
            self.prefix = string.join( pieces[:-1], '.' )
        self.tablename = None
        self.loc = None
        self.tabletype = format
        self.source = None
        if source is not None:
            self.source = source.strip().replace(' ','_')
        self.antennas = antenna
        self.spwids = []
        self.polids = []

        # check file format of self.filename if not specified
        self._setTableName()
        if self.tabletype is not None:
            self._setLocation()

        # extract antenna names
        if self.tabletype is not None and self.antennas is None:
            self._extractAntennaName()

    def __del__(self):
        del self._tb
        #del self._tbtool

    def getImagePrefix( self, ant, spwid, poltype, **kwargs ):
        """
        get data product name for (moment) image.

        ant
        spwid
        poltype
        polids
        cont
        """
        # check if antenna id is valid
        antenna = self.__antennaName( ant, 'getImagePrefix()' )
        
        # optional keyward arguments
        continuum = False 
        if 'cont' in kwargs.keys():
            if kwargs['cont']: continuum = True

        polids = kwargs['polids'] if kwargs.has_key('polids') else None
        polspec = self.__polspec(poltype, polids)

        # construct name
        # <source spec>.<antenna spec>.<spw spec>.<poltype>.<type>.<ex>
        # <source spec>.<antenna spec>.<line spec>.<poltype>.<type>.<ex>
        productname = string.join([self.source,antenna,'spw%s'%(spwid),polspec,'sd'], '.')

        productname = self.workdir + productname

        return productname

    
    def getImageName( self, ant, poltype, **kwargs ):
        """
        get data product name for (moment) image.

        ant
        poltype
        polids
        spwid
        spwtype
        cont
        line
        linetype
        lineid
        moment
        mtype
        fits
        """
        # check if antenna id is valid
        antenna = self.__antennaName( ant, 'getImageName()' )
        
        # optional keyward arguments
        lineimage = False
        winspec = 'line'
        if 'line' in kwargs.keys():
            if kwargs['line']: lineimage = True
        if lineimage:
            if 'linetype' in kwargs.keys():
                winspec = kwargs['linetype']
            elif 'lineid' in kwargs.keys():
                winspec = 'line%s'%kwargs['lineid']
        else:
            if 'spwid' in kwargs.keys():
                winspec = 'spw%s'%kwargs['spwid']
            elif 'spwtype' in kwargs.keys():
                winspec = kwargs['spwtype']
            else:
                winspec = 'spw'
        
        continuum = False 
        if 'cont' in kwargs.keys():
            if kwargs['cont']: continuum = True

        momentimage = False
        momenttype = 'moment0'
        if 'moment' in kwargs.keys():
            if kwargs['moment']: momentimage = True
            if 'mtype' in kwargs.keys():
                momenttype = kwargs['mtype']

        fitsimage = False
        if 'fits' in kwargs.keys():
            if kwargs['fits']: fitsimage = True

        polids = kwargs['polids'] if kwargs.has_key('polids') else None
        polspec = self.__polspec(poltype, polids)

        # construct name
        # <source spec>.<antenna spec>.<spw spec>.<poltype>.<type>.<ex>
        # <source spec>.<antenna spec>.<line spec>.<poltype>.<type>.<ex>
        elems = [self.source]
        if antenna != 'COMBINE': elems.append( antenna )
        elems += [winspec, polspec, 'sd']
        if continuum: elems.append( 'cont' )
        if momentimage: elems.append( momenttype )
        if fitsimage: elems.append( 'fits' )
        else: elems.append( 'im' )
        productname = string.join( elems, '.' )

        productname = self.workdir + productname

        return productname
        

    def getDataName( self, ant, baseline, **kwargs ):
        """
        get data product name for spectral data.

        ant
        baseline
        spwid
        polstr
        poltype
        polid
        cont
        """
        #print kwargs
        
        # check if antenna id is valid
        antenna = self.__antennaName( ant, 'getDataName()' )
        
        # optional keyward arguments
        continuum = False 
        if 'cont' in kwargs.keys():
            if kwargs['cont']: continuum = True
        
        spwid = 0
        if 'spwid' in kwargs.keys():
            spwid = kwargs['spwid']

        ptype = 'none'
        if 'polstr' in kwargs.keys():
            ptype = kwargs['polstr']
        else:
            poltype = 'stokes'
            if 'poltype' in kwargs.keys():
                poltype = kwargs['poltype']
            if 'polid' in kwargs.keys():
                ptype = self._getPolString( poltype, kwargs['polid'] )
        

        # construct name
        # <data spec>.<antenna spec>.<type>.ms
        # <data spec>.<antenna spec>.<spw spec>.<pol>.<type>.asap
        elems = [self.prefix]
        if antenna != 'COMBINE': elems.append( antenna )
        if baseline:
            elems += ['baselined', 'ms']
        else:
            elems.append( 'spw%s'%spwid )
            if continuum: elems.append( 'cont' )
            elems += [ptype, 'gridded', 'asap']
        productname = string.join( elems, '.' )

        productname = self.workdir + productname

        return productname


    def getGlobalHtmlName( self, dataid=None ):
        """
        get data product name for root directory of HTML front page.

        dataid
        """
        dataspec = dataid
        if dataspec is None:
            dataspec = self.prefix
        
        # construct name
        # <data spec>_html or
        # <data spec>.html
        productname = string.join([dataspec],'.')+'_html'

        productname = self.workdir + productname

        return productname
        

    def getLocalHtmlName( self, dataid=None, ant=0, relative=True):
        """
        get data product name for HTML output for each antenna.

        NOTE:
        It is intended to use in combination with getGlobalHtmlName()

        dataid
        ant
        relative
        """
        # check if antenna id is valid
        antenna = self.__antennaName( ant, 'getLocalHtmlName()' )

        dataspec = dataid
        if dataspec is None:
            dataspec = self.prefix
        
        # construct name
        # <data spec>_html/<antenna spec> or
        # <data spec>.html/<antenna spec>
        elems=[]
        if not relative:
            elems.append( self.getGlobalHtmlName( dataid ) )
        elems.append( self.filename.rstrip('/')+'.'+antenna )
        #elems.append( antenna )
        productname = string.join( elems, '/' )

        return productname

    def getHtmlName( self, dataid=None, ant=0):
        """
        get data product name for root directory of HTML output.

        dataid
        ant
        """
        # check if antenna id is valid
        antenna = self.__antennaName( ant, 'getHtmlName()' )

        dataspec = dataid
        if dataspec is None:
            dataspec = self.prefix
        
        # construct name
        # <data spec>.<antenna spec>_html or
        # <data spec>.<antenna spec>.html
        productname = string.join([dataspec,antenna], '.')+'_html'

        productname = self.workdir + productname

        return productname

    def getProductName( self, ant ):
        """
        """
        antenna = self.__antennaName( ant, "getProductName()" )
        #identifier = 'product'
        #postfix = 'tbl'
        identifier = productIdentifier()

        # construct name
        # <data spec>.<antenna spec>.<identifier>.tbl
        productname = string.join([self.prefix,antenna,identifier],'.')
        productname = os.path.join(self.workdir,productname)

        return productname

    def setFormat( self, format=None ):
        """
        set format

        if format is None, try to find from the data.
        """
        if format is None:
            self._checkTableType()
        else:
            self.tabletype = format
        self._setLocation()
        self._setTableName()
        if self.antennas is None:
            self._extractAntennaName()

    def setSource( self, name=None ):
        """
        set source name.

        if name is None, try to find from the data.
        """
        tname = self.loc + self.tablename
        if name is None:
            if self.tabletype == 'asap' or self.tabletype == 'fits':
                self._tb.open( tname )
                self.source = self._tb.getcell( 'SRCNAME', 0 )
                self._tb.close()
            elif self.tabletype == 'asdm' or self.tabletype == 'ms2':
                self._tb.open( tname )
                srctab = self._tb.getkeyword( 'SOURCE' ).lstrip('Table: ')
                self._tb.close()
                self._tb.open( srctab )
                self.source = self._tb.getcell( 'NAME', 0 )
                self._tb.close()
        else:
            self.source = name
        self.source = self.source.strip().replace(' ','_')
        
        
    def _checkTableType( self ):
        """
        """
        name = self.dirname + self.filename
        if os.path.isdir( name ):
            if is_ms( name ):
                self.tabletype = 'ms2'
                self.loc = self.dirname
            elif is_scantable( name ):
                self.tabletype = 'asap'
                self.loc = self.dirname
            elif os.path.exists( name+'/ASDM.xml' ):
                self.tabletype = 'asdm'
                self.loc = self.workdir
        elif re.search( '\.fits$', self.filename ) is not None:
            self.tabletype = 'fits'
            self.loc = self.workdir
        else:
            # set 'fits' at the moment
            self.tabletype = 'fits'
            self.loc = self.workdir

    def _setLocation( self ):
        """
        """
        self.loc = self.dirname
        if self.tabletype == 'asdm' or self.tabletype == 'fits':
            self.loc = self.workdir
        
    def _extractAntennaName( self ):
        """
        """
        name = self.loc + self.tablename
        if self.tabletype == 'asdm' or self.tabletype == 'ms2':
            self._tb.open( name )
            anttab = self._tb.getkeyword( 'ANTENNA' ).lstrip('Table: ')
            self._tb.close()
            self._tb.open( anttab )
            self.antennas = self._tb.getcol( 'NAME' )
            self._tb.close()
        elif self.tabletype == 'asap' or self.tabletype == 'fits':
            self.antennas = []
            s = sd.scantable( name, False )
            self.antennas.append( s.get_antennaname() )
            del s

        # initialize other attributes
        self.spwids = []
        self.polids = []
        for iant in xrange(len(self.antennas)):
            self.spwids.append( [] )
            self.polids.append( [] )
            
    def _setTableName( self ):
        self.tablename = self.filename
        if self.tabletype is not None:
            if self.tabletype == 'asdm':
                self.tablename = self.tablename.rstrip('/') + '.ms'
            elif self.tabletype == 'fits':
                self.tablename = self.tablename + '.asap'
            
            
    def _getPolString( self, type, id ):
        ptype = 'none'
        if self.polMap.has_key(type):
            if self.polMap[type].has_key(id):
                ptype = self.polMap[type][id]
        return ptype
        
    def _checkAntennaId( self, id ):
        ok = True
        if type(self.antennas) == dict:
            if id not in self.antennas.keys():
                ok = False
        else:
            if id < 0 or id >= len(self.antennas):
                ok = False
        return ok

    def __antennaName( self, v, origin='__antennaName' ):
        ret = v
        if type(v) is not str:
            if not self._checkAntennaId( v ):
                raise Exception( 'SDDataProductName.%s: Bad antenna id'%(origin) )
            ret = self.antennas[v]
        return ret

    def __polspec(self, poltype, polids=None):
        polspec = ''
        pols = []
        if poltype in self.polMap.keys():
            if polids is not None:
                if type(polids) == list: pols = polids
                else: pols = [polids]
            for idx in pols:
                polspec += self._getPolString( poltype, idx )
        else:
            polspec = poltype
        return polspec
