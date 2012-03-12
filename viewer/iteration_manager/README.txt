This directory contains the deadend of work on:

https://bugs.nrao.edu/browse/CAS-2794

it added an "iteration manager" which was going to align channels from
different cubes based on approximate matching.

This worked this out to the point where the iteration manager had the
reference vectors for all of the display data cubes, it sorted them,
and then populated slots with the last channel of a given cube,
essentially spearing a channel from one cube across a number of
channels of the second cube.

This sort of lineup didn't seem particularly useful (scientifically),
and the potential mismatch of rasters/contours that were being displayed
at the same time coupled with the inconsistencies already programmed into
the viewer (e.g. field of view (RA/DEC) for an image is set by the controlling
cube, but the length of the movie is controlled by the longest cube) made
this less than desirable.
