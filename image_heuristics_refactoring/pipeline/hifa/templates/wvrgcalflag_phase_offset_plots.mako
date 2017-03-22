<%!
rsc_path = ""
import collections
SELECTORS = ['vis', 'spw', 'ant']
HISTOGRAM_LABELS = collections.OrderedDict([
	('ratio', 'Ratio of phase RMS before/after WVR application')
])

# set all X-axis labels to Kelvin
HISTOGRAM_AXES = collections.OrderedDict([
	('ratio', 'PLOTS.xAxisLabels["WVR phase RMS"]')
])
%>
<%inherit file="generic_x_vs_y_spw_ant_plots.mako"/>
