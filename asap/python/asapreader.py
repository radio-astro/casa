from asap._asap import stfiller
from asap import print_log

class reader(stfiller):
    """
    This class allows the user to import single dish files
    (rpfits,sdfits,ms).
    The reader reads in integrations from the file and remains at
    the fileposition afterwards.
    Available functions are:

    read() # read until the (current) end of file)

    Example:
        r = reader('/tmp/P389.rpf')
        scans r.read() # reads in the complete file into 'scans'
        print scans    # summarises the contents
        del r          # destroys the reader

    IMPORTANT: Due to limitations in the rpfits library, only one reader
               can be created at a time.
               r = reader('XYZ.rpf')
               r2 = reader('ABC.rpf')
               is NOT possible. This is a limitation affecting
               rpfits ONLY.
    """

    def __init__(self, filename, unit=None, theif=None, thebeam=None):
        self.unit = unit
        """
        Parameters:
            filename:    the name of an rpfits/sdfits/ms file on disk
            unit:        brightness unit; must be consistent with K or Jy.
                         The default is that a unit is set depending on
                         the telescope.  Setting this over-rides that choice.
            theif:       select a specific IF (default is all)
            thebeam:     select a specific beam (default is all)
        Example:
            r = reader('/tmp/2001-09-01_0332_P363.rpf', theif=2)
        """
        if theif is None:
            theif = -1
        if thebeam is None:
            thebeam = -1
        from os.path import expandvars
        filename = expandvars(filename)
        stfiller.__init__(self, filename, theif, thebeam)
        print_log()

    def read(self):
        """
        Reads in all integrations in the data file.
        """
        from asap import scantable
        from asap import asaplog
        asaplog.push("Reading integrations from disk...")
        stfiller._read(self)
        tbl = stfiller._getdata(self)
        if self.unit is not None:
            tbl.set_fluxunit(self.unit)
        print_log()
        return scantable(tbl)

    def close(self):
        """
        Close the reader.
        """
        self._close()

    def summary(self, name=None):
        """
        Print a summary of all scans/integrations. This reads through the
        whole file once.
        Parameters:
             None
        Example:
             r.summary()
        """
        stfiller._read(self)
        from asap import scantable
        tbl = scantable(stfiller._getdata(self))
        tbl.summary(name)
        del tbl
        return
