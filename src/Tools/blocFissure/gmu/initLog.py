# -*- coding: utf-8 -*-

import logging

debug = 10
info = 20
warning = 30
error = 40
critical = 50

loglevel = warning

def setDebug(logfile=None):
  global loglevel
  if logfile:
    logging.basicConfig(filename=logfile,
                        format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                        level=logging.DEBUG)
  else:
    logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                        level=logging.DEBUG)    
  loglevel = debug
  logging.info('start Debug %s', loglevel)

def setVerbose(logfile=None):
  global loglevel
  if logfile:
    logging.basicConfig(filename=logfile,
                        format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                        level=logging.INFO)
  else:
    logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                        level=logging.INFO)    
  loglevel = info
  logging.info('start Verbose %s', loglevel)

def setRelease(logfile=None):
  global loglevel
  if logfile:
    logging.basicConfig(filename=logfile,
                        format='%(funcName)s[%(lineno)d] %(message)s',
                        level=logging.WARNING)
  else:
    logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                        level=logging.WARNING)
  loglevel = warning
  logging.warning('start Release %s', loglevel)
  
def setUnitTests(logfile=None):
  global loglevel
  if logfile:
    logging.basicConfig(filename=logfile,
                        format='%(funcName)s[%(lineno)d] %(message)s',
                        level=logging.CRITICAL)
  else:
    logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                        level=logging.CRITICAL)
  loglevel = critical
  logging.critical('start UnitTests %s', loglevel)
  
def setPerfTests(logfile=None):
  global loglevel
  if logfile:
    logging.basicConfig(filename=logfile,
                        format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                        level=logging.CRITICAL)
  else:
    logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                        level=logging.CRITICAL)    
  loglevel = critical
  logging.info('start PerfTests %s', loglevel)
  
def getLogLevel():
  return loglevel
  
  #logging.basicConfig(filename='myapp.log',
  #                    format='%(asctime)s %(message)s',
  #                    datefmt='%m/%d/%Y %I:%M:%S %p',
  #                    level=logging.DEBUG)
