#ifndef PGQUERYVIEW_P_H
#define PGQUERYVIEW_P_H

#include "qtsqltableview_p.h"
#include "pgqueryview.h"

#include <QtSql/QSqlIndex>

class PGQueryModel;
class PGQueryViewPrivate : public QtSqlTableViewPrivate
{
  Q_DECLARE_PUBLIC(PGQueryView);

public:
  PGQueryViewPrivate();
  virtual ~PGQueryViewPrivate();

  PGQueryModel *model;

  QSqlIndex lastUsed;
  int lastUsedColumn;
  QPoint lastIndexPos;
};

#endif // PGQUERYVIEW_P_H
