// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_Hypotheses.h
// Author : Julia DOROVSKIKH, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_HYPOTHESES_H
#define SMESHGUI_HYPOTHESES_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_HypothesesUtils.h"

// Qt includes
#include <QtxDialog.h>
#include <QVariant>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

class QEventLoop;

/*!
 * \brief Auxiliary class for creation of hypotheses
*/
class SMESHGUI_EXPORT SMESHGUI_GenericHypothesisCreator : public QObject
{
  Q_OBJECT

public:
  SMESHGUI_GenericHypothesisCreator( const QString& );
  virtual ~SMESHGUI_GenericHypothesisCreator();

  void                         create( SMESH::SMESH_Hypothesis_ptr,
                                       const QString&, QWidget*, QObject*, const QString& );
  void                         create( bool, const QString&, QWidget*, QObject*, const QString& );
  void                         edit( SMESH::SMESH_Hypothesis_ptr, const QString&, QWidget*, QObject*, const QString& );
  void                         setInitParamsHypothesis(SMESH::SMESH_Hypothesis_ptr);

  virtual bool                 checkParams( QString& ) const;
  virtual void                 onReject();
  virtual QString              helpPage() const;

  QString                      hypType() const;
  QString                      hypName() const;
  bool                         isCreation() const;
  
  QString                      getShapeEntry() const { return myShapeEntry; }
  void                         setShapeEntry( const QString& theEntry );

  QString                      getMainShapeEntry() const { return myMainShapeEntry; }
  void                         setMainShapeEntry( const QString& theEntry ) { myMainShapeEntry = theEntry; }

signals:
  void                         finished( int );

protected:
  struct StdParam
  {
    QString   myName;
    QVariant  myValue;
    bool      isVariable;
    QString   myText;
    StdParam(){
      isVariable = false;
    }
    const char* text() const { return myText.toLatin1().constData(); }
  };

  typedef QList<StdParam>      ListOfStdParams;
  typedef QList<QWidget*>      ListOfWidgets;

  SMESH::SMESH_Hypothesis_var  hypothesis() const;
  SMESH::SMESH_Hypothesis_var  initParamsHypothesis(const bool strict=false) const;
  bool                         hasInitParamsHypothesis() const;
  const ListOfWidgets&         widgets() const;
  ListOfWidgets&               changeWidgets();
  QtxDialog*                   dlg() const;
  QString                      getVariableName(const char* methodName) const;

  virtual QFrame*              buildFrame() = 0;
          QFrame*              buildStdFrame();
  virtual void                 retrieveParams() const = 0;
  virtual QString              storeParams() const = 0;
  virtual bool                 stdParams( ListOfStdParams& ) const;
  bool                         getStdParamFromDlg( ListOfStdParams& ) const;
  virtual QStringList          getVariablesFromDlg() const;
  static  QString              stdParamValues( const ListOfStdParams& );
  virtual void                 attuneStdWidget( QWidget*, const int ) const;
  virtual QWidget*             getCustomWidget( const StdParam&, 
                                                QWidget*, const int ) const;
  virtual bool                 getParamFromCustomWidget( StdParam&, QWidget* ) const;
  virtual void                 valueChanged( QWidget* );
  virtual QString              caption() const;
  virtual QPixmap              icon() const;
  virtual QString              type() const;

protected slots:
  virtual void                 onValueChanged();

private slots:
  virtual void                 onDialogFinished( int );

private:
  void                         editHypothesis( SMESH::SMESH_Hypothesis_ptr, const QString&, QWidget*, QObject* obj, const QString& );

private:
  SMESH::SMESH_Hypothesis_var  myHypo, myInitParamsHypo;
  QString                      myHypName;
  QString                      myHypType;
  ListOfWidgets                myParamWidgets;
  bool                         myIsCreate;
  QtxDialog*                   myDlg;
  QString                      myShapeEntry;
  QString                      myMainShapeEntry;
};

class SMESHGUI_HypothesisDlg : public QtxDialog
{
  Q_OBJECT

public:
  SMESHGUI_HypothesisDlg( SMESHGUI_GenericHypothesisCreator*, QWidget* );
  virtual ~SMESHGUI_HypothesisDlg();

  void setHIcon( const QPixmap& );
  void setCustomFrame( QFrame* );
  void setType( const QString& );

protected slots:
  virtual void accept();
  virtual void reject();
  void onHelp(); 

private:
  SMESHGUI_GenericHypothesisCreator* myCreator;
  QLabel *myIconLabel, *myTypeLabel;
  QString myHelpFileName;
};

/*!
 * \brief Class containing information about hypothesis
*/
class HypothesisData
{
public:
  HypothesisData( const QString&, const QString&, const QString&,
                  const QString&, const QString&, const QString&,
                  const QList<int>&, const bool,
                  const QStringList&, const QStringList&,
                  const QStringList&, const QStringList&,
                  const bool=true, const bool supportSub=false );

  QString TypeName;        //!< hypothesis type name
  QString PluginName;      //!< plugin name
  QString ServerLibName;   //!< server library name
  QString ClientLibName;   //!< client library name
  QString Label;           //!< label
  QString IconId;          //!< icon identifier
  QList<int> Dim;          //!< list of supported dimensions (see SMESH::Dimension enumeration)
  bool IsAux;              //!< TRUE if given hypothesis is auxiliary one, FALSE otherwise
  bool IsNeedGeometry;     //!< TRUE if the algorithm works with shapes only, FALSE otherwise
  bool IsSupportSubmeshes; //!< TRUE if the algo building all-dim elems supports submeshes

  // for algorithm only: dependencies algo <-> algo and algo -> hypos
  QStringList NeededHypos;  //!< list of obligatory hypotheses
  QStringList OptionalHypos;//!< list of optional hypotheses
  QStringList InputTypes;   //!< list of element types required as a prerequisite
  QStringList OutputTypes;  //!< list of types of generated elements
};

/*!
 * \brief Class containing set of hypotheses
 *
 * Actually it contains list of hypo types
 */
class HypothesesSet
{
public:
  HypothesesSet( const QString& );
  HypothesesSet( const QString&, const QStringList&, const QStringList& );

  QString name() const;
  void set( bool, const QStringList& );
  int count( bool ) const;

  void setIsCustom( bool );
  bool getIsCustom() const;
  int maxDim() const;

  bool isAlgo() const;

  //this method sets internal index to -1, thus before any data access it is necessary to call next()
  void init( bool );

  bool more() const;
  void next();
  QString current() const;

private:
  QStringList* list(bool) const;
  QStringList* list() const;

private:
  QString     myHypoSetName;
  QStringList myHypoList, myAlgoList;
  bool myIsAlgo, myIsCustom;
  int myIndex;
};

#endif // SMESHGUI_HYPOTHESES_H
