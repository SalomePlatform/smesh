# -*- coding: utf-8 -*-

import logging

debug = 10
info = 20
warning = 30
error = 40
critical = 50

loglevel = warning

def setDebug():
  global loglevel
  logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                      level=logging.DEBUG)
  loglevel = debug
  logging.info('start Debug %s', loglevel)

def setVerbose():
  global loglevel
  logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                      level=logging.INFO)
  loglevel = info
  logging.info('start Verbose %s', loglevel)

def setRelease():
  global loglevel
  logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                      level=logging.WARNING)
  loglevel = warning
  logging.warning('start Release %s', loglevel)
  
def setUnitTests():
  global loglevel
  logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                      level=logging.CRITICAL)
  loglevel = critical
  logging.critical('start UnitTests %s', loglevel)
  
def setPerfTests():
  global loglevel
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
