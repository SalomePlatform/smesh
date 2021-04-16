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

LOGFILE = os.path.join(tempfile.gettempdir(),"blocFissure.log")

LOG_LEVEL = warning

logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s', \
                    level=logging.WARNING, \
                    filename=LOGFILE, filemode='w')
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
  global LOG_LEVEL
  LOG_LEVEL = debug
  level = logging.DEBUG
  formatter = logging.Formatter('%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start Debug %s', LOG_LEVEL)

def setVerbose(logfile=None):
  """setVerbose"""
  global LOG_LEVEL
  LOG_LEVEL = info
  level = logging.INFO
  formatter = logging.Formatter('%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start Verbose %s', LOG_LEVEL)

def setRelease(logfile=None):
  """setRelease"""
  global LOG_LEVEL
  LOG_LEVEL = warning
  level = logging.WARNING
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.warning('start Release %s', LOG_LEVEL)

def setUnitTests(logfile=None):
  """setUnitTests"""
  global LOG_LEVEL
  LOG_LEVEL = critical
  level = logging.CRITICAL
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.critical('start UnitTests %s', LOG_LEVEL)

def setPerfTests(logfile=None):
  """setPerfTests"""
  global LOG_LEVEL
  LOG_LEVEL = critical
  level = logging.CRITICAL
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start PerfTests %s', LOG_LEVEL)

def setAlways(logfile=None):
  """setAlways"""
  global LOG_LEVEL
  LOG_LEVEL = always
  level = logging.CRITICAL
  formatter = logging.Formatter('%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start Always %s', LOG_LEVEL)

def getLogLevel():
  """getLogLevel"""
  return LOG_LEVEL
