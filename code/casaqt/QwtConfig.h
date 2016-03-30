#pragma once
#include <qwt_global.h>
#if QWT_VERSION >= 0x060000
#include <qwt_compat.h>
#include <qwt_series_data.h>
#include <qwt_plot_canvas.h>

typedef QwtIntervalSeriesData qwt_interval_t;
#else
#include <qwt_interval_data.h>
typedef QwtIntervalData qwt_interval_t;
#endif
