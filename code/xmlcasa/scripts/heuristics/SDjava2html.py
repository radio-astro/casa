#!/usr/bin/env python

import os
import sys

# Usage: SDjava2html.py RootDirectoryName


JavaScript1 = '\
<script type="text/javascript">\n \
<!--\n \
var n = 0;\n \
var m = 0;\n \
var k = 0;\n \
var nIm = 0;\n \
var img = new Array();\n \
var btn = new Array();'

JavaScript2a = '\
function prev(){\n \
    n = window.parent.submenu1.curBtn;\n \
    if (--n < 0) { n = m - 1; }\n \
    setBtnColor(btn[n]);\n \
    parent.submenu1.document.getElementById(btn[n]).focus();\n \
    window.parent.main.location.href = img[n];\n \
    if (n == m) {\n \
       resetBtnColor(btn[0]);\n \
    }\n \
     else {\n \
       resetBtnColor(btn[n+1]);\n \
    }\n \
}\n \
function next(){\n \
    n = window.parent.submenu1.curBtn;\n \
    if (++n == m) { n = 0; }\n \
    setBtnColor(btn[n]);\n \
    parent.submenu1.document.getElementById(btn[n]).focus();\n \
    resetBtnColor(btn[n-1]);\n \
    window.parent.main.location.href = img[n];\n \
}\n '

JavaScript2b = '\
var curBtn = 0;\n \
function hit(win){\n \
    n = win.value;\n \
    window.parent.main.location.href = img[n];\n \
}\n \
function active(win){\n \
    curBtn = win.value;\n \
    window.parent.main.location.href = img[n];\n \
}\n '

JavaScript3 = '\
function setBtnColor(btname){\n \
    window.parent.submenu1.document.getElementById(btname).style.backgroundColor=\'lightblue\';\n \
}\n \
function resetBtnColor(btname){\n \
    window.parent.submenu1.document.getElementById(btname).style.backgroundColor=\'lightgrey\';\n \
}\n \
// -->\n \
</script>'

JavaScript4 = '\
<script type="text/javascript">\n \
<!--\n \
setBtnColor(btn[%s]);\n \
document.getElementById(btn[%s]).focus();\n \
// -->\n \
</script>'




