###
# createTaskList.py
#
# This module creates HTML summary of currently available tasks.
# The summary is based on XML files in <modules>/cli where 
# <modules> is h, hif, hifa, hifv, and hsd.
#
# USAGE:
#    import pipeline.infrastructure.docutils.createTaskList as tu
#    tu.create3()
#
#    It will create directory named 'tasklist' on current directory.
#    tasklist/index.html is top page for the task list.
#
###
import os
import commands
import time
import string
import re

class taskutil( object ):
    def __init__( self, dirname='', title='' ):
        self.taskdir=dirname
        self.title=title
        self.tasks=[]
        
    def createtasklist( self ):
        s=commands.getoutput('ls '+self.taskdir+'/*.xml')
        tasks=s.split('\n')
        for i in xrange(len(tasks)):
            tasks[i]=tasks[i].split('/')[-1].replace('.xml','')
        self.tasks=tasks
        
    def gettasklist( self ):
        return self.tasks

    def _replacelfcode( self, desc ):
        return desc.replace('\n','<BR>')

    def _replacews( self, desc ):
        br = '<BR>'
        ws = '&nbsp;'
        descs = desc.split( br )
        for i in xrange(len(descs)):
            #print 'before: ',descs[i]
            if len(descs[i]) == 0:
                continue
            pos = 0
            tmp=''
            while( descs[i][pos:].startswith(' ') ):
                tmp += ws
                pos += 1
            tmp += descs[i][pos:]
            descs[i]=tmp
            #print 'after: ',descs[i]
        return string.join( descs, br )

    def _processstring( self, desc ):
        # replace '\n' with '<BR>'
        desc = self._replacelfcode( desc )

        # replace ' ' with '&nbsp;' at the beggining of the line
        desc = self._replacews( desc )

        return desc

    def _getstring( self, node, tagname ):
        s=''
        childs=node.getElementsByTagName(tagname)
        if len(childs) > 0:
            child=childs[0]
            if child.hasChildNodes():
                s=child.lastChild.data.encode('UTF-8')
        return s

    def getshortdescription( self, task ):
        shortdesc='No description'
        if not ( task in self.tasks ):
            print task+' does not exist.'
            return shortdesc
        xmlfile=self.taskdir+'/'+task+'.xml'
        import xml.dom.minidom as DOM
        dom3=DOM.parse( xmlfile )
        rootnode=dom3.getElementsByTagName('task')[0]
        shortdesc=self._getstring( rootnode, 'shortdescription' )
        dom3.unlink()
        if len(shortdesc) == 0:
            shortdesc = 'No description'
        return shortdesc

    def gettaskdescription( self, task ):
        docstr = ''
        if not ( task in self.tasks ):
            print task+' does not exist.'
            return docstr
        xmlfile=self.taskdir+'/'+task+'.xml'
        import xml.dom.minidom as DOM
        dom3=DOM.parse( xmlfile )
        rootnode=dom3.getElementsByTagName('task')[0]
        shortdesc = self._getstring( rootnode, 'shortdescription' )
        example = self._getstring( rootnode, 'example' )
        dom3.unlink()
        if len(shortdesc) > 0:
            docstr += shortdesc + '<BR>'
        if len(example) > 0:
            docstr += example + '<BR>'
        return docstr

    def getparamlist( self, task ):
        #print 'task = ', task
        paramlist=[]
        if not ( task in self.tasks ):
            print task+' does not exist.'
            return paramlist
        xmlfile=self.taskdir+'/'+task+'.xml'
        import xml.dom.minidom as DOM
        dom3=DOM.parse( xmlfile )
        rootnode=dom3.getElementsByTagName('task')[0]
        inputs=rootnode.getElementsByTagName('input')[0]
        childs=inputs.getElementsByTagName('param')
        for i in xrange(len(childs)):
            attr=childs[i].attributes
            keys=attr.keys()
            namekey=keys.index('name')
            typekey=keys.index('type')
            values=attr.values()
            name=values[namekey].value.encode('UTF-8')
            #print '   ',name
            myproperty={}
            ptype=values[typekey].value.encode('UTF-8')
            defaults=childs[i].getElementsByTagName('value')[0]
            if defaults.hasChildNodes():
                if defaults.lastChild.hasChildNodes():
                    defaults=defaults.lastChild
                defaultvalue=defaults.lastChild.data.encode('UTF-8')
                if len(defaultvalue) == 0:
                    defaultvalue = 'None'
            else:
                defaultvalue='None'
            #descs=childs[i].getElementsByTagName('description')[0]
            descs=childs[i].getElementsByTagName('description')
            if len(descs) != 0:
                descs=descs[0]
                if descs.hasChildNodes():
                    description=descs.lastChild.data.encode('UTF-8')
                    if len(description) == 0:
                        description = 'None'
                else:
                    description = 'None'
            else:
                description = 'None'
            myproperty['2_default']=defaultvalue
            myproperty['1_type']=ptype
            myproperty['0_name']=name
            myproperty['3_description']=description
            paramlist.append(myproperty)
        dom3.unlink()
        return paramlist

    def createHTML( self, outdir='./tasklist' ):
        if not os.path.exists( outdir ):
            os.makedirs( outdir )
        htmlfile='tasklist.html'
        tasklist=self.gettasklist()
        f=open( outdir+'/'+htmlfile, 'w' )
        print >> f, '<HTML><BODY>'
        print >> f, '<H1>Summary of tasks and parameters</H1>'
        print >> f, '<HR>'
        for i in xrange(len(tasklist)):
            print >> f, '<H3><A NAME="%s">%s</A></H3>'%(tasklist[i],tasklist[i])
            print >> f, '<DL>'
            
            # task description
            #print tasklist[i], docstring
            print >> f, '<DT> <B>Task Description</B><BR>'
            print >> f, '<DD><BR>'
            docstring=self.gettaskdescription( tasklist[i] )
            ## replace ' ' with '&nbsp;' at the beggining of the each line
            docstring=self._processstring( docstring )
            print >> f, docstring
            ## alternative: use <PRE> tag
            #print >> f, '<PRE>'
            #print >> f, self._replacelfcode( docstring )
            #print >> f, '</PRE>'
            print >> f, '</DL><BR>'

            # parameter list
            paramlist=self.getparamlist( tasklist[i] )
            print >> f, '<DT> <B>Parameter List</B><BR>'
            print >> f, '<DD><P>'
            if len(paramlist) == 0:
                print >> f, 'No parameter<P>'
            else:
                print >> f, '<TABLE BORDER="1">'
                keys=paramlist[0].keys()
                keys.sort()
                line=''
                for j in xrange(len(keys)):
                    truekey=keys[j][2:]
                    line=line+'<TH ALIGN="left">%s</TH>'%truekey
                print >> f, '<TR>%s</TR>'%line
                for j in xrange(len(paramlist)):
                    line=''
                    for k in xrange(len(keys)):
                        line=line+'<TD ALIGN="left">%s</TD>'%paramlist[j][keys[k]]
                    print >> f, '<TR>%s</TR>'%line
                print >> f, '</TABLE><P>'

            print >> f, '<P ALIGN="left"><A HREF="../index.html">Back</A><P>'
            print >> f, '<HR>'
        #print >> f, '<HR>'
        print >> f, '<SMALL>'
        print >> f, 'Created at %s UTC<BR>'%(time.asctime(time.gmtime()))
        print >> f, 'This page is automatically generated from XML interface definition files.'
        print >> f, '</SMALL>'
        print >> f, '</BODY></HTML>'
        f.close()

