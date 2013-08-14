

class callibrary(object):

    def __init__(self):
        self.cld={}   # as dict

    def __repr__(self):
        return "<callibrary>"

    def cldefinstance(self):
        definst={
            "field" :"",
            "intent":"",
            "spw": "",
            "obs": "",
            "fldmap" : [],
            "obsmap" : [],
            "spwmap" : [],
            "antmap" : [],
            #"calwt" : False,
            "tinterp" : "",
            "finterp" : "",
            "reach" : ""
            }
        return definst


#    def addold(self,gaintable,gainfield,
#               interp,spwmap,calwt):
#
#        # TBD: handle gainfield='nearest'
#
#        self.add(caltable=gaintable,ctfield=gainfield,
#                 interp=interp,spwmap=spwmap,calwt=calwt)


    # 
    def add(self,caltable,
            field='',intent='',spw='',obs='',
            tinterp='',finterp='',reach='',calwt=True,
            obsmap=[],fldmap=[],spwmap=[],antmap=[]):

        # if caltable is a list, insist others are, too
        if (isinstance(caltable,list)):

            if (not isinstance(field,list)):
                field=[field]
            if (not isinstance(intent,list)):
                intent=[intent]
            if (not isinstance(spw,list)):
                spw=[spw]
            if (not isinstance(obs,list)):
                obs=[obs]

            if (not isinstance(tinterp,list)):
                tinterp=[tinterp]
            if (not isinstance(finterp,list)):
                finterp=[finterp]
            if (not isinstance(reach,list)):
                reach=[reach]
            if (not isinstance(calwt,list)):
                calwt=[calwt]

            if (len(obsmap)>0):
                if not isinstance(obsmap[0],list):
                    obsmap=[obsmap]  # nested list
            if (len(fldmap)>0):
                if not isinstance(fldmap[0],list):
                    fldmap=[fldmap]  # nested list
            if (len(spwmap)>0):
                if not isinstance(spwmap[0],list):
                    spwmap=[spwmap]  # nested list
            if (len(antmap)>0):
                if not isinstance(antmap[0],list):
                    antmap=[antmap]  # nested list
            
            igt=0
            for ct in caltable:
                self.parsetorec(caltable=ct,
                                field=field[igt] if (igt<len(field)) else "",
                                intent=intent[igt] if (igt<len(intent)) else "",
                                spw=spw[igt] if (igt<len(spw)) else "",
                                obs=obs[igt] if (igt<len(obs)) else "",
                                tinterp=tinterp[igt] if (igt<len(tinterp)) else "",
                                finterp=finterp[igt] if (igt<len(finterp)) else "",
                                reach=reach[igt] if (igt<len(reach)) else "",
                                calwt=calwt[igt] if (igt<len(calwt)) else calwt[(len(calwt)-1)],
                                obsmap=obsmap[igt] if (igt<len(obsmap)) else [],
                                fldmap=fldmap[igt] if (igt<len(fldmap)) else [],
                                spwmap=spwmap[igt] if (igt<len(spwmap)) else [],
                                antmap=antmap[igt] if (igt<len(antmap)) else [],
                                )
                igt+=1
        else:
            self.parsetorec(caltable=caltable,
                            field=field,intent=intent,spw=spw,obs=obs,
                            tinterp=tinterp,finterp=finterp,
                            reach=reach,calwt=calwt,
                            obsmap=obsmap,fldmap=fldmap,spwmap=spwmap,antmap=antmap)
            
    def parsetorec(self,caltable,
                   field='',intent='',spw='',obs='',
                   tinterp='',finterp='',reach='',calwt=True,
                   obsmap=[],fldmap=[],spwmap=[],antmap=[]):
        d0=self.cldefinstance()
        d0["field"]=field
        d0["intent"]=intent
        d0["spw"]=spw
        d0["obs"]=obs

        d0["obsmap"]=obsmap
        d0["fldmap"]=fldmap
        d0["spwmap"]=spwmap
        d0["antmap"]=antmap

        d0["tinterp"]=tinterp
        d0["finterp"]=finterp
        d0["reach"]=reach

        self.addrec({caltable : d0},calwt)

    def addrec(self,crec,calwt):

        ctname=crec.keys()[0]

        irec=0
        if (ctname in self.cld):
            # ctname exists, will add a new instance
            irec=len(self.cld[ctname])-1

            # prefer already-set calwt
            calwt0=self.cld[ctname]["calwt"]
            if calwt!=calwt0:
                print 'WARNING: For caltable=\''+ctname+'\' using already-set calwt='+str(calwt0)+'.'
        else:
            # ctname does not yet exist, add it
            self.cld[ctname] = {}
            self.cld[ctname]["calwt"]=calwt


        self.cld[ctname][str(irec)]=crec[ctname]

    def list(self):
        print 'There are '+str(len(self.cld))+' caltables in the cal library:'
        keys=self.cld.keys()
        keys.sort()
        for ct in keys:
            print ct+': calwt='+str(self.cld[ct]['calwt'])+str(' (')+str(len(self.cld[ct])-1)+str(' instance[s]):')
            for ims in self.cld[ct].keys():
                if (isinstance(self.cld[ct][ims],dict)):
                    print ' field=\''+str(self.cld[ct][ims]['field'])+'\'',
                    print ' intent=\''+str(self.cld[ct][ims]['intent'])+'\'',
                    print ' spw=\''+str(self.cld[ct][ims]['spw'])+'\'',
                    print ' obs=\''+str(self.cld[ct][ims]['obs'])+'\''
                    print '  tinterp=\''+str(self.cld[ct][ims]['tinterp'])+'\'',
                    print ' finterp=\''+str(self.cld[ct][ims]['finterp'])+'\''
                    #print ' reach=\''+str(self.cld[ct][ims]['reach'])+'\''
                    print '  obsmap='+str(self.cld[ct][ims]['obsmap']),
                    print ' fldmap='+str(self.cld[ct][ims]['fldmap']),
                    print ' spwmap='+str(self.cld[ct][ims]['spwmap']),
                    print ' antmap='+str(self.cld[ct][ims]['antmap'])


    def write(self,filename):
        if len(filename)<1:
            raise Exception, 'Please specify a filename'
        if len(self.cld)<1:
            raise Exception, 'There is no cal library to write'
        
        f=open(filename,"w")
        keys0=self.cld.keys()
        keys0.sort()
        for ct in keys0:
            ict0=self.cld[ct]
            keys1=ict0.keys()
            keys1.sort()
            for ims in keys1:
                ict1=ict0[ims]
                if isinstance(ict1,dict):
                    print >>f, 'caltable=\''+ct+'\'',
                    print >>f, 'calwt='+str(ict0['calwt']),
                    if len(ict1['field'])>0:
                        print >>f, 'field=\''+str(ict1['field'])+'\'',
                    if len(ict1['intent'])>0:
                        print >>f, 'intent=\''+str(ict1['intent'])+'\'',
                    if len(ict1['spw'])>0:
                        print >>f, 'spw=\''+str(ict1['spw'])+'\'',
                    if len(ict1['obs'])>0:
                        print >>f, 'obs=\''+str(ict1['obs'])+'\'',

                    if len(ict1['tinterp'])>0:
                        print >>f, 'tinterp=\''+str(ict1['tinterp'])+'\'',
                    if len(ict1['finterp'])>0:
                        print >>f, 'finterp=\''+str(ict1['finterp'])+'\'',
                    if len(ict1['reach'])>0:
                        print >>f, 'reach=\''+str(ict1['reach'])+'\'',

                    if len(ict1['obsmap'])>0:
                        print >>f, 'obsmap='+str(ict1['obsmap']),
                    if len(ict1['fldmap'])>0:
                        if isinstance(ict1['fldmap'],str):
                            print >>f, 'fldmap=\''+str(ict1['fldmap'])+'\'',
                        else:
                            print >>f, 'fldmap='+str(ict1['fldmap']),
                    if len(ict1['spwmap'])>0:
                        print >>f, 'spwmap='+str(ict1['spwmap']),
                    if len(ict1['antmap'])>0:
                        print >>f, 'antmap='+str(ict1['antmap']),

                    print >>f, ''

    def read(self,filename):
        for line in open(filename):
            line=line.strip()  # remove leading/trailing whitespace
            line=' '.join(line.split())  # reduce whitespace
            line=line.replace(' ',',')
            line=line.replace(',,',',')
            exec('self.parsetorec('+line+')')

    def compare(self,other):
        return self.cld==other.cld


