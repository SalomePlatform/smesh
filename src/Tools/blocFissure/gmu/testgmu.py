
from blocFissure import gmu
from blocFissure.gmu.initEtude import initEtude
initEtude()
from blocFissure.gmu.triedreBase import triedreBase
O, OX, OY, OZ = triedreBase()

from blocFissure.gmu.distance2 import distance2
a=[10, 20, 30]
b=[5, 7, 3]
c=distance2(a,b)

import unittest
from blocFissure.gmu import initLog
initLog.setUnitTests()

from blocFissure.gmu import distance2

suite = unittest.TestLoader().loadTestsFromTestCase(distance2.Test_distance2)
unittest.TextTestRunner(verbosity=2).run(suite)


