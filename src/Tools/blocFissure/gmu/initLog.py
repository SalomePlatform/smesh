# -*- coding: utf-8 -*-

import logging

def setDebug():
  logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                      level=logging.DEBUG)
  logging.info('start Debug')

def setVerbose():
  logging.basicConfig(format='%(relativeCreated)d %(funcName)s[%(lineno)d] %(message)s',
                      level=logging.INFO)
  logging.info('start Verbose')

def setRelease():
  logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                      level=logging.WARNING)
  logging.warning('start Release')
  
def setUnitTests():
  logging.basicConfig(format='%(funcName)s[%(lineno)d] %(message)s',
                      level=logging.CRITICAL)
  logging.critical('start UnitTests')
  
  #logging.basicConfig(filename='myapp.log',
  #                    format='%(asctime)s %(message)s',
  #                    datefmt='%m/%d/%Y %I:%M:%S %p',
  #                    level=logging.DEBUG)
