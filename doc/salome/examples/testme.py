#!/usr/bin/env python

import unittest, sys

class SalomeSession(object):
    def __init__(self, script):
        import runSalome
        sys.argv  = ["runSalome.py"]
        sys.argv += ["--terminal"]
        sys.argv += ["--modules=GEOM,MED,SMESH"]
        sys.argv += ["--execute=%s" % script]
        clt, d = runSalome.main()
        self.port = d['port']
        return

    def __del__(self):
        port = self.port
        import killSalomeWithPort
        killSalomeWithPort.killMyPort(port)
        return
    pass

class MyTest(unittest.TestCase):
    def testFunction(self):
        SalomeSession(sys.argv[1])
    pass

unittest.main(argv=sys.argv[:1])
