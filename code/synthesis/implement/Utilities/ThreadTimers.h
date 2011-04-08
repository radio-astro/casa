//-*-C++-*-
#include <time.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <msvis/MSVis/UtilJ.h>

#ifndef SYNTHESIS_THREADTIMERS_H
#define SYNTHESIS_THREADTIMERS_H
namespace casa {
  
  class Timers
  {
  public:
    Timers () { * this = getTime();}
    Double cpu () const { return cpu_p;}
    Double elapsed () const { return elapsed_p;}
    
    static Timers
    getTime (){
      
      struct timeval tVal;
      gettimeofday (& tVal, NULL);
      
      Double elapsed = tVal.tv_sec + tVal.tv_usec * 1e-6;
      
      //Double cpu = ((Double) clock ()) / CLOCKS_PER_SEC; // should be in seconds
      
      struct rusage usage;
      
      int failed = getrusage (RUSAGE_THREAD, & usage);
      //      assert (! failed);
      
      Double cpu = toSeconds (usage.ru_utime) + toSeconds (usage.ru_stime);
      
      return Timers (elapsed, cpu);
    }
    
    static Double
    toSeconds (const struct timeval & t)
    {
      return t.tv_sec + t.tv_usec * 1e-6;
    }
    
  protected:
    
    Double cpu_p;
    Double elapsed_p;
    
    Timers (Double elapsed, Double cpu) : cpu_p (cpu), elapsed_p (elapsed) {}
  };
  
  class DT : private Timers
  {
    friend DT operator- (const Timers & tLater, const Timers & tEarlier);
  public:
    
    DT () : Timers (0, 0), n_p (0) {}
    
    DT &
    operator += (const DT & other)
    {
      cpu_p += other.cpu();
      elapsed_p += other.elapsed();
      n_p += 1;
      
      return * this;
    }
    
    Double cpu () const { return Timers::cpu();}
    Double elapsed () const { return Timers::elapsed();}
    Int n() const { return n_p;}
    
    String
    formatAverage (const String & floatFormat = "%6.1f",
                   Double scale=1000.0,
                   const String & units = "ms")  // to convert to ms
      const
    {
      String realFormat = casa::utilj::format ("(el=%s,cp=%s,%%4.1f%%%%) %s",
					       floatFormat.c_str(), floatFormat.c_str(), units.c_str());
      Int n = n_p != 0 ? n_p : 1;
      Double c = cpu_p / n * scale;
      Double e = elapsed_p / n * scale;
      Double p = c / e * 100;
      
      String result = casa::utilj::format (realFormat.c_str(), e, c, p);
      
      return result;
    }
    
  protected:
    
    DT (Double elapsed, Double cpu, Int n) : Timers (elapsed, cpu), n_p (0) {}
    
  private:
    
    Int n_p;
  };
  
  DT operator- (const Timers & tLater, const Timers & tEarlier);
}

#endif
