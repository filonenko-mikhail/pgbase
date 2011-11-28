#include "findactionsmanagerhelper.h"

FindActionsManagerHelper::FindActionsManagerHelper(QObject* parent)
  : QObject(parent) {}

FindActionsManagerHelper::~FindActionsManagerHelper() {}


// ----------------------------------------------------------
// ItemView
#include <QtGui/QAbstractItemView>
#include <QtGui/QItemSelectionModel>
ItemViewFindActionsManagerHelper::ItemViewFindActionsManagerHelper(QObject* parent)
  : FindActionsManagerHelper(parent) {}

unsigned char ItemViewFindActionsManagerHelper::approach(QWidget* widget) const
{
  return qobject_cast<QAbstractItemView*>(widget) ? 10 : 0;
}

void ItemViewFindActionsManagerHelper::activateFor(QWidget* widget)
{
  itemView = qobject_cast<QAbstractItemView*>(widget);
}

void ItemViewFindActionsManagerHelper::deactivate()
{
  itemView = 0;
}

bool ItemViewFindActionsManagerHelper::canFind() const
{
  return true;
}

bool ItemViewFindActionsManagerHelper::canReplace() const
{
  return true;
}

QString ItemViewFindActionsManagerHelper::initialFindString() const
{
  if (itemView->model()) return itemView->currentIndex().data().toString();
  return QString::null;
}

QString ItemViewFindActionsManagerHelper::initialFindHintString() const
{
  return tr("<Enter find string>");
}

void ItemViewFindActionsManagerHelper::findNext(const QVariant& value, Qt::MatchFlags flags)
{
  QAbstractItemModel* model;
  if ((model = itemView->model())) {
    QString status;
    QModelIndex currentIndex = itemView->currentIndex();
    QItemSelectionModel* selectionModel = itemView->selectionModel();

    QModelIndexList indexes = model->match(currentIndex, Qt::DisplayRole, value, 2, flags);
    if (indexes.count() == 2) {
      if (indexes.at(0) == currentIndex) {
        selectionModel->setCurrentIndex(indexes.at(1), QItemSelectionModel::Current
                                        | QItemSelectionModel::Select);
        itemView->scrollTo(indexes.at(1));
      } else {
        selectionModel->setCurrentIndex(indexes.at(0), QItemSelectionModel::Current
                                        | QItemSelectionModel::Select);
        itemView->scrollTo(indexes.at(0));
      }
    } else if (indexes.count() == 1) {
      if (currentIndex == indexes.at(0)) {
        selectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current
                                        | QItemSelectionModel::Select);

      } else {
        selectionModel->setCurrentIndex(indexes.at(0), QItemSelectionModel::Current
                                        | QItemSelectionModel::Select);
        itemView->scrollTo(indexes.at(0));
      }
    } else {
      selectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current
                                      | QItemSelectionModel::Select);
      status = tr("Search completed");
    }
    itemView->setStatusTip(status);
  }
}

void ItemViewFindActionsManagerHelper::findPrevious(const QVariant& value, Qt::MatchFlags flags)
{

  QAbstractItemModel* model;
  if ((model = itemView->model())) {
    QString status;
    QModelIndex currentIndex = itemView->currentIndex();
    QModelIndex rootIndex = itemView->rootIndex();
    QItemSelectionModel* selectionModel = itemView->selectionModel();

    QModelIndexList indexes;
    if (currentIndex.isValid())
      indexes = model->match(currentIndex, Qt::DisplayRole, value, 2, flags);
    else
      indexes = model->match(model->index(model->rowCount(rootIndex) - 1
                                          , model->columnCount(rootIndex) - 1
                                          , rootIndex)
                             , Qt::DisplayRole, value, 2, flags);
    if (indexes.count() == 2) {
      if (indexes.at(0) == currentIndex) {
        selectionModel->setCurrentIndex(indexes.at(1), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(1));
      } else {
        selectionModel->setCurrentIndex(indexes.at(0), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(0));
      }
    } else if (indexes.count() == 1) {
      if (currentIndex == indexes.at(0)) {
        selectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
      } else {
        selectionModel->setCurrentIndex(indexes.at(0), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(0));
      }
    } else {
      selectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current
                                      | QItemSelectionModel::ClearAndSelect);
      status = tr("Search completed");
    }
    itemView->setStatusTip(status);
  }
}

