import os
import webbrowser
import subprocess

def doc(remote=False):
    if remote:
        return webbrowser.open("https://casa.nrao.edu/casadocs/")
    else:
        path = casa['dirs']['doc'] + "/casa.nrao.edu/casadocs/stable.html"
        if os.path.exists(path):
            return webbrowser.open("file://" + path)
        else:
            print "local documentation tree not found..."
            return False

def doc_fetch( ):
    if casa['dirs']['doc'] is None:
        print "casa['dirs']['doc'] has not been set..."
        return False
    if not os.path.exists(casa['dirs']['doc']):
        print ("directory %s does not exist..." % casa['dirs']['doc'])
        return False
    url = "https://casa.nrao.edu/casadocs/stable"
    return subprocess.call( [ "wget", "--no-parent", "--no-check-certificate", "--html-extension", "--convert-links", "--recursive",
                              "--level=inf", "--page-requisites", "-e", "robots=off", "--wait=0", "--quota=inf", "--reject",
                              '*_form,RSS,*login*,logged_in,*logout*,logged_out,createObject*,select_default_page,selectViewTemplate*,object_cut,object_copy,object_rename,delete_confirmation,content_status_*,addtoFavorites,pdf.html,print.html',
                              "--exclude-directories='search,*com_mailto*'", "--directory-prefix=" + casa['dirs']['doc'],
                              "--convert-links", url], stderr=subprocess.STDOUT, stdout=open(os.devnull,"w") )
