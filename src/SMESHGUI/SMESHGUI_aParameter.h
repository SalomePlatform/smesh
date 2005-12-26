//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_aParameter.h
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_aParameter_H
#define SMESHGUI_aParameter_H

#include <boost/shared_ptr.hpp>
#include <qstringlist.h>
#include <qmap.h>
#include <qtable.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QWidget;
class SMESHGUI_aParameter;

typedef boost::shared_ptr<SMESHGUI_aParameter> SMESHGUI_aParameterPtr;

/*!
 *  \brief This class is the base class of all parameters
 */
class SMESHGUI_aParameter
{ 
public:
  typedef bool (*VALIDATION_FUNC)( SMESHGUI_aParameter* );

  SMESHGUI_aParameter(const QString& label, const bool = false );
  virtual ~SMESHGUI_aParameter();

  enum Type { INT, DOUBLE, STRING, ENUM, BOOL, TABLE };
  virtual Type GetType() const = 0;
  virtual bool GetNewInt( int & Value ) const = 0;
  virtual bool GetNewDouble( double & Value ) const = 0;
  virtual bool GetNewText( QString & Value ) const = 0;
  virtual void TakeValue( QWidget* ) = 0;
  virtual QWidget* CreateWidget( QWidget* ) const = 0;
  virtual void InitializeWidget( QWidget* ) const = 0;

  bool needPreview() const;

  /*!
   *  \brief Returns string representation of signal emitted when value in corrsponding widget is changed
   */
  virtual QString sigValueChanged() const;

  QString & Label();
  
protected:
  QString _label;
  bool    _needPreview;
};

/*!
 *  \brief This class provides parameter with integer value
 */
class SMESHGUI_intParameter: public SMESHGUI_aParameter
{ 
public:
  SMESHGUI_intParameter(const int      initValue = 0,
			const QString& label     = QString::null,
			const int      bottom    = 0,
			const int      top       = 1000,
			const bool = false );
  int & InitValue() { return _initValue; }
  int & Top()       { return _top; }
  int & Bottom()    { return _bottom; }
  virtual Type GetType() const;
  virtual bool GetNewInt( int & Value ) const;
  virtual bool GetNewDouble( double & Value ) const;
  virtual bool GetNewText( QString & Value ) const;
  virtual void TakeValue( QWidget* );
  virtual QWidget* CreateWidget( QWidget* ) const;
  virtual void InitializeWidget( QWidget* ) const;

  virtual QString sigValueChanged() const;
  
protected:
  int _top, _bottom;
  int _initValue, _newValue;
};

/*!
 *  \brief This class provides parameter with double value
 */
class SMESHGUI_doubleParameter: public SMESHGUI_aParameter
{ 
public:
  SMESHGUI_doubleParameter(const double   initValue = 0.0,
			   const QString& label     = QString::null,
			   const double   bottom    = -1E6,
			   const double   top       = +1E6,
			   const double   step      = 1.0,
			   const int      decimals  = 3,
			   const bool = false);
  double & InitValue() { return _initValue; }
  double & Top()       { return _top; }
  double & Bottom()    { return _bottom; }
  double & Step()      { return _step; }
  int    & Decimals()  { return _decimals; }
  virtual Type GetType() const;
  virtual bool GetNewInt( int & Value ) const;
  virtual bool GetNewDouble( double & Value ) const;
  virtual bool GetNewText( QString & Value ) const;
  virtual QWidget* CreateWidget( QWidget* ) const;
  virtual void InitializeWidget( QWidget* ) const;
  virtual void TakeValue( QWidget* );

  virtual QString sigValueChanged() const;
  
protected:
  double _top, _bottom, _step;
  double _initValue, _newValue;
  int _decimals;
};

/*!
 *  \brief This class provides parameter with string value
 */
class SMESHGUI_strParameter: public SMESHGUI_aParameter
{ 
public:
  SMESHGUI_strParameter( const QString& initValue = "",
                         const QString& label     = QString::null,
			 const bool = false );
  QString& InitValue() { return _initValue; }
  virtual Type GetType() const;
  virtual bool GetNewInt( int & Value ) const;
  virtual bool GetNewDouble( double & Value ) const;
  virtual bool GetNewText( QString & Value ) const;
  virtual QWidget* CreateWidget( QWidget* ) const;
  virtual void InitializeWidget( QWidget* ) const;
  virtual void TakeValue( QWidget* );

  virtual QString sigValueChanged() const;
  
protected:
  QString _initValue, _newValue;
};


