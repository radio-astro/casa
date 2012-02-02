#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <measures/Measures/UVWMachine.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCBaseline.h>

using namespace casa;

int main()
{

    MeasurementSet ms("/tmp/ngc5921.ms");//, Table::Update);
    ROMSColumns msc(ms);


    //    ROMSFieldColumns msfcs(msc.field());

    //MDirection pd0(msc.field().phaseDirMeas(0));

    //    cout << msc << endl;
    
    for (unsigned row = 0; row < msc.field().nrow(); row++) {
        
        cout << "Phase direction = " << msc.field().phaseDirMeas(row, 1423) << endl; //.getType() << endl;
        cout << "Delay direction = " << msc.field().delayDirMeas(row, 1423) << endl;
        cout << "Reference direction = " << msc.field().referenceDirMeas(row, 1423) << endl;
    }


    for (unsigned row = 0; row < msc.antenna().nrow(); row++) {
        cout << "diameter = " << msc.antenna().dishDiameter()(row) 
             << ", flagrow = " << msc.antenna().flagRow()(row)
             << ", mount = " << msc.antenna().mount()(row)
            //             << ", type = " << msc.antenna().type()(row)
             << ", name = " << msc.antenna().name()(row)
             << ", position = " << msc.antenna().positionMeas()(row)
             << ", offset = " << msc.antenna().offset()(row)
             << endl;
    }

    /*
      Pseudo-code for computing the UVW:

      Get field direction  d(t)  (in J2000)

      Get antenna positions p1, p2  (in ITRF)

      Compute baseline b = p2 - p1
      Convert baseline to J2000

      Project the antenna positions onto the field plane normal to d(t)

      Then uvw = difference in projected positions


      // See also derviedmscal/DerivedMC/MSCalEngine.cc.

     */

    const ROMSAntennaColumns &msac = msc.antenna();
    const ROScalarMeasColumn<MPosition> &position_column = msac.positionMeas();
    //    const ROScalarMeasColumn<MPosition> &offset_column = msac.offsetMeas();

    MeasRef<MPosition> meas_ref(position_column.getMeasRef());
    cout << "Position Measure reference: " << meas_ref << endl;
    cout << "Position Measure reference: " << meas_ref.showMe() << endl;
    cout << "Position Measure reference: " << meas_ref.getType() << endl;

    ROScalarColumn<Double> times(ROMSMainColumns(ms).time());
    ROScalarColumn<Int> fields(ROMSMainColumns(ms).fieldId());
    ROScalarMeasColumn<Muvw> uvw(ROMSMainColumns(ms).uvwMeas());
    //    ROArrayColumn<Double> uvw(ROMSMainColumns(ms).uvw());
    ROScalarColumn<Int> antenna1(ROMSMainColumns(ms).antenna1());
    ROScalarColumn<Int> antenna2(ROMSMainColumns(ms).antenna2());

    for (unsigned i = 31; i < 32; i++) {
        cout << "row = " << i << endl;
        cout << "time = " << times(i) << endl;
        cout << "field = " << fields(i) << endl;

        MDirection field_dir = msc.field().phaseDirMeas(fields(i), times(i));
        
        cout << "Field direction: " << field_dir << endl;
        cout << "Field direction: " << field_dir.tellMe() << endl;
        cout << "Field direction: " << field_dir.showMe() << endl;
        cout << "Field direction: " << field_dir.getRefString() << endl;
        
        int a1 = antenna1(i);
        int a2 = antenna2(i);

        cout << "antenna1 = " << a1 << endl;
        cout << "antenna2 = " << a2 << endl;

        MEpoch epoch(MVEpoch(times(i)));
        
        // If we don't set the information in the frame, the conversion below
        // will fail. Why isn't this information already in the POSITION column frame?

        meas_ref.getFrame().set(epoch);
        meas_ref.getFrame().set(position_column(a2));
        meas_ref.getFrame().set(field_dir);
        
        MBaseline::Convert toJ2000;
        
        // Set a new model and reference */
        toJ2000.set(MBaseline(), MBaseline::Ref(MBaseline::J2000,
                                                meas_ref.getFrame()));

        // Form an MBaseline (use first antenna as baseline origin).
        Vector<Double> pos1 = position_column(a1).getValue().getVector();
        Vector<Double> pos2 = position_column(a2).getValue().getVector();

        cout << "position1 = " << pos1 << endl;
        cout << "position2 = " << pos2 << endl;

        MVPosition mvpos((pos2[0] - pos1[0]),
                         (pos2[1] - pos1[1]),
                         (pos2[2] - pos1[2]));
        
        MBaseline baseline(MVBaseline(mvpos), MBaseline::ITRF);
        
        cout << "baseline = " << baseline << endl;
        cout << "baseline = " << baseline.getRefString() << endl;

        toJ2000.setModel(baseline);
        
        MVBaseline bas = toJ2000().getValue();
        
        cout << "baseline(J2000) = " << bas << endl;
        
        MVuvw vuvw(bas, field_dir.getValue());

        Vector<double> antUvw = Muvw(vuvw, Muvw::J2000).getValue().getVector();
        
        cout << "antUvw = " << antUvw << endl;

        /* Should agree with the recomputed UVW, but doesn't. */
        cout << "reference uvw = " << uvw(i) << endl;
        cout << "reference uvw = " << uvw.getMeasRef() << endl;
    }


    return 0;
}
