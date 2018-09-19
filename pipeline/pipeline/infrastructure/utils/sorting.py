"""
The sorting module contains utility functions used to sort pipeline input and
output.
"""
import re

from .. import logging

LOG = logging.get_logger(__name__)

__all__ = ['natural_sort', 'numeric_sort']


def natural_sort(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower() for text in re.split(_nsre, s)]


def _string_split_by_numbers(x):
    r = re.compile('(\d+)')
    return [int(y) if y.isdigit() else y for y in r.split(x)]


def numeric_sort(l):
    """
    Sort a list numerically, eg.

    ['9,11,13,15', '11,13', '9'] -> ['9', '9,11,13,15', '11,13']
    """
    return sorted(l, key=_string_split_by_numbers)
