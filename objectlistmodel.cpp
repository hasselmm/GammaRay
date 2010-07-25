#include "objectlistmodel.h"

using namespace Endoscope;

ObjectListModel::ObjectListModel(QObject* parent): ObjectModelBase< QAbstractTableModel >(parent)
{
}

QVariant ObjectListModel::data(const QModelIndex& index, int role) const
{
  if ( index.row() >= 0 && index.row() < m_objects.size() ) {
    QObject *obj = m_objects.at( index.row() );
    return dataForObject( obj, index, role );
  }
  return QVariant();
}

int ObjectListModel::columnCount(const QModelIndex& parent) const
{
  if ( parent.isValid() )
    return 0;
  return ObjectModelBase<QAbstractTableModel>::columnCount( parent );
}

int ObjectListModel::rowCount(const QModelIndex& parent) const
{
  if ( parent.isValid() )
    return 0;
  return m_objects.size();
}

void Endoscope::ObjectListModel::objectAdded(QObject* obj)
{
  beginInsertRows( QModelIndex(), m_objects.size(), m_objects.size() );
  m_objects.push_back( obj );
  endInsertRows();
}

void Endoscope::ObjectListModel::objectRemoved(QObject* obj)
{
  const int index = m_objects.indexOf( obj );
  if ( index < 0 || index >= m_objects.size() )
    return;
  beginRemoveRows( QModelIndex(), index, index );
  m_objects.remove( index );
  endRemoveRows();
}

#include "objectlistmodel.moc"