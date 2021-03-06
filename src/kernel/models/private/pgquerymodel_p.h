#ifndef PGQUERYMODEL_P_H
#define PGQUERYMODEL_P_H

#include "pgquerymodel.h"

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QDebug>

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtCore/QMultiHash>

#define PRINT_DEBUG(x) { qWarning() << Q_FUNC_INFO; qWarning() << x; }

class PGQueryModelPrivate
{
  Q_DECLARE_PUBLIC(PGQueryModel);
protected:
  PGQueryModel * q_ptr;
public:
  PGQueryModelPrivate();
  virtual ~PGQueryModelPrivate();

  QSqlDatabase database;
  QList<int> orderedColumnsList;
  QMap<int, QString> orderedColumns;

  QString selectStatment();
  QString whereStatment();
  QString orderStatment();

  QSqlError lastError;
  QSqlRecord record;
  QSqlIndex primaryKey;

  QString queryString;


  QString filter; // WHERE clause without "WHERE"

  QList<QList<QVariant> > values;
  QHash<int, QHash<int, QVariant> > columnHeaders;


  QHash<int, PGSqlRelation> relations;

  /*!
    Очистка всей модели, в том числе фильтра, сортировок, первичного ключа.
После данной функции необходимо вывать reset();		*/
  void clearModel();

  bool select();

  bool addRelation(int sourceColumn, const QString& tableName, const QString& indexColumn
                   , const QString& displayColumn);

  bool removeRelation(int sourceColumn);

  void refreshRelations();

  QSqlIndex primaryKeyForRow(int row) const;
  int rowForPrimaryKey(const QSqlIndex& primaryKey) const;

  QList<QVariant> valuesForRecord(const QSqlRecord& record) const;

  QVariant castedValue(const QSqlField &field, const QVariant &value) const;
};

#endif // SQLQUERYMODEL_P_H
