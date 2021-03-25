# Copyright (C) 2013-2021  EDF R&D
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

from qtsalome import *
from nomBase_ui import Ui_LEDataBaseName


class DataBaseName(Ui_LEDataBaseName,QDialog):

     def __init__(self, parent):
         QDialog.__init__(self,parent)
         self.setupUi(self)
         self.setModal(True)
         self.parent=parent
         self.LEBaseName.returnPressed.connect(self.LEDBreturnPressed)


     def LEDBreturnPressed(self):
         self.parent.nomBase=self.LEBaseName.text()
         self.close()
