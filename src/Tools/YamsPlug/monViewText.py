# -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2013  EDF R&D
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

# Modules Python
import string,types,os
import traceback

from PyQt4 import *
from PyQt4.QtGui import *
from PyQt4.QtCore import *

# Import des panels

# ------------------------------- #
from ViewText import Ui_ViewExe
class MonViewText(Ui_ViewExe,QDialog):
# ------------------------------- #
    """
    Classe permettant la visualisation de texte
    """
    def __init__(self,parent,txt):
        QDialog.__init__(self,parent)
        self.setupUi(self)
        self.resize( QSize(600,600).expandedTo(self.minimumSizeHint()) )
        self.connect( self.PB_Ok,SIGNAL("clicked()"), self, SLOT("close()") )
        self.connect( self.PB_Save,SIGNAL("clicked()"), self.saveFile )
        self.monExe=QProcess(self)


        self.connect(self.monExe, SIGNAL("readyReadStandardOutput()"), self.readFromStdOut )
        self.connect(self.monExe, SIGNAL("readyReadStandardError()"), self.readFromStdErr )
      
        # Je n arrive pas a utiliser le setEnvironment du QProcess
        # fonctionne hors Salome mais pas dans Salome ???
        LICENCE=os.environ['DISTENE_LICENCE_FILE_FOR_YAMS']
        txt='export DISTENE_LICENSE_FILE='+LICENCE+';'+ txt
        pid=self.monExe.pid()
        nomFichier='/tmp/yam_'+str(pid)+'.py'
        f=open(nomFichier,'w')
        f.write(txt)
        f.close()

        maBidouille='sh  ' + nomFichier
        self.monExe.start(maBidouille)
        self.monExe.closeWriteChannel()
        self.show()

        
    def saveFile(self):
        #recuperation du nom du fichier
        savedir=os.environ['HOME']
        fn = QFileDialog.getSaveFileName(None, self.trUtf8("Save File"),savedir)
        if fn.isNull() : return
        ulfile = os.path.abspath(unicode(fn))
        try:
           f = open(fn, 'wb')
           f.write(str(self.TB_Exe.toPlainText()))
           f.close()
        except IOError, why:
           QMessageBox.critical(self, self.trUtf8('Save File'),
                self.trUtf8('The file <b>%1</b> could not be saved.<br>Reason: %2')
                    .arg(unicode(fn)).arg(str(why)))

    def readFromStdErr(self):
        a=self.monExe.readAllStandardError()
        self.TB_Exe.append(QString.fromUtf8(a.data(),len(a))) ;

    def readFromStdOut(self) :
        a=self.monExe.readAllStandardOutput()
        self.TB_Exe.append(QString.fromUtf8(a.data(),len(a))) ;