void ItemViewFindActionsManagerHelper::replaceNext(const QVariant& find, const QString& replace, Qt::MatchFlags flags)
{
  QAbstractItemModel* model;
  if ((model = itemView->model())) {
    QModelIndex currentIndex = itemView->currentIndex();
    QItemSelectionModel* selectionModel = itemView->selectionModel();
    QString status;
    QModelIndexList indexes = model->match(currentIndex, Qt::DisplayRole, find, 2, flags);
    if (indexes.count() == 2) {
      if (indexes.at(0) == currentIndex) {
        status = QString(tr("Replace %1 with %2")).arg(currentIndex.data().toString(), replace);
        if (currentIndex.flags() & Qt::ItemIsEditable)
          model->setData(currentIndex, replace, Qt::EditRole);
        selectionModel->setCurrentIndex(indexes.at(1), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(1));
      } else {
        selectionModel->setCurrentIndex(indexes.at(0), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(0));
      }
    } else if (indexes.count() == 1) {
      if (currentIndex == indexes.at(0)) {
        status = QString(tr("Replace %1 with %2")).arg(currentIndex.data().toString(), replace);
        if (currentIndex.flags() & Qt::ItemIsEditable)
          model->setData(currentIndex, replace, Qt::EditRole);
        selectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
      } else {
        selectionModel->setCurrentIndex(indexes.at(0), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(0));
      }
    } else {
      selectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current
                                      | QItemSelectionModel::ClearAndSelect);
      status = tr("Replacing complete");
    }
    itemView->setStatusTip(status);
  }
}

void ItemViewFindActionsManagerHelper::replacePrevious(const QVariant& find
    , const QString& replace
    , Qt::MatchFlags flags)
{
  QAbstractItemModel* model;
  if ((model = itemView->model())) {
    QModelIndex currentIndex = itemView->currentIndex();
    QModelIndex rootIndex = itemView->rootIndex();
    QItemSelectionModel* selectionModel = itemView->selectionModel();

    QString status;

    QModelIndexList indexes;
    if (currentIndex.isValid())
      indexes = model->match(currentIndex, Qt::DisplayRole, find, 2, flags);
    else
      indexes = model->match(model->index(model->rowCount(rootIndex) - 1
                                          , model->columnCount(rootIndex) - 1
                                          , rootIndex)
                             , Qt::DisplayRole, find, 2, flags);
    if (indexes.count() == 2) {
      if (indexes.at(0) == currentIndex) {
        status = QString(tr("Replace %1 with %2")).arg(currentIndex.data().toString(), replace);
        if (currentIndex.flags() & Qt::ItemIsEditable)
          model->setData(currentIndex, replace);
        selectionModel->setCurrentIndex(indexes.at(1), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(1));
      } else {
        selectionModel->setCurrentIndex(indexes.at(0), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(0));
      }
    } else if (indexes.count() == 1) {
      if (currentIndex == indexes.at(0)) {
        status = QString(tr("Replace %1 with %2")).arg(currentIndex.data().toString(), replace);
        if (currentIndex.flags() & Qt::ItemIsEditable)
          model->setData(currentIndex, replace);
        selectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
      } else {
        selectionModel->setCurrentIndex(indexes.at(0), QItemSelectionModel::Current
                                        | QItemSelectionModel::ClearAndSelect);
        itemView->scrollTo(indexes.at(0));
      }
    } else {
      selectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::Current
                                      | QItemSelectionModel::ClearAndSelect);
      status = tr("Replacing completed");
    }
    itemView->setStatusTip(status);
  }
}

void ItemViewFindActionsManagerHelper::replaceAll(const QVariant& find
    , const QString& replace
    , Qt::MatchFlags flags)
{
  QAbstractItemModel* model;
  if ((model = itemView->model())) {
    QString status;
    QModelIndexList indexes = model->match(QModelIndex(), Qt::DisplayRole, find, -1, flags);
    foreach(QModelIndex index, indexes)
    if (index.flags() & Qt::ItemIsEditable)
      model->setData(index, replace);

    status = QString(tr("Replaced %1 values")).arg(QString::number(indexes.count()));
    itemView->setStatusTip(status);
  }
}

void ItemViewFindActionsManagerHelper::findEdited(const QVariant& value, Qt::MatchFlags flags)
{
  QAbstractItemModel* model;
  if ((model = itemView->model())) {
    QModelIndex currentIndex = itemView->currentIndex();
    QModelIndex rootIndex = itemView->rootIndex();
    QItemSelectionModel* selectionModel = itemView->selectionModel();

    QString status;

    if (value.toString().isEmpty() || model->rowCount(rootIndex) > 400)
      return;

    flags |= Qt::MatchWrap;

    QModelIndexList indexes = model->match(currentIndex, Qt::DisplayRole, value, -1, flags);
    if (indexes.count()) {
      status = QString(tr("Found %1 values")).arg(QString::number(indexes.count()));
      QItemSelection selection;
      foreach(QModelIndex index, indexes)
      selection.append(QItemSelectionRange(index, index));

      selectionModel->select(selection, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
    } else {
      selectionModel->select(QItemSelection(), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
      status = tr("Nothing found");
    }
    itemView->setStatusTip(status);
  }
}
