#include "pgtablemodel.h"
#include "private/pgtablemodel_p.h"

#include <QtDebug>

#include <QtCore/QStringList>
#include <QtCore/QMimeData>


#include <QtGui/QUndoStack>

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlIndex>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>

#include "qtcsvparser.h"

#define PRINT_DEBUG(x) { \
  qWarning() << Q_FUNC_INFO; \
  qWarning() << x; \
  }

namespace PG {
  enum {
    UndoRedoSetDataId
  };
  enum {
    UndoRedoEditRole = Qt::UserRole + Qt::EditRole
    , UndoRedoDisplayRole = Qt::UserRole + Qt::DisplayRole
  };
}


class PGSetDataCommand : public QUndoCommand
{
public:
  PGSetDataCommand(PGTableModel* parentModel, const QModelIndex& index, const QVariant& data, const QVariant& oldData, int role
                   , QUndoCommand* parent = 0)
                     :QUndoCommand(parent)
                     , model(parentModel)
                     , modelIndex(index)
                     , newModelData(data)
                     , oldModelData(oldData)
                     , modelRole(role){};
  PGSetDataCommand(PGTableModel* parentModel, const QModelIndex& index, const QVariant& data, const QVariant& oldData, int role
                   , const QString& text, QUndoCommand* parent = 0)
    :QUndoCommand(text, parent)
    , model(parentModel)
    , modelIndex(index)
    , newModelData(data)
    , oldModelData(oldData)
    , modelRole(role){}

  virtual int id() const { return PG::UndoRedoSetDataId; }


  virtual void redo() {
    model->setData(modelIndex, newModelData, modelRole);
  }

  virtual void undo() {
    model->setData(modelIndex, oldModelData, modelRole);
  }
private:
  PGTableModel *model;
  QPersistentModelIndex modelIndex;
  QVariant newModelData;
  QVariant oldModelData;
  int modelRole;
};


PGTableModel::PGTableModel(QObject *parent)
  :PGQueryModel( *new PGTableModelPrivate(), parent)
{
  init();
}

PGTableModel::PGTableModel(PGTableModelPrivate &dd, QObject * parent)
  :PGQueryModel(dd, parent)
{
  init();
}

PGTableModel::~PGTableModel() {}

void PGTableModel::init()
{
  Q_D(PGTableModel);
  setObjectName("pgTableModel");
  d->undoStack = new QUndoStack(this);
}

void PGTableModel::setTable(const QString &tableName, QSqlDatabase database)
{
  Q_D(PGTableModel);

  database = database.isValid()?database:QSqlDatabase::database();

  // очищаем undo redo stack
  d->undoStack->clear();
  d->readOnlyColumns.clear();

  d->tableName = tableName;

  QSqlRecord newRecord = database.record(tableName);

  if (newRecord.count() > d->record.count()) {
    beginInsertColumns(QModelIndex(), d->record.count(), newRecord.count() - 1);
    d->record = newRecord;
    endInsertColumns();
  } else if (newRecord.count() < d->record.count()){
    beginRemoveColumns(QModelIndex(), newRecord.count(), d->record.count() - 1);
    d->record = newRecord;
    endRemoveColumns();
  }

  d->newRow = d->record;
  d->defaultValues = d->record;
  d->primaryKey = database.primaryIndex(d->tableName);

  setQuery(database.driver()->sqlStatement(QSqlDriver::SelectStatement
                                           , tableName
                                           , database.record(tableName)
                                           , false), database);

  // Получаем комментарии для столбцов
  QSqlQuery query(d->database);
  if (query.exec(QString("select objsubid, description from pg_description where objsubid > 0 and objoid = '%1'::regclass "
    " order by objsubid")
    .arg(d->tableName))) {
    QVariant headerValue;
    int column = 0;
    while(query.next()) {
      headerValue = query.value(1);
      if (!headerValue.isNull()) {
        d->columnHeaders[column][Qt::EditRole] = headerValue;
        d->columnHeaders[column][Qt::ToolTipRole] = headerValue;
        d->columnHeaders[column][Qt::StatusTipRole] = headerValue;
      }
      ++column;
    }
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
  }
}

QString PGTableModel::tableName() const
{
  Q_D(const PGTableModel);
  return d->tableName;
}

Qt::ItemFlags PGTableModel::flags(const QModelIndex &index) const
{
  Q_D(const PGTableModel);

  if (!index.isValid())
    return 0;

  Qt::ItemFlags result;

  // Можно делать все что sqlquerymodel позволяет
  result |= PGQueryModel::flags(index);

  // Можно писать?
  if (!d->readOnlyColumns.contains(index.column()))
    result |= Qt::ItemIsDropEnabled | Qt::ItemIsEditable;

  return result;
}

