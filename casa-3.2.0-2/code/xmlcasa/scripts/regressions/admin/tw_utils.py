import os
#from math import *

#directions for htmlPub() with python interpreter
#>>>a=htmlPub(filename,title), where filename is output html page
#>>>header=text for header of the page section
#>>>text1=[list of string statements for above image]
#>>>text2=[list of string statements for below image]
#>>>a.doBlk(text1,text2,imagePath,header=''), repeat this for all images
#>>>a.doFooter(), to close out the htmlfile and terminate file stream

class htmlPub:
        def __init__(self,file,title='Performance tests'):
                self.fd=os.open(file,os.O_CREAT|os.O_WRONLY,0644)
                self.title = title
                self.doTitle()

        def doTitle(self):
                s='<html>\n<head><title>'
                s+=self.title+'</title>\n'
                os.write(self.fd,s)
                os.write(self.fd,'<body>\n')

        def doHeader(self,header,mode):
		if mode==0:
                 s='Performance test from'
                 ss=os.path.basename(header)
                 os.write(self.fd,'<h2>%s %s</h2>'%(s,ss[:-4]))
		elif mode==1:
	 	 os.write(self.fd,'<h2>%s</h2>'%(header))

        def doBlk(self, body1, body2, image_path,header=''):
		if header=='': self.doHeader(image_path,0)
                else: self.doHeader(header,1)

                for x in body1:
                        os.write(self.fd,'<p>\n'+x+'\n')
                self.doImage(image_path)
                for x in body2:
                        os.write(self.fd,'<p>\n'+x+'\n')

        def doImage(self,image_path):
                s='<IMG src=\"'+image_path+'\" alt=\"image\" width=\"900\" height=\"600\">\n'
                os.write(self.fd,s)

        def doFooter(self):
                os.write(self.fd,'</body>\n')
                s='</html>\n'
                os.write(self.fd,s)
                self.closeFD()

        def doPage(self,body1,body2,image_path): #deprecated
                self.doBlk(body1,body2,image_path)
                self.doFooter()

        def closeFD(self):
                os.close(self.fd)
