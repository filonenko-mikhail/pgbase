#ifndef COLUMNFILTERMODEL_P_H
#define COLUMNFILTERMODEL_P_H

#include "filtermodel.h"

#include <QtCore/QSet>

class FilterModelPrivate
{
  Q_DECLARE_PUBLIC(FilterModel);
public:
  FilterModel * q_ptr;

  FilterModelPrivate();
  virtual ~FilterModelPrivate();

  QAbstractItemModel *sourceModel;
  QPersistentModelIndex sourceRootIndex;
  int sourceRole;
  Qt::Orientation orientation;

  // list of sections ( list of (value, list of rows) )
  QList< QMap<QVariant, QSet<int> > > groupedValues;

  // list of sections ( list of (roles, values) )
  QHash< int, QHash< int, QVariant> > columnHeaders;

  // list of sections ( list of row in this model )
  QHash< int, QSet<int> > falseStateMap;


  void loadDataFromSourceModel();
  QMap<QVariant, QSet<int> > sourceModelSection(int section);
};


#endif //#ifndef COLUMNFILTERMODEL_H
