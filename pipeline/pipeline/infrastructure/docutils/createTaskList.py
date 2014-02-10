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
    def __init__( self, dir='' ):
        self.taskdir=dir
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
            property={}
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
            property['2_default']=defaultvalue
            property['1_type']=ptype
            property['0_name']=name
            property['3_description']=description
            paramlist.append(property)
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
    def __init__( self, dir='', prefix=None ):
        super(taskutil2,self).__init__( dir=dir )
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
    def __init__( self, dir='/home/nakazato/ALMA/PIPELINE/Heuristics/src/heuristics', sddir='hsd/TaskInterface', ifdir='hif/TaskInterface' ):
        self.phdir=dir
        self.taskdir={}
        self.keys = ['IF','SD']
        self.taskdir['IF']=self.phdir+'/'+ifdir
        self.taskdir['SD']=self.phdir+'/'+sddir
        self.tasks={}
        self.tasks['IF']=taskutil( self.taskdir['IF'] )
        self.tasks['SD']=taskutil( self.taskdir['SD'] )
        self.titles={}
        self.titles['IF']='Interferometry'
        self.titles['SD']='Single-Dish'
        
    def createtasklist( self ):
        for key in self.keys:
            self.tasks[key].createtasklist()

    def gettasklist( self ):
        tasklist={}
        for key in self.keys:
            tasklist[key]=self.tasks[key].gettasklist()
        return tasklist

    def getparamlist( self, task ):
        obj=''
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
                basedirs[key]='h'+str(key).lower()+'_tasklist'
            else:
                basedirs[key]='h_tasklist'
        for key in self.keys:
            #print 'creating h%s...'%(key.lower())
            outdirs[key]=os.path.join(outdir,basedirs[key])
            self.tasks[key].createHTML( outdir=outdirs[key] )
        htmlfile=outdir+'/index.html'
        f=open( htmlfile, 'w' )
        print >> f, '<HTML><BODY>'
        print >> f, '<H1>List of Heuristics Tasks</H1>'
        print >> f, '<HR>'
        for key in self.keys:
            print >> f, '<H3>%s</H3>'%self.titles[key]
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
    def __init__( self, dir='', cli='' ):
        super(hetaskutil2,self).__init__(dir=dir,ifdir=cli,sddir=cli)
        self.taskdir['COMMON']=self.phdir+'/'+cli
        self.titles['COMMON']='Common'
        self.tasks['COMMON']=taskutil2( self.taskdir['COMMON'], prefix='h' )
        self.tasks['IF']=taskutil2( self.taskdir['IF'], prefix='hif' )
        self.tasks['SD']=taskutil2( self.taskdir['SD'], prefix='hsd' )
        self.keys=['COMMON']+self.keys

class hetaskutil3( hetaskutil ):
    def __init__( self, dir='', cli='' ):
        super(hetaskutil3,self).__init__(dir=dir,ifdir='hif/cli',sddir='hsd/cli')
        self.taskdir['COMMON']=os.path.join(self.phdir, 'h', cli)
        self.taskdir['IFA']=os.path.join(self.phdir, 'hifa', cli)
        self.taskdir['IFV']=os.path.join(self.phdir, 'hifv', cli)
        self.titles['COMMON']='Common'
        self.titles['IFA']='Interferometry ALMA'
        self.titles['IFV']='Interferometry EVLA'
        self.titles['IF']='Interferometry Common'
        self.tasks['COMMON']=taskutil2( self.taskdir['COMMON'], prefix='h' )
        self.tasks['IF']=taskutil2( self.taskdir['IF'], prefix='hif' )
        self.tasks['IFA']=taskutil2( self.taskdir['IFA'], prefix='hifa' )
        self.tasks['IFV']=taskutil2( self.taskdir['IFV'], prefix='hifv' )
        self.tasks['SD']=taskutil2( self.taskdir['SD'], prefix='hsd' )
        self.keys.extend(['COMMON', 'IFA', 'IFV'])
        self.keys.sort()

def create( dirname='/home/nakazato/ALMA/PIPELINE/Heuristics/src/heuristics' ):
    het=hetaskutil( dir=dirname )
    het.createtasklist()
    tasklist=het.gettasklist()
    het.createHTML()

def create2( dirname='' ):
    het=hetaskutil2( dir=dirname, cli='cli' )
    het.createtasklist()
    tasklist=het.gettasklist()
    het.createHTML()

def create3(dirname=None):
    print __file__
    if dirname is None:
        # assume that this file is located at pipeline/infrastructure/docutils
        dirname = '/' + os.path.join(*(__file__.split('/')[:-3]))
    print 'pipeline directory is %s'%(dirname)
    het=hetaskutil3( dir=dirname, cli='cli' )
    het.createtasklist()
    tasklist=het.gettasklist()
    het.createHTML()
