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

# Modules Python
import string,types,os,sys
import tempfile
import traceback

from PyQt4 import *
from PyQt4.QtGui import *
from PyQt4.QtCore import *

# Import des panels

from MGCleanerViewText_ui import Ui_ViewExe

class MGCleanerMonViewText(Ui_ViewExe, QDialog):
    """
    Classe permettant la visualisation de texte
    """
    def __init__(self, parent, txt, ):
        QDialog.__init__(self,parent)
        self.setupUi(self)
        self.resize( QSize(1000,600).expandedTo(self.minimumSizeHint()) )
        #self.connect( self.PB_Ok,SIGNAL("clicked()"), self, SLOT("close()") )
        self.connect( self.PB_Ok,SIGNAL("clicked()"), self.theClose )
        self.connect( self.PB_Save,SIGNAL("clicked()"), self.saveFile )
        self.PB_Save.setToolTip("Save trace in log file")
        self.PB_Ok.setToolTip("Close view")
        self.monExe=QProcess(self)

        self.connect(self.monExe, SIGNAL("readyReadStandardOutput()"), self.readFromStdOut )
        self.connect(self.monExe, SIGNAL("readyReadStandardError()"), self.readFromStdErr )
      
        # Je n arrive pas a utiliser le setEnvironment du QProcess
        # fonctionne hors Salome mais pas dans Salome ???
        cmds=''
        '''
        try :
          LICENCE_FILE=os.environ["DISTENE_LICENCE_FILE_FOR_MGCLEANER"]
        except:
          LICENCE_FILE=''
        try :
          PATH=os.environ["DISTENE_PATH_FOR_MGCLEANER"]
        except:
          PATH=''
        if LICENCE_FILE != '': 
          cmds+='source '+LICENCE_FILE+'\n'
        else:
          cmds+="# $DISTENE_LICENCE_FILE_FOR_MGCLEANER NOT SET\n"
        if PATH != '': 
          cmds+='export PATH='+PATH+':$PATH\n'
        else:
          cmds+="# $DISTENE_PATH_FOR_MGCLEANER NOT SET\n"
        #cmds+='env\n'
        cmds+='rm -f '+self.parent().fichierOut+'\n'
        '''
        cmds+=txt+'\n'
        cmds+='echo END_OF_MGCleaner\n'
        ext=''
        if sys.platform == "win32":
            ext = '.bat'
        else:
            ext = '.sh'
        nomFichier=tempfile.mktemp(suffix=ext,prefix="MGCleaner_")
        f=open(nomFichier,'w')
        f.write(cmds)
        f.close()

        maBidouille=nomFichier
        self.monExe.start(maBidouille)
        self.monExe.closeWriteChannel()
        self.enregistreResultatsDone=False
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
        self.TB_Exe.append(QString.fromUtf8(a.data(),len(a)))

    def readFromStdOut(self) :
        a=self.monExe.readAllStandardOutput()
        aa=QString.fromUtf8(a.data(),len(a))
        self.TB_Exe.append(aa)
        if "END_OF_MGCleaner" in aa:
          self.parent().enregistreResultat()
          self.enregistreResultatsDone=True
          #self.theClose()
    
    def theClose(self):
      if not self.enregistreResultatsDone:
        self.parent().enregistreResultat()
        self.enregistreResultatsDone=True
      self.close()
