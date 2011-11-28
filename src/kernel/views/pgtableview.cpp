#include "pgtableview.h"
#include "private/pgtableview_p.h"

PGTableView::PGTableView(QWidget *parent)
  :PGQueryView(*new PGTableViewPrivate, parent)
{
  init();
}

PGTableView::PGTableView(PGTableViewPrivate &dd, QWidget *parent)
  :PGQueryView(dd, parent)
{
  init();
}

void PGTableView::init()
{
  //Q_D(PGTableView);
  setObjectName("pgTableView");

  // Настройка таблицы
  EditTriggers triggers = AnyKeyPressed
                          | EditKeyPressed
                          | SelectedClicked
                          | DoubleClicked;
  setEditTriggers(triggers); // Редактирование если выделена только одна ячейка
  // А также вызываем редактирование в обработке MouseRelease

  setAcceptDrops(true);
  setDropIndicatorShown(true);
}

PGTableViewPrivate::PGTableViewPrivate()
  :PGQueryViewPrivate(){}
