#ifndef PGTABLEVIEW_H
#define PGTABLEVIEW_H

#include "pgqueryview.h"

/*!
*/
class PGTableViewPrivate;
class PGTableView : public PGQueryView
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(PGTableView)

  void init();
protected:
  PGTableView(PGTableViewPrivate &dd, QWidget* parent);
public:
  explicit PGTableView(QWidget *parent = 0);
};

#endif // PGTABLEVIEW_H
