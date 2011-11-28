#ifndef SQLUTILS_H
#define SQLUTILS_H

#include <QtCore/QString>
class QSqlDriver;

#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtCore/QVariant>

/*!
 Если !record.field.isGenerated || !record.field.isValid значение в запрос не добавляется
 */
QString PGInsertClause(const QSqlDriver* driver
                       , const QString& tableName
                       , const QSqlRecord& record);

/*!
 Если !record.field.isGenerated || !record.field.isValid значение в запрос не добавляется
 */
QString PGUpdateClause(QSqlDriver* driver
                       , const QString& tableName
                       , const QSqlRecord& record
                       , const QSqlRecord& primaryKey);

QString PGDeleteClause(QSqlDriver* driver
                         , const QString& tableName
                         , const QSqlRecord& primaryKey);

QStringList PGDeleteClauses(QSqlDriver* driver
                           , const QString& tableName
                           , const QList<QSqlRecord>& primaryKeys
                           , int rowsInOneQuery = 10);

QString PGSelectClause(QSqlDriver* driver
                                         , const QString& tableName
                                         , const QSqlRecord& record
                                         , const QSqlRecord& primaryKey = QSqlRecord());

inline QDataStream& operator >> (QDataStream& stream, QSqlField& field)
{
  QVariant::Type type;
  QVariant value;
  QString name;
  bool isAuto;
  bool isGenerated;
  bool isReadOnly;
  stream >> type
      >> isAuto
      >> isGenerated
      >> isReadOnly
      >> name
      >> value;
  field.setName(name);
  field.setAutoValue(isAuto);
  field.setGenerated(isGenerated);
  field.setReadOnly(isReadOnly);
  field.setType(type);
  field.setValue(value);
  return stream;
}

inline QDataStream& operator << (QDataStream& stream, const QSqlField& field)
{
  stream << field.type()
      << field.isAutoValue()
      << field.isGenerated()
      << field.isReadOnly()
      << field.name()
      << field.value();
  return stream;
}

inline QDataStream& operator >> (QDataStream& stream, QSqlRecord& record)
{
  int count;
  QSqlField field;
  stream >> count;
  for (int i = 0 ; i < count; ++i) {
    stream >> field;
    record.append(field);
  }
  return stream;
}

inline QDataStream& operator << (QDataStream& stream, const QSqlRecord& record)
{
  int count = record.count();
  stream << count;
  for (int i = 0; i < count; ++i)
    stream << record.field(i);
  return stream;
}

#endif // SQLUTILS_H
