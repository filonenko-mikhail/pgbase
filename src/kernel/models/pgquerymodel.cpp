#include "pgquerymodel.h"
#include "private/pgquerymodel_p.h"

#include <QtCore/QStringList>
#include <QtCore/QMimeData>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>

//#include <QtGui/QTextDocument>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>

namespace{
  QString toHtml(const QList<QStringList>& plainText, const QString& tableTag = "table"
                 , const QString& tdTag = "td"
                 , const QString& trTag = "tr")
  {

    QString htmlText(QString("<%1>"));

    for (int i = 0; i < plainText.count(); ++i) {
      htmlText.append("<%2>");
      QStringList items = plainText.at(i);
      for (int g = 0; g < items.count(); ++g) {
        htmlText.append("<%3>");
        htmlText.append(items.at(g));
        htmlText.append("</%3>");
      }
      htmlText.append("</%2>");
    }
    htmlText.append("</%1>");
    return htmlText.arg(tableTag, trTag, tdTag);
  }

  QString toCsv(const QList<QStringList>& plainText, const QChar& delimiter = QLatin1Char(',')
                            ,const QChar& enclose = QLatin1Char('"')
                            , const QString& terminator = QString(QLatin1Char('\n')))
  {
    if (!plainText.count())
      return QString::null;
    QStringList resultRow;
    QStringList resultRows;
    QString resultValue;
    foreach(QStringList row, plainText) {
      foreach(QString value, row) {
        //value.replace(QRegExp("(\r\n|\n)"), " ");
        if(!enclose.isNull()) {
          resultValue.append(enclose);
          resultValue.append(value.replace(enclose, QString(2, enclose)));
          resultValue.append(enclose);
        } else
          resultValue.append(value);

        resultRow.append(resultValue);
        resultValue.clear();
      }
      resultRows.append(resultRow.join(delimiter));
      resultRow.clear();
    }
    return resultRows.join(terminator);
  }
}

#define PRINT_DEBUG(x) { qWarning() << Q_FUNC_INFO; qWarning() << x; }

PGSqlRelation::PGSqlRelation()
  :QHash<QVariant, QVariant>(){}

PGSqlRelation::PGSqlRelation(const PGSqlRelation &other)
  :QHash<QVariant, QVariant>(other)
, mDatabase(other.mDatabase)
, mTableName(other.mTableName)
, mIndexColumn(other.mIndexColumn)
, mDisplayColumn(other.mDisplayColumn){

  const_iterator otherIt = other.begin();
  const_iterator otherEnd = other.end();

  for (;otherIt != otherEnd;++otherIt)
    insert(otherIt.key(), otherIt.value());
}

PGSqlRelation::~PGSqlRelation()
{
  this->~QHash<QVariant, QVariant>();
}

PGSqlRelation& PGSqlRelation::operator=(const PGSqlRelation& other)
{
  mDatabase = other.mDatabase;
  mTableName = other.mTableName;
  mIndexColumn = other.mIndexColumn;
  mDisplayColumn = other.mDisplayColumn;

  const_iterator otherIt = other.begin();
  const_iterator otherEnd = other.end();

  for (;otherIt != otherEnd;++otherIt)
    insert(otherIt.key(), otherIt.value());
  return *this;
}

bool PGSqlRelation::setRelation(const QString &tableName
                                , const QString &indexColumn
                                , const QString &displayColumn
                                , QSqlDatabase database)

{
  mDatabase = database.isValid()?database:QSqlDatabase::database();
  mTableName = tableName;
  mIndexColumn = indexColumn;
  mDisplayColumn = displayColumn;
  return refresh();
}

bool PGSqlRelation::refresh()
{
  clear();
  QSqlQuery sqlQuery(mDatabase);

  QSqlRecord record = mDatabase.record(mTableName);
  QSqlDriver *driver = mDatabase.driver();

  //if (record.contains(mIndexColumn) && record.contains(mDisplayColumn)) {
    QSqlRecord selectRecord;
    selectRecord.append(record.field(mIndexColumn));
    selectRecord.append(record.field(mDisplayColumn));

    //QString query = mDatabase.driver()->sqlStatement(QSqlDriver::SelectStatement, mTableName, selectRecord, false);
    QString query = QString("SELECT %1 as first_column, %2 as second_column FROM %3")
                    .arg(driver->escapeIdentifier(mIndexColumn, QSqlDriver::FieldName)
                         , mDisplayColumn
                         , driver->escapeIdentifier(mTableName, QSqlDriver::TableName));
    //qDebug() << query;
    sqlQuery.setForwardOnly(true);
    if (sqlQuery.exec(query)) {
      while (sqlQuery.next())
        insert(sqlQuery.value(0), sqlQuery.value(1));
      return true;
    } else {
      PRINT_DEBUG(sqlQuery.lastError().text())
    }
  //}
  return false;
}

