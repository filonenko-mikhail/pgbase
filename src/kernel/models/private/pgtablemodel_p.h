#ifndef PGTABLEMODEL_P_H
#define PGTABLEMODEL_P_H

#include "pgtablemodel.h"
#include "private/pgquerymodel_p.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>

class PGTableModelPrivate : public PGQueryModelPrivate
{
  Q_DECLARE_PUBLIC(PGTableModel);
public:
  PGTableModelPrivate();

  QString tableName;

  // В этой записи хранятся значения для вставляемой строки
  QSqlRecord newRow;
  // В этой записи хранятся значения по умолчанию
  QSqlRecord defaultValues;

  // В этой записи храним флаги "только для чтения" для столбцов
  QSet<int> readOnlyColumns; // column

  QUndoStack *undoStack;

  bool updateRowInDatabase(int row, QSqlRecord& record);
  void updateRowInModel(int row, const QSqlRecord& record);

  bool insertRowInDatabase(QSqlRecord& record);
  void insertRowInModel(int before, const QSqlRecord& record);

  void pushSetDataCommand(const QModelIndex& index, const QVariant &newValue, const QVariant &oldValue, int role);
};

#endif // PGTABLEMODEL_P_H
