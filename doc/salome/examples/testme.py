#!/usr/bin/env python

import unittest, sys, os

class SalomeSession(object):
    def __init__(self, script):
        import runSalome
        sys.argv  = ["runSalome.py"]
        sys.argv += ["--terminal"]
        sys.argv += ["--modules=GEOM,MED,SMESH"]
        sys.argv += ["--execute=%s" % script]
        clt, d = runSalome.main()
        return

    def __del__(self):
        port = os.getenv('NSPORT')
        import killSalomeWithPort
        killSalomeWithPort.killMyPort(port)
        return
    pass

class MyTest(unittest.TestCase):
    def testFunction(self):
        SalomeSession(sys.argv[1])
    pass

unittest.main(argv=sys.argv[:1])