PGSqlRelation::operator const QHash<QVariant, QVariant>&() const
{
  return *this;
}

PGSqlRelation::operator QHash<QVariant, QVariant> &() const
{
  return *const_cast<PGSqlRelation*>(this);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const PGSqlRelation &relation)
{
  dbg.nospace() << "SqlRelation("
      << "database:" << relation.mDatabase << ' '
      << "table:" << relation.mTableName << ' '
      << "key column:" << relation.mIndexColumn << ' '
      << "display column:" << relation.mDisplayColumn
      << ')';

  return dbg.space();
}
#endif  // #ifndef QT_NO_DEBUG_STREAM

PGQueryModel::PGQueryModel(QObject* parent)
  :QAbstractTableModel(parent)
  , d_ptr(new PGQueryModelPrivate)
{
  init();
}

PGQueryModel::PGQueryModel(PGQueryModelPrivate &dd, QObject* parent)
  :QAbstractTableModel(parent)
  , d_ptr(&dd)
{
  init();
}

PGQueryModel::~PGQueryModel()
{
}

void PGQueryModel::init()
{
  setObjectName("pgQueryModel");

  Q_D(PGQueryModel);
  d->q_ptr = this;
  setSupportedDragActions(Qt::CopyAction);
}

QSqlError PGQueryModel::lastError() const
{
  Q_D(const PGQueryModel);
  return d->lastError;
}

QString PGQueryModel::queryString() const
{
  Q_D(const PGQueryModel);
  return d->queryString;
}

QSqlDatabase PGQueryModel::database() const
{
  Q_D(const PGQueryModel);
  return d->database;
}

QSqlIndex PGQueryModel::primaryKey(int row) const
{
  Q_D(const PGQueryModel);

  return d->primaryKeyForRow(row);

}

void PGQueryModel::setPrimaryKey(const QSqlIndex& primaryKey)
{
  Q_D(PGQueryModel);
  d->primaryKey = primaryKey;
}

QSqlRecord PGQueryModel::record(int row) const
{
  Q_D(const PGQueryModel);

  QSqlRecord result = d->record;
  if (-1 < row && row < rowCount()) {
    for (int i = 0; i < d->record.count(); ++i) {
      result.setValue(i, index(row, i).data(Qt::EditRole));
      result.setGenerated(i, false);
    }
  }

  return result;
}

void PGQueryModel::setQuery(const QString &query, QSqlDatabase database)
{
  Q_D(PGQueryModel);

  d->filter.clear();
  d->columnHeaders.clear();
  d->lastError.setType(QSqlError::NoError);

  d->queryString = query;
  d->database = database.isValid()?database:QSqlDatabase::database();
}

void PGQueryModel::clear()
{
  Q_D(PGQueryModel);
  d->clearModel();
  reset();
}


/*!
  Номер строки для первичного ключа
  */
int PGQueryModel::rowForPrimaryKey(const QSqlIndex& primaryKey) const
{
  Q_D(const PGQueryModel);

  return d->rowForPrimaryKey(primaryKey);
}

bool PGQueryModel::addRelation(const QString& column, const QString& tableName
                                , const QString& indexColumn, const QString& displayColumn)
{
  Q_D(PGQueryModel);
  return addRelation(d->record.indexOf(column), tableName, indexColumn, displayColumn);
}

bool PGQueryModel::addRelation(int column, const QString& tableName
                                , const QString& indexColumn, const QString& displayColumn)
{
  Q_D(PGQueryModel);
  if (d->addRelation(column, tableName, indexColumn, displayColumn)) {
    emit dataChanged(index(0, column), index(rowCount() - 1, column));
    return true;
  }
  return false;
}

QString PGQueryModel::relationPart(int column, RelationPart part)
{
  Q_D(PGQueryModel);
  QString result;
  if (d->relations.contains(column)) {
    switch (part) {
      case RelationTableName:
      result = d->relations.value(column).tableName();
      break;
      case RelationIndexColumn:
      result = d->relations.value(column).indexColumn();
      break;
      case RelationDisplayColumn:
      result = d->relations.value(column).displayColumn();
      break;
    }
  }
  return result;
}

void PGQueryModel::removeRelation(const QString& column)
{
  Q_D(PGQueryModel);
  removeRelation(d->record.indexOf(column));
}

void PGQueryModel::removeRelation(int column)
{
  Q_D(PGQueryModel);
  if (d->removeRelation(column))
    emit dataChanged(index(0, column), index(rowCount() - 1, column));
}

void PGQueryModel::setFilter(const QString& filter)
{
  Q_D(PGQueryModel);
  d->filter = filter;
}

QString PGQueryModel::filter() const
{
  Q_D(const PGQueryModel);
  return d->filter;
}

/*!
  Данные модели, внешние связи перезагружаются. Перед обновлением генерируется preRefresh(), после postRefresh()
  */
