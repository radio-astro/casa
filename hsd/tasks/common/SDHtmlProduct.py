--- ../../../../org/pipeline/hsd/tasks/common/SDHtmlProduct.py	2013-02-08 08:33:35.000000000 +0900
+++ SDHtmlProduct.py	2013-02-14 16:01:57.446663645 +0900
@@ -1185,7 +1185,7 @@
 def HtmlBaselineFit2(Directory):
 
     #from heuristics.resources.pathfinder import getPath
-    from pipeline.infrastructure.renderer import getPath
+    from pipeline.infrastructure.renderer.logger import getPath
     import zipfile
     import subprocess
     
@@ -1199,13 +1199,15 @@
     fin.close()
     
     # tools for thumbnail plot
-    jQuery = getPath('jquery.js')
+    #jQuery = getPath('jquery.js')
+    jQuery = getPath('templates/resources/jquery-1.8.1.js')
     fin = open(jQuery,'r')
     fout = open(Directory+'/BF_Fit/jquery.js','w')
     fout.write(fin.read())
     fin.close()
     fout.close()
-    fancyBoxZip = getPath('fancybox.zip')
+    #fancyBoxZip = getPath('fancybox.zip')
+    fancyBoxZip = getPath('templates/resources/fancybox.zip')
     z = zipfile.ZipFile(fancyBoxZip)
     z.extractall(path=Directory+'/BF_Fit')
     del z
