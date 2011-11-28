#ifndef PGTABLEVIEW_P_H
#define PGTABLEVIEW_P_H

#include "pgtableview.h"
#include "private/pgqueryview_p.h"

class PGTableModel;

class PGTableViewPrivate : public PGQueryViewPrivate
{
  Q_DECLARE_PUBLIC(PGTableView);
public:
  PGTableViewPrivate();
};


#endif // PGTABLEVIEW_P_H
