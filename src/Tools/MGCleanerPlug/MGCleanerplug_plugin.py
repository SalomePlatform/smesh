# -*- coding: utf-8 -*-
# Copyright (C) 2013-2016  EDF R&D
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

# if you already have plugins defined in a salome_plugins.py file, add this file at the end.
# if not, copy this file as ${HOME}/Plugins/smesh_plugins.py or ${APPLI}/Plugins/smesh_plugins.py

def MGCleanerLct(context):
  # get context study, studyId, salomeGui
  study = context.study
  studyId = context.studyId
  sg = context.sg
  
  import os
  import subprocess
  import tempfile
  from qtsalome import QFileDialog, QMessageBox
  
  #prior test to avoid unnecessary user GUI work with ending crash
  try :
      os.environ['DISTENE_LICENSE_FILE']
  except:
      QMessageBox.warning(None, "Products", """\
Distene's product MeshGem Cleaner is not installed.
required environment variable:
DISTENE_LICENSE_FILE='.../dlim8.var.sh'""")
      return
  import MGCleanerMonPlugDialog
  window = MGCleanerMonPlugDialog.getDialog()
  window.show()

