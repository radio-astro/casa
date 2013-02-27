#*******************************************************************************
# ALMA - Atacama Large Millimeter Array
# Copyright (c) NAOJ - National Astronomical Observatory of Japan, 2011
# (in the framework of the ALMA collaboration).
# All rights reserved.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#*******************************************************************************
#
# $Revision: 1.3.2.2 $
# $Date: 2012/04/02 15:32:42 $
# $Author: tnakazat $
#
SDFlagRule = {\
    'WeatherFlag':\
        {'isActive': False, \
         'Reversible': False, \
         'Threshold': 'unknown'}, \
    'TsysFlag':\
        {'isActive': True, \
         'Reversible': False, \
         'Threshold': 3.0}, \
    'UserFlag':\
        {'isActive': True, \
         'Reversible': False, \
         'Threshold': 'unknown'}, \
    'RmsPreFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         'Threshold': 4.5}, \
         #'Threshold': 2.5}, \
    'RmsPostFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         'Threshold': 4.0}, \
    'RunMeanPreFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         'Threshold': 5.5, \
         'Nmean': 5}, \
    'RunMeanPostFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         'Threshold': 5.0, \
         'Nmean': 5}, \
    'RmsExpectedPreFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         #'Threshold': 2.66666}, \
         'Threshold': 3.0}, \
    'RmsExpectedPostFitFlag':\
        {'isActive': True, \
         'Reversible': True, \
         #'Threshold': 1.5}, \
         'Threshold': 1.33333}, \
    'Flagging':\
        {'ApplicableDuration': 'raster'}, \
        #{'ApplicableDuration': 'subscan'}, \
}

# ApplicableDuration: 'raster' | 'subscan'

