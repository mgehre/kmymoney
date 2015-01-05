/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "payeeidentifiermodel.h"
#include "mymoney/mymoneyfile.h"
#include "payeeidentifier/payeeidentifierloader.h"
#include "payeeidentifier/payeeidentifier.h"

#include <QDebug>

#include <KLocalizedString>

payeeIdentifierModel::payeeIdentifierModel( QObject* parent )
  : QAbstractListModel(parent),
  m_data(QSharedPointer<MyMoneyPayeeIdentifierContainer>())
{
}

QVariant payeeIdentifierModel::data(const QModelIndex& index, int role) const
{
  // Needed for the selection box and it prevents a crash if index is out of range
  if ( m_data.isNull() || index.row() >= rowCount(index.parent())-1 )
    return QVariant();

  const ::payeeIdentifier ident = m_data->payeeIdentifiers().at(index.row());

  if (role == payeeIdentifier) {
    return QVariant::fromValue< ::payeeIdentifier >(ident);
  } else if (ident.isNull()) {
    return QVariant();
  } else if (role == payeeIdentifierType) {
    return ident.iid();
  } else if (role == Qt::DisplayRole) {
    // The custom delegates won't ask for this role
    return QVariant::fromValue( i18n("The plugin to show this information could not be found.") );
  }
  return QVariant();
}

bool payeeIdentifierModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if ( !m_data.isNull() && role == payeeIdentifier ) {
    ::payeeIdentifier ident = value.value< ::payeeIdentifier >();
    if ( index.row() == rowCount(index.parent())-1 ) {
      // The new row will be the last but one
      beginInsertRows(index.parent(), index.row()-1, index.row()-1);
      m_data->addPayeeIdentifier(ident);
      endInsertRows();
    } else {
      m_data->modifyPayeeIdentifier(index.row(), ident);
      emit dataChanged(createIndex(index.row(), 0), createIndex(index.row(), 0));
    }
    return true;
  }
  return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags payeeIdentifierModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
  const QString type = data(index, payeeIdentifierType).toString();
  // type.isEmpty() means the type selection can be shown
  if ( !type.isEmpty() && payeeIdentifierLoader::instance()->hasItemEditDelegate(type) )
    flags |= Qt::ItemIsEditable;
  return flags;
}

int payeeIdentifierModel::rowCount(const QModelIndex& parent) const
{
  if (m_data.isNull())
    return 0;
  // Always a row more which creates new entries
  return m_data->payeeIdentifiers().count()+1;
}

/** @brief unused at the moment */
bool payeeIdentifierModel::insertRows(int row, int count, const QModelIndex& parent)
{
  return false;
}

bool payeeIdentifierModel::removeRows(int row, int count, const QModelIndex& parent)
{
  if (m_data.isNull())
    return false;

  if (count < 1 || row+count >= rowCount(parent))
    return false;

  beginRemoveRows(parent, row, row+count-1);
  for( unsigned int i = row; i < row+count; ++i) {
    m_data->removePayeeIdentifier(i);
  }
  endRemoveRows();
  return true;
}

void payeeIdentifierModel::setSource( const MyMoneyPayeeIdentifierContainer data)
{
  beginResetModel();
  m_data = QSharedPointer<MyMoneyPayeeIdentifierContainer>(new MyMoneyPayeeIdentifierContainer(data));
  endResetModel();
}

void payeeIdentifierModel::closeSource()
{
  beginResetModel();
  m_data = QSharedPointer<MyMoneyPayeeIdentifierContainer>();
  endResetModel();
}

QList< ::payeeIdentifier > payeeIdentifierModel::identifiers() const
{
  if ( m_data.isNull() )
    return QList< ::payeeIdentifier >();
  return m_data->payeeIdentifiers();
}