/*!
 *  \brief This class represents the base parameter which contains dependency of
 *  shown state of other parameters on value of current
 */
class SMESHGUI_dependParameter: public SMESHGUI_aParameter
{
public:
  /*!
   *  \brief This map describes what parameters must be shown when this parameter has value as key
   *  The list contains some indices of parameters (for example, order in some list)
   *  Value is integer based 0. If map is empty, it means that there is no dependencies.
   */
  typedef QValueList< int > IntList;
  typedef QMap< int, IntList >  ShownMap;

public:
  SMESHGUI_dependParameter( const QString& = QString::null, const bool = false );

  const ShownMap&    shownMap() const;
  ShownMap&          shownMap();
  
private:
  ShownMap     myShownMap;
};

/*!
 *  \brief This class represents parameter which can have value from fixed set
 */
class SMESHGUI_enumParameter: public SMESHGUI_dependParameter
{
public:
  /*!
   *  \brief Creates parameter with set of values 'values', default value 'init' and title 'label'
   *  Every value can be described both by integer based 0 or by string value
   */
  SMESHGUI_enumParameter( const QStringList& values,
                          const int init = 0,
                          const QString& label = QString::null,
			  const bool = false );
  virtual ~SMESHGUI_enumParameter();

  /*!
   *  \brief Returns count of possible values
   */
  int            Count() const;

  int& InitValue() { return myInitValue; }
  virtual Type GetType() const;
  virtual bool GetNewInt( int& ) const;
  virtual bool GetNewDouble( double& ) const;
  virtual bool GetNewText( QString& ) const;
  virtual QWidget* CreateWidget( QWidget* ) const;
  virtual void InitializeWidget( QWidget* ) const;
  virtual void TakeValue( QWidget* );

  virtual QString sigValueChanged() const;
    
protected:
  int         myInitValue, myValue;
  QStringList myValues;
};


/*!
 *  \brief This class represents parameter which can have value true or false
 */
class SMESHGUI_boolParameter: public SMESHGUI_dependParameter
{
public:
  SMESHGUI_boolParameter( const bool = false,
                          const QString& = QString::null,
			  const bool = false );
  virtual ~SMESHGUI_boolParameter();

  bool& InitValue() { return myInitValue; }
  virtual Type GetType() const;
  virtual bool GetNewInt( int& ) const;
  virtual bool GetNewDouble( double& ) const;
  virtual bool GetNewText( QString& ) const;
  virtual QWidget* CreateWidget( QWidget* ) const;
  virtual void InitializeWidget( QWidget* ) const;
  virtual void TakeValue( QWidget* );

  virtual QString sigValueChanged() const;
  
protected:
  bool myInitValue, myValue;
};


class QButton;
class SMESHGUI_tableParameter;


/*!
 *  \brief This class represents custom table. It has only double values and
    editor for every cell has validator
 */
class SMESHGUI_Table : public QTable
{
  Q_OBJECT
  
public:
  SMESHGUI_Table( const SMESHGUI_tableParameter*, int numRows, int numCols, QWidget* = 0, const char* = 0 );
  virtual ~SMESHGUI_Table();

/*!
 *  \brief Hides current editor of cell
 */
  void stopEditing();
  
  virtual QSize sizeHint() const;

/*!
 *  \brief Returns parameters of double validator corresponding to cell (row,col)
 */
  void validator( const int row, const int col, double&, double&, int& );
  
/*!
 *  \brief Sets the double validator parameters to every cell in row range [rmin,rmax]
 *         and column range [cmin,cmax].
 *         If rmin=-1 then rmin is set to 0, if rmax=-1 then rmax = last row.
 *         Analogically cmin and cmax are set
 */
  void setValidator( const double, const double, const int,
                     const int rmin = -1, const int rmax = -1,
                     const int cmin = -1, const int cmax = -1 );  

protected:
  virtual void keyPressEvent( QKeyEvent* );
  virtual bool eventFilter( QObject*, QEvent* );
  virtual QWidget* createEditor( int, int, bool ) const;

private:
  SMESHGUI_tableParameter*   myParam;
};


/*!
 *  \brief This class represents frame for table and buttons
 */
class SMESHGUI_TableFrame : public QFrame
{
  Q_OBJECT
  
public:
/*!
 *  \brief Values corresponding to buttons for table resize
 */
  typedef enum { ADD_COLUMN, REMOVE_COLUMN, ADD_ROW, REMOVE_ROW } Button;

public:
  SMESHGUI_TableFrame( const SMESHGUI_tableParameter*, QWidget* );
  ~SMESHGUI_TableFrame();

