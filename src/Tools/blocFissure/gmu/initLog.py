# -*- coding: utf-8 -*-

import logging
loglevel = 3

def setDebug():
  global loglevel
  logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                      level=logging.DEBUG)
  loglevel = 1
  logging.info('start Debug %s', loglevel)

def setVerbose():
  global loglevel
  logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                      level=logging.INFO)
  loglevel = 2
  logging.info('start Verbose %s', loglevel)

def setRelease():
  global loglevel
  logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                      level=logging.WARNING)
  loglevel = 3
  logging.warning('start Release %s', loglevel)
  
def setUnitTests():
  global loglevel
  logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                      level=logging.CRITICAL)
  loglevel = 4
  logging.critical('start UnitTests %s', loglevel)
  
def setPerfTests():
  global loglevel
  logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                      level=logging.CRITICAL)
  loglevel = 5
  logging.info('start PerfTests %s', loglevel)
  
def getLogLevel():
  return loglevel
  
  #logging.basicConfig(filename='myapp.log',
  #                    format='%(asctime)s %(message)s',
  #                    datefmt='%m/%d/%Y %I:%M:%S %p',
  #                    level=logging.DEBUG)