void PGQueryModel::refresh()
{
  Q_D(PGQueryModel);

  emit preRefresh();

  if (d->values.count()) {
    beginRemoveRows(QModelIndex(), 0, d->values.count() - 1);
    d->values.clear();
    endRemoveRows();
  }

  d->select();
  d->refreshRelations();
  QList<int> columns = d->relations.keys();
  for (int i = 0; i < columns.count(); ++i)
    emit dataChanged(index(0, columns.at(i)), index(rowCount() - 1, columns.at(i)));

  emit headerDataChanged(Qt::Horizontal, 0, d->record.count() - 1);

  //reset();
  emit postRefresh();
}

QModelIndex PGQueryModel::indexStr(int row, const QString& column, const QModelIndex &parent) const
{
  Q_D(const PGQueryModel);
  return QAbstractTableModel::index(row, d->record.indexOf(column), parent);
}

/*!
  Доступно и можно выделять/таскать
  */
Qt::ItemFlags PGQueryModel::flags(const QModelIndex &/*index*/) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

/*!
  Кол-во загруженный строк
  */
int PGQueryModel::rowCount(const QModelIndex& parent) const
{
  Q_D(const PGQueryModel);
  return parent.isValid()?0:d->values.count();
}

/*!
  Кол-во столбцов в запросе/модели
  */
int PGQueryModel::columnCount(const QModelIndex& parent) const
{
  Q_D(const PGQueryModel);
  return parent.isValid()?0:d->record.count();
}

/*!
  Возращает значения для роли для заголовков, для горизонтального все роли, для вертикального возращается пордяковый номер строки и только DisplayRole
  */
QVariant PGQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_D(const PGQueryModel);

  if (orientation == Qt::Horizontal) {
      QVariant value;
      if (d->columnHeaders.contains(section))
        value = d->columnHeaders.value(section).value(role);
      if (role == Qt::DisplayRole && !value.isValid())
          value = d->columnHeaders.value(section).value(Qt::EditRole);
      if (value.isValid())
          return value;
      if ((role == Qt::DisplayRole || role == Qt::EditRole) && section < d->record.count())
          return d->record.fieldName(section);
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

/*!
  Устновка данных для заголовка модели, поддерживаются все роли и только горизонтальный заголовок.
  */
bool PGQueryModel::setHeaderData(int section
                                  , Qt::Orientation orientation
                                  , const QVariant& value, int role)
{
  Q_D(PGQueryModel);
  if (orientation != Qt::Horizontal ||  0 > section ||  section >= columnCount())
      return false;

  d->columnHeaders[section][role] = value;
  emit headerDataChanged(orientation, section, section);
  return true;
}

/*!
  Если установлена внешняя связь вернет значение из связанной таблицы
  */
QVariant PGQueryModel::data(const QModelIndex& index, int role) const
{
  Q_D(const PGQueryModel);

  if (!index.isValid()) return QVariant();

  int column = index.column();
  int row = index.row();

  QVariant itemData;
  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
  case Qt::ToolTipRole:
  case Qt::StatusTipRole:
    itemData = d->values.at(row).at(column);
    break;
  default: ;//return itemData;
  }

  // Если установлен внешний ключ для столбца
  if (d->relations.contains(column)) {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::StatusTipRole:
      itemData = d->relations.value(column).value(itemData, itemData);
      break;
    default: ;
    }
  }

  if (role == Qt::ToolTipRole || role == Qt::StatusTipRole)
    itemData = itemData.isNull()?"NULL":itemData.toString();

  return itemData;
}

/*!
  Порядок сортировки для данного столбца
  */
QString PGQueryModel::sortClause(int column) const
{
  Q_D(const PGQueryModel);
  return d->orderedColumns.value(column);
}
/*!
  Убрать сортировку со столбца. После данного метода необходим вызов refresh() для обновления модели
  */
void PGQueryModel::removeSort(int column)
{
  sort(column, QString::null);
}
/*!
  Убрать сортировку со всех столбцов. После данного метода необходим вызов refresh() для обновления модели
  */
void PGQueryModel::clearSort()
{
  sort(-1);
}

/*!
  Сортировать данные по столбцу. Данная сортировка добавляется к уже установленной. После установки сортировки необходимо обновить модель методом refresh().
Для сброса все сортировок вместо column необходимо передать -1, или вызвать clearSort.
  */
void PGQueryModel::sort(int column, Qt::SortOrder order)
{
  QString orderString = (order == Qt::AscendingOrder)?"ASC":"DESC";
  sort(column, orderString);
}

void PGQueryModel::sort(const QString& column, Qt::SortOrder order)
{
  Q_D(PGQueryModel);
  sort(d->record.indexOf(column), order);
}

void PGQueryModel::sort(const QString &column, Qt::SortOrder order, bool nullsFirst)
{
  Q_D(PGQueryModel);
  sort(d->record.indexOf(column), order, nullsFirst);
}