class taskutil2( taskutil ):
    def __init__( self, dirname='', title='', prefix=None ):
        super(taskutil2,self).__init__( dirname=dirname, title=title )
        self.prefix = prefix

    def createtasklist( self ):
        s=commands.getoutput('ls '+self.taskdir+'/*.xml')
        tasks=s.split('\n')
        self.tasks = []
        for i in xrange(len(tasks)):
            task=os.path.basename(tasks[i]).replace('.xml','')
            #print 'task=',task
            if self.prefix is None or \
               re.match('%s_.*'%(self.prefix),task) is not None:
                #task[:len(self.prefix)] == self.prefix:
                #print 'added', task 
                self.tasks.append(task)
        

class hetaskutil( object ):
    #def __init__( self, dir='/home/nakazato/ALMA/PIPELINE/Heuristics/src/heuristics', sddir='SDPipelineTaskInterface', ifdir='SFIPipelineTaskInterface' ):
    def __init__( self, dirname ):
        self.phdir=dirname
        self.tasks={}
        self.addmodule('hif', 'Interferometry')
        self.addmodule('hsd', 'Single-Dish')
        
    @property
    def keys(self):
        keys = self.tasks.keys()[:]
        keys.sort()
        return keys    
    
    def addmodule(self, name, title, version=2):
        if version == 2:
            self.tasks[name] = taskutil2(dirname=os.path.join(self.phdir, name, 'cli'), title=title, prefix=name)
        else:
            self.tasks[name] = taskutil(dirname=os.path.join(self.phdir, name, 'cli'), title=title)
        
    def createtasklist( self ):
        for key in self.keys:
            self.tasks[key].createtasklist()

    def gettasklist( self ):
        tasklist={}
        for key in self.keys:
            tasklist[key]=self.tasks[key].gettasklist()
        return tasklist

    def getparamlist( self, task ):
        paramlist={}
        tasklist=self.gettasklist()
        target=None
        for key in self.keys:
            if task in tasklist[key]:
                target=key
        if target is None:
            print task+' does not exist.'
            return paramlist
        paramlist=self.tasks[target].getparamlist( task )
        return paramlist

    def createHTML( self, outdir='./tasklist' ):
        if not os.path.exists( outdir ):
            os.makedirs( outdir )
        tasklist=self.gettasklist()
        outdirs={}
        basedirs={}
        for key in self.keys:
            if str(key).lower() != 'common':
                basedirs[key]=str(key).lower()+'_tasklist'
            else:
                basedirs[key]='h_tasklist'
        for key in self.keys:
            print 'creating %s...'%(key.lower())
            outdirs[key]=os.path.join(outdir,basedirs[key])
            self.tasks[key].createHTML( outdir=outdirs[key] )
        htmlfile=outdir+'/index.html'
        f=open( htmlfile, 'w' )
        print >> f, '<HTML><BODY>'
        print >> f, '<H1>List of Heuristics Tasks</H1>'
        print >> f, '<HR>'
        for key in self.keys:
            print >> f, '<H3>%s</H3>'%self.tasks[key].title 
            print >> f, '%s tasks available.<BR><BR>'%len(tasklist[key])
            print >> f, '<TABLE>'
            print >> f, '<TR><TH ALIGN="left">task name</TH><TH ALIGN="left">description</TH></TR>'
            for i in xrange(len(tasklist[key])):
                print >> f, '<TR><TD><A HREF="./%s/tasklist.html#%s">%s</A></TD><TD>%s</TD></TR>'%(basedirs[key],tasklist[key][i],tasklist[key][i],self.tasks[key].getshortdescription(tasklist[key][i]))
            print >> f, '</TABLE>'
        print >> f, '<HR>'
        print >> f, '<SMALL>'
        print >> f, 'Created at %s UTC<BR>'%(time.asctime(time.gmtime()))
        print >> f, 'This page is automatically generated from XML interface definition files.'
        print >> f, '</SMALL>'
        print >> f, '</BODY></HTML>'
        f.close()

