import xml.etree.cElementTree as eltree
from xml.dom import minidom

class PipelineManifest(object):
    """
    Class for creating the pipeline data product manifest 
    """
    def __init__(self, ouss_id):
        self.ouss_id = ouss_id
	self.piperesults = eltree.Element("piperesults", name=ouss_id)

    def set_ous(self, ous_name):
	"""
        Set an OUS element and return it 
	"""
        return eltree.SubElement(self.piperesults, "ous", name=ous_name)

    def set_session (self, ous, session_name):
	"""
        Set a SESSION element in an OUS element and return it 
	"""
	return eltree.SubElement (ous, "session", name=session_name)

    def add_caltables (self, session, caltables_file):
        eltree.SubElement(session, "caltables", name=caltables_file)

    def add_asdm (self, session, asdm_name, flags_file, calapply_file):
	"""
        Add an ASDM element to a SESSION element
	"""
        asdm = eltree.SubElement (session, "asdm", name=asdm_name)
	eltree.SubElement(asdm, "finalflags", name=flags_file)
	eltree.SubElement(asdm, "applycmds", name=calapply_file)

    def add_pprfile(self, ous, ppr_file):
	"""
        Add the pipeline processing request file to the OUS element
	"""
        eltree.SubElement (ous, "piperequest", name=ppr_file)

    def add_images(self, ous, imagelist, type):
	"""
        Add a list of images to the OUS element
	"""
        for image in imagelist:
	    eltree.SubElement(ous, "image", name=image, imtype=type)

    def add_pipescript(self, ous, pipescript):
	"""
        Add the pipeline processing script to the OUS element
	"""
        eltree.SubElement (ous, "pipescript", name=pipescript)

    def add_restorescript(self, ous, restorescript):
	"""
        Add the pipeline restore script to the OUS element
	"""
        eltree.SubElement (ous, "restorescript", name=restorescript)

    def add_weblog (self, ous, weblog):
	"""
        Add the weblog to the OUS element
	"""
        eltree.SubElement (ous, "weblog", name=weblog)

    def add_casa_cmdlog (self, ous, casa_cmdlog):
	"""
        Add the CASA commands log to the OUS element
	"""
        eltree.SubElement (ous, "casa_cmdlog", name=casa_cmdlog)

    def add_aqua_report (self, ous, aqua_report):
	"""
        Add the AQAUA report to the OUS element
	"""
        eltree.SubElement (ous, "aqua_report", name=aqua_report)

    def write (self, filename):
	"""
        Convert the document to a nicely formatted XML string
	and save it in a file
	"""
        xmlstr = eltree.tostring(self.piperesults, 'utf-8')

	# Reformat it to prettyprint style
	reparsed = minidom.parseString(xmlstr)
	reparsed_xmlstr = reparsed.toprettyxml(indent="  ")

	# Save it to a file.
        with open (filename, "w") as manifestfile:
            manifestfile.write(reparsed_xmlstr)