def testcallib0():

    c=calibrary()
    c.add(caltable="G0",field="0",tinterp="nearest")
    c.add(caltable="G0",field="1,2",tinterp="linear")
    c.add(caltable="B0",tinterp="nearest",finterp="linear")
    c.list()
    return c


def testcallib1():

    c1=calibrary()
    c1.add(caltable=['B','phase','flux'],field='0~1,3~4',
          tinterp=['nearest','linear'],ctfield=['0',''])
    c1.add(caltable=['B','phase','flux'],field='2',
          finterp=['nearest','linear'],ctfield=['0','3,4'])
    print ''
    print 'c1: -----------------------------------------------'
    c1.list()

    c2=calibrary()
    c2.add(gaintable=['B','flux'],field='0~4',
          tinterp=['nearest'],ctfield=['0'])
    c2.add(gaintable='phase',field='0~1,3~4',
          tinterp='linear',ctfield='')
    c2.add(gaintable='phase',field='2',
          tinterp='linear',ctfield='3,4')
    print ''
    print 'c2: -----------------------------------------------'
    c2.list()

    return (c1,c2)


def testcallib2():

    c1=calibrary()
    c1.add(caltable=['B','flux'],field='0~4',
          tinterp=['nearest'],ctfield=['0'])
    c1.add(caltable='phase',field='0~1,3~4',
          tinterp='linear',ctfield='')
    c1.add(caltable='phase',field='2',
          tinterp='linear',ctfield='3,4')
    c1.write('testcallib2.txt')

    c2=calibrary()
    c2.read('testcallib2.txt')

    print 'Cal libraries match?', c2.cld==c1.cld

    return (c1,c2)


