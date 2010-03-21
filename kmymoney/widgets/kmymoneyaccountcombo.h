/***************************************************************************
                         kmymoneyaccountcombo  -  description
                            -------------------
   begin                : Mon May 31 2004
   copyright            : (C) 2000-2004 by Michael Edwardes
   email                : mte@users.sourceforge.net
                          Javier Campos Morales <javi_c@users.sourceforge.net>
                          Felix Rodriguez <frodriguez@users.sourceforge.net>
                          John C <thetacoturtle@users.sourceforge.net>
                          Thomas Baumgart <ipwizard@users.sourceforge.net>
                          Kevin Tambascio <ktambascio@users.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYACCOUNTCOMBO_H
#define KMYMONEYACCOUNTCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QMouseEvent>
#include <QKeyEvent>
#include <QList>
#include <QItemSelection>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "accountsmodel.h"

class kMyMoneyAccountCompletion;

/**
  * A proxy model used to filter all the data from the core accounts model leaving
  * only the name of the accounts so this model can be used in the account
  * completion combo.
  *
  * @see AccountsModel
  * @see AccountsFilterProxyModel
  *
  * @author Cristian Onet 2010
  *
  */
class AccountNamesFilterProxyModel : public AccountsFilterProxyModel
{
  Q_OBJECT

public:
  AccountNamesFilterProxyModel(QObject *parent = 0);

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

protected:
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
};

/**
  *
  *
  * @author Cristian Onet
  */
class KMyMoneyMVCAccountCombo : public KComboBox
{
  Q_OBJECT
public:
  explicit KMyMoneyMVCAccountCombo(AccountNamesFilterProxyModel *model, QWidget *parent = 0);
  ~KMyMoneyMVCAccountCombo();

  void setSelected(const QString& id);

public slots:
  void expandAll();

protected:
  virtual void wheelEvent(QWheelEvent *ev);

protected slots:
  void activated();

signals:
  void accountSelected(const QString&);

private:
  class Private;
  Private* const d;
};

/**
  * @author Thomas Baumgart
  */
class KMyMoneyAccountCombo : public KComboBox
{
  Q_OBJECT
public:
  KMyMoneyAccountCombo(QWidget* parent = 0);
  ~KMyMoneyAccountCombo();

  /**
    * Method returns how many items are in the account selector list.
    */
  int count(void) const;

  /**
    * This method loads the set of accounts into the widget
    * as defined by the parameter @p accountIdList. @p accountIdList is
    * a QValueList of account ids.
    *
    * @param baseName QString which should be used as group text
    * @param accountIdList QValueList of QString account ids
    *                 which should be loaded into the widget
    * @param clear if true (default) clears the widget before populating
    * @return This method returns the number of accounts loaded into the list
    */
  int loadList(const QString& baseName, const QList<QString>& accountIdList, const bool clear = true);

  QStringList accountList(const QList<MyMoneyAccount::accountTypeE>& list = QList<MyMoneyAccount::accountTypeE>()) const;

  int loadList(KMyMoneyUtils::categoryTypeE typeMask);
  int loadList(const QList<int>& list);
  int loadList(MyMoneyAccount::accountTypeE type);

  void setSelected(const QString& id);
  void setSelected(const MyMoneyAccount& acc);

  /**
    * This method returns the list of selected account id's. If
    * no account is selected, the list is empty.
    *
    * @return list of selected accounts
    */
  QStringList selectedAccounts(void) const;

  virtual void keyPressEvent(QKeyEvent* e);

public slots:
  void slotButtonPressed(void);
  void slotSelected(const QString&);

protected slots:

signals:
  void accountSelected(const QString&);

  void pressed();
  void released();
  void clicked();

protected:
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

  void setText(const QString& txt);

private:
  kMyMoneyAccountCompletion*    m_completion;
  bool                          m_mlbDown;
};

#endif
