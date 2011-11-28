#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QtCore/QAbstractItemModel>

class FilterModelPrivate;
class FilterModel : public QAbstractItemModel
{
  Q_OBJECT

  Q_DECLARE_PRIVATE(FilterModel);

  void init(Qt::Orientation orientation);
protected:
  FilterModel(FilterModelPrivate& dd, Qt::Orientation orientation, QObject* parent);
  const QScopedPointer<FilterModelPrivate> d_ptr;

public:
  FilterModel(Qt::Orientation orientation, QObject* parent = 0);
  virtual ~FilterModel();

  virtual void setSourceModel(QAbstractItemModel* sourceModel);
  QAbstractItemModel* sourceModel() const;

  void setSourceRootIndex(const QModelIndex& rootIndex);
  QModelIndex sourceRootIndex() const;

  void setSourceRole(int role);
  int sourceRole() const;

  void setOrientation(Qt::Orientation orientation);
  Qt::Orientation orientation() const;

  QSet<int> filteredSections() const;

  virtual QModelIndex index(int row, int column,
                            const QModelIndex& parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex& child) const;

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

signals:
  void filterChanged();

protected slots:
  virtual void sourceModelColumnsInserted(const QModelIndex& parent, int start, int end);
  virtual void sourceModelColumnsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
  virtual void sourceModelColumnsRemoved(const QModelIndex& parent, int start, int end);
  virtual void sourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
  virtual void sourceModelHeaderDataChanged(Qt::Orientation orientation, int first, int last);
  virtual void sourceModelModelReset();
  virtual void sourceModelRowsInserted(const QModelIndex& parent, int start, int end);
  virtual void sourceModelRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
  virtual void sourceModelRowsRemoved(const QModelIndex& parent, int start, int end);

protected:
  void refresh();

  bool isSourceSectionIndex(const QModelIndex& index) const;
  bool isSourceSectionData(const QModelIndex& index) const;
};

#endif //#ifndef FILTERMODEL_H
