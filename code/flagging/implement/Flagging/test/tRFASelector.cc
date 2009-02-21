#include <flagging/Flagging/RFASelector.h>

#include <msvis/MSVis/VisSet.h>

#include <casa/aips.h>
#include <casa/namespace.h>


int main (int,char *)
{
    /*
      First work a bit to an RFASelector object,
      so that we can test its methods
     */
    Float vmin, vmax;
    uInt f0(1);
    Record spec, parm;

    Flagger rf;

    VisibilityIterator vi; 

    VisBuffer vb;
    PGPlotterInterface *pgp_screen(NULL);
    PGPlotterInterface *pgp_report(NULL);

    RFChunkStats chunk(vi, vb, rf, pgp_screen, pgp_report);

    RFASelector s(chunk, parm); 


    // test parseMinMax

    // empty record
    cout << "Calling parseMinMax with: " << vmin << ", " << vmax << ", " << spec << ", " << f0 << endl;
    assert( !s.fortestingonly_parseMinMax( vmin, vmax, spec, f0) );

    // longer record
    spec.define("name", "Selector");
    spec.define("spwid", False);
    spec.define("field", False);
    spec.define("fq", False);
    spec.define("chan", False);
    spec.define("corr", False);
    spec.define("ant", False);
    spec.define("baseline", False);
    spec.define("timerng", False);
    spec.define("autocorr", False);

    Record clip;
    clip.define("expr", "ABS RR");
    clip.define("min", 0.4);
    clip.define("max", 0.5);

    spec.defineRecord("clip", clip);

#if 0
  timeslot: Bool 0
  dtime: Double 10.0000000000000000
  quack: Bool 0
  flagrange: Bool 0
  unflag: Bool 0
  scan: Bool 0
  array: Bool 0
  feed: Bool 0
  uvrange: Bool 0
  id: String "select"
  column: String "CORRECTED"
  reset: Bool 0
#endif

    cout << "Calling parseMinMax with: " << vmin << ", " << vmax << ", " << clip << ", " << f0 << endl;
    assert( s.fortestingonly_parseMinMax( vmin, vmax, clip, f0) );
    cout << "vmin: " << vmin << endl;
    cout << "vmax: " << vmax << endl;
    Double epsilon(0.001);
    assert( fabs(vmin - 0.4) < epsilon );
    assert( fabs(vmax - 0.5) < epsilon );

    // test parseClipField
    Bool c = True;
    cout << "Calling parseClipField with: " << spec << ", " << c << endl;

    s.fortestingonly_parseClipField(clip, c);

    return 0;
}
