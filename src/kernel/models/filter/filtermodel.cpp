#include "filtermodel.h"
#include "private/filtermodel_p.h"

#include <QtDebug>

#include <QtCore/QMetaMethod>
#include <QtCore/QDateTime>

namespace
{
  class NullModel : public QAbstractItemModel
  {
  public:
    explicit NullModel(QObject* parent = 0) : QAbstractItemModel(parent) {}
    QModelIndex index(int, int, const QModelIndex&) const {return QModelIndex();}
    QModelIndex parent(const QModelIndex&) const {return QModelIndex();}
    int rowCount(const QModelIndex&) const {return 0;}
    int columnCount(const QModelIndex&) const {return 0;}
    bool hasChildren(const QModelIndex&) const {return false;}
    QVariant data(const QModelIndex&, int) const {return QVariant();}
  };

  static NullModel nullModel;

  bool operator < (const QVariant& left, const QVariant& right)
  {
    if (left.type() == right.type()) {
      // NULL value always less than NOT NULL
      if (left.isNull() && !right.isNull())
        return true;

      bool leftOk;
      bool rightOk;
      left.toDouble(&leftOk);
      right.toDouble(&rightOk);
      if (leftOk && rightOk)
        return left.toDouble() < right.toDouble();

      switch (left.type()) {
      case QVariant::Bool:
        return left.toBool() < right.toBool();
      case QVariant::UInt:
      case QVariant::ULongLong:
      case QVariant::LongLong:
      case QVariant::Int:
      case QVariant::Double:
        return left.toDouble() < right.toDouble();
      case QVariant::Date:
        return left.toDate() < right.toDate();
      case QVariant::DateTime:
        return left.toDateTime() < right.toDateTime();
      default:
        return left.toString().localeAwareCompare(right.toString()) < 0;
      }
    }

    return left.type() < right.type();
  }
}

FilterModel::FilterModel(FilterModelPrivate& dd, Qt::Orientation orientation, QObject* parent)
  : QAbstractItemModel(parent)
  , d_ptr(&dd)
{
  init(orientation);
}

FilterModel::FilterModel(Qt::Orientation orientation, QObject* parent)
  : QAbstractItemModel(parent)
  , d_ptr(new FilterModelPrivate)
{
  init(orientation);
}

FilterModel::~FilterModel()
{
}

void FilterModel::init(Qt::Orientation orientation)
{
  setObjectName("filterModel");
  Q_D(FilterModel);
  d->q_ptr = this;
  d->orientation = orientation;

  d->columnHeaders[0][Qt::EditRole] = tr("Columns");
  d->columnHeaders[1][Qt::EditRole] = tr("Count");
}

void FilterModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  Q_D(FilterModel);
  disconnect(d->sourceModel, 0, this, 0);

  d->sourceModel = sourceModel ? sourceModel : &nullModel;
  d->sourceRootIndex = QModelIndex();

  const QMetaObject* sourceModelMetaObject = d->sourceModel->metaObject();
  QMetaMethod metaMethod;

  for (int i = QAbstractItemModel::staticMetaObject.methodOffset(); i < sourceModelMetaObject->methodCount(); ++i) {
    metaMethod = sourceModelMetaObject->method(i);
    QString sourceModelSignal = QString(metaMethod.signature());
    sourceModelSignal.replace(0, 1, sourceModelSignal.at(0).toTitleCase());
    sourceModelSignal.prepend("sourceModel");
    QString thisSlotSignature = sourceModelSignal;
    if (metaMethod.methodType() == QMetaMethod::Signal
        && (metaObject()->indexOfSlot(thisSlotSignature.toLatin1().data()) != -1)) {
      connect(d->sourceModel, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data()
              , this, QString("%1%2").arg(QSLOT_CODE).arg(thisSlotSignature.toLatin1().data()).toLatin1().data());
    }
  }
  refresh();
}

QAbstractItemModel* FilterModel::sourceModel() const
{
  Q_D(const FilterModel);
  return d->sourceModel != &nullModel ? d->sourceModel : 0;
}

void FilterModel::setSourceRootIndex(const QModelIndex& rootIndex)
{
  Q_D(FilterModel);
  d->sourceRootIndex = rootIndex;
  refresh();
}

QModelIndex FilterModel::sourceRootIndex() const
{
  Q_D(const FilterModel);
  return d->sourceRootIndex;
}

void FilterModel::setSourceRole(int role)
{
  Q_D(FilterModel);
  d->sourceRole = role;
  refresh();
}


int FilterModel::sourceRole() const
{
  Q_D(const FilterModel);
  return d->sourceRole;
}

void FilterModel::setOrientation(Qt::Orientation orientation)
{
  Q_D(FilterModel);
  if (d->orientation != orientation) {
    d->orientation = orientation;
    refresh();
  }
}

Qt::Orientation FilterModel::orientation() const
{
  Q_D(const FilterModel);
  return d->orientation;
}

