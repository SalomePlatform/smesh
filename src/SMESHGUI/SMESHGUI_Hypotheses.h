//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  CEA
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_Hypotheses.h
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_Hypotheses_HeaderFile
#define SMESHGUI_Hypotheses_HeaderFile

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include <QtxDialog.h>
#include <qvariant.h>

class QVBoxLayout;
class QPixmap;

/*!
 * \brief Auxiliary class for creation of hypotheses
*/
class SMESHGUI_GenericHypothesisCreator : public QObject
{
  Q_OBJECT

public:
  SMESHGUI_GenericHypothesisCreator( const QString& );
  virtual ~SMESHGUI_GenericHypothesisCreator();

          void create( const bool isAlgo, QWidget* );
          void edit( SMESH::SMESH_Hypothesis_ptr, QWidget* );
  virtual bool checkParams() const = 0;

  QString                     hypType() const;
  bool                        isCreation() const;

protected:
  typedef struct
  {
    QString   myName;
    QVariant  myValue;

  } StdParam;

  typedef QValueList<StdParam>   ListOfStdParams;
  typedef QPtrList<QWidget>      ListOfWidgets;

  SMESH::SMESH_Hypothesis_var hypothesis() const;
  const ListOfWidgets&        widgets() const;
  ListOfWidgets&              changeWidgets();

  virtual QFrame*  buildFrame    () = 0;
          QFrame*  buildStdFrame ();
  virtual void     retrieveParams() const = 0;
  virtual void     storeParams   () const = 0;
  virtual bool     stdParams     ( ListOfStdParams& ) const;
          bool     getStdParamFromDlg( ListOfStdParams& ) const;
  virtual void     attuneStdWidget( QWidget*, const int ) const;
  virtual QString  caption() const;
  virtual QPixmap  icon() const;
  virtual QString  type() const;

protected slots:
  virtual void onValueChanged();

private:
          bool editHypothesis( SMESH::SMESH_Hypothesis_ptr, QWidget* );

private:
  SMESH::SMESH_Hypothesis_var  myHypo;
  QString                      myHypType;
  ListOfWidgets                myParamWidgets;
  bool                         myIsCreate;
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

private:
  SMESHGUI_GenericHypothesisCreator*   myCreator;
  QVBoxLayout*                         myLayout;
  QLabel                              *myIconLabel, *myTypeLabel;
};

/*!
 * \brief Class containing information about hypothesis
*/
class HypothesisData
{
 public:
  HypothesisData( const QString& thePluginName,
                  const QString& theServerLibName,
                  const QString& theClientLibName,
                  const QString& theLabel,
                  const QString& theIconId,
                  const QValueList<int>& theDim,
                  const bool theIsAux ) 
: PluginName( thePluginName ),
  ServerLibName( theServerLibName ),
  ClientLibName( theClientLibName ),
  Label( theLabel ),
  IconId( theIconId ),
  Dim( theDim ),
  IsAux( theIsAux )
 {};

 QString PluginName;      //!< plugin name
 QString ServerLibName;   //!< server library name
 QString ClientLibName;   //!< client library name
 QString Label;           //!< label
 QString IconId;          //!< icon identifier
 QValueList<int> Dim;     //!< list of supported dimensions (see SMESH::Dimension enumeration)
 bool IsAux;              //!< TRUE if given hypothesis is auxiliary one, FALSE otherwise
};

/*!
 * \brief Class containing set of hypotheses
 *
 * Actually it contains list of hypo types
 */
class HypothesesSet
{
 public:
  HypothesesSet( const QString& theSetName ) 
    : HypoSetName( theSetName ) {};
  HypothesesSet( const QString&     theSetName,
                 const QStringList& theHypoList,
                 const QStringList& theAlgoList )
    : HypoSetName( theSetName ), HypoList(theHypoList), AlgoList(theAlgoList) {};
  QString     HypoSetName;
  QStringList HypoList, AlgoList;
};
#endif