def SDjava2html(RootDirectoryName):

    NewDirectory = RootDirectoryName + '_html'
    RelativePath = RootDirectoryName.split('/')[-1]
    if not (os.access(RootDirectoryName+'.logs', os.F_OK) and \
       os.access(RootDirectoryName+'.summary', os.F_OK) and \
       os.access(RootDirectoryName+'.plots', os.F_OK)):
        print 'Directory not found: %s + .logs, .summary, .plot' % RootDirectoryName
        return

    if os.access(NewDirectory, os.F_OK):
        os.system('\\rm -r %s' % NewDirectory)
    os.mkdir(NewDirectory)

    CMD = 'cd %s; ln -s ../%s.summary/measurementset ./Summary' % (NewDirectory, RelativePath)
    os.system(CMD)
    os.mkdir(NewDirectory+'/Logs')
    CMD = 'cd %s; ln -s ../../%s.logs/' % (NewDirectory+'/Logs', RelativePath)
    os.system(CMD+'BF_Cluster.log ./BF_Cluster.txt')
    os.system(CMD+'BF_Fit.log ./BF_Fit.txt')
    os.system(CMD+'BF_Grouping.log ./BF_Grouping.txt')
    os.system(CMD+'Flagger.log ./Flagger.txt')
    os.system(CMD+'PIPELINE.log ./PIPELINE.txt')
    os.system(CMD+'BF_DetectLine.log ./BF_DetectLine.txt')
    os.system(CMD+'BF_FitOrder.log ./BF_FitOrder.txt')
    os.system(CMD+'Data.log ./Data.txt')
    os.system(CMD+'Gridding.log ./Gridding.txt')
    CMD = 'cd %s; ln -s ../%s.plots/' % (NewDirectory, RelativePath)
    os.system(CMD+'BF_Clstr BF_Clstr')
    os.system(CMD+'BF_Fit BF_Fit')
    #os.system(CMD+'BF_Stat BF_Stat')
    os.system('cd %s; mkdir BF_Stat' % NewDirectory)
    os.system(CMD+'ChannelMap ChannelMap')
    os.system(CMD+'Gridding Gridding')
    os.system(CMD+'SparseSpMap SparseSpMap')

    Out = open(NewDirectory+'/index.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAME src="Summary/index.html" name="submenu1">'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/menu.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Pipeline Menu</title>\n<style>'
    print >> Out, '.ttl{font-size:20px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '</style>\n</head>\n<body>'
    print >> Out, '<p class="ttl">SD Pipeline</p>'
    print >> Out, '<ul><small>'
    print >> Out, '<li><a href="summary.html" target="_parent">Summary</a></li>'
    print >> Out, '<li><a href="clustering.html" target="_parent">Clustering</a></li>'
    print >> Out, '<li><a href="baselinefit.html" target="_parent">BaselineFit</a></li>'
    print >> Out, '<li><a href="fitstatistics.html" target="_parent">FitStatistics</a></li>'
    print >> Out, '<li><a href="multispectra.html" target="_parent">MultiSpectra</a></li>'
    print >> Out, '<li><a href="gridding.html" target="_parent">Gridding</a></li>'
    print >> Out, '<li><a href="sparsespectramap.html" target="_parent">SparseSpectraMap</a></li>'
    print >> Out, '<li><a href="log.html" target="_parent">Log</a></small></li>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/summary.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAME src="Summary/index.html" name="submenu1">'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/log.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAMESET rows="260, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAME src="log1.html" name="submenu1">'
    print >> Out, '</FRAMESET>'
    print >> Out, '<FRAME src="Logs/PIPELINE.txt" name="main">'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/log1.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Pipeline Log Menu</title>\n<style>'
    print >> Out, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '</style>\n</head>\n<body>'
    print >> Out, '<p class="ttl">Log Window</p>'
    print >> Out, '<ul><small>'
    print >> Out, '<li><a href="Logs/PIPELINE.txt" target="main">Pipeline</a></li>'
    print >> Out, '<li><a href="Logs/Data.txt" target="main">Data</a></li>'
    print >> Out, '<li><a href="Logs/BF_Grouping.txt" target="main">Grouping</a></li>'
    print >> Out, '<li><a href="Logs/BF_DetectLine.txt" target="main">LineDetection</a></li>'
    print >> Out, '<li><a href="Logs/BF_Cluster.txt" target="main">Clustering</a></li>'
    print >> Out, '<li><a href="Logs/BF_FitOrder.txt" target="main">FittingOrder</a></li>'
    print >> Out, '<li><a href="Logs/BF_Fit.txt" target="main">Fitting</a></li>'
    print >> Out, '<li><a href="Logs/Flagger.txt" target="main">Flagging</a></li>'
    print >> Out, '<li><a href="Logs/Gridding.txt" target="main">Gridding</a></li>'
    print >> Out, '</small></ul>\n</body>\n</html>'
    Out.close()

    Outh = open(NewDirectory+'/baselinefit1h.html', 'w')
    Out = open(NewDirectory+'/baselinefit1.html', 'w')
    FileName = RootDirectoryName+'.plots/BF_Fit/listofplots.txt'
    baselinefitINITpage = "helpbaselinefit.html"
    Images = []
    Values = []
    if os.access(FileName, os.F_OK):
        File = open(FileName, 'r')
        INIT = True
        i = 0
        while 1:
            line = File.readline()
            if not line: break
            if line.find(':') != -1: continue
            Plot = line.split()[0]
            (IF, POL, ITER, PAGE) = Plot.split('.')[0].split('_')[1:]
            Images.append("BF_Fit/"+Plot)
            #Values.append((ITER, IF, POL, PAGE, i))
            bname='btn'+str(i)
            Values.append((ITER, IF, POL, PAGE, bname, i))
            i += 1
            if INIT:
                baselinefitINITpage = "BF_Fit/"+Plot
                INIT = False
        File.close()
    print >> Outh, '<html>\n<head>\n<style>'
    print >> Outh, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Outh, '.stt{font-size:12px;font-weight:bold;}'
    print >> Outh, '.stc{font-size:12px;font-weight:normal;}'
    print >> Outh, '.cap{font-size:12px;font-weight:normal;}'
    print >> Outh, '.btn{font-size:10px;font-weight:normal;}'
    print >> Outh, '</style>'
    print >> Outh, JavaScript1
    print >> Out, '<html>\n<head>\n<style>'
    print >> Out, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '.stt{font-size:12px;font-weight:bold;}'
    print >> Out, '.stc{font-size:12px;font-weight:normal;}'
    print >> Out, '.cap{font-size:12px;font-weight:normal;}'
    print >> Out, '.btn{font-size:10px;font-weight:normal;}'
    print >> Out, '</style>'
    print >> Out, JavaScript1
    for i in range(len(Images)):
        print >> Outh, 'img[m++] = "%s";' % Images[i]
        print >> Outh, 'btn[k++] = "%s";' % Values[i][4] 
        print >> Out, 'img[m++] = "%s";' % Images[i]
        print >> Out, 'btn[k++] = "%s";' % Values[i][4] 
    print >> Outh, JavaScript2a
    print >> Outh, JavaScript3
    print >> Outh, '</head>\n<body>'
    print >> Out, JavaScript2b
    print >> Out, JavaScript3
    print >> Out, '</head>\n<body>'
    print >> Outh, '<p class="ttl">Baseline Fit</p>'
    print >> Outh, '<a href="helpbaselinefit.html" target="main">Description</a>'
    print >> Outh, '<p class="cap">Itr: Iteration Cycle</br>'
    print >> Outh, 'IF: Frequency ID</br>'
    print >> Outh, 'Pol: Polarization</br>\n</p>'
    #print >> Outh, 'Page</br>\n</p>'
    print >> Outh, '<form action="#">'
    print >> Outh, '<input type="button" value="&lt; Prev" onclick="prev()" class="stt">'
    print >> Outh, '<input type="button" value="Next &gt;" onclick="next()" class="stt">'
    print >> Outh, '</form>\n</body>\n</html>'
    print >> Out, '<table border="1">'
    print >> Out, '<tr align="center" class="stt"><th>Itr</th><th>IF</th><th>Pol</th><th>Page</th><th>&nbsp;</th></tr>'
    for i in range(len(Images)):
        print >> Out, '<tr align="right" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th><input id=%s type="button" value=%s onclick="hit(this); setBtnColor(this.id)" onblur="resetBtnColor(this.id)" onfocus="active(this)" class="btn"/></th></tr>' % Values[i] 
    print >> Out, '</table>\n'
    print >> Out, JavaScript4 % (0,0)
    print >> Out, '</body>\n</html>'
    Out.close()
    Outh.close()

    Out = open(NewDirectory+'/clustering1.html', 'w')
    Outh = open(NewDirectory+'/clustering1h.html', 'w')
    FileName = RootDirectoryName+'.plots/BF_Clstr/listofplots.txt'
    clusteringINITpage = "helpclustering.html"
    Images = []
    Values = []
    if os.access(FileName, os.F_OK):
        File = open(FileName, 'r')
        INIT = True
        i = 0
        while 1:
            line = File.readline()
            if not line: break
            if line.find(':') != -1: continue
            Plot = line.split()[0]
            (IF, POL, ITER, PAGE) = Plot.split('.')[0].split('_')[1:]
            if PAGE.upper() == 'DETECTION': STAGE = 'D'
            elif PAGE.upper() == 'VALIDATION': STAGE = 'V'
            elif PAGE.upper() == 'SMOOTHING': STAGE = 'S'
            else: STAGE = 'R'
            if STAGE == 'D':
                PlotMap = Plot.replace('detection', 'ChannelSpace')
                Images.append("BF_Clstr/"+PlotMap)
                #Values.append((ITER, IF, POL, 'Map', i))
                bname='btn'+str(i)
                Values.append((ITER, IF, POL, 'Map', bname, i))
                i += 1
                if INIT:
                    clusteringINITpage = "BF_Clstr/"+PlotMap
                    INIT = False
            Images.append("BF_Clstr/"+Plot)
            #Values.append((ITER, IF, POL, PAGE, i))
            bname='btn'+str(i)
            PAGE = PAGE.capitalize()
            Values.append((ITER, IF, POL, PAGE, bname, i))
            i += 1
            #if INIT:
                #clusteringINITpage = "BF_Clstr/"+Plot
                #INIT = False
        File.close()
    print >> Outh, '<html>\n<head>\n<style>'
    print >> Outh, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Outh, '.stt{font-size:12px;font-weight:bold;}'
    print >> Outh, '.stc{font-size:12px;font-weight:normal;}'
    print >> Outh, '.cap{font-size:12px;font-weight:normal;}'
    print >> Outh, '.btn{font-size:10px;font-weight:normal;}'
    print >> Outh, '</style>'
    print >> Out, '<html>\n<head>\n<style>'
    print >> Out, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '.stt{font-size:12px;font-weight:bold;}'
    print >> Out, '.stc{font-size:12px;font-weight:normal;}'
    print >> Out, '.cap{font-size:12px;font-weight:normal;}'
    print >> Out, '.btn{font-size:10px;font-weight:normal;}'
    print >> Out, '</style>'
    print >> Outh, JavaScript1
    print >> Out, JavaScript1
    for i in range(len(Images)):
        print >> Outh, 'img[m++] = "%s";' % Images[i]
        print >> Outh, 'btn[k++] = "%s";' % Values[i][4] 
        print >> Out, 'img[m++] = "%s";' % Images[i]
        print >> Out, 'btn[k++] = "%s";' % Values[i][4] 
    #print >> Out, 'n = 1;'
    print >> Outh, JavaScript2a
    print >> Outh, JavaScript3
    print >> Out, JavaScript2b
    print >> Out, JavaScript3
    print >> Outh, '</head>\n<body>'
    print >> Out, '</head>\n<body>'
    print >> Outh, '<p class="ttl">Clustering Analysis</p>'
    print >> Outh, '<a href="helpclustering.html" target="main">Description</a>'
    print >> Outh, '<p class="cap">Itr: Iteration Cycle</br>'
    print >> Outh, 'IF: Frequency ID</br>'
    print >> Outh, 'Pol: Polarization</br>'
    print >> Outh, 'Stage</br>\n'
    print >> Outh, '&nbsp&nbsp Map: Cluster Space</br>'
    print >> Outh, '&nbsp&nbsp D: Detection</br>'
    print >> Outh, '&nbsp&nbsp V: Validation</br>'
    print >> Outh, '&nbsp&nbsp S: Smoothing</br>'
    print >> Outh, '&nbsp&nbsp R: Regions</br></p>'
    print >> Outh, '<form action="#">'
    print >> Outh, '<input type="button" value="&lt; Prev" onclick="prev()" class="stt">'
    print >> Outh, '<input type="button" value="Next &gt;" onclick="next()" class="stt">'
    print >> Outh, '</form>\n</body>\n</html>'
    print >> Out, '<table border="1">'
    print >> Out, '<tr align="center" class="stt"><th>Itr</th><th>IF</th><th>Pol</th><th>Stage</th><th>&nbsp;</th></tr>'
    for i in range(len(Images)):
        print >> Out, '<tr align="right" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th><input id=%s type="button" value=%s onclick="hit(this); setBtnColor(this.id)" onblur="resetBtnColor(this.id)" onfocus="active(this)" class="btn"/></th></tr>' % Values[i] 
    print >> Out, '</table>\n'
    # 2008/11/11 initial plot is a map
    #print >> Out, JavaScript3 % (1,1)
    print >> Out, JavaScript4 % (0,0)
    print >> Out, '</body>\n</html>'
    Outh.close()
    Out.close()

    Outh = open(NewDirectory+'/fitstatistics1h.html', 'w')
    Out = open(NewDirectory+'/fitstatistics1.html', 'w')
    FileName = RootDirectoryName+'.plots/BF_Stat/listofplots.txt'
    fitstatisticsINITpage = "helpfitstatistic.html"
    Images = []
    Values = []
    i = 0
    if os.access(FileName, os.F_OK):
        File = open(FileName, 'r')
        INIT = True
        (IF0, POL0, ITER0) = (-1, -1, -1)
        while 1:
            line = File.readline()
            if not line: break
            if line.find(':') != -1: continue
            Plot = line.split()[0]
            PlotRoot = Plot.split('.')[0]
            (IF, POL, ITER, PAGE) = PlotRoot.split('_')[1:]
            OldPlot = RootDirectoryName+'.plots/BF_Stat/'+Plot
            NewPlot = NewDirectory+'/BF_Stat/'+PlotRoot+'_trim.png'
            os.system('convert %s -trim %s' % (OldPlot, NewPlot))
            if (IF0, POL0, ITER0) != (IF, POL, ITER):
                if IF0 != -1:
                    print >> Out2, '</body>\n</html>'
                    Out2.close()
                (IF0, POL0, ITER0) = (IF, POL, ITER)
                Fname = 'Stat_%s_%s_%s.html' % (IF, POL, ITER)
                if INIT:
                    fitstatisticsINITpage = "BF_Stat/"+Fname
                    INIT = False
                Out2 = open(NewDirectory+'/BF_Stat/'+Fname, 'w')
                if os.access(RootDirectoryName+'.plots/BF_Stat/'+Fname, os.F_OK):
                    ReadFile = open(RootDirectoryName+'.plots/BF_Stat/'+Fname, 'r')
                    while 1:
                        line0 = ReadFile.readline().split('\n')[0]
                        if line0.find('</body>') >= 0:
                            print >> Out2, '<HR>'
                            ReadFile.close()
                            break
                        print >> Out2, line0
                else:
                    print >> Out2, '<html>\n<head>\n</head>\n<body>'
                print >> Out2, 'Note to all the plots below: short green vertical lines indicate position gaps; short cyan vertical lines indicate time gaps<HR>'
                print >> Out2, '<img src="%s">\n<HR>' % (PlotRoot+'_trim.png')
                Images.append("BF_Stat/"+Fname)
                #Values.append((ITER, IF, POL, i))
                bname='btn'+str(i)
                Values.append((ITER, IF, POL, bname, i))
                i += 1
            else:
                print >> Out2, '<img src="%s">\n<HR>' % (PlotRoot+'_trim.png')
        File.close()
    print >> Outh, '<html>\n<head>\n<style>'
    print >> Outh, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Outh, '.stt{font-size:12px;font-weight:bold;}'
    print >> Outh, '.stc{font-size:12px;font-weight:normal;}'
    print >> Outh, '.cap{font-size:12px;font-weight:normal;}'
    print >> Outh, '.btn{font-size:10px;font-weight:normal;}'
    print >> Outh, '</style>'
    print >> Out, '<html>\n<head>\n<style>'
    print >> Out, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '.stt{font-size:12px;font-weight:bold;}'
    print >> Out, '.stc{font-size:12px;font-weight:normal;}'
    print >> Out, '.cap{font-size:12px;font-weight:normal;}'
    print >> Out, '.btn{font-size:10px;font-weight:normal;}'
    print >> Out, '</style>'
    print >> Outh, JavaScript1
    print >> Out, JavaScript1
    for i in range(len(Images)):
        print >> Outh, 'img[m++] = "%s";' % Images[i]
        print >> Outh, 'btn[k++] = "%s";' % Values[i][3] 
        print >> Out, 'img[m++] = "%s";' % Images[i]
        print >> Out, 'btn[k++] = "%s";' % Values[i][3] 
    print >> Outh, JavaScript2a
    print >> Outh, JavaScript3
    print >> Out, JavaScript2b
    print >> Out, JavaScript3
    print >> Outh, '</head>\n<body>'
    print >> Out, '</head>\n<body>'
    print >> Outh, '<p class="ttl">Fitting Statistics</p>'
    print >> Outh, '<a href="helpfitstatistics.html" target="main">Description</a>'
    print >> Outh, '<p class="cap">Itr: Iteration Cycle</br>'
    print >> Outh, 'IF: Frequency ID</br>'
    print >> Outh, 'Pol: Polarization</br>\n</p>'
    print >> Outh, '<form action="#">'
    print >> Outh, '<input type="button" value="&lt; Prev" onclick="prev()" class="stt">'
    print >> Outh, '<input type="button" value="Next &gt;" onclick="next()" class="stt">'
    print >> Outh, '</form>\n</body>\n</html>'
    print >> Out, '<table border="1">'
    print >> Out, '<tr align="center" class="stt"><th>Itr</th><th>IF</th><th>Pol</th><th>&nbsp;</th></tr>'
    for i in range(len(Images)):
        print >> Out, '<tr align="right" class="stc"><th>%s</th><th>%s</th><th>%s</th><th><input id=%s type="button" value=%s onclick="hit(this); setBtnColor(this.id)" onblur="resetBtnColor(this.id)" onfocus="active(this)" class="btn"/></th></tr>' % Values[i] 
    print >> Out, '</table>\n</form>\n'
    print >> Out, JavaScript4 % (0,0)
    print >> Out, '</body>\n</html>'
    Out.close()
    Outh.close()

    Outh = open(NewDirectory+'/gridding1h.html', 'w')
    Out = open(NewDirectory+'/gridding1.html', 'w')
    FileName = RootDirectoryName+'.plots/ChannelMap/listofplots.txt'
    griddingINITpage = "helpgridding.html"
    Images = []
    Values = []
    if os.access(FileName, os.F_OK):
        File = open(FileName, 'r')
        INIT = True
        i = 0
        while 1:
            line = File.readline()
            if not line: break
            if line.find(':') != -1: continue
            Plot = line.split()[0]
            (IF, POL, ITER, PAGE) = Plot.split('.')[0].split('_')[1:]
            Images.append("ChannelMap/"+Plot)
            #Values.append((ITER, IF, POL, PAGE, i))
            bname='btn'+str(i)
            Values.append((ITER, IF, POL, PAGE, bname, i))
            i += 1
            if INIT:
                griddingINITpage = "ChannelMap/"+Plot
                INIT = False
        File.close()
    print >> Outh, '<html>\n<head>\n<style>'
    print >> Outh, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Outh, '.stt{font-size:12px;font-weight:bold;}'
    print >> Outh, '.stc{font-size:12px;font-weight:normal;}'
    print >> Outh, '.cap{font-size:12px;font-weight:normal;}'
    print >> Outh, '.btn{font-size:10px;font-weight:normal;}'
    print >> Outh, '</style>'
    print >> Out, '<html>\n<head>\n<style>'
    print >> Out, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '.stt{font-size:12px;font-weight:bold;}'
    print >> Out, '.stc{font-size:12px;font-weight:normal;}'
    print >> Out, '.cap{font-size:12px;font-weight:normal;}'
    print >> Out, '.btn{font-size:10px;font-weight:normal;}'
    print >> Out, '</style>'
    print >> Outh, JavaScript1
    print >> Out, JavaScript1
    for i in range(len(Images)):
        print >> Outh, 'img[m++] = "%s";' % Images[i]
        print >> Outh, 'btn[k++] = "%s";' % Values[i][4] 
        print >> Out, 'img[m++] = "%s";' % Images[i]
        print >> Out, 'btn[k++] = "%s";' % Values[i][4] 
    print >> Outh, JavaScript2a
    print >> Outh, JavaScript3
    print >> Out, JavaScript2b
    print >> Out, JavaScript3
    print >> Outh, '</head>\n<body>'
    print >> Out, '</head>\n<body>'
    print >> Outh, '<p class="ttl">Gridding</p>'
    print >> Outh, '<a href="helpgridding.html" target="main">Description</a>'
    print >> Outh, '<p class="cap">Itr: Iteration Cycle</br>'
    print >> Outh, 'IF: Frequency ID</br>'
    print >> Outh, 'Pol: Polarization</br>\n</p>'
    #print >> Outh, 'Page</br>\n</p>'
    print >> Outh, '<form action="#">'
    print >> Outh, '<input type="button" value="&lt; Prev" onclick="prev()" class="stt">'
    print >> Outh, '<input type="button" value="Next &gt;" onclick="next()" class="stt">'
    print >> Outh, '</form>\n</body>\n</html>'
    print >> Out, '<table border="1">'
    print >> Out, '<tr align="center" class="stt"><th>Itr</th><th>IF</th><th>Pol</th><th>Page</th><th>&nbsp;</th></tr>'
    for i in range(len(Images)):
        print >> Out, '<tr align="right" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th><input id=%s type="button" value=%s onclick="hit(this); setBtnColor(this.id)" onblur="resetBtnColor(this.id)" onfocus="active(this)" class="btn"/></th></tr>' % Values[i] 
    print >> Out, '</table>\n'
    print >> Out, JavaScript4 % (0,0)
    print >> Out, '</body>\n</html>'
    Out.close()
    Outh.close()

    Outh = open(NewDirectory+'/multispectra1h.html', 'w')
    Out = open(NewDirectory+'/multispectra1.html', 'w')
    FileName = RootDirectoryName+'.plots/Gridding/listofplots.txt'
    multispectraINITpage = "helpmultispectra.html"
    Images = []
    Values = []
    if os.access(FileName, os.F_OK):
        File = open(FileName, 'r')
        INIT = True
        i = 0
        while 1:
            line = File.readline()
            if not line: break
            if line.find(':') != -1: continue
            Plot = line.split()[0]
            (IF, POL, ITER, PAGE) = Plot.split('.')[0].split('_')[1:]
            Images.append("Gridding/"+Plot)
            #Values.append((ITER, IF, POL, PAGE, i))
            bname='btn'+str(i)
            Values.append((ITER, IF, POL, PAGE, bname, i))
            i += 1
            if INIT:
                multispectraINITpage = "Gridding/"+Plot
                INIT = False
        File.close()
    print >> Outh, '<html>\n<head>\n<style>'
    print >> Outh, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Outh, '.stt{font-size:12px;font-weight:bold;}'
    print >> Outh, '.stc{font-size:12px;font-weight:normal;}'
    print >> Outh, '.cap{font-size:12px;font-weight:normal;}'
    print >> Outh, '.btn{font-size:10px;font-weight:normal;}'
    print >> Outh, '</style>'
    print >> Out, '<html>\n<head>\n<style>'
    print >> Out, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '.stt{font-size:12px;font-weight:bold;}'
    print >> Out, '.stc{font-size:12px;font-weight:normal;}'
    print >> Out, '.cap{font-size:12px;font-weight:normal;}'
    print >> Out, '.btn{font-size:10px;font-weight:normal;}'
    print >> Out, '</style>'
    print >> Outh, JavaScript1
    print >> Out, JavaScript1
    for i in range(len(Images)):
        print >> Outh, 'img[m++] = "%s";' % Images[i]
        print >> Outh, 'btn[k++] = "%s";' % Values[i][4] 
        print >> Out, 'img[m++] = "%s";' % Images[i]
        print >> Out, 'btn[k++] = "%s";' % Values[i][4] 
    print >> Outh, JavaScript2a
    print >> Outh, JavaScript3
    print >> Out, JavaScript2b
    print >> Out, JavaScript3
    print >> Outh, '</head>\n<body>'
    print >> Out, '</head>\n<body>'
    print >> Outh, '<p class="ttl">MultiSpectra</p>'
    print >> Outh, '<a href="helpmultispectra.html" target="main">Description</a>'
    print >> Outh, '<p class="cap">Itr: Iteration Cycle</br>'
    print >> Outh, 'IF: Frequency ID</br>'
    print >> Outh, 'Pol: Polarization</br>\n</p>'
    #print >> Outh, 'Page</br>\n</p>'
    print >> Outh, '<form action="#">'
    print >> Outh, '<input type="button" value="&lt; Prev" onclick="prev()" class="stt">'
    print >> Outh, '<input type="button" value="Next &gt;" onclick="next()" class="stt">'
    print >> Outh, '</form>\n</body>\n</html>'
    print >> Out, '<table border="1">'
    print >> Out, '<tr align="center" class="stt"><th>Itr</th><th>IF</th><th>Pol</th><th>Page</th><th>&nbsp;</th></tr>'
    for i in range(len(Images)):
        print >> Out, '<tr align="right" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th><input id=%s type="button" value=%s onclick="hit(this); setBtnColor(this.id)" onblur="resetBtnColor(this.id)" onfocus="active(this)" class="btn"/></th></tr>' % Values[i] 
    print >> Out, '</table>\n'
    print >> Out, JavaScript4 % (0,0)
    print >> Out, '</body>\n</html>'
    Out.close()
    Outh.close()

    Outh = open(NewDirectory+'/sparsespectramap1h.html', 'w')
    Out = open(NewDirectory+'/sparsespectramap1.html', 'w')
    FileName = RootDirectoryName+'.plots/SparseSpMap/listofplots.txt'
    sparsespectramapINITpage = "helpsparsespectramap.html"
    Images = []
    Values = []
    if os.access(FileName, os.F_OK):
        File = open(FileName, 'r')
        INIT = True
        i = 0
        while 1:
            line = File.readline()
            if not line: break
            if line.find(':') != -1: continue
            Plot = line.split()[0]
            (IF, POL, ITER, PAGE) = Plot.split('.')[0].split('_')[1:]
            Images.append("SparseSpMap/"+Plot)
            #Values.append((ITER, IF, POL, i))
            bname='btn'+str(i)
            Values.append((ITER, IF, POL, bname, i))
            i += 1
            if INIT:
                sparsespectramapINITpage = "SparseSpMap/"+Plot
                INIT = False
        File.close()
    print >> Outh, '<html>\n<head>\n<style>'
    print >> Outh, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Outh, '.stt{font-size:12px;font-weight:bold;}'
    print >> Outh, '.stc{font-size:12px;font-weight:normal;}'
    print >> Outh, '.cap{font-size:12px;font-weight:normal;}'
    print >> Outh, '.btn{font-size:10px;font-weight:normal;}'
    print >> Outh, '</style>'
    print >> Out, '<html>\n<head>\n<style>'
    print >> Out, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '.stt{font-size:12px;font-weight:bold;}'
    print >> Out, '.stc{font-size:12px;font-weight:normal;}'
    print >> Out, '.cap{font-size:12px;font-weight:normal;}'
    print >> Out, '.btn{font-size:10px;font-weight:normal;}'
    print >> Out, '</style>'
    print >> Outh, JavaScript1
    print >> Out, JavaScript1
    for i in range(len(Images)):
        print >> Outh, 'img[m++] = "%s";' % Images[i]
        print >> Outh, 'btn[k++] = "%s";' % Values[i][3] 
        print >> Out, 'img[m++] = "%s";' % Images[i]
        print >> Out, 'btn[k++] = "%s";' % Values[i][3] 
    print >> Outh, JavaScript2a
    print >> Outh, JavaScript3
    print >> Out, JavaScript2b
    print >> Out, JavaScript3
    print >> Outh, '</head>\n<body>'
    print >> Out, '</head>\n<body>'
    print >> Outh, '<p class="ttl">Sparse Spectra Map</p>'
    print >> Outh, '<a href="helpsparsespectramap.html" target="main">Description</a>'
    print >> Outh, '<p class="cap">Itr: Iteration Cycle</br>'
    print >> Outh, 'IF: Frequency ID</br>'
    print >> Outh, 'Pol: Polarization</br></p>'
    print >> Outh, '<form action="#">'
    print >> Outh, '<input type="button" value="&lt; Prev" onclick="prev()" class="stt">'
    print >> Outh, '<input type="button" value="Next &gt;" onclick="next()" class="stt">'
    print >> Outh, '</form>\n</body>\n</html>'
    print >> Out, '<table border="1">'
    print >> Out, '<tr align="center" class="stt"><th>Itr</th><th>IF</th><th>Pol</th><th>&nbsp;</th></tr>'
    for i in range(len(Images)):
        print >> Out, '<tr align="right" class="stc"><th>%s</th><th>%s</th><th>%s</th><th><input id=%s type="button" value=%s onclick="hit(this); setBtnColor(this.id)" onblur="resetBtnColor(this.id)" onfocus="active(this)" class="btn"/></th></tr>' % Values[i] 
    print >> Out, '</table>\n'
    print >> Out, JavaScript4 % (0,0)
    print >> Out, '</body>\n</html>'
    Out.close()
    Outh.close()

    Out = open(NewDirectory+'/baselinefit.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAMESET rows="260, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAMESET rows="195, *">'
    print >> Out, '<FRAME src="baselinefit1h.html" name="submenu1h" frameborder="NO" scrolling="NO">'
    print >> Out, '<FRAME src="baselinefit1.html" name="submenu1" frameborder="NO">'
    print >> Out, '</FRAMESET>'
    print >> Out, '</FRAMESET>'
    print >> Out, '<FRAME src="%s" name="main">' % baselinefitINITpage
    print >> Out, '</FRAMESET>'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/clustering.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAMESET rows="260, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAMESET rows="285, *">'
    print >> Out, '<FRAME src="clustering1h.html" name="submenu1h" frameborder="NO" scrolling="NO">'
    print >> Out, '<FRAME src="clustering1.html" name="submenu1" frameborder="NO">'
    print >> Out, '</FRAMESET>'
    print >> Out, '</FRAMESET>'
    print >> Out, '<FRAME src="%s" name="main">' % clusteringINITpage
    print >> Out, '</FRAMESET>'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/fitstatistics.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAMESET rows="260, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAMESET rows="175, *">'
    print >> Out, '<FRAME src="fitstatistics1h.html" name="submenu1h" frameborder="NO" scrolling="NO">'
    print >> Out, '<FRAME src="fitstatistics1.html" name="submenu1" frameborder="NO">'
    print >> Out, '</FRAMESET>'
    print >> Out, '</FRAMESET>'
    print >> Out, '<FRAME src="%s" name="main">' % fitstatisticsINITpage
    print >> Out, '</FRAMESET>'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/gridding.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAMESET rows="260, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAMESET rows="195, *">'
    print >> Out, '<FRAME src="gridding1h.html" name="submenu1h" frameborder="NO" scrolling="NO">'
    print >> Out, '<FRAME src="gridding1.html" name="submenu1" frameborder="NO">'
    print >> Out, '</FRAMESET>'
    print >> Out, '</FRAMESET>'
    print >> Out, '<FRAME src="%s" name="main">' % griddingINITpage
    print >> Out, '</FRAMESET>'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/multispectra.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAMESET rows="260, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAMESET rows="200, *">'
    print >> Out, '<FRAME src="multispectra1h.html" name="submenu1h" frameborder="NO" scrolling="NO">'
    print >> Out, '<FRAME src="multispectra1.html" name="submenu1" frameborder="NO">'
    print >> Out, '</FRAMESET>'
    print >> Out, '</FRAMESET>'
    print >> Out, '<FRAME src="%s" name="main">' % multispectraINITpage
    print >> Out, '</FRAMESET>'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/sparsespectramap.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % RootDirectoryName
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAMESET rows="260, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAMESET rows="190, *">'
    print >> Out, '<FRAME src="sparsespectramap1h.html" name="submenu1h" frameborder="NO" scrolling="NO">'
    print >> Out, '<FRAME src="sparsespectramap1.html" name="submenu1" frameborder="NO">'
    print >> Out, '</FRAMESET>'
    print >> Out, '</FRAMESET>'
    print >> Out, '<FRAME src="%s" name="main">' % sparsespectramapINITpage
    print >> Out, '</FRAMESET>'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/helpbaselinefit.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Baseline Fitting process:</h1>'
    print >> Out, '<p>Baseline fitting......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/helpclustering.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Clustering Analysis:</h1>'
    print >> Out, '<p>Baseline fitting......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/helpfitstatistics.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Fitting Statistics:</h1>'
    print >> Out, '<p>Baseline fitting......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/helpgridding.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Gridding process:</h1>'
    print >> Out, '<p>Baseline fitting......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/helpmultispectra.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the MultiSpectra:</h1>'
    print >> Out, '<p>Baseline fitting......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(NewDirectory+'/helpsparsespectramap.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Sparse Spectra Map:</h1>'
    print >> Out, '<p>Baseline fitting......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    print 'Directory %s was created.' % NewDirectory
    print 'Please read %s/index.html in your web browser.' % NewDirectory

    return


if (__name__ == "__main__"):
    if len(sys.argv) != 2:
        print 'Usage: SDjava2html.py RootDirectoryName'
        sys.exit(1)

    SDjava2html(sys.argv[1])