QSet<int> FilterModel::filteredSections() const
{
  Q_D(const FilterModel);
  QSet<int> filteredSections;
  QHash<int, QSet<int> >::ConstIterator falseStatesIt = d->falseStateMap.constBegin();
  QHash<int, QSet<int> >::ConstIterator falseStatesEnd = d->falseStateMap.constEnd();

  QSet<int>::ConstIterator filteredRowsIt;
  QSet<int>::ConstIterator filteredRowsEnd;

  while (falseStatesIt != falseStatesEnd) {
    filteredRowsIt = falseStatesIt.value().constBegin();
    filteredRowsEnd = falseStatesIt.value().constEnd();
    while (filteredRowsIt != filteredRowsEnd) {
      filteredSections.unite((d->groupedValues.at(falseStatesIt.key()).begin() + *filteredRowsIt).value());
      ++filteredRowsIt;
    }
    ++ falseStatesIt;
  }

  return filteredSections;
}

QModelIndex FilterModel::index(int row, int column, const QModelIndex& parent) const
{
  // First hierarchy level
  // Parent is invalid
  if (!parent.isValid()) {
    if (column == 0) {
      if (-1 < row && row < rowCount(parent)) {
        return createIndex(row, column, -1);
      }
    } else {
    }
    // second hierarchy level
  } else if (isSourceSectionIndex(parent)) {
    if (-1 < column && column < columnCount(parent) && -1 < row && row < rowCount(parent))
      return createIndex(row, column, parent.row());
  }
  return QModelIndex();
}

QModelIndex FilterModel::parent(const QModelIndex& child) const
{
  // invalid level
  if (!child.isValid())
    return QModelIndex();

  // first hierarchy level
  if (isSourceSectionIndex(child))
    return QModelIndex();

  // second hierarchy level
  return index(child.internalId(), 0, QModelIndex());
}

int FilterModel::rowCount(const QModelIndex& parent) const
{
  Q_D(const FilterModel);

  // root hierarchy level
  if (!parent.isValid())
    return d->groupedValues.count();

  // first hierarchy level
  int result = 0;
  if (isSourceSectionIndex(parent))
    result = d->groupedValues.at(parent.row()).count();

  return result?result + 1: result;
}

int FilterModel::columnCount(const QModelIndex& parent) const
{
  //Q_D(const ColumnFilterModel);
  // always
  // first column hierarchy
  // second column count of values
  return 2;
}

bool FilterModel::hasChildren(const QModelIndex& parent) const
{
  Q_D(const FilterModel);

  if (!parent.isValid())
    return d->groupedValues.count();

  if (isSourceSectionIndex(parent))
    return d->groupedValues.at(parent.row()).count();

  return false;
}

QVariant FilterModel::data(const QModelIndex& index, int role) const
{
  Q_D(const FilterModel);
  QVariant itemData;

  if (!index.isValid())
    return itemData;
  // first level
  if (isSourceSectionIndex(index))
    return d->sourceModel->headerData(index.row(), d->orientation, role);

  // second level
  if (role == Qt::DisplayRole) {
    if (index.row() == 0)
      return tr("All");
    QVariant itemData;
    // first column filter values
    if (index.column() == 0)
      itemData = (d->groupedValues.at(index.internalId()).begin() + index.row() - 1).key();
      // second column, number of filter values
    else
      itemData = (d->groupedValues.at(index.internalId()).begin() + index.row() - 1).value().count();
    if (itemData.isNull())
      itemData = tr("NULL(%1)").arg(itemData.typeName());

    return itemData;
  }
  if (role == Qt::CheckStateRole && index.column() == 0) {
    if (index.row() == 0) {
      if (d->falseStateMap.contains(index.internalId())) {
        if (d->falseStateMap.value(index.internalId()).count() == rowCount(index.parent()) - 1)
          return Qt::Unchecked;
        else
          return Qt::PartiallyChecked;
      } else {
        return Qt::Checked;
      }
    }
    return d->falseStateMap.value(index.internalId()).contains(index.row() - 1) ? Qt::Unchecked : Qt::Checked;
  }

  return QVariant();
}

bool FilterModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  Q_D(FilterModel);

  if (!index.isValid())
    return false;

  if (role == Qt::CheckStateRole && isSourceSectionData(index) && index.column() == 0) {
    // if first row then all indexes are updated
    if (index.row() == 0) {
      if (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked) {
        d->falseStateMap.remove(index.internalId());
        emit dataChanged(index, this->index(rowCount(index.parent()) - 1, index.column(), index.parent()));
      } else {
        QSet<int> falseStates;
        int falseStatesCount = d->groupedValues.at(index.internalId()).count();
        for (int i = 0; i < falseStatesCount; ++i)
          falseStates.insert(i);
        d->falseStateMap.insert(index.internalId(), falseStates);

        emit dataChanged(index, this->index(rowCount(index.parent()) - 1, index.column(), index.parent()));
      }
      emit filterChanged();
    // not first rows
    } else {
      if (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked)
        d->falseStateMap[index.parent().row()].remove(index.row() - 1);
      else
        d->falseStateMap[index.parent().row()].insert(index.row() - 1);
      emit dataChanged(index, index);
      if (!d->falseStateMap.value(index.parent().row()).count()) {
        d->falseStateMap.remove(index.parent().row());
        emit dataChanged(this->index(0, index.column(), index.parent()), this->index(0, index.column(), index.parent()));
      }
      emit filterChanged();
    }

    return true;
  }

  return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags FilterModel::flags(const QModelIndex& index) const
{
  //Q_D(const ColumnFilterModel);
  if (!index.isValid()) return 0;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
  if (index.internalId() != -1 && index.column() == 0) {
    flags |= Qt::ItemIsUserCheckable;
    if (index.row() == 0) flags |= Qt::ItemIsTristate;
  }
  return flags;
}

