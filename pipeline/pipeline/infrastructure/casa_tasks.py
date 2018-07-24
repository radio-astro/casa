"""
This module contains a wrapper function for every CASA task. The signature of
each methods exactly matches that of the CASA task it mirrors. However, rather
than executing the task directly when these methods are called,
CASATaskJobGenerator returns a JobRequest for every invocation; these jobs
then be examined and executed at a later date.

The CASA task implementations are located at run-time and proxies for each
task attached to this class at runtime. The name and signature of each
method will match those of the tasks in the CASA environment when this
module was imported.
"""
from __future__ import absolute_import

import applycal_pg
import bandpass_pg
import calstat_cli
import clean_cli
import clearcal_cli
import delmod_cli
import exportfits_cli
import flagcmd_cli
import flagdata_pg
import flagmanager_cli
import fluxscale_pg
import gaincal_pg
import gencal_cli
import hanningsmooth_cli
import imdev_cli
import imhead_cli
import immath_cli
import immoments_cli
import impbcor_pg
import importasdm_cli
import imregrid_pg
import imstat_cli
import imsubimage_cli
import initweights_cli
import listobs_cli
import mstransform_cli
import partition_cli
import plotants_cli
import plotbandpass_cli
import plotcal_cli
import plotms_pg
import plotweather_cli
import polcal_cli
import sdbaseline_pg
import sdcal_pg
import sdimaging_cli
import setjy_pg
import split_cli
import statwt_cli
import tclean_pg
import visstat_cli
import wvrgcal_cli

from .jobrequest import JobRequest
from . import logging

LOG = logging.get_logger(__name__)


def applycal(*v, **k):
    return _get_job(applycal_pg.applycal_pg, *v, **k)

def bandpass(*v, **k):
    return _get_job(bandpass_pg.bandpass_pg, *v, **k)

def calstat(*v, **k):
    return _get_job(calstat_cli.calstat_cli, *v, **k)

def clean(*v, **k):
    return _get_job(clean_cli.clean_cli, *v, **k)

def clearcal(*v, **k):
    return _get_job(clearcal_cli.clearcal_cli, *v, **k)

def delmod(*v, **k):
    return _get_job(delmod_cli.delmod_cli, *v, **k)

def exportfits(*v, **k):
    return _get_job(exportfits_cli.exportfits_cli, *v, **k)

def gaincal(*v, **k):
    return _get_job(gaincal_pg.gaincal_pg, *v, **k)

def flagcmd(*v, **k):
    return _get_job(flagcmd_cli.flagcmd_cli, *v, **k)

def flagdata(*v, **k):
    return _get_job(flagdata_pg.flagdata_pg, *v, **k)

def flagmanager(*v, **k):
    return _get_job(flagmanager_cli.flagmanager_cli, *v, **k)

def fluxscale(*v, **k):
    # work around problem with fluxscale_pg that says:
    # An error occurred running task fluxscale: in method 'calibrater_fluxscale', argument 15 of type 'bool'
    if 'display' not in k:
        k['display'] = False
    return _get_job(fluxscale_pg.fluxscale_pg, *v, **k)

def gencal(*v, **k):
    return _get_job(gencal_cli.gencal_cli, *v, **k)

def hanningsmooth(*v, **k):
    return _get_job(hanningsmooth_cli.hanningsmooth_cli, *v, **k)

def imdev(*v, **k):
    return _get_job(imdev_cli.imdev_cli, *v, **k)

def imhead(*v, **k):
    return _get_job(imhead_cli.imhead_cli, *v, **k)

def immath(*v, **k):
    return _get_job(immath_cli.immath_cli, *v, **k)

def immoments(*v, **k):
    return _get_job(immoments_cli.immoments_cli, *v, **k)

def imregrid(*v, **k):
    return _get_job(imregrid_pg.imregrid_pg, *v, **k)

def impbcor(*v, **k):
    return _get_job(impbcor_pg.impbcor_pg, *v, **k)

def importasdm(*v, **k):
    return _get_job(importasdm_cli.importasdm_cli, *v, **k)

def imstat(*v, **k):
    return _get_job(imstat_cli.imstat_cli, *v, **k)

def imsubimage(*v, **k):
    return _get_job(imsubimage_cli.imsubimage_cli, *v, **k)

def initweights(*v, **k):
    return _get_job(initweights_cli.initweights_cli, *v, **k)

def listobs(*v, **k):
    return _get_job(listobs_cli.listobs_cli, *v, **k)

def mstransform(*v, **k):
    return _get_job(mstransform_cli.mstransform_cli, *v, **k)

def partition(*v, **k):
    return _get_job(partition_cli.partition_cli, *v, **k)

def plotants(*v, **k):
    return _get_job(plotants_cli.plotants_cli, *v, **k)

def plotbandpass(*v, **k):
    return _get_job(plotbandpass_cli.plotbandpass_cli, *v, **k)

def plotcal(*v, **k):
    return _get_job(plotcal_cli.plotcal_cli, *v, **k)

def plotms(*v, **k):
    return _get_job(plotms_pg.plotms_pg, *v, **k)

def plotweather(*v, **k):
    return _get_job(plotweather_cli.plotweather_cli, *v, **k)

def polcal(*v, **k):
    return _get_job(polcal_cli.polcal_cli, *v, **k)

def setjy(*v, **k):
    return _get_job(setjy_pg.setjy_pg, *v, **k)

def split(*v, **k):
    return _get_job(split_cli.split_cli, *v, **k)

def statwt(*v, **k):
    return _get_job(statwt_cli.statwt_cli, *v, **k)

def statwt2(*v, **k):
    return _get_job(statwt2_cli.statwt2_cli, *v, **k)

def tclean(*v, **k):
    return _get_job(tclean_pg.tclean_pg, *v, **k)

def wvrgcal(*v, **k):
    return _get_job(wvrgcal_cli.wvrgcal_cli, *v, **k)

def visstat(*v, **k):
    return _get_job(visstat_cli.visstat_cli, *v, **k)

def uvcontfit(*v, **k):
    # Note this is pipeline CASA style task not a CASA task
    import pipeline.hif.cli.task_uvcontfit as task_uvcontfit
    return _get_job(task_uvcontfit.uvcontfit, *v, **k)

def sdimaging(*v, **k):
    return _get_job(sdimaging_cli.sdimaging_cli, *v, **k)

def sdcal(*v, **k):
    return _get_job(sdcal_pg.sdcal_pg, *v, **k)

def sdbaseline(*v, **k):
    return _get_job(sdbaseline_pg.sdbaseline_pg, *v, **k)

def _get_job(task, *v, **k):
    return JobRequest(task, *v, **k)