int PGTableModel::rowCount(const QModelIndex &parent) const
{
  //Q_D(const PGTableModel);

  if (!parent.isValid())
    return PGQueryModel::rowCount() + 1;

  return 0;
}

QVariant PGTableModel::data(const QModelIndex& item, int role) const
{
  Q_D(const PGTableModel);
  if (item.isValid()) {
    int row = item.row();
    int column = item.column();
    QVariant itemData;
    if (PGQueryModel::rowCount() == row) {
      switch (role) {
      case Qt::DisplayRole:
      case Qt::EditRole:
        if (d->newRow.isNull(column))
          itemData = QVariant(d->newRow.field(column).type());
        else
          itemData = d->castedValue(d->newRow.field(column), d->newRow.value(column));
      }

      // Если установлена внешняя связь
      if (d->relations.contains(column) && role == Qt::DisplayRole)
        itemData = d->relations.value(column).value(itemData, itemData);

      // Возвращаем значения вставляемой строки
      return itemData;
    }
  }
  return PGQueryModel::data(item, role);
}

/*!
  WORKAROUND
  Перегружаем чтобы не выполнялись ленивые вычисления
  */
bool PGTableModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    bool b = true;
    QMap<int, QVariant>::const_iterator it = roles.begin();
    QMap<int, QVariant>::const_iterator end = roles.end();
    for (; it != end; ++it)
        setData(index, it.value(), it.key());
    return b;
}

/*!
  Установка значения в модели, данное значения немедленно отправляется в базу данных
  */
bool PGTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  Q_D(PGTableModel);

  QVariant newValue = value;
  int row = index.row();
  int column = index.column();

  // столбец только для чтения
  if (d->readOnlyColumns.contains(index.column()))
    return false;

  bool fromUndoStack = false;
  if (role == PG::UndoRedoEditRole) {
    role = Qt::EditRole;
    fromUndoStack = true;
  }

  if (((role != Qt::EditRole) && (role != Qt::DisplayRole)) || !index.isValid())
    return false;

  if (role == Qt::DisplayRole) {
    if (d->relations.contains(column))
      newValue = d->relations.value(column).value(value);
    if (!newValue.isValid())
      return false;
    role = Qt::EditRole;
  }

  QVariant oldValue = index.data(role);
  //if ((oldValue == newValue) && (oldValue.isNull() == newValue.isNull()))
//    return true;

  // Если последняя строка, которая используется для добавления новых записей.
  if (row == PGQueryModel::rowCount()) {
    d->newRow.setValue(column, newValue);
    // Оповещаем что и где изменили
    emit dataChanged(index, index);

    QSqlQuery insertQuery(d->database);

    // Создаем запрос
    d->lastError.setType(QSqlError::NoError);
    // Если запрос успешно выполнен
    if (insertQuery.exec(PGInsertClause(d->database.driver(), d->tableName, d->newRow))) {
      // Если получили ответ с sql сервера (использовалась returning clause)
      insertQuery.next();
      d->newRow = insertQuery.record();
      // Обновляем внешние ключи
      for (int g = 0; g < d->newRow.count(); ++g) {
        if (d->relations.contains(g))
          if (!d->relations.value(g).contains(d->newRow.value(g)))
            d->relations[g].refresh();
      }

      d->newRow = d->defaultValues;

      // Вставляем строку со звездочкой
      beginInsertRows(QModelIndex(), row + 1, row + 1);
      d->insertRowInModel(row, insertQuery.record());
      endInsertRows();

      // Оповещаем об измененнии бывшей строки со звездочкой.
      emit dataChanged(this->index(row, 0),
                       this->index(row, columnCount() - 1));
      // Изменилась подпись на вертикальном заголовке
      emit headerDataChanged(Qt::Vertical, row, row);
    } else {
      // Ошибка не отображаем, так как это только попытка вставить строку
      // , возможно пользователь недоввел необходимые данные
      d->lastError = insertQuery.lastError();
      PRINT_DEBUG(insertQuery.lastQuery());
      PRINT_DEBUG(d->lastError.text());
    }

    // Создаем QUndoCommand
    if (!fromUndoStack)
      d->pushSetDataCommand(index, newValue, oldValue, PG::UndoRedoEditRole);
    // Так или иначе значение в модели сохраняется
    return true;
  } else {
    QSqlRecord updatingRecord = record(row);
    updatingRecord.setValue(column, newValue);
    updatingRecord.setGenerated(column, true);
    bool result;
    if ((result = d->updateRowInDatabase(row, updatingRecord))) {
      d->updateRowInModel(row, updatingRecord);

      // Обновляем внешние ключи
      for (int g = 0; g < d->newRow.count(); ++g) {
        if (d->relations.contains(g))
          if (!d->relations.value(g).contains(d->newRow.value(g)))
            d->relations[g].refresh();
      }
      emit dataChanged(this->index(row, 0), this->index(row, columnCount() - 1));
      if (!fromUndoStack)
        d->pushSetDataCommand(index, newValue, oldValue, PG::UndoRedoEditRole);
    }
    return result;
  }
  return false;
}