void FilterModel::sourceModelColumnsInserted(const QModelIndex& parent, int start, int end)
{
  Q_D(FilterModel);
  if (d->sourceRootIndex != parent) return;

  if (d->orientation == Qt::Horizontal) {
    refresh();
  } else {
    refresh();
  }
}

void FilterModel::sourceModelColumnsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  Q_D(FilterModel);
  if (d->sourceRootIndex != parent) return;
  if (d->orientation == Qt::Horizontal) {} else {}
}

void FilterModel::sourceModelColumnsRemoved(const QModelIndex &parent, int start, int end)
{
  Q_D(FilterModel);
  if (d->sourceRootIndex != parent) return;
  if (d->orientation == Qt::Horizontal) {
    refresh();
  } else {
    refresh();
  }
}

void FilterModel::sourceModelDataChanged(const QModelIndex& topLeft
                                               , const QModelIndex& bottomRight)
{
  Q_D(FilterModel);
  if (topLeft.parent() != d->sourceRootIndex) return;

  if (d->orientation == Qt::Horizontal) {
    for (int i = topLeft.column(); i <= bottomRight.column(); ++i) {
      d->groupedValues.insert(i, d->sourceModelSection(i));
      d->falseStateMap.remove(i);
    }
  } else {
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
      d->groupedValues.insert(i, d->sourceModelSection(i));
      d->falseStateMap.remove(i);
    }
  }
  reset();
}

void FilterModel::sourceModelHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
  Q_D(FilterModel);
  if (orientation == d->orientation)
    emit dataChanged(index(first, 0, QModelIndex()), index(last, 0, QModelIndex()));
}

void FilterModel::sourceModelModelReset()
{
  refresh();
}

void FilterModel::sourceModelRowsInserted(const QModelIndex& parent, int start, int end)
{
  Q_D(FilterModel);
  if (parent != d->sourceRootIndex) return;
  if (d->orientation == Qt::Vertical) {
    refresh();
  } else {
    refresh();
  }
}

void FilterModel::sourceModelRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  Q_D(FilterModel);
  if (d->sourceRootIndex != parent) return;
  if (d->orientation == Qt::Vertical) {} else {}
}

void FilterModel::sourceModelRowsRemoved(const QModelIndex& parent, int start, int end)
{
  Q_D(FilterModel);
  if (d->sourceRootIndex != parent) return;
  if (d->orientation == Qt::Vertical) {
    refresh();
  } else {
    refresh();
  }
}

void FilterModel::refresh()
{
  Q_D(FilterModel);
  d->loadDataFromSourceModel();
  reset();
  emit filterChanged();
}

FilterModelPrivate::FilterModelPrivate()
  : sourceModel(&nullModel), sourceRole(Qt::DisplayRole), orientation(Qt::Horizontal) {}

FilterModelPrivate::~FilterModelPrivate() {}

void FilterModelPrivate::loadDataFromSourceModel()
{
  falseStateMap.clear();
  groupedValues.clear();
  if (orientation == Qt::Horizontal) {
    for (int column = 0; column < sourceModel->columnCount(sourceRootIndex); ++column) {
      groupedValues.append(sourceModelSection(column));
    }
  } else {
    for (int row = 0; row < sourceModel->rowCount(sourceRootIndex); ++row) {
      groupedValues.append(sourceModelSection(row));
    }
  }
}

QMap<QVariant, QSet<int> > FilterModelPrivate::sourceModelSection(int section)
{
  QMap<QVariant, QSet<int> > result;
  QVariant sourceItemData;
  if (orientation == Qt::Horizontal) {
    for (int i = 0; i < sourceModel->rowCount(sourceRootIndex); ++i) {
      sourceItemData = sourceModel->index(i, section, sourceRootIndex).data(sourceRole);
      result[sourceItemData].insert(i);
    }
  } else {
    for (int i = 0; i < sourceModel->columnCount(sourceRootIndex); ++i) {
      sourceItemData = sourceModel->index(section, i, sourceRootIndex).data(sourceRole);
      result[sourceItemData].insert(i);
    }
  }
  return result;
}

bool FilterModel::isSourceSectionIndex(const QModelIndex &index) const
{
  return index.internalId() == -1;
}

bool FilterModel::isSourceSectionData(const QModelIndex &index) const
{
  return 0 <= index.internalId() && index.internalId() < rowCount(QModelIndex());
}
