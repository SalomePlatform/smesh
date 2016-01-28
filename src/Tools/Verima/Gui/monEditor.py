from qtsalome import *

class TableEditor(QDialog):

     def __init__(self, table,parent):
         QDialog.__init__(self)
         self.setModal(False)
         self.model= QSqlTableModel()
         self.table=table
         self.model.setTable(table.nom);
         self.model.setEditStrategy(QSqlTableModel.OnManualSubmit)
         self.model.select()
         self.view = QTableView(self)
         self.view.setModel(self.model)
         

         quitButton=QPushButton("Quit")
         buttonBox=QDialogButtonBox(Qt.Vertical)
         buttonBox.addButton(quitButton, QDialogButtonBox.RejectRole);
         quitButton.clicked.connect(self.close())

         mainLayout = QHBoxLayout(self)
         mainLayout.addWidget(self.view)
         mainLayout.addWidget(buttonBox)
         self.setLayout(mainLayout)

         self.setWindowTitle("Table des " + table.nom)
         self.setSizePolicy(QSizePolicy.Expanding,QSizePolicy.Expanding)
         self.view.setSizePolicy(QSizePolicy.Expanding,QSizePolicy.Expanding)
         self.view.resizeColumnsToContents()
         self.view.adjustSize()
         self.adjustSize()
         self.view.pressed[QModelIndex].connect(self.donneLigne)
         self.view.itemPressed[QTableWidgetItem].conect(self.donneLigne)
         self.view.clicked[QModelIndex].connect(self.donneLigne)
         self.view.clicked.connect(self.donneLigne)
         self.view.pressed.connect(self.donneLigne)

     def donneLigne(self):
         print "jjjjjjjjjjjjjjjj"

     def setTitle(self):
         fields=self.table.getFields()
         index=0
         for f in fields:
           self.model.setHeaderData(index,Qt.Horizontal,fields)
           index=index+1




#void TableEditor::submit()
#{
#     model->database().transaction();
#     if (model->submitAll()) {
#         model->database().commit();
#     } else {
#         model->database().rollback();
#         QMessageBox::warning(this, tr("Cached Table"),
#                              tr("The database reported an error: %1")
#                              .arg(model->lastError().text()));
#     }
#}
#     submitButton = new QPushButton(tr("Submit"));
#     submitButton->setDefault(true);
#     revertButton = new QPushButton(tr("&Revert"));
#     buttonBox->addButton(submitButton, QDialogButtonBox::ActionRole);
#     buttonBox->addButton(revertButton, QDialogButtonBox::ActionRole);
#     connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
#     connect(revertButton, SIGNAL(clicked()), model, SLOT(revertAll()));
#     quitButton = new QPushButton(tr("Quit"));
