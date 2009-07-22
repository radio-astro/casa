import os
import sys

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




def HtmlInit(Directory):
    baselinefitINITpage = "helpbaselinefit.html"
    clusteringINITpage = "helpclustering.html"
    fitstatisticsINITpage = "helpfitstatistics.html"
    griddingINITpage = "helpgridding.html"
    multispectraINITpage = "helpmultispectra.html"
    sparsespectramapINITpage = "helpsparsespectramap.html"

    TargetSrc = Directory.split('/')[-1].replace('_html', '')
    Out = open(Directory+'/index.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAME src="Summary/index.html" name="submenu1">'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(Directory+'/menu.html', 'w')
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

    Out = open(Directory+'/summary.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAME src="Summary/index.html" name="submenu1">'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(Directory+'/log.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
    print >> Out, '<FRAMESET cols="175, *">'
    print >> Out, '<FRAMESET rows="260, *">'
    print >> Out, '<FRAME src="menu.html" name="menu">'
    print >> Out, '<FRAME src="log1.html" name="submenu1">'
    print >> Out, '</FRAMESET>'
    print >> Out, '<FRAME src="Logs/PIPELINE.txt" name="main">'
    print >> Out, '<NOFRAMES><p>No frame is supported in your browser</p></NOFRAMES>'
    print >> Out, '</FRAMESET>\n</html>'
    Out.close()

    Out = open(Directory+'/log1.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Pipeline Log Menu</title>\n<style>'
    print >> Out, '.ttl{font-size:16px;font-weight:bold;color:white;background-color:navy;}'
    print >> Out, '</style>\n</head>\n<body>'
    print >> Out, '<p class="ttl">Log Window</p>'
    print >> Out, '<ul><small>'
    if ( os.path.exists( Directory+'/Logs/Calibration.txt' ) ):
        print >> Out, '<li><a href="Logs/Calibration.txt" target="main">Calibration</a></li>'
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

    Out = open(Directory+'/baselinefit.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
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
    Out = open(Directory+'/baselinefit1h.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()
    Out = open(Directory+'/baselinefit1.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()

    Out = open(Directory+'/clustering.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
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
    Out = open(Directory+'/clustering1.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()
    Out = open(Directory+'/clustering1h.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()

    Out = open(Directory+'/fitstatistics.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
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
    Out = open(Directory+'/fitstatistics1.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()
    Out = open(Directory+'/fitstatistics1h.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()

    Out = open(Directory+'/gridding.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
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
    Out = open(Directory+'/gridding1.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()
    Out = open(Directory+'/gridding1h.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()

    Out = open(Directory+'/multispectra.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
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
    Out = open(Directory+'/multispectra1.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()
    Out = open(Directory+'/multispectra1h.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()

    Out = open(Directory+'/sparsespectramap.html', 'w')
    print >> Out, '<html xmlns="http://www.w3.org/1999/xhtml">'
    print >> Out, '<head>\n<title>SD Heuristic Pipeline for %s</title>\n</head>' % TargetSrc
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
    Out = open(Directory+'/sparsespectramap1.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()
    Out = open(Directory+'/sparsespectramap1h.html', 'w')
    print >> Out, 'Not Processed Yet'
    Out.close()

    Out = open(Directory+'/helpbaselinefit.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Baseline Fitting process:</h1>'
    print >> Out, '<p>Baseline fit Help......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(Directory+'/helpclustering.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Clustering Analysis:</h1>'
    print >> Out, '<p>Clustering Help......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(Directory+'/helpfitstatistics.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Fitting Statistics:</h1>'
    print >> Out, '<p>FitStatistics Help......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(Directory+'/helpgridding.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Gridding process:</h1>'
    print >> Out, '<p>Gridding Help......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(Directory+'/helpmultispectra.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the MultiSpectra:</h1>'
    print >> Out, '<p>MultiSpectra Help......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()

    Out = open(Directory+'/helpsparsespectramap.html', 'w')
    print >> Out, '<html>\n<head>\n</head>\n<body>'
    print >> Out, '<h1>Descriptions for the Sparse Spectra Map:</h1>'
    print >> Out, '<p>SparseSpectraMap Help......</p>'
    print >> Out, '</body>\n</html>'
    Out.close()


def HtmlBaselineFit(Directory):

    Outh = open(Directory+'/baselinefit1h.html', 'w')
    Out = open(Directory+'/baselinefit1.html', 'w')
    FileName = Directory+'/BF_Fit/listofplots.txt'
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


def HtmlClustering(Directory):

    Out = open(Directory+'/clustering1.html', 'w')
    Outh = open(Directory+'/clustering1h.html', 'w')
    FileName = Directory+'/BF_Clstr/listofplots.txt'
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


def HtmlFitStatistics(Directory):

    Outh = open(Directory+'/fitstatistics1h.html', 'w')
    Out = open(Directory+'/fitstatistics1.html', 'w')
    FileName = Directory+'/BF_Stat/listofplots.txt'
    fitstatisticsINITpage = "helpfitstatistics.html"
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
            if (IF0, POL0, ITER0) != (IF, POL, ITER):
                if IF0 != -1:
                    print >> Out2, '</body>\n</html>'
                    Out2.close()
                (IF0, POL0, ITER0) = (IF, POL, ITER)
                Fname = 'Stat_%s_%s_%s.html' % (IF, POL, ITER)
                NewFname = 'FitStat_%s_%s_%s.html' % (IF, POL, ITER)
                if INIT:
                    fitstatisticsINITpage = "BF_Stat/"+NewFname
                    INIT = False
                Out2 = open(Directory+'/BF_Stat/'+NewFname, 'w')
                if os.access(Directory+'/BF_Stat/'+Fname, os.F_OK):
                    ReadFile = open(Directory+'/BF_Stat/'+Fname, 'r')
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
                Images.append("BF_Stat/"+NewFname)
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


def HtmlGridding(Directory):

    Outh = open(Directory+'/gridding1h.html', 'w')
    Out = open(Directory+'/gridding1.html', 'w')
    FileName = Directory+'/ChannelMap/listofplots.txt'
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


def HtmlMultiSpectra(Directory):

    Outh = open(Directory+'/multispectra1h.html', 'w')
    Out = open(Directory+'/multispectra1.html', 'w')
    FileName = Directory+'/Gridding/listofplots.txt'
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


def HtmlSparseSpectraMap(Directory):

    Outh = open(Directory+'/sparsespectramap1h.html', 'w')
    Out = open(Directory+'/sparsespectramap1.html', 'w')
    FileName = Directory+'/SparseSpMap/listofplots.txt'
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


