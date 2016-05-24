# -*- coding: utf-8 -*-

import logging
import os

debug = 10
info = 20
warning = 30
error = 40
critical = 50

loglevel = warning
logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                    level=logging.WARNING)
ch = None
fh = None

def setLogger(logfile, level, formatter):
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
  global loglevel
  loglevel = debug
  level = logging.DEBUG
  formatter = logging.Formatter('%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start Debug %s', loglevel)

def setVerbose(logfile=None):
  global loglevel
  loglevel = info
  level = logging.INFO
  formatter = logging.Formatter('%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start Verbose %s', loglevel)

def setRelease(logfile=None):
  global loglevel
  loglevel = warning
  level = logging.WARNING
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.warning('start Release %s', loglevel)
  
def setUnitTests(logfile=None):
  global loglevel
  loglevel = critical
  level = logging.CRITICAL
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.critical('start UnitTests %s', loglevel)
  
def setPerfTests(logfile=None):
  global loglevel
  loglevel = critical
  level = logging.CRITICAL
  formatter = logging.Formatter('%(funcName)s[%(lineno)d] %(message)s')
  setLogger(logfile, level, formatter)
  logging.info('start PerfTests %s', loglevel)
  
def getLogLevel():
  return loglevel
