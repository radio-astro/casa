import os
import datetime
import webbrowser
import subprocess
import sys
import xml.etree.cElementTree as ET
import urllib2

class __doc(object):
    "command-line Plone help"

    def __init__( self ):
        self.local_toc = None
        self.remote_toc = None


    def __call__( self, sec=None, remote=False ):
        if remote:
            if sec is None:
                return webbrowser.open("https://casa.nrao.edu/casadocs/")
            else:
                def show_toc( ):
                    width = max(len(key) for key in self.remote_toc.keys( ))+3
                    for i in self.remote_toc.keys( ):
                        print "".join([i.ljust(width),self.remote_toc[i]['desc']])

                def entry_to_dict(acc,e):
                    if e.tag == 'entry':
                        acc[e.find('key').text] = {
                            'desc': e.find('desc').text,
                            'type': e.find('type').text,
                            'path': e.find('path').text }
                    return acc
                if self.remote_toc is None:
                    self.remote_toc =  reduce( entry_to_dict, ET.ElementTree(file=urllib2.urlopen('https://casa.nrao.edu/PloneResource/stable/toc.xml')).getroot( ).getchildren( ), { } )
                if sec == 'toc':
                    show_toc( )
                elif self.remote_toc.has_key(sec):
                    return webbrowser.open("https://casa.nrao.edu/casadocs/stable/" + self.remote_toc[sec]['path'])
                else:
                    print "Sorry '%s' is not a recognized section..." % sec
                    print "-------------------------------------------------------------"
                    show_toc( )
        else:
            path = casa['dirs']['doc'] + "/casa.nrao.edu/casadocs/stable.html"
            if os.path.exists(path):
                return webbrowser.open("file://" + path)
            else:
                print "local documentation tree not found..."
                return False


    def fetch( self ):
        if casa['dirs']['doc'] is None:
            print "casa['dirs']['doc'] has not been set..."
            return False
        if not os.path.exists(casa['dirs']['doc']):
            print ("directory %s does not exist..." % casa['dirs']['doc'])
            return False

        ## rename existing directory
        if os.path.exists(casa['dirs']['doc'] + "/casa.nrao.edu"):
            path = casa['dirs']['doc'] + "/casa.nrao.edu"
            now = datetime.datetime.now( ).isoformat('-')
            os.rename(path, path + "." + now)

        print "this will take some time..."
        print "relax..."
        print "do not hit ^C ..."
        print "do not expect output..."
        url = "https://casa.nrao.edu/casadocs/stable"

        wgetcmd = "wget"

        if sys.platform == "darwin":
            wgetcmd = casa['dirs']['root'] + "/Resources/wget"

        return subprocess.call( [ wgetcmd, "--no-parent", "--no-check-certificate", "--html-extension", "--convert-links", "--recursive",
                                  "--level=inf", "--page-requisites", "-e", "robots=off", "--wait=0", "--quota=inf", "--reject",
                                  '*_form,RSS,*login*,logged_in,*logout*,logged_out,createObject*,select_default_page,selectViewTemplate*,object_cut,object_copy,object_rename,delete_confirmation,content_status_*,addtoFavorites,pdf.html,print.html',
                                  "--exclude-directories='search,*com_mailto*'", "--directory-prefix=" + casa['dirs']['doc'],
                                  "--convert-links", url], stderr=subprocess.STDOUT, stdout=open(os.devnull,"w") )


doc = __doc( )
