from asap import scantable
from matplotlib.pylab import *
from numpy import array, ma, logical_not
from numpy.fft import fft, ifft

class lagplotter(object):
    def __init__(self, stable):
        if not isinstance(stable, scantable):
            raise TypeError("This works only on scantables")
        self._scan = stable
        self.figure = None
        self.dataaxes = None
        self.fftaxes = None
        self.resultaxes = None
        self.flags = []

    def _init_plot(self):
        if not self.figure:
            self.figure = figure()
            self.dataaxes = self.figure.add_subplot(3,1,1)
            self.fftaxes = self.figure.add_subplot(3,1,2)
            self.resultaxes = self.figure.add_subplot(3,1,3)
        else:
            self.dataaxes.cla()
            self.fftaxes.cla()
            self.resultaxes.cla()


    def flag(self):
        self._init_plot()
        for i in xrange(len(self._scan)):
            self.dataaxes.cla()
            self.fftaxes.cla()
            self.resultaxes.cla()
            x = array(self._scan.get_abcissa(i)[0])
            y = array(self._scan.get_spectrum(i))
            msk = self._scan.get_mask(i)
            marr = ma.MaskedArray(y, logical_not(msk), fill_value=0.0)
            self.dataaxes.plot(x, marr)
            nfft = len(marr)/2+1
            yfft = fft(marr.filled())
            self.fftaxes.semilogy(abs(yfft)[0:nfft])
            self.figure.show()
            raw_input("Press any key to continue...")
            print "Now select a start and end point by clicking on the middle plot"
            print "Start point ..."
            flagstart = int(ginput(show_clicks=False)[0][0]+0.5)
            print "End point ..."
            flagend = int(ginput(show_clicks=False)[0][0]+0.5)
            xfft = range(len(yfft))
            self.fftaxes.axvspan(flagstart, flagend, alpha=0.3)
            yfft[flagstart:flagend] = 0+0j
            yfft[-flagend:-flagstart] = 0+0j
            yi = ifft(yfft)
            self.resultaxes.plot(x, yi)
            self.figure.show()
            inp = raw_input("Commit flags (c), keep (k) or ignore(i)? ")\
                            .lower()
            if inp.startswith("c"):
                self.flags.append([flagstart, flagend])
                self._scan.set_spectrum(yi.real, i)
            elif inp.startswith("k"):
                self.flags.append([flagstart, flagend])
            else:
                del self.fftaxes.patches[-1]
            cont = raw_input("Continue (c) or quit (q)? ")
            if not cont == "c":
                return self.flags
        return self.flags