void PGQueryModel::sort(int column, Qt::SortOrder order, bool nullsFirst)
{
  QString orderString = (order == Qt::AscendingOrder)?"ASC":"DESC";
  orderString += (nullsFirst)?" NULLS FIRST":" NULLS LAST";
  sort(column, orderString);
}

void PGQueryModel::sort(const QString &column, const QString &subClause)
{
  Q_D(PGQueryModel);
  sort(d->record.indexOf(column), subClause);
}

void PGQueryModel::sort(int column, const QString &subClause)
{
  Q_D(PGQueryModel);

  if (column < 0) {
    d->orderedColumnsList.clear();
    d->orderedColumns.clear();
    return;
  }

  if (subClause.isEmpty()) {
    d->orderedColumnsList.removeOne(column);
    d->orderedColumns.remove(column);
  } else {
    d->orderedColumnsList.append(column);
    d->orderedColumns.insert(column, subClause);
  }
}

QList<int> PGQueryModel::sortedColumns() const
{
  Q_D(const PGQueryModel);
  return d->orderedColumnsList;
}

QStringList PGQueryModel::mimeTypes() const
{
  // create
  QStringList types(QAbstractTableModel::mimeTypes());
  types << "text/plain" << "text/csv;encoding=utf8" << "text/html";
  return types;
}

QMimeData* PGQueryModel::mimeData(const QModelIndexList &indexes) const
{
  //Q_D(const PGQueryModel);
  QMimeData *mimeData = QAbstractTableModel::mimeData(indexes);
  if (!mimeData) mimeData = new QMimeData();
  QMap<int, QMap<int, QVariant> > mapGrid;
  QStringList row;
  QList<QStringList> linesList;

  foreach(QModelIndex item, indexes) {
    mapGrid[item.row()][item.column()] = item.data();
  }

  QMap<int, QMap<int, QVariant> >::ConstIterator rowIt = mapGrid.constBegin();
  QMap<int, QMap<int, QVariant> >::ConstIterator rowEnd = mapGrid.constEnd();
  QMap<int, QVariant>::ConstIterator columnIt;
  QMap<int, QVariant>::ConstIterator columnEnd;

  for (;rowIt != rowEnd; ++rowIt) {
    columnIt = rowIt.value().constBegin();
    columnEnd = rowIt.value().constEnd();
    for (;columnIt != columnEnd; ++columnIt) {
      row.append(columnIt.value().toString());
    }
    linesList.append(row);
    row.clear();
  }

  QString htmlText = toHtml(linesList);

  QString plainText = toCsv(linesList, '\t', QChar(), "\r\n");
  QString csvText = toCsv(linesList).toUtf8();

  mimeData->setText(plainText);
  mimeData->setHtml(htmlText);
  mimeData->setData("text/csv;encoding=utf8", csvText.toUtf8());

  return mimeData;
}

