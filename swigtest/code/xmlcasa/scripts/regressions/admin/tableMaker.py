#a class for making html tables of results
import os
import time 
import string
import shutil
import inspect
import sys

import socket

class tableMaker:
 def __init__(self, thedir, testname, header='', footer=''):

  a=inspect.stack()
  stacklevel=0
  for k in range(len(a)):
      if (string.find(a[k][1], 'ipython console') > 0):
          stacklevel=k     
  myf=sys._getframe(stacklevel).f_globals
  self.header=header
  self.footer=footer
  self.thedir=thedir
  self.pagename=thedir+'/result-'+time.strftime('%Y_%m_%d_%H_%M.html')

  #self.pagename='result-test.html'
  #if access(self.pagename,F_OK): 
  # remove(self.pagename)
  # print 'removing old junk'
  self.oldarchive(thedir)
  if self.header != '' and os.path.isfile(self.header):
      shutil.copy(self.header, self.pagename)
  self.fd=os.open(self.pagename,os.O_APPEND|os.O_WRONLY,0644)
  s='<title>'
  s+='Test results for '+time.strftime('%Y_%m_%d') +'</title>\n'
  os.write(self.fd,s)
  os.write(self.fd,'<body>\n')
  os.write(self.fd, '<p><A HREF="%s">%s</A>\n<p>'%('archive.html','ARCHIVE OF PREVIOUS RUNS'))
  os.write(self.fd, '<CENTER><TABLE border="1" cellpadding=5 summary="Quick view over tests."><CAPTION><EM>Summary  of Tests on %s --  %s -- %s</EM></CAPTION>\n'%(os.uname()[0],os.uname()[1], myf['casalog'].version()) )
  os.write(self.fd, '<TR><TH align=middle>Test Name<TH align=middle>Image Name<TH align=middle>Quick Result<TH align=middle> Status <TH align=middle> Link to results\n')
 
 def addRows(self,stuff): #values has two components, number+status
  #print stuff
  
  for i in stuff:
   if(i[2]==1): status='<font color="green">Passed</font>'
   elif (i[2]==2): status='<font color="blue">Unknown</font>'
   else: status='<font color="red">Failed</font>'
#   link='<A HREF="file:///%s">%s</A>\n'%(i[3],i[3])
   link='<A HREF="%s">%s</A>\n'%(i[3],i[3]) 
   s='<TR><TD>%s<TD>%s<TD>%s<TD>%s<TD>%s\n'%(i[0],i[4],i[1],status,link)
   os.write(self.fd,s) 

 def done(self):
  os.write(self.fd, '</CENTER></TABLE>\n')
  os.write(self.fd,'</body>\n')
  s='</html>\n'
  os.write(self.fd,s)
  os.close(self.fd)
  if self.footer != '' and os.path.isfile(self.footer):
      os.system('cat '+self.footer+' >> '+self.pagename)
  print 'table written to %s\n'%self.pagename
  return self.pagename

 def oldarchive(self, result_dir):
     #RESULT_DIR='/home/ballista3/Regression/Result'

##remove archive file
     os.system('rm -f '+result_dir+'/archive.html')
     a=os.listdir(result_dir)
     b=os.listdir(result_dir)
     for k in range(len(b)) :
         if(string.count(b[k], '.html') < 1):
             a.remove(b[k])

     a.sort(reverse=True)

     fd=os.open(result_dir+'/archive.html', os.O_CREAT|os.O_WRONLY,0644)
     s='<title>'
     s+='Archive of test results '+time.strftime('%Y_%m_%d') +'</title>\n'
     os.write(fd,s)
     os.write(fd,'<body>\n')
     for k in range(len(a)):
         os.write(fd, '<p><A HREF="%s">%s</A>\n<p>'%(a[k],a[k]))

     os.write(fd,'</body>\n')
     s='</html>\n'
     os.write(fd,s)
     os.close(fd)
