#ifndef PGQUERYMODEL_H
#define PGQUERYMODEL_H

#define FETCH_COUNT 255

#include <QtCore/QVariant>
#include <QtCore/QHash>
#include <QtCore/QDebug>
#include <QtGui/QColor>

namespace Qt {
  // big value for non-colision
  enum MatchFlagEx {
    MatchBackward = 0x8000
  };
//	enum MatchFlag {
////			MatchExactly = 0,
////			MatchContains = 1,
////			MatchStartsWith = 2,
////			MatchEndsWith = 3,
////			MatchRegExp = 4,
////			MatchWildcard = 5,
////			MatchFixedString = 8,
////			MatchCaseSensitive = 16,
////			MatchWrap = 32,
////			MatchRecursive = 64,
//			MatchBackward = 0x8000
//	};
//	Q_DECLARE_FLAGS(MatchFlags, MatchFlag)
}

inline uint qHash(const QVariant& value)
{
  QString stringType;
  switch (value.type()) {
  case QVariant::UInt:
  case QVariant::ULongLong:
  case QVariant::Int:
  case QVariant::LongLong:
  case QVariant::Double:
    stringType = "Number";
    break;
  default:
    stringType = value.typeName();
    break;
  }

  QString stringValue = value.isNull()?"NULL":value.toString();
  return qHash(static_cast<const QString &>(QString("%1(%2)").arg(stringType, stringValue)));
}


#include <QtCore/QStringList>
#include <QtCore/QAbstractTableModel>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlIndex>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "modelinterfaces.h"

/*!
  Класс внешнего ключа для PGQueryModel.
*/
class PGSqlRelation : public QHash<QVariant, QVariant>
{
  QSqlDatabase mDatabase;
  QString mTableName;
  QString mIndexColumn;
  QString mDisplayColumn;
public:
  PGSqlRelation();
  PGSqlRelation(const PGSqlRelation &other);
  virtual ~PGSqlRelation();

  PGSqlRelation& operator=(const PGSqlRelation& other);

  bool setRelation(const QString& tableName
                   , const QString& indexColumn
                   , const QString& displayColumn
                   , QSqlDatabase database = QSqlDatabase());

  bool refresh();
  inline QSqlDatabase database() const {return mDatabase;};
  inline QString tableName() const {return mTableName;}
  inline QString indexColumn() const {return mIndexColumn;}
  inline QString displayColumn() const {return mDisplayColumn;}
  bool isValid() const;

  operator const QHash<QVariant, QVariant>&() const;
  operator QHash<QVariant, QVariant>&() const;

  friend QDebug operator<<(QDebug dbg, const PGSqlRelation &relation);
};

Q_DECLARE_TYPEINFO(PGSqlRelation, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const PGSqlRelation &);
#endif
/*!
  \ingroup models
  Модель для отображения результата выполнения sql запроса для postgresql.
*/
class PGQueryModelPrivate;
class PGQueryModel : public QAbstractTableModel, public IRefreshModel
{
  Q_OBJECT
  Q_INTERFACES(IRefreshModel)

  Q_DECLARE_PRIVATE(PGQueryModel);

  Q_PROPERTY (QString filter READ filter WRITE setFilter);

  Q_ENUMS(RelationPart)
  // Meta Flags for TableView
  void init();
protected:
  PGQueryModel(PGQueryModelPrivate &dd, QObject * parent);

  const QScopedPointer<PGQueryModelPrivate> d_ptr;
public:
  /*!
    Создает пустую модель \a parent.
   */
  explicit PGQueryModel(QObject* parent = 0);
  virtual ~PGQueryModel();

  /*!
    Первичный ключ для строки row. Если row = -1, тогда первичный ключ содержит sql поля без значений (QSqlFields::value().isNull). Если модель неактивна или первичный ключ не установлен возвращается пустой первичный ключ.
    \sa setPrimaryKey()
    */
  virtual QSqlIndex primaryKey(int row = -1) const;

  /*!
    \sa setPrimaryKey(),
    */
  virtual QSqlRecord record(int row = -1) const;
  /*!
    Возвращает последнюю ошибку полученную при работе модели.
    */
  QSqlError lastError() const;
  /*!
    Функция возвращает соритровку для данного столбца. Если сортировки нет, то результат равен Qt::AscendingOrder
   */
  QString sortClause(int column) const;
  /*!
    Функция убирает сортировку с данного столбца. Данная функция требует последующего вызова refresh().
    \sa refresh(), clearSort(), sort()
    */
  void removeSort(int column);
  /*!
    Функция убирает сортировку с модели. Требует последующего вызова refresh().
    \sa refresh(), removeSort(), sort()
    */
  void clearSort();

  //! wrapper
  virtual void sort(const QString& column, Qt::SortOrder order = Qt::AscendingOrder);
  /*!
    Добавлет сортировку по данному столбцу. Если column = -1, то сортировка убирается со всех столбцов.
    \note Требует последующего вызова refresh()
    \sa refresh(), removeSort(), clearSort()
    */
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  //! wrapper
  virtual void sort(const QString& column, Qt::SortOrder order, bool nullsFirst);
  //! wrapper
  virtual void sort(int column, Qt::SortOrder order, bool nullsFirst);

