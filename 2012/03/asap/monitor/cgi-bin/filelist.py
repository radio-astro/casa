#!/usr/bin/python
import os
import sre
from obsconfig import observatory

class FileList:

    def __init__(self, loc=None, projectcode=None):
        self.message ="""Content-Type: text/xml

<?xml version="1.0" encoding="ISO-8859-1"?>"""
        self.error = None
        if loc is None:
            loc = observatory['rpfpath'][0]
        else:
            loc = int(loc)
            if loc< 0 or loc >=len(observatory['rpfpath']):
                 self.error = "Invalid Path"
                 return
            else:
                loc = observatory['rpfpath'][loc]
        if os.path.exists(loc) and os.path.isdir(loc):
            rx = sre.compile("\d{4}-\d{2}-d{2}_\d{4}-M\d{3}.rpf.*")
            self.files = [ f for f in os.listdir(loc) if sre.match(rx, f) ]
	    if projectcode is not None:
		self.files = [ f for f in self.files if sre.match(projectcode,
								  f) ]
            if len(self.files) == 0:
                self.error = "No rpfits files found"
        else:
            self.error = "Invalid Path"


    def __str__(self):
        if self.error:
            self.message += "<Error>\n"
            self.message += self.error
            self.message += "</Error>\n"
        else:
            self.message += "<Listing>\n"
            for s in self.files:
                self.message += "<File>" + s + "</File>\n"
            self.message += "</Listing>\n"
        return self.message
if __name__ == "__main__":
    import cgi
    form = cgi.FieldStorage()
    pfilter = None
    if form.has_key('project'):
	pfilter = form.getfirst("project", None)
    if form.has_key('path'):
        pathindex = form.getfirst("path",0)
        print FileList(pathindex, pfilter)
    else:
        print FileList()
