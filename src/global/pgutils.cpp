#include "pgutils.h"

#include <QtCore/QVariant>

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>

QString PGInsertClause(const QSqlDriver *driver, const QString &tableName, const QSqlRecord &record)
{
  if (tableName.isEmpty()
    || record.isEmpty())
    return QString::null;

  QString result(driver->sqlStatement(QSqlDriver::InsertStatement, tableName, record, false));
  QStringList returnFields;
  for (int i = 0; i < record.count(); ++i)
    returnFields << driver->escapeIdentifier(record.fieldName(i), QSqlDriver::FieldName);
  result.append(" RETURNING ");
  result.append(returnFields.join(","));
  return result;
}

QString PGUpdateClause(QSqlDriver *driver, const QString &tableName, const QSqlRecord &record, const QSqlRecord &primaryKey)
{

  if (tableName.isEmpty()
    || record.isEmpty()
    || primaryKey.isEmpty())
    return QString::null;

  QString result(driver->sqlStatement(QSqlDriver::UpdateStatement, tableName, record, false));
  if (result.isEmpty())
    return QString::null;
  result.append(' ');
  result.append(driver->sqlStatement(QSqlDriver::WhereStatement, tableName, primaryKey, false));
  QStringList returnFields;
  for (int i = 0; i < record.count(); ++i)
    returnFields << driver->escapeIdentifier(record.fieldName(i), QSqlDriver::FieldName);
  result.append(" RETURNING ");
  result.append(returnFields.join(","));
  return result;
}

/*!
 */
QString PGDeleteClause(QSqlDriver* driver
                                    , const QString& tableName
                                    , const QSqlRecord& primaryKey)
{
  if (tableName.isEmpty()
    || primaryKey.isEmpty())
    return QString::null;

  QString result(driver->sqlStatement(QSqlDriver::DeleteStatement, tableName, primaryKey, false));
  result.append(' ');
  result.append(driver->sqlStatement(QSqlDriver::WhereStatement, tableName, primaryKey, false));

  return result;
}

/*!
 */
QStringList PGDeleteClauses(QSqlDriver* driver
                                           , const QString& tableName
                                           , const QList<QSqlRecord>& primaryKeys
                                           , int rowsInOneQuery)
{

  if (tableName.isEmpty()
    || primaryKeys.isEmpty())
    return QStringList();

  QString escapedTableName(driver->escapeIdentifier(tableName, QSqlDriver::TableName));
  QStringList resultList;
  QString result("DELETE FROM %1 WHERE %2");

  QString currentColumnValuePair;
  QStringList primaryKeyValues;
  QStringList whereList;

  int pkeyValuesCount = primaryKeys.at(0).count();
  if (!pkeyValuesCount)
    return QStringList();

  QList<QSqlRecord>::const_iterator currentPkey = primaryKeys.begin();
  QList<QSqlRecord>::const_iterator endPkey = primaryKeys.end();

  int g = 0;
  while (currentPkey != endPkey) {
    for (int i = 0; i < pkeyValuesCount; ++i) {
      currentColumnValuePair.append(driver->escapeIdentifier((*currentPkey).fieldName(i), QSqlDriver::FieldName));
      if ((*currentPkey).value(i).isNull()) {
        currentColumnValuePair.append(" IS NULL ");
      } else {
        currentColumnValuePair.append('=');
        currentColumnValuePair.append(driver->formatValue((*currentPkey).field(i)));
      }
      primaryKeyValues.append(currentColumnValuePair);
      currentColumnValuePair.clear();
    }

    whereList.append("(" + primaryKeyValues.join(" AND ") + ")");
    primaryKeyValues.clear();

    ++currentPkey;
    ++g;
    if (!(g % rowsInOneQuery)) {
      result = result.arg(escapedTableName, whereList.join(" OR "));
      resultList.append(result);
      whereList.clear();
      result = QString("DELETE FROM %1 WHERE %2 ");
    }
  }
  if (g % rowsInOneQuery) {
    result = result.arg(escapedTableName, whereList.join(" OR "));
    resultList.append(result);
  }

  return resultList;
}

QString PGSelectClause(QSqlDriver* driver
                       , const QString& tableName
                       , const QSqlRecord& record
                       , const QSqlRecord& primaryKey)
{
  QString result(driver->sqlStatement(QSqlDriver::SelectStatement, tableName, record, false));
  result.append(' ');
  result.append(driver->sqlStatement(QSqlDriver::WhereStatement, tableName, primaryKey, false));
  return result;
}
