#ifndef PGQUERYVIEW_H
#define PGQUERYVIEW_H

#include "qtsqltableview.h"

#include <QtGui/QTableView>
#include <QtGui/QStyledItemDelegate>

#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>

class PGQueryModel;
class QSqlRecord;
class QtHeaderView;

class PGQueryViewPrivate;
class PGQueryView : public QtSqlTableView
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(PGQueryView)

  void init();
protected:
  PGQueryView(PGQueryViewPrivate& dd, QWidget* parent);
public:
  explicit PGQueryView(QWidget* parent = 0);

  virtual void setModel(PGQueryModel* model);

  virtual void setItemDelegateForColumn(int column, QAbstractItemDelegate* delegate);
  virtual void setItemDelegateForColumn(const QString& column, QAbstractItemDelegate* delegate);

  void freezeSqlColumns(const QStringList& columns);

public slots:
  void hideColumn(int column);
  void hideColumn(const QString& column);
  void hideColumns(const QStringList& columns);

  virtual void refresh();

protected slots:
  virtual void handleSortIndicatorsChanged();

  virtual void preRefresh();
  virtual void postRefresh();

protected:

private slots:
  // internal
  virtual void disconnectFromModel(PGQueryModel* model);
  virtual void connectToModel(PGQueryModel* model);
};


#endif // PGQUERYVIEW_H