bool PGTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
  Q_D(PGTableModel);

  if (parent.isValid() || row < 0 || row >= PGQueryModel::rowCount() || !count)
    return false;

  if (row + count > d->values.count())
    count = d->values.count() - row;

  if (!count)
    return false;

  // Список первичнх ключей удаляемого набора строк
  QList<int> removedRows;
  QSqlQuery deleteQuery(d->database);
  for(int i = row; i < row + count; ++i) {
    QString query = PGDeleteClause(d->database.driver(), d->tableName, primaryKey(i));
    if (deleteQuery.exec(query)) {
      removedRows.append(i);
    } else {
      PRINT_DEBUG(deleteQuery.lastError().text());
    }
  }

  if (removedRows.count() == count) {
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    d->values.erase(d->values.begin() + row, d->values.begin() + row + count);
    endRemoveRows();
  } else {
    for (int i = 0; i < removedRows.count(); ++i) {
      beginRemoveRows(QModelIndex(), removedRows.at(i) - i, removedRows.at(i) - i);
      d->values.erase(d->values.begin() + removedRows.at(i) - i);
      endRemoveRows();
    }
  }

  return removedRows.count();
}

bool PGTableModel::dropMimeData(const QMimeData *data,
                                 Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  if (PGQueryModel::dropMimeData(data, action, row, column, parent))
    return true;

  if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction))
    return false;

  // if the drop is on an item, replace the data in the items
  if (parent.isValid() && row == -1 && column == -1) {
    if (data->hasFormat("text/csv")) {
      QtCsvParser parser;
      QByteArray csvData = data->data("text/csv");
      QString csvString(QString::fromUtf8(csvData));
      parser.setSource(csvString);
      int i = parent.row();
      int g = parent.column();
      QStringList currentLine = parser.parseLine(true);
      while (currentLine.count()) {
        for (int j = 0; j < currentLine.count() && (g + j < columnCount()); ++j)
          if (hasIndex(i, g + j))
            setData(index(i, g + j), currentLine.at(j));
        currentLine = parser.parseLine(true);
        ++i;
      }
      return true;
    } else if (data->hasText()) {
      QtCsvParser parser;
      QString string = data->text();
      parser.setSource(string);
      parser.setDelimiter('\t');
      int i = parent.row();
      int g = parent.column();
      QStringList currentLine = parser.parseLine(true);
      while (currentLine.count()) {
        for (int j = 0; j < currentLine.count() && (g + j < columnCount()); ++j)
          if (hasIndex(i, g + j))
            setData(index(i, g + j), currentLine.at(j));
        currentLine = parser.parseLine(true);
        ++i;
      }
      return true;
    }
  }
  return false;
}

/*!
  Для вертикального заголовка, для последней строки - "*"
  */
QVariant PGTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical && (section == PGQueryModel::rowCount())) {
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return QVariant("*");
    case Qt::ToolTipRole:
    case Qt::StatusTipRole:
      return tr("Row for inserting new record");
    default:;
    }
  }

  return PGQueryModel::headerData(section, orientation, role);
}

/*!
  Добавление строк, значения столбцов - 'DEFAULT'
  */
bool PGTableModel::insertRows(int row, int count, const QModelIndex& parent)
{
  Q_D(PGTableModel);

  if ((count < 1) || parent.isValid())
    return false;

  if (row < 0 || row > d->values.count())
    row = d->values.count();

  QSqlRecord insertingRecord;

  int inserted;
  for (inserted = 0; inserted < count; ++inserted) {
    insertingRecord = d->defaultValues;
    // Генерируем сигнал, может разработчик как-то будет заполнять строку
    emit primeInsert(row, insertingRecord);

    if (d->insertRowInDatabase(insertingRecord))
      d->insertRowInModel(row, insertingRecord);
  }

  // Если ничего не вставили, возвращаем 0
  return inserted;
}

bool PGTableModel::isColumnReadOnly(int section) const
{
  Q_D(const PGTableModel);
  return d->readOnlyColumns.contains(section);
}

void PGTableModel::setColumnReadOnly(const QString &section, bool readOnly)
{
  Q_D(PGTableModel);
  setColumnReadOnly(d->newRow.indexOf(section), readOnly);
}

void PGTableModel::setColumnReadOnly(int section, bool readOnly)
{
  Q_D(PGTableModel);
  if (readOnly)
    d->readOnlyColumns.insert(section);
  else
    d->readOnlyColumns.remove(section);
}

void PGTableModel::setColumnsReadOnly(const QStringList &sections)
{
  foreach (QString section, sections)
    setColumnReadOnly(section);
}

