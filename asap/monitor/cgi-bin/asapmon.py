#!/usr/bin/python
import sys,os
import cgi
#cgi debug
import cgitb; cgitb.enable()

from simpletal import simpleTAL, simpleTALES

#absolute home
abspath= "/var/www/asapmon/"
cgiloc = "/cgi-bin/asapmon/"
tmppath = os.path.join(abspath,"tmp/")
htmlloc = "/asapmon/"
tmploc = "/asapmon/tmp/"

from obsconfig import *

# a redirection object for stdout/stderr
class WritableObject:
    def __init__(self):
        self.content = []
    def write(self, string):
        self.content.append(string)

logsink = WritableObject()
logsink2 = WritableObject()
sys.stdout = logsink
sys.stderr = logsink2
import asap
sys.stdout = sys.__stdout__
sys.stderr = sys.__stderr__

def resetstd():
    sys.stdout = sys.__stdout__
    sys.stderr = sys.__stderr__


class myForm:
    def __init__(self):
        self.fields = {}
        self.form = cgi.FieldStorage()
        self.context = simpleTALES.Context(allowPythonPath=1)
        self.logsink = WritableObject()

    def decodePath(self):
        pi = self.fields['cdir']
        fi = self.fields['cfile']
        p = observatory['rpfpath'][pi]
        from filelist import FileList
        fl = FileList(pi)
        if  fl.error:
            return None
        out = []
        for i in fi:
            out.append(os.path.join(p,fl.files[i]))
        return out

    def decodeWindow(self,window):
        if not len(window.strip()): return None,None
        x = window.split(", ")
        return [float(x[0].strip()), float(x[1].strip())]

    def decodeWindows(self, window):
        import re
        p = re.compile("(\\[*\d+\\.*\d*,\d+\\.*\d*\\]*)")
        r = re.compile("[\\[\\]]")
        return [self.decodeWindow(re.sub(r, '', s)) for s in re.findall(p, window)]

    def setDefaultFields(self):
        self.fields['directories'] = observatory['rpfpath']
        self.fields['cdir'] = len(self.fields['directories'])-1
        from filelist import FileList
        files = []
        fl = FileList(len(observatory['rpfpath'])-1)
        if not fl.error:
            self.fields['files'] = fl.files
        self.fields['cfile'] = len(fl.files)-1
        self.fields['restfreqs'] = observatory['lines'].keys()
        self.fields['border'] = range(10)
        self.fields['imagename'] = ""
        self.fields['cunit'] = 0
        self.fields['units'] = ["channel", "km/s", "GHz"]
        self.fields['baseline'] = 0
        self.fields['cpolyorder'] = 0
        self.fields['quotient'] = 0
        self.fields['average'] = 0
        self.fields['doppler'] = "RADIO"
        self.fields['frame'] = "LSRK"
        self.fields['restn'] = [11, 1]
        self.fields['stokes'] = 0
        self.fields['summary'] = ""
        self.fields['bin'] = 0
        self.fields['brangewindow'] = ""
        self.fields['nif'] = []
        self.fields['sourcenames'] = []
        self.fields['csource'] = ""


    def getFormFields(self):
        self.fields['cunit'] = int(self.form.getfirst("unit", 0))
        self.fields['frame'] = self.form.getfirst("frame", "TOPO")
        self.fields['doppler'] = self.form.getfirst("doppler", "RADIO")
        self.fields['restn'] = []

        self.fields['plotwindow'] = self.form.getfirst("plotwindow", "")
        self.fields['baseline'] = int(self.form.has_key("baseline"))
        self.fields['cpolyorder'] = int(self.form.getfirst("polyorder", 0))
        self.fields['quotient'] = int(self.form.has_key("quotient"))
        self.fields['doppler'] = self.form.getfirst("doppler", "RADIO")
        self.fields['frame'] = self.form.getfirst("frame", "LSRK")
        self.fields['cdir'] = int(self.form.getfirst("dlist", None))
        self.fields['cfile'] = [int(k) for k in self.form.getlist("list")]
        self.fields['average'] = int(self.form.has_key("average"))
        self.fields['stokes'] = int(self.form.has_key("stokes"))
        self.fields['bin'] = int(self.form.has_key("bin"))
        self.fields['debug'] = ""#self.fields['restn']
        self.fields['csource'] = self.form.getfirst("csource", "")

    def getRest(self):
        alllines = observatory['lines'].values()
        lines = []
        for i in self.fields['restn']:
            lines.append(alllines[i])
        return lines

    def plotForm(self):
        self.getFormFields()
        # decode file location
        from filelist import FileList
        fl = FileList(self.fields['cdir'])
        self.fields['files'] = fl.files
        files = self.decodePath()
        # catch all stdout/err
        sys.stdout = logsink
        sys.stderr = logsink2
        try:
            s = asap.scantable(files)
            outscans = None
            self.fields['nif'] = range(s.nif())
            for i in self.fields['nif']:
                name = "rest%d" % i
                self.fields['restn'].append(int(self.form.getfirst(name, 0)))
            restfs = self.getRest()

            # source name selection
            import re
            srcnames = s.get_sourcename()
            for i in srcnames:
                # only add the names once
                i = not i in self.fields['sourcenames'] and i
                if i:
                    # filter off scans
                    i = not re.search(re.compile("_[R, e, w]$"), i) and i
                    if i:
                        self.fields['sourcenames'].append(i)
            # form quotient
            if self.form.has_key("quotient"):
                s = s.auto_quotient()
            # get source by name
            cs = self.fields['csource']
            if len(cs) > 0:
                if cs in self.fields['sourcenames']:
                    ss = s.get_scan(self.fields['csource'])
                    if isinstance(ss, asap.scantable):
                        s = ss
                    del ss
            else:
                # get only the last source in the table if not averaging
                s = s.get_scan(self.fields['sourcenames'][-1])
                #self.fields['debug'] = "DEBUG"
                self.fields['csource'] = s.get_sourcename()[-1]
            if self.fields['cunit'] == 1:
                srest = s._getrestfreqs()
                if isinstance(srest, tuple) and len(srest) != s.nif():
                    s.set_restfreqs(restfs, unit="GHz")
            s.set_unit(self.fields['units'][self.fields['cunit']])
            s.set_freqframe(self.form.getfirst("frame", "LSRK"))
            s.set_doppler(self.form.getfirst("doppler", "RADIO"))

            # baseline
            if self.form.has_key('baseline'):
                order = self.fields['cpolyorder']
                brstr = self.form.getfirst('brangewindow','').strip()
                # auto baseline or user window
                if brstr:
                    self.fields['brangewindow'] = brstr
                    brange = self.decodeWindows(brstr)
                    if len(brange):
                        self.fields['brangewindow'] = brstr
                        m = s.create_mask(brange)
                        s.poly_baseline(mask=m, order=order)
                else:
                    s.auto_poly_baseline(order=order)
            outscans = None

            if self.fields['average']:
                outscans = s.average_time(weight='tsys')
            else:
                outscans = s
            del s

            if self.fields['bin']:
                outscans.bin()

            self.fields['summary'] = str(outscans)
            asap.rcParams['plotter.decimate'] = True
            asap.rcParams['plotter.ganged'] = False
            from matplotlib import rcParams as rcp
            rcp['xtick.labelsize'] = 8
            rcp['ytick.labelsize'] = 8
            rcp['axes.labelsize'] = 8
            rcp['axes.titlesize'] = 12
            rcp['figure.subplot.wspace'] = 0.3
            rcp['figure.subplot.hspace'] = 0.3
            del asap.plotter
            # plotter without GUI
            asap.plotter = asap.asapplotter(False)
            if outscans.nif() > 1:
                asap.plotter.set_mode("p", "i")
            else:
                if outscans.npol() > 2:
                    asap.plotter.set_mode("t", "p")
                else:
                    asap.plotter.set_mode("p", "t")
            asap.plotter.plot(outscans)
            if self.fields['stokes']:
                pols = "I"
                if outscans.npol() > 2:
                    pols += " Q U V"
                sel = asap.selector()
                sel.set_polarisations(pols)
                asap.plotter.set_selection(sel)
            x0, x1 = self.decodeWindow(self.fields['plotwindow'])
            asap.plotter.set_range(x0, x1)
            imname = tmppath+"plot.png"
            asap.plotter.save(imname, dpi=96)
            self.fields['imagename'] = tmploc+"plot.png"
        except RuntimeError, e:
            self.fields['debug'] = e
            return


    def buildContext (self, title):
        self.context.addGlobal("fields", self.fields)
        self.context.addGlobal("title", title)

    def expandTemplate (self, templateName):
        sys.stdout.write ("Content-Type: text/html\n")
        sys.stdout.write ("\n")
        # Expand the template and print it out
        templateFile = open(templateName, 'r')
        template = simpleTAL.compileHTMLTemplate(templateFile)
        templateFile.close()
        # Expand the template as HTML using this context
        template.expand(self.context, sys.stdout)
        sys.exit(0)

    def main(self):
        self.setDefaultFields()
        title = "ASAP %s Online Monitor" % (observatory['name'])
        tmplname = abspath+"asapmon.html.template"
        if ( self.form.has_key("plot")):
            self.plotForm()
        self.buildContext(title)
        resetstd()
        #os.system('rm -rf /var/www/asapmon/tmp/.matplotlib')
        self.expandTemplate(tmplname)


f = myForm()
f.main()
