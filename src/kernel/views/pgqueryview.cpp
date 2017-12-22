#include "pgqueryview.h"
#include "private/pgqueryview_p.h"

#include <QtDebug>

#include <QtCore/QRegExp>

#include <QtSql/QSqlField>

#include "qtheaderview.h"

#include "pgquerymodel.h"

PGQueryView::PGQueryView(QWidget* parent)
  : QtSqlTableView(*new PGQueryViewPrivate, parent)
{
  init();
}

PGQueryView::PGQueryView(PGQueryViewPrivate& dd, QWidget* parent)
  : QtSqlTableView(dd, parent)
{
  init();
}

void PGQueryView::init()
{
  setObjectName("pgQueryView");
  //Q_D(PGQueryView);
}

void PGQueryView::setModel(PGQueryModel* model)
{
  Q_D(PGQueryView);

  if (d->model)
    // Disconnect from old model
    disconnectFromModel(d->model);

  QtSqlTableView::setModel(model);
  d->model = model;

  if (d->model)
    // Connect to new model
    connectToModel(d->model);
}


void PGQueryView::setItemDelegateForColumn(int column, QAbstractItemDelegate* delegate)
{
  //Q_D(PGQueryView);
  QtSqlTableView::setItemDelegateForColumn(column, delegate);
}

void PGQueryView::setItemDelegateForColumn(const QString& column, QAbstractItemDelegate* delegate)
{
  Q_D(PGQueryView);
  if (!d->model) return;
  QSqlRecord record = d->model->record();
  QtSqlTableView::setItemDelegateForColumn(record.indexOf(column), delegate);
}


void PGQueryView::disconnectFromModel(PGQueryModel* model)
{
  //Q_D(PGQueryView);
  // отключаемся от всех сигналов
  disconnect(model, 0, this, 0);
}

void PGQueryView::connectToModel(PGQueryModel* model)
{
  connect(model, SIGNAL(preRefresh()),
          this, SLOT(preRefresh()));
  connect(model, SIGNAL(postRefresh()),
          this, SLOT(postRefresh()));
}

void PGQueryView::preRefresh()
{
  Q_D(PGQueryView);
  if (!d->model) return;
  d->lastUsed = d->model->primaryKey(currentIndex().row());

  d->lastUsedColumn = currentIndex().column();

  d->lastIndexPos.setX(currentIndex().column());
  d->lastIndexPos.setY(currentIndex().row());
}

void PGQueryView::postRefresh()
{
  Q_D(PGQueryView);
  if (!d->model) return;
  int row = d->model->rowForPrimaryKey(d->lastUsed);
  //QModelIndex idx = d->model->index(row, d->lastUsedColumn, rootIndex());
  QModelIndex idx = d->model->index(d->lastIndexPos.y(), d->lastIndexPos.x(), rootIndex());
  setCurrentIndex(idx);
  scrollTo(idx, PositionAtCenter);
}

void PGQueryView::hideColumn(int column)
{
  Q_D(PGQueryView);
  if (!d->model) return;
  d->horizontalHeader->hideSection(column);
}


void PGQueryView::hideColumn(const QString& column)
{
  Q_D(PGQueryView);
  if (!d->model) return;
  hideColumn(d->model->record().indexOf(column));
}

void PGQueryView::hideColumns(const QStringList& columns)
{
  for (int i = 0; i < columns.count(); ++i)
    hideColumn(columns.at(i));
}

void PGQueryView::refresh()
{
  Q_D(PGQueryView);
  if (d->model) d->model->refresh();
}

void PGQueryView::freezeSqlColumns(const QStringList& columns)
{
  Q_D(PGQueryView);
  if (!d->model) return;
  for (int i = 0; i < columns.count(); ++i)
    setColumnFrozen(d->model->record().indexOf(columns.at(i)), true);
}

void PGQueryView::handleSortIndicatorsChanged()
{
  Q_D(PGQueryView);
  QtSqlTableView::handleSortIndicatorsChanged();
  d->model->refresh();
}

PGQueryViewPrivate::PGQueryViewPrivate()
  : QtSqlTableViewPrivate()
  , model(0){}

PGQueryViewPrivate::~PGQueryViewPrivate() {}