void PGTableModel::setDefault(const QString &section, const QVariant &value)
{
  Q_D(PGTableModel);
  setDefault(d->newRow.indexOf(section), value);
}

void PGTableModel::setDefault(int section, const QVariant &value)
{
  Q_D(PGTableModel);
  if (value.isNull()) {
    d->defaultValues.setNull(section);
    d->newRow.setNull(section);
  } else {
    d->defaultValues.setValue(section, d->castedValue(d->defaultValues.field(section), value));
    d->newRow.setValue(section, d->defaultValues.value(section));
  }
  emit dataChanged(index(0, section), index(0, section));
}

QVariant PGTableModel::defaultValue(int section) const
{
  Q_D(const PGTableModel);

  return d->castedValue(d->defaultValues.field(section), d->defaultValues.value(section));
}

void PGTableModel::clearDefaultValues()
{
  Q_D(PGTableModel);

  d->defaultValues.clearValues();

  emit dataChanged(index(rowCount(), 0), index(rowCount(), columnCount() - 1));
}

QUndoStack *PGTableModel::undoStack() const
{
  Q_D(const PGTableModel);
  return d->undoStack;
}

bool PGTableModel::setRecord(int row, const QSqlRecord &record)
{
  Q_D(PGTableModel);
  QSqlRecord newRecord(record);
  bool result;
  if ((result = d->updateRowInDatabase(row, newRecord)))
    d->updateRowInModel(row, newRecord);
  return result;
}

bool PGTableModel::insertRecord(int row, const QSqlRecord &record)
{
  Q_D(PGTableModel);
  bool result;
  QSqlRecord newRecord(record);
  if ((result = d->insertRowInDatabase(newRecord)))
    d->insertRowInModel(row, newRecord);
  return result;
}

bool PGTableModel::prependRecord(const QSqlRecord &record)
{
  //Q_D(PGTableModel);
  return insertRecord(0, record);
}

bool PGTableModel::appendRecord(const QSqlRecord &record)
{
  Q_D(PGTableModel);
  return insertRecord(d->values.count(), record);
}

void PGTableModel::refresh()
{
  Q_D(PGTableModel);
  d->undoStack->clear();
  PGQueryModel::refresh();
}


PGTableModelPrivate::PGTableModelPrivate()
  :PGQueryModelPrivate(){}

bool PGTableModelPrivate::updateRowInDatabase(int row, QSqlRecord &record)
{
  QSqlQuery query(database);
  QSqlDriver *driver = database.driver();

  lastError.setType(QSqlError::NoError);
  QString updateClause = PGUpdateClause(driver, tableName, record, primaryKeyForRow(row));

  if (!query.exec(updateClause)) {
    PRINT_DEBUG(query.lastError().text());
    return false;
  }
  query.next();
  record = query.record();
  return true;
}

void PGTableModelPrivate::updateRowInModel(int row, const QSqlRecord &record)
{
  QList<QList<QVariant> >::iterator rowIt = values.begin() + row;
  QList<QList<QVariant> >::iterator rowEnd = values.end();
  if (rowIt != rowEnd)
    (*rowIt) = valuesForRecord(record);
}


bool PGTableModelPrivate::insertRowInDatabase(QSqlRecord& record)
{
  QSqlQuery query(database);
  QSqlDriver *driver = database.driver();

  lastError.setType(QSqlError::NoError);
  QString insertClause = PGInsertClause(driver, tableName, record);

  if (!query.exec(insertClause)) {
    lastError = query.lastError();
    PRINT_DEBUG(query.lastError().text());
    return false;
  }
  query.next();
  record = query.record();
  return true;
}

void PGTableModelPrivate::insertRowInModel(int before, const QSqlRecord& record)
{
  QList<QList<QVariant> >::iterator rowIt = values.begin() + before;
  values.insert(rowIt, valuesForRecord(record));
}

void PGTableModelPrivate::pushSetDataCommand(const QModelIndex& index, const QVariant &newValue, const QVariant &oldValue, int role)
{
  Q_Q(PGTableModel);
  QString oldValueString;
  QString newValueString;
  if (relations.contains(index.column())) {
    oldValueString = relations.value(index.column()).value(oldValue).toString();
    newValueString = relations.value(index.column()).value(newValue).toString();
  } else {
    oldValueString = (oldValue.isNull())?QString(q->tr("NULL")):oldValue.toString();
    newValueString = (newValue.isNull())?QString(q->tr("NULL")):newValue.toString();
  }
  if (oldValueString.isEmpty())
    oldValueString = q->tr("Empty");
  if (newValueString.isEmpty())
    newValueString = q->tr("Empty");
  undoStack->push(new PGSetDataCommand(q, index, newValue, oldValue, role
                                          , q->tr("%1 -> %2").arg(oldValueString, newValueString)));
}
