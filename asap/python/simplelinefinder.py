import math
from asap.logging import asaplog

class StatCalculator:
   def __init__(self):
       self.s=0.
       self.s2=0.
       self.cnt=0

   def mean(self):
       if self.cnt<=0:
          raise RuntimeError, "At least one data point has to be defined"
       return self.s/float(self.cnt)

   def variance(self):
       if self.cnt<=1:
          raise RuntimeError, "At least two data points has to be defined"
       return math.sqrt((self.s2/self.cnt)-(self.s/self.cnt)**2+1e-12)

   def rms(self):
       """
          return rms of the accumulated sample
       """
       if self.cnt<=0:
          raise RuntimeError, "At least one data point has to be defined"
       return math.sqrt(self.s2/self.cnt)

   def add(self, pt):
       self.s = self.s + pt
       self.s2 = self.s2 + pt*pt
       self.cnt = self.cnt + 1


class simplelinefinder:
   '''
       A simplified class to search for spectral features. The algorithm assumes that the bandpass
       is taken out perfectly and no spectral channels are flagged (except some edge channels).
       It works with a list or tuple rather than a scantable and returns the channel pairs.
       There is an optional feature to attempt to split the detected lines into components, although
       it should be used with caution. This class is largely intended to be used with scripts.

       The fully featured version of the algorithm working with scantables is called linefinder.
   '''

   def __init__(self):
      '''
         Initialize the class.
      '''
      self._median = None
      self._rms = None

   def writeLog(self, str):
      """
         Write user defined string into log file
      """
      asaplog.push(str)

   def invertChannelSelection(self, nchan, chans, edge = (0,0)):
      """
         This method converts a tuple with channel ranges to a tuple which covers all channels
         not selected by the original tuple (optionally edge channels can be discarded)

         nchan - number of channels in the spectrum.
         chans - tuple (with even number of elements) containing start and stop channel for all selected ranges
         edge - one number or two element tuple (separate values for two ends) defining how many channels to reject

         return:  a tuple with inverted channel selection
         Note, at this stage channel ranges are assumed to be sorted and without overlap
      """
      if nchan<=1:
         raise RuntimeError, "Number of channels is supposed to be at least 2, you have %i"% nchan
      if len(chans)%2!=0:
         raise RuntimeError, "chans is supposed to be a tuple with even number of elements"
      tempedge = edge
      if not isinstance(tempedge,tuple):
         tempedge = (edge,edge)
      if len(tempedge)!=2:
         raise RuntimeError, "edge parameter is supposed to be a two-element tuple or a single number"
      if tempedge[0]<0 or tempedge[1]<0:
         raise RuntimeError, "number of edge rejected cannels is supposed to be positive"
      startchan = tempedge[0]
      stopchan = nchan - tempedge[1]
      if stopchan-startchan<0:
         raise RuntimeError, "Edge rejection rejected all channels"
      ranges = []
      curstart = startchan
      for i in range(0,len(chans),2):
          if chans[i+1]<curstart:
             continue
          elif chans[i]<=curstart:
             curstart = chans[i+1]+1
          else:
             ranges.append(curstart)
             ranges.append(chans[i]-1)
             curstart = chans[i+1]+1
      if curstart<stopchan:
         ranges.append(curstart)
         ranges.append(stopchan-1)
      return tuple(ranges)

   def channelRange(self, spc, vel_range):
      """
         A helper method which works on a tuple with abcissa/flux vectors (i.e. as returned by uvSpectrum). It
         allows to convert supplied velocity range into the channel range.

         spc - tuple with the spectrum (first is the vector of abcissa values, second is the spectrum itself)
         vel_range - a 2-element tuple with start and stop velocity of the range

         return: a 2-element tuple with channels
         Note, if supplied range is completely outside the spectrum, an empty tuple will be returned
      """
      if len(spc) != 2:
         raise RuntimeError, "spc supplied to channelRange is supposed to have 2 elements"
      if len(spc[0]) != len(spc[1]):
         raise RuntimeError, "spc supplied to channelRange is supposed to have 2 vectors of equal length"
      if len(vel_range) != 2:
         raise RuntimeError, "vel_range supplied to channelRange is supposed to have 2 elements"
      if vel_range[0]>=vel_range[1]:
         raise RuntimeError, "start velocity is supposed to be less than end velocity, vel_range: %s" % vel_range
      if len(spc[0])<=2:
         raise RuntimeError, "Spectrum should contain more than 2 points, you have %i" % len(spc[0])
      chans = list(vel_range)
      for j in range(len(chans)):
          chans[j] = -1
      for i in range(len(spc[0])):
          if i!=0:
             prev_vel = spc[0][i-1]
          else:
             prev_vel = spc[0][i+1]
          delta = max(prev_vel, spc[0][i]) - min(prev_vel, spc[0][i])
          for j in range(len(vel_range)):
              if abs(vel_range[j]-spc[0][i])<delta:
                  chans[j] = i
      if chans[0] == chans[1]:
         return ()
      if chans[1] == -1:
         chans[1] = len(spc[0])-1
      if chans[0] == -1:
         chans[0] = 0
      if chans[0]<=chans[1]:
         return tuple(chans)
      return (chans[1],chans[0])

   def _absvalComp(self,x,y):
      """
         A helper method to compare x and y by absolute value (to do sorting)

         x - first value
         y - second value

         return -1,0 or 1 depending on the result of comparison
      """
      if abs(x)<abs(y):
         return -1
      elif abs(x)>abs(y):
         return 1
      else:
         return 0

   def rms(self):
      """
         Return rms scatter of the spectral points (with respect to the median) calculated
         during last find_lines call. Note, this method throws an exception if
         find_lines has never been called.
      """
      if self._rms==None:
         raise RuntimeError, "call find_lines before using the rms method"
      return self._rms

   def median(self):
      """
         Return the median of the last spectrum passed to find_lines.
         Note, this method throws an exception if find_lines has never been called.
      """
      if self._median==None:
         raise RuntimeError, "call find_lines before using the median method"
      return self._median

   def _mergeIntervals(self, lines, spc):
      """
         A helper method to merge intervals.

         lines - list of tuples with first and last channels of all intervals
         spc - spectrum (to be able to test whether adjacent intervals have the
               same sign.
      """
      toberemoved = []
      for i in range(1,len(lines)):
          if lines[i-1][1]+1>=lines[i][0]:
             if (spc[lines[i-1][1]]>self._median) == (spc[lines[i][0]]>self._median):
                 lines[i] = (lines[i-1][0],lines[i][1])
                 toberemoved.append(i-1)
      toberemoved.sort()
      for i in range(len(toberemoved)-1,-1,-1):
          if toberemoved[i]>=len(lines):
             raise RuntimeError, "this shouldn't have happened!"
          lines.pop(toberemoved[i])

   def _splitIntervals(self,lines,spc,threshold=3,minchan=3):
      """
         A helper method used in the spectral line detection routine. It splits given intervals
         into a number of "spectral lines". Each line is characterised by a single extremum.
         Noise is dealt with by taking into account only those extrema, where a difference with
         respect to surrounding spectral points exceeds threshold times rms (stored inside this
         class, so the main body of the line detection should be executed first) and there are
         at least minchan such points.
      """
      if minchan<1:
		 raise RuntimeError, "minchan in _splitIntervals is not supposed to be less than 1, you have %s" % minchan
      newlines = []
      for line in lines:
	  if line[1]-line[0]+1 <= minchan:
	     newlines.append(line)
          wasIncreasing = None
          derivSignReversals = []
          for ch in range(line[0]+1,line[1]+1):
              diff=spc[ch]-spc[ch-1]
              isIncreasing = (diff>0)
              if wasIncreasing != None:
                 if isIncreasing != wasIncreasing:
                    derivSignReversals.append((ch,isIncreasing))
              wasIncreasing = isIncreasing
          if len(derivSignReversals)==0:
             newlines.append(line)
          elif len(derivSignReversals)%2 != 1:
             self.writeLog("SplitIntervals warning. Derivative is expected to have odd number of reversals within the interval: \"%s\" " % derivSignReversals);
             newlines.append(line)
          elif derivSignReversals[0][1]!=derivSignReversals[-1][1]:
             self.writeLog("SplitIntervals warning. Derivative is expected to have the same sign at the start and at the end of each interval: \"%s\"" % derivSignReversals)
             newlines.append(line)
          else:
             startchan = line[0]
             for i in range(len(derivSignReversals)):
                if i%2 == 1:
                   newlines.append((startchan,derivSignReversals[i][0]-1))
                   startchan = derivSignReversals[i][0]
             newlines.append((startchan,line[1]))
      return newlines

   def find_lines(self,spc,threshold=3,edge=0,minchan=3, tailsearch = True, splitFeatures = False):
      """
	 A simple spectral line detection routine, which assumes that bandpass has been
	 taken out perfectly and no channels are flagged within the spectrum. A detection
	 is reported if consequtive minchan number of channels is consistently above or
	 below the median value. The threshold is given in terms of the rms calculated
	 using 80% of the lowest data points by the absolute value (with respect to median)

	 spc - a list or tuple with the spectrum, no default
	 threshold - detection threshold, default is 3 sigma, see above for the definition
	 edge - if non-zero, this number of spectral channels will be rejected at the edge.
		Default is not to do any rejection.
	 minchan -  minimum number of consequitive channels exceeding threshold to claim the
		    detection, default is 3.
	 tailsearch - if True (default), the algorithm attempts to widen each line until
		    its flux crosses the median. It merges lines if necessary. Set this
		    option off if you need to split the lines according to some criterion
         splitFeatures - if True, the algorithm attempts to split each detected spectral feature into
                    a number of spectral lines (just one local extremum). The default action is
                    not to do it (may give an adverse results if the noise is high)

	 This method returns a list of tuples each containing start and stop 0-based channel
	 number of every detected line. Empty list if nothing has been detected.

	 Note. The median and rms about this median is stored inside this class and can
	 be obtained with rms and median methods.
      """
      if edge<0:
	 raise RuntimeError, "edge parameter of find_lines should be non-negative, you have %s" % edge
      if 2*edge>=len(spc):
	 raise RuntimeError, "edge is too high (%i), you rejected all channels (%i)" % (edge, len(spc))
      if threshold<=0:
	 raise RuntimeError, "threshold parameter of find_lines should be positive, you have %s" % threshold
      if minchan<=0:
	 raise RuntimeError, "minchan parameter of find_lines should be positive, you have %s" % minchan

      # temporary storage to get statistics, apply edge rejection here
      tmpspc = spc[edge:len(spc)-edge+1]
      if len(tmpspc)<2:
	 raise RuntimeError, "Too many channels are rejected. Decrease edge parameter or provide a longer spectrum."
      tmpspc.sort()
      self._median=tmpspc[len(tmpspc)/2]
      # work with offsets from the median and sort by absolute values
      for i in range(len(tmpspc)):
	  tmpspc[i]-=self._median
      tmpspc.sort(cmp=self._absvalComp)
      sc = StatCalculator()
      for i in tmpspc[:-int(0.2*len(tmpspc))]:
	  sc.add(i)
      self._rms=sc.rms()

      self.writeLog("Spectral line detection with edge=%i, threshold=%f, minchan=%i and tailsearch=%s" % (edge,threshold, minchan, tailsearch))
      self.writeLog("statistics: median=%f, rms=%f" % (self._median, self._rms))

      #actual line detection
      lines=[]
      wasAbove = None
      nchan = 0
      startchan=None
      for i in range(edge,len(spc)-edge):
	  if abs(spc[i]-self._median)>threshold*self._rms:
	     isAbove=(spc[i] > self._median)
	     if nchan!=0:
		if wasAbove == isAbove:
		   nchan+=1
		else:
		   if nchan>=minchan:
		      lines.append((startchan,i-1))
		   nchan=1
		   wasAbove = isAbove
		   startchan = i
	     else:
		nchan=1
		wasAbove = isAbove
		startchan = i
	  else:
	     if nchan>=minchan:
		lines.append((startchan,i-1))
	     nchan = 0
      if nchan>=minchan:
	 lines.append((startchan,len(spc)-edge-1))

      if tailsearch:
	 for i in range(len(lines)):
	     wasAbove = None
	     curRange = list(lines[i])
	     for x in range(curRange[0],edge,-1):
		 isAbove=(spc[x] > self._median)
		 if wasAbove == None:
		    wasAbove = isAbove
		 if isAbove!=wasAbove:
		    curRange[0]=x+1
		    break
	     for x in range(curRange[1],len(spc)-edge):
		 isAbove=(spc[x] > self._median)
		 if isAbove!=wasAbove:
		    curRange[1]=x-1
		    break
	     lines[i]=tuple(curRange)
	 self._mergeIntervals(lines,spc)
      if splitFeatures:
         return self._splitIntervals(lines,spc,threshold,minchan)
      return lines
