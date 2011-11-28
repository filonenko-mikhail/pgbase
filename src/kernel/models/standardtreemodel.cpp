
#include "standardtreemodel.h"

#include <QtGui/QStandardItem>

StandardTreeModel::StandardTreeModel(int rows, int columns, QObject *parent)
  :QStandardItemModel(rows, columns, parent)
{
}

StandardTreeModel::StandardTreeModel(QObject *parent)
  :QStandardItemModel(parent)
{
}

QStandardItem* StandardTreeModel::itemForPath(const QString& path
    , int sectionPos, QStandardItem* parentItem)
{
  if (!parentItem)
    parentItem = invisibleRootItem();

  QString section = path.section('/', sectionPos + 1, sectionPos + 1);
  if (section.isNull())
    return parentItem;


  for (int i = 0; i < parentItem->rowCount(); ++i)
    if (!QString::compare(parentItem->child(i, 0)->text(), section))
      return itemForPath(path, sectionPos + 1, parentItem->child(i, 0));

  QStandardItem* item = new QStandardItem(section);
  parentItem->setChild(parentItem->rowCount(), item);
  return itemForPath(path, sectionPos + 1, item);
}


bool StandardTreeModel::removeItemWithPath(const QString& path
    , int sectionPos, QStandardItem* parentItem)
{
  if (!parentItem)
    parentItem = invisibleRootItem();

  QString section = path.section('/', sectionPos + 1, sectionPos + 1);
  if (section.isNull()) {
    delete parentItem;
    return true;
  }

  for (int i = 0; i < parentItem->rowCount(); ++i)
    if (!QString::compare(parentItem->child(i, 0)->text(), section))
      return itemForPath(path, sectionPos + 1, parentItem->child(i, 0));

  return false;
}