class hetaskutil2( hetaskutil ):
    def __init__( self, dirname='', cli='' ):
        super(hetaskutil2,self).__init__(dirname=dirname)
        self.addmodule('h', 'Common')

class hetaskutil3( hetaskutil ):
    def __init__( self, dirname='', cli='' ):
        super(hetaskutil3,self).__init__(dirname=dirname)
        self.addmodule('h', 'Common')
        self.addmodule('hifa', 'Interferometry ALMA')
        self.addmodule('hifv', 'Interferometry EVLA')
        self.addmodule('hif', 'Interferometry Common')

def create( dirname='/home/nakazato/ALMA/PIPELINE/Heuristics/src/heuristics' ):
    het=hetaskutil( dirname=dirname )
    het.createtasklist()
    het.createHTML()

def create2( dirname=None ):
    if dirname is None:
        # assume that this file is located at pipeline/infrastructure/docutils
        dirname = '/' + os.path.join(*(__file__.split('/')[:-3]))
    print 'pipeline directory is %s'%(dirname)
    het=hetaskutil2(dirname=dirname)
    het.createtasklist()
    het.createHTML()

def create3(dirname=None):
    if dirname is None:
        # assume that this file is located at pipeline/infrastructure/docutils
        dirname = '/' + os.path.join(*(__file__.split('/')[:-3]))
    print 'pipeline directory is %s'%(dirname)
    het=hetaskutil3(dirname=dirname)
    het.createtasklist()
    het.createHTML()
