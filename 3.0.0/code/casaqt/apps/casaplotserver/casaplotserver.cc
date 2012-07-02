#include <signal.h>
#include <math.h>
#include <QApplication>
#include <casaqt/QtPlotServer/QtPlotServer.qo.h>
#include <casaqt/QtPlotServer/QtPlotSvrPanel.qo.h>

int main( int argc, char *argv[ ] ) {

    // don't let ^C [from casapy] kill the plot server...
//     signal(SIGINT,SIG_IGN);

    QApplication qapp(argc, argv); 

    casa::QtPlotServer plot_server;

    return qapp.exec();

#if 0
    casa::QtPlotSvrPanel plot_server2("my test plot","bottom");
    plot_server2.show( );

#if 0
#define POINTS 2000
#define STEP   0.05
#else
#define POINTS 100
#define STEP   1
#endif

    std::vector<double> x(POINTS);
    std::vector<double> y(POINTS);
    std::vector<double> y2(POINTS);
    x[0] = 0;
    y[0] = sin(x[0]);
    y2[0] = (x[0]*x[0])/10000.0;
//     y2[0] = 0.5 * cos(x[0]);
    const double increment=STEP;
    for ( int i=1; i < x.size( ); ++i ) {
	x[i] = x[i-1] + increment;
	y[i] = sin(x[i]);
// 	y2[i] = 0.5 * cos(x[i]);
	y2[i] = (x[i]*x[i])/10000.0;
    }

    plot_server2.scatter(x,y,"green","symbols");
    plot_server2.line(x,y2,"blue","second line");
    return qapp.exec();
#endif
}