  SMESHGUI_Table* table() const;

/*!
 *  \brief Changes shown state of some button for table resize
 */   
  void setShown( const Button, const bool );

/*!
 *  \brief Returns shown state of some button for table resize
 */
  bool isShown( const Button ) const;
  
private:
  QButton* button( const Button ) const;

private slots:
  void onButtonClicked();
  
signals:
/*!
 *  \brief This signal is emitted if some of button for table resize is clicked
 *         Second parameter is current column for ADD_COLUMN, REMOVE_COLUMN buttons
 *         and current row for ADD_ROW, REMOVE_ROW buttons. Take into account that
 *         this object resize table ( returned by table() ) automatically
 */
  void toEdit( SMESHGUI_TableFrame::Button, int );
  void valueChanged( int, int );

private:
  QButton *myAddColumn, *myRemoveColumn, *myAddRow, *myRemoveRow;
  SMESHGUI_Table*  myTable;
};


/*!
 *  \brief This class represents parameter which can have two-dimensional array of values
 */
class SMESHGUI_tableParameter: public QObject, public SMESHGUI_aParameter
{
  Q_OBJECT
  
public:
/*!
 *  \brief Creates table parameter with default value 'init' and title 'label'.
 *         The default value means that by default the table is filled with default value
 *         and if new column or row is added then it is filled with default value
 */
  SMESHGUI_tableParameter( const double init = 0.0,
                           const QString& label = QString::null,
			   const bool preview = false );
  virtual ~SMESHGUI_tableParameter();

  virtual Type GetType() const;
  virtual bool GetNewInt( int& ) const;
  virtual bool GetNewDouble( double& ) const;
  virtual bool GetNewText( QString& ) const;
  virtual QWidget* CreateWidget( QWidget* ) const;
  virtual void InitializeWidget( QWidget* ) const;
  virtual void TakeValue( QWidget* );

  static void sortData( SMESH::double_array& );

/*!
 *  \brief Updates look of widget in accordance with all parameters of this object
 */
  void update( QWidget* ) const;
  
/*!
 *  \brief Returns data taken from widget. Please don't forget to call TakeValue before.
 */
  void data( SMESH::double_array& ) const;

/*!
 *  \brief Sets data. The InitializeWidget must be called in order to change values in widget
 */
  void setData( const SMESH::double_array& );

/*!
 *  \brief Sets count of columns and updates widget
 */
  void setColCount( const int, QWidget* = 0 );

/*!
 *  \brief Sets count of rows and updates widget
 */
  void setRowCount( const int, QWidget* = 0 );
  
/*!
 *  \brief Binds count of columns to some parameter and updates widget. Take into account that
 *         if this parameter is changed, the update() must be called to resize table
 */
  void setColCount( const SMESHGUI_aParameterPtr, QWidget* = 0 );

/*!
 *  \brief Binds count of rows to some parameter and updates widget. Take into account that
 *         if this parameter is changed, the update() must be called to resize table
 */
  void setRowCount( const SMESHGUI_aParameterPtr, QWidget* = 0 );
  
/*!
 *  \brief Enables or disables to change count of columns by buttons
 */
  void setEditCols( const bool );

/*!
 *  \brief Enables or disables to change count of rows by buttons
 */
  void setEditRows( const bool );  

  virtual QString sigValueChanged() const;
  
  void setValidator( const int col, const double, const double, const int );
  void validator( const int col, double&, double&, int& ) const;

/*!
 *  \brief These methods allow to read and change name of column
 */
  void    setColName( const int, const QString& );
  QString colName( const int ) const;

private slots:
  void onEdit( SMESHGUI_TableFrame::Button, int );
  void onEdit( SMESHGUI_Table*, SMESHGUI_TableFrame::Button, int );

private:
  void setItems( QWidget*, int = -1, int = -1, int = -1, int = -1 ) const;

private:
  typedef struct
  {
    double myMin, myMax;
    int    myDecimals;
  } ValidatorInfo;

  typedef QMap<int, ValidatorInfo>  ValidatorsMap;
  
private:
  int                      myColsInt, myRowsInt;
  SMESHGUI_aParameterPtr   myCols, myRows;
  double                   myInitValue;
  SMESH::double_array      myData;
  ValidatorsMap            myValidators;
  bool                     myEditCols, myEditRows;
  QMap< int, QString >     myColNames;

  friend class SMESHGUI_Table;
};

#endif // SMESHGUI_aParameter.h

