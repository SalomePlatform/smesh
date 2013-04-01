# Copyright (C) 2006-2013  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

def YamsLct(context):
  # get context study, studyId, salomeGui
  study = context.study
  studyId = context.studyId
  sg = context.sg
  
  import os
  import subprocess
  import tempfile
  from PyQt4 import QtCore
  from PyQt4 import QtGui
  from PyQt4.QtGui import QFileDialog
  from PyQt4.QtGui import QMessageBox
  
  try :
      os.environ['DISTENE_LICENCE_FILE_FOR_YAMS']
  except:
      QMessageBox.warning(None,"Products","Distene's products are not installed")
      return
  import monYamsPlugDialog
  window=monYamsPlugDialog.getDialog() 
  window.show()