  //! wrapper
  virtual void sort(const QString& column, const QString& subClause);
  //! general method
  virtual void sort(int column, const QString& subClause);

  QList<int> sortedColumns() const;

  /*!
    Устанавливает фильтр для sql запроса. filter должен быть sql выражанием без ключевого слова WHERE. Для того чтобы применить фильтр необходимо вызвать метод refresh().
    \warning Метод fetchMore() загружает данные текущего запроса. Метод refresh пересоздает запрос с установленными фильтром и сортировками.
    \sa refresh(), filter()
    */
  virtual void setFilter(const QString& filter);
  /*!
    Возвращает установленный фильтр без ключевого слова WHERE
    \sa setFilter()
    */
  virtual QString filter() const;

  // AbstractSqlModel
  /*!
    Возвращает строку sql запроса. Может вернуться пустая строка, если запрос для модели не устанавливался.
    \sa setQueryString()
    */
  QString queryString() const;
  /*!
  \sa select()
    */
  void setQuery(const QString& query, QSqlDatabase database = QSqlDatabase());

  /*!
    Очищение модели
    */
  void clear();

  QSqlDatabase database() const;

  /*!
    Возвращает номер строки для данного первичного ключа. primaryKey должен содержать столбцы с установленными значениями
    QSqlRecord::setValue(). Если значения primaryKey не найдены результат будет равен -1.
    \sa QSqlRecord::setValue()
    */
  int rowForPrimaryKey(const QSqlIndex& primaryKey) const;

  /*!
    Добавить связь с другой таблицей. В ячейке модели в роли Qt::DisplayRole будет отображаться значение displayColumn таблицы tableName при значении indexColumn = data(Qt::EditRole);
  * @param column столбец данной модели
  * @param tableName таблица внешнего ключа
  * @param indexColumn связуемый столбец
  * @param displayColumn отображаемый столбец
  * @return bool результат
  \retval false связь не создана
  \retval true связь создана
  */
  bool addRelation(const QString& column
                   , const QString& tableName
                   , const QString& indexColumn
                   , const QString& displayColumn);
  /*!
    \overload
  */
  bool addRelation(int column
                   , const QString& tableName
                   , const QString& indexColumn
                   , const QString& displayColumn);
  /*!
    \overload
  */
  void removeRelation(const QString& column);
  /*!
    Удаляет внешний ключ с данного столбца модели
  * @param column
  */
  void removeRelation(int column);

  enum RelationPart {RelationTableName, RelationIndexColumn, RelationDisplayColumn};
  QString relationPart(int column, RelationPart part);

  QModelIndex indexStr(int row, const QString& column, const QModelIndex &parent = QModelIndex()) const;
  /*!
    Enabled, Selectable
  * @param index
  * @return Qt::ItemFlags
  */
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole);

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  /*!
    Searching value in all model data. Only Qt::Display and Qt::EditRole are supported.
    if start invalid search from (0,0). if flags | Qt::MatchBackward (qt.h) then search are created backward of start.
    hits = -1 all match indexes.
    */
  virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;

  /*!
    First format must be internal for use QAbstractItemModel::mimeData(), e.g. "application/myfancyitemmodel"
  * @return QStringList
  */
  virtual QStringList mimeTypes() const;
  virtual QMimeData* mimeData(const QModelIndexList &indexes) const;

public slots:
  /*!
     Обновляет данные модели. Столбцы, фильтр и сортировки не меняются. Загружается первая порция данных.
  */
  virtual void refresh();

signals:
  /*!
    Перед обновлением модели. В присоединенном слоте рекомендуется сделать primaryKey(currentRow())
  */
  void preRefresh();
  /*!
    После обновления модели. В присоединенном слоте рекомендуется сделать currentRow = rowForPrimaryKey();
  */
  void postRefresh();

protected:
  /*!
    Устанавливает первичный ключ для данной модели. Данная возможность может использоваться при наследовании от данного класа.
    \sa primaryKey()
    */
  virtual void setPrimaryKey(const QSqlIndex& primaryKey);

  QSqlRecord primaryKeyFromRecord(const QSqlRecord &rec) const;
  QSqlRecord primaryKeyFromRow(int row);
};

/// WORKAROUND FOR COLORIZE MODEL
class OddColorModel : public PGQueryModel
{
  Q_OBJECT
public:
  OddColorModel(QObject *parent)
    :PGQueryModel(parent)
  {

  }

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
    if (role == Qt::BackgroundColorRole) {
      if (PGQueryModel::data(this->index(index.row(), 1), Qt::DisplayRole).toInt() % 2)
        return QColor(0xfffe73);
      else
        return Qt::white;
    }
    return PGQueryModel::data(index, role);
  }
};

#endif // PGQUERYMODEL_H