QModelIndexList PGQueryModel::match(const QModelIndex &start, int role, const QVariant &value, int hits
                                     , Qt::MatchFlags flags) const
{
  // model tests
  if (role == -1 || !hits)
    return QModelIndexList();

  Q_D(const PGQueryModel);
  QModelIndexList result;

  //	MatchExactly = 0,
  //	MatchContains = 1,
  //	MatchStartsWith = 2,
  //	MatchEndsWith = 3,
  //	MatchRegExp = 4,
  //	MatchWildcard = 5,
  //	MatchFixedString = 8,
  //	MatchCaseSensitive = 16,
  //	MatchWrap = 32,
  //	MatchRecursive = 64
  //	MatchReverse = 0x8000

  // We can do match only with regexp, but i think that qstring-based compare more quickly.

  // support flags Qt::MatchCaseSensitive
  // String based searching
  // Regexp based searching
  QRegExp rexp;
  QString strValue;
  Qt::CaseSensitivity sens = Qt::CaseInsensitive;

  switch (flags & (Qt::MatchExactly
                   | Qt::MatchContains
                   | Qt::MatchStartsWith
                   | Qt::MatchEndsWith
                   | Qt::MatchRegExp
                   | Qt::MatchWildcard
                   | Qt::MatchFixedString))
  {
  case Qt::MatchContains:
  case Qt::MatchStartsWith:
  case Qt::MatchEndsWith:
  case Qt::MatchFixedString:
    strValue = value.toString();
    sens = (flags & Qt::MatchCaseSensitive)?Qt::CaseSensitive:Qt::CaseInsensitive;
    break;
  case Qt::MatchRegExp:
    rexp = value.toRegExp();
    sens = (flags & Qt::MatchCaseSensitive)?Qt::CaseSensitive:Qt::CaseInsensitive;
    rexp.setCaseSensitivity(sens);
    break;
  case Qt::MatchWildcard:
    rexp = QRegExp(value.toString());
    sens = (flags & Qt::MatchCaseSensitive)?Qt::CaseSensitive:Qt::CaseInsensitive;
    rexp.setCaseSensitivity(sens);
#ifdef Q_OS_UNIX
    rexp.setPatternSyntax(QRegExp::WildcardUnix);
#else
    rexp.setPatternSyntax(QRegExp::Wildcard);
#endif
    break;
  case Qt::MatchExactly:;
  }

  // slow but легко поддерживаемый/расширяемый поиск
  // Для быстрого поиска необходимо минимизировать количество проверок в цикле, что увеличивает количество циклов.
  // Start iteration from this column

  // Куда смещаемся (вправо-вниз/влево-вверх)
  int columnDelta = 1;
  int rowDelta = 1;
  // С какой позвиции начнинаем
  int column = 0;
  int row = 0;
  // Сколько нашли
  int founded = 0;
  // С какого угла колонки начинаем
  int rowStart = 0;
  int columnStart = 0;

  // Валидный индекс
  if (start.isValid()) {
    column = start.column();
    row = start.row();
  }

  // Заканчиваем справа внизу
  int matchRowCount = rowCount();
  int matchColumnCount = columnCount();
  // Начинаем с данной строки
  int g = row;
  // Надо искать в обратном направлении
  if ((flags & Qt::MatchBackward)) {
    // Начинаем с правого нижнего
    rowStart = rowCount() - 1;
    columnStart = rowCount() - 1;

    // Заканчиваем левым верхним углом
    matchColumnCount = -1;
    matchRowCount = -1;
    // Влево-вверх
    columnDelta = -1;
    rowDelta = -1;
  }

  // Ищем вниз-направо, либо вверх-налево
  for (int i = column; i != matchColumnCount; i += columnDelta) {
    for (; g != matchRowCount; g += rowDelta) {
      switch (role) {
      case Qt::DisplayRole: {
          switch (flags & (Qt::MatchExactly
                           | Qt::MatchContains
                           | Qt::MatchStartsWith
                           | Qt::MatchEndsWith
                           | Qt::MatchRegExp
                           | Qt::MatchFixedString))
          {
          case Qt::MatchExactly: {
              if (d->relations.contains(i)) {
                if (d->relations.value(i).value(d->values.value(g).value(i)) == value) {
                  result.append(createIndex(g, i));
                  ++ founded;
                }
              } else
                if (d->values.value(g).value(i) == value) {
                result.append(createIndex(g, i));
                ++ founded;
              }
            } break;
          case Qt::MatchContains: {
              if (d->relations.contains(i)) {
                if (d->relations.value(i).value(d->values.value(g).value(i))
                  .toString().contains(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }else
                if (d->values.value(g).value(i).toString().contains(strValue, sens)) {
                result.append(createIndex(g, i)); ++ founded;
              }
            }break;
          case Qt::MatchStartsWith: {
              if (d->relations.contains(i)) {
                if (d->relations.value(i).value(d->values.value(g).value(i))
                  .toString().startsWith(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              } else
                if (d->values.value(g).value(i).toString().startsWith(strValue, sens)) {
                result.append(createIndex(g, i)); ++ founded;
              }
            }
            break;
          case Qt::MatchEndsWith: {
              if (d->relations.contains(i)) {
                if (d->relations.value(i).value(d->values.value(g).value(i))
                  .toString().endsWith(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }else
                if (d->values.value(g).value(i).toString().endsWith(strValue, sens)) {
                result.append(createIndex(g, i)); ++ founded;
              }
            } break;
          case Qt::MatchFixedString: {
              if (d->relations.contains(i)) {
                if (!d->relations.value(i).value(d->values.value(g).value(i))
                  .toString().compare(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              } else
                if (!d->values.value(g).value(i).toString().compare(strValue, sens)) {
                result.append(createIndex(g, i)); ++ founded;
              }
            }break;
          case Qt::MatchRegExp: {
              if (d->relations.contains(i)) {
                if (rexp.exactMatch(d->relations.value(i).value(d->values.value(g).value(i))
                  .toString())) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }else
                if (rexp.exactMatch(d->values.value(g).value(i).toString())) {
                result.append(createIndex(g, i)); ++ founded;
              }
            }break;
          }
        } break;
      case Qt::EditRole: {
          switch (flags & (Qt::MatchExactly
                           | Qt::MatchContains
                           | Qt::MatchStartsWith
                           | Qt::MatchEndsWith
                           | Qt::MatchRegExp
                           | Qt::MatchFixedString))
          {
          case Qt::MatchExactly: {
              if (d->values.value(g).value(i) == value) {
                result.append(createIndex(g, i)); ++ founded;
              }
            } break;
          case Qt::MatchContains: {
              if (d->values.value(g).value(i).toString().contains(strValue, sens)) {
                result.append(createIndex(g, i)); ++ founded;
              }
            }break;
          case Qt::MatchStartsWith: {
              if (d->values.value(g).value(i).toString().startsWith(strValue, sens)) {
                result.append(createIndex(g, i)); ++ founded;
              }
            } break;
          case Qt::MatchEndsWith: {
              if (d->values.value(g).value(i).toString().endsWith(strValue, sens)) {
                result.append(createIndex(g, i)); ++ founded;
              }
            } break;
          case Qt::MatchFixedString: {
              if (!d->values.value(g).value(i).toString().compare(strValue, sens)) {
                result.append(createIndex(g, i)); ++ founded;
              }
            } break;
          case Qt::MatchRegExp: {
              if (rexp.exactMatch(d->values.value(g).value(i).toString())) {
                result.append(createIndex(g, i)); ++ founded;
              }
            } break;
          }
        } break;
      }
      if (founded == hits)
        goto exit_match;
    }
    g = rowStart;
  }

  // Ищем с левого верхнего, либо с правого нижнего угла, до стартовой позиции
  if (flags & Qt::MatchWrap) {
    for (int i = columnStart; i != column; i += columnDelta) {
      for (; g != matchRowCount; g += rowDelta) {
        switch (role) {
        case Qt::DisplayRole: {
            switch (flags & (Qt::MatchExactly
                             | Qt::MatchContains
                             | Qt::MatchStartsWith
                             | Qt::MatchEndsWith
                             | Qt::MatchRegExp
                             | Qt::MatchFixedString))
            {
            case Qt::MatchExactly: {
                if (d->relations.contains(i)) {
                  if (d->relations.value(i).value(d->values.value(g).value(i)) == value) {
                    result.append(createIndex(g, i)); ++ founded;
                  }
                }else
                  if (d->values.value(g).value(i) == value) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              } break;
            case Qt::MatchContains: {
                if (d->relations.contains(i)) {
                  if (d->relations.value(i).value(d->values.value(g).value(i))
                    .toString().contains(strValue, sens)) {
                    result.append(createIndex(g, i)); ++ founded;
                  }
                }else
                  if (d->values.value(g).value(i).toString().contains(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }break;
            case Qt::MatchStartsWith: {
                if (d->relations.contains(i)) {
                  if (d->relations.value(i).value(d->values.value(g).value(i)).toString()
                    .startsWith(strValue, sens)) {
                    result.append(createIndex(g, i)); ++ founded;
                  }
                }else
                  if (d->values.value(g).value(i).toString().startsWith(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }
              break;
            case Qt::MatchEndsWith: {
                if (d->relations.contains(i)) {
                  if (d->relations.value(i).value(d->values.value(g).value(i))
                    .toString().endsWith(strValue, sens)) {
                    result.append(createIndex(g, i)); ++ founded;
                  }
                }else
                  if (d->values.value(g).value(i).toString().endsWith(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              } break;
            case Qt::MatchFixedString: {
                if (d->relations.contains(i)) {
                  if (!d->relations.value(i).value(d->values.value(g).value(i)).toString().compare(strValue, sens)) {
                    result.append(createIndex(g, i)); ++ founded;
                  }
                }else
                  if (!d->values.value(g).value(i).toString().compare(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }break;
            case Qt::MatchRegExp: {
                if (d->relations.contains(i)) {
                  if (rexp.exactMatch(d->relations.value(i).value(d->values.value(g).value(i)).toString())) {
                    result.append(createIndex(g, i)); ++ founded;
                  }
                }else
                  if (rexp.exactMatch(d->values.value(g).value(i).toString())) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }break;
            }
          }break;
        case Qt::EditRole: {
            switch (flags & (Qt::MatchExactly
                             | Qt::MatchContains
                             | Qt::MatchStartsWith
                             | Qt::MatchEndsWith
                             | Qt::MatchRegExp
                             | Qt::MatchFixedString))
            {
            case Qt::MatchExactly: {
                if (d->values.value(g).value(i) == value) {
                  result.append(createIndex(g, i));
                  ++ founded;
                }
              } break;
            case Qt::MatchContains: {
                if (d->values.value(g).value(i).toString().contains(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }break;
            case Qt::MatchStartsWith: {
                if (d->values.value(g).value(i).toString().startsWith(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }
              break;
            case Qt::MatchEndsWith: {
                if (d->values.value(g).value(i).toString().endsWith(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              } break;
            case Qt::MatchFixedString: {
                if (!d->values.value(g).value(i).toString().compare(strValue, sens)) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }break;
            case Qt::MatchRegExp: {
                if (rexp.exactMatch(d->values.value(g).value(i).toString())) {
                  result.append(createIndex(g, i)); ++ founded;
                }
              }break;
            }
          } break;
        }
        if (founded == hits)
          goto exit_match;
      }
      g = rowStart;
    }
    for (; g != row; g+=rowDelta) {
      switch (role) {
      case Qt::DisplayRole: {
          switch (flags & (Qt::MatchExactly
                           | Qt::MatchContains
                           | Qt::MatchStartsWith
                           | Qt::MatchEndsWith
                           | Qt::MatchRegExp
                           | Qt::MatchFixedString))
          {
          case Qt::MatchExactly: {
              if (d->relations.contains(column)) {
                if (d->relations.value(column).value(d->values.value(g).value(column)) == value) {
                  result.append(createIndex(g, column));
                  ++ founded;
                }
              }else
                if (d->values.value(g).value(column) == value) {
                result.append(createIndex(g, column));
                ++ founded;
              }
            } break;
          case Qt::MatchContains: {
              if (d->relations.contains(column)) {
                if (d->relations.value(column).value(d->values.value(g).value(column))
                  .toString().contains(strValue, sens)) {
                  result.append(createIndex(g, column));
                }
              }else
                if (d->values.value(g).value(column).toString().contains(strValue, sens)) {
                result.append(createIndex(g, column)); ++ founded;
              }
            }break;
          case Qt::MatchStartsWith: {
              if (d->relations.contains(column)) {
                if (d->relations.value(column).value(d->values.value(g).value(column))
                  .toString().startsWith(strValue, sens)) {
                  result.append(createIndex(g, column)); ++ founded;
                }
              }else
                if (d->values.value(g).value(column).toString().startsWith(strValue, sens)) {
                result.append(createIndex(g, column)); ++ founded;
              }
            }
            break;
          case Qt::MatchEndsWith: {
              if (d->relations.contains(column)) {
                if (d->relations.value(column).value(d->values.value(g).value(column))
                  .toString().endsWith(strValue, sens)) {
                  result.append(createIndex(g, column)); ++ founded;
                }
              }else
                if (d->values.value(g).value(column).toString().endsWith(strValue, sens)) {
                result.append(createIndex(g, column)); ++ founded;
              }
            } break;
          case Qt::MatchFixedString: {
              if (d->relations.contains(column)) {
                if (!d->relations.value(column).value(d->values.value(g).value(column))
                  .toString().compare(strValue, sens)) {
                  result.append(createIndex(g, column)); ++ founded;
                }
              }else
                if (!d->values.value(g).value(column).toString().compare(strValue, sens)) {
                result.append(createIndex(g, column)); ++ founded;
              }
            }break;
          case Qt::MatchRegExp: {
              if (d->relations.contains(column)) {
                if (rexp.exactMatch(d->relations.value(column).value(
                    d->values.value(g).value(column)).toString())) {
                  result.append(createIndex(g, column)); ++ founded;
                }
              }else
                if (rexp.exactMatch(d->values.value(g).value(column).toString())) {
                result.append(createIndex(g, column)); ++ founded;
              }
            }break;
          }
        }break;
      case Qt::EditRole: {
          switch (flags & (Qt::MatchExactly
                           | Qt::MatchContains
                           | Qt::MatchStartsWith
                           | Qt::MatchEndsWith
                           | Qt::MatchRegExp
                           | Qt::MatchFixedString))
          {
          case Qt::MatchExactly: {
              if (d->values.value(g).value(column) == value) {
                result.append(createIndex(g, column));
                ++ founded;
              }
            } break;
          case Qt::MatchContains: {
              if (d->values.value(g).value(column).toString().contains(strValue, sens)) {
                result.append(createIndex(g, column)); ++ founded;
              }
            }break;
          case Qt::MatchStartsWith: {
              if (d->values.value(g).value(column).toString().startsWith(strValue, sens)) {
                result.append(createIndex(g, column)); ++ founded;
              }
            }
            break;
          case Qt::MatchEndsWith: {
              if (d->values.value(g).value(column).toString().endsWith(strValue, sens)) {
                result.append(createIndex(g, column)); ++ founded;
              }
            } break;
          case Qt::MatchFixedString: {
              if (!d->values.value(g).value(column).toString().compare(strValue, sens)) {
                result.append(createIndex(g, column)); ++ founded;
              }
            }break;
          case Qt::MatchRegExp: {
              if (rexp.exactMatch(d->values.value(g).value(column).toString())) {
                result.append(createIndex(g, column)); ++ founded;
              }
            }break;
          }
        } break;
      }
      if (founded == hits)
        goto exit_match;
    }
  }

  exit_match:
  return result;
}

PGQueryModelPrivate::PGQueryModelPrivate(){}

PGQueryModelPrivate::~PGQueryModelPrivate(){}

/*!
  Загрузка данных из запроса
  */
bool PGQueryModelPrivate::select()
{
  Q_Q(PGQueryModel);
  QSqlQuery selectQuery(selectStatment(), database);
  selectQuery.setForwardOnly(true);

  QSqlRecord newRecord = selectQuery.record();

  //if (newRecord.)
  if (newRecord.count() > record.count()) {
    q->beginInsertColumns(QModelIndex(), record.count(), newRecord.count() - 1);
    record = newRecord;
    q->endInsertColumns();
  } else if (newRecord.count() < record.count()){
    q->beginRemoveColumns(QModelIndex(), newRecord.count(), record.count() - 1);
    record = newRecord;
    q->endRemoveColumns();
  }

  if (selectQuery.lastError().isValid()) {
    PRINT_DEBUG(selectQuery.lastError().text());
    return false;
  }

#ifdef DEMO_VERSION


#endif

  q->beginInsertRows(QModelIndex(), 0, selectQuery.size() - 1);
  while (selectQuery.next())
    values.append(valuesForRecord(selectQuery.record()));
  q->endInsertRows();

  return true;
}

/*!
  Полностью очищаем модель, столбцы и данные и запрос
  */
void PGQueryModelPrivate::clearModel()
{
  columnHeaders.clear();
  values.clear();

  orderedColumns.clear();

  queryString.clear();
  filter.clear();
  database = QSqlDatabase();

  relations.clear();
}

bool PGQueryModelPrivate::addRelation(int sourceColumn
                                      , const QString &tableName
                                      , const QString &indexColumn, const QString &displayColumn)
{
  PGSqlRelation relation;
  if (relation.setRelation(tableName, indexColumn, displayColumn, database)) {
    relations.insert(sourceColumn, relation);
    return true;
  }
  return false;
}

bool PGQueryModelPrivate::removeRelation(int sourceColumn)
{
  return relations.remove(sourceColumn);
}

void PGQueryModelPrivate::refreshRelations()
{
  QHash<int, PGSqlRelation>::iterator it = relations.begin();
  QHash<int, PGSqlRelation>::iterator end = relations.end();
  for (;it != end; ++it)
    it.value().refresh();
}

QString PGQueryModelPrivate::selectStatment()
{
  return QString("select a.* from (%1) as a %2 %3").arg(queryString, whereStatment(), orderStatment());
}

QString PGQueryModelPrivate::whereStatment()
{
  return (filter.isEmpty())?QString::null:QString("WHERE %1").arg(filter);
}

QString PGQueryModelPrivate::orderStatment()
{
  QStringList sortingColumnsClause;
  QString localOrderClause = (orderedColumnsList.isEmpty())?"":"ORDER BY ";

  for (int i = 0; i < orderedColumnsList.count(); ++i) {
    sortingColumnsClause << record.fieldName(orderedColumnsList.at(i))
        + " " + orderedColumns.value(orderedColumnsList.at(i));
  }

  return localOrderClause + sortingColumnsClause.join(",");
}

QSqlIndex PGQueryModelPrivate::primaryKeyForRow(int row) const
{
  QSqlIndex result = primaryKey;

  if (-1 < row && row < values.count())
    for (int i = 0; i < result.count(); ++i) {
      result.setValue(i, values.value(row).value(record.indexOf(result.fieldName(i))));
      result.setGenerated(i, true);
    }

  return result;
}

int PGQueryModelPrivate::rowForPrimaryKey(const QSqlIndex &primaryKey) const
{
  int row = -1;

  QHash<int, int> pkeyMap;
  bool success;
  int pkeyColumnCount = primaryKey.count();
  for (int i = 0; i < pkeyColumnCount; ++i)
    pkeyMap[i] = record.indexOf(primaryKey.fieldName(i));

  for (int i = 0; i < values.count(); ++i) {
    success = true;
    for (int g = 0; g < pkeyColumnCount; ++g)
      if (primaryKey.value(g) != values.at(i).at(pkeyMap.value(g))) {
        success = false;
        break;
      }
    if (success) {
      row = i;
      break;
    }
  }
  return row;
}

QList<QVariant> PGQueryModelPrivate::valuesForRecord(const QSqlRecord &record) const
{
  QList<QVariant> result;
  for (int i = 0; i < record.count(); ++i)
    if (record.isNull(i))
      result.append(QVariant(record.field(i).type()));
    else
      result.append(castedValue(record.field(i), record.value(i)));

  return result;
}

QVariant PGQueryModelPrivate::castedValue(const QSqlField &field, const QVariant &value) const
{
  QVariant result(field.type());
  if (value.isNull()) return result;

  switch(field.type()) {
  case QVariant::Bool:
    result = value.toBool();
    break;
  case QVariant::UInt:
    result = value.toUInt();
    break;
  case QVariant::ULongLong:
    result = value.toULongLong();
    break;
  case QVariant::LongLong:
    result = value.toLongLong();
    break;
  case QVariant::Double:
    result = value.toDouble();
    break;
  case QVariant::Int:
    result = value.toInt();
    break;
  case QVariant::DateTime:
    result = value.toDateTime();
    break;
  case QVariant::Date:
    result = value.toDate();
    break;
  default:
    result = value;
    break;
  }

  return result;
}
