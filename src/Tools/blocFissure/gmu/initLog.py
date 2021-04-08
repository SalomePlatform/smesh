# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
"""Initialisations des impressions"""

import logging
import os
import tempfile

debug = 10
info = 20
warning = 30
error = 40
critical = 50
always = 100

filelog = os.path.join(tempfile.gettempdir(),"blocFissure.log")

loglevel = warning

logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s', \
                    level=logging.WARNING, \
                    filename=filelog, filemode='w')
ch = None
fh = None

def setLogger(logfile, level, formatter):
  """setLogger"""
  global ch, fh
  rootLogger = logging.getLogger('')
  if fh is not None:
    rootLogger.removeHandler(fh)
    fh = None
  if ch is not None:
    rootLogger.removeHandler(ch)
    ch = None
  if logfile:
    if os.path.exists(logfile):
      os.remove(logfile)
    fh = logging.FileHandler(logfile)
    rootLogger.addHandler(fh)
    fh.setFormatter(formatter)
  else:
    ch = logging.StreamHandler()
    rootLogger.addHandler(ch)
    ch.setFormatter(formatter)
  rootLogger.setLevel(level)

def setDebug(logfile=None):
  """setDebug"""
  global loglevel
  loglevel = debug
  level = logging.DEBUG
  formatter = logging.Formatter('%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start Debug %s', loglevel)

def setVerbose(logfile=None):
  """setVerbose"""
  global loglevel
  loglevel = info
  level = logging.INFO
  formatter = logging.Formatter('%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start Verbose %s', loglevel)

def setRelease(logfile=None):
  """setRelease"""
  global loglevel
  loglevel = warning
  level = logging.WARNING
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.warning('start Release %s', loglevel)

def setUnitTests(logfile=None):
  """setUnitTests"""
  global loglevel
  loglevel = critical
  level = logging.CRITICAL
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.critical('start UnitTests %s', loglevel)

def setPerfTests(logfile=None):
  """setPerfTests"""
  global loglevel
  loglevel = critical
  level = logging.CRITICAL
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start PerfTests %s', loglevel)

def getLogLevel():
  """getLogLevel"""
  return loglevel
