# -*- coding: utf-8 -*-

import os
import initLog

# --- calcul path blocFissure

apath = initLog.__file__
isabs = os.path.isabs(apath)
pathGmu = os.path.split(apath)[0]
if isabs:
  pathBloc = os.path.join(pathGmu, '..')
else:
  pathBloc = os.path.join(os.getcwd(), pathGmu, '..')
pathBloc = os.path.normpath(pathBloc)
