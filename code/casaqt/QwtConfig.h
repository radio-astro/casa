#pragma once
#include <qwt_global.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_curve.h>
#include <qwt_color_map.h>
#if QWT_VERSION >= 0x060000
#include <qwt_compat.h>
#include <qwt_series_data.h>
#include <qwt_plot_canvas.h>

typedef QwtIntervalSeriesData qwt_interval_t;
inline void set_data(QwtPlotCurve *curve, const QVector<double> &x, const QVector<double> &y) {
  curve->setSamples(x,y);
}
inline void set_color_map( QwtScaleWidget *scale, QwtDoubleInterval &range, const QwtLinearColorMap &colorMap ) {
  QwtLinearColorMap* copyMap = new QwtLinearColorMap( colorMap.color1( ), colorMap.color2( ) );
  QVector<double> colorStops = colorMap.colorStops();
  for ( int i = 1; i < colorStops.size() - 1; ++i) {
    copyMap->addColorStop( colorStops[i], QColor(colorMap.rgb(QwtInterval(0,1), colorStops[i])) );
  }
  scale->setColorMap( range, copyMap );
}
template<class T> inline void set_symbol( T *qwt_thing, const QwtSymbol &sym ) {
  qwt_thing->setSymbol(&sym);
}
#else
#include <qwt_interval_data.h>
typedef QwtIntervalData qwt_interval_t;
inline void set_data(QwtPlotCurve *curve, const QVector<double> &x, const QVector<double> &y) {
  curve->setData(x,y);
}
inline void set_color_map( QwtScaleWidget *scale, QwtDoubleInterval &range, const QwtLinearColorMap &colorMap ) {
  scale->setColorMap( range, colorMap );
}
template<class T> inline void set_symbol( T *qwt_thing, const QwtSymbol &sym ) {
  qwt_thing->setSymbol(sym);
}
#endif
