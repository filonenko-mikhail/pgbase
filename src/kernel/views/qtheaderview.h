#ifndef QTHEADERVIEW_H
#define QTHEADERVIEW_H

#include <QtGui/QHeaderView>
#include <QtGui/QStyledItemDelegate>

class QtHeaderDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  QtHeaderDelegate(QObject* parent = 0);
  virtual ~QtHeaderDelegate();

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, Qt::Orientation orientation
                        , int section) const;

  void setEditorData(QWidget *editor, QAbstractItemModel *model, Qt::Orientation orientation
                     , int section) const;

  void setModelData(QWidget *editor, QAbstractItemModel *model, Qt::Orientation orientation
                    , int section) const;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class FilterModel;
class SectionsProxyModel;
/*!
  Класс представления заголовка
*/
class QtHeaderViewPrivate;
class QtHeaderView : public QHeaderView
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(QtHeaderView);

  Q_ENUMS(SortFlag);

  Q_PROPERTY(SortFlag sortFlag READ sortFlag WRITE setSortFlag);

  Q_PROPERTY(bool enableShowHideSections READ isShowHideSectionsEnabled WRITE setShowHideSectionsEnabled)
  Q_PROPERTY(bool enableFilter READ isFilterEnabled WRITE setFilterEnabled)
  Q_PROPERTY(bool enableFreeze READ isFreezeEnabled WRITE setFreezeEnabled)
  Q_PROPERTY(bool editable READ isEditable WRITE setEditable)

  void init();
protected:
  QtHeaderView(QtHeaderViewPrivate &dd, Qt::Orientation orientation, QWidget* parent);

  const QScopedPointer<QtHeaderViewPrivate> d_ptr;
public:

  QtHeaderView(Qt::Orientation orientation, QWidget* parent = 0);
  virtual ~QtHeaderView();

  /*!
    Тип сортировки отображаемой виджетом
  * @enum SortFlag	*/
  enum SortFlag {
    NoSort, //! Нельзя сортировать
    SimpleSort, //! Сортировка по одному столбцу
    MultiSort, //! Сортировка по нескольким столбцам
  };
  SortFlag sortFlag() const;
  void setSortFlag(SortFlag enable);

  bool isShowHideSectionsEnabled() const;
  void setShowHideSectionsEnabled(bool enabled);

  bool isFilterEnabled() const;
  void setFilterEnabled(bool enabled);

  bool isFreezeEnabled() const;
  void setFreezeEnabled(bool enabled);

  bool isEditable() const;
  void setEditable(bool editable);

  // Sorting insterface
  //! Удалить индикатор сортировки
  void removeSortIndicator(int logicalIndex);
  //! Удалить все индикаторы сортировки
  void clearSortIndicators();
  //! Установлен ли индикатор
  bool isSortIndicatorShown(int logicalIndex) const;
  //! Установить индикатор, если это уже второй индикатор справа отобразить число порядка
  void setSortIndicator(int logicalIndex, Qt::SortOrder order);
  //! Какая стрелочка отображена для данной колонки. Если никакой, то результат Qt::Asceding
  Qt::SortOrder sortIndicatorOrder(int logicalIndex) const;
  //! Список столбцов в которых производилась сортировка в том порядке в котором они добавлялись
  QList<int> sortIndicatorSections() const;

  // Show hide interface

  // Filtered sections interface
  QSet<int> filteredSections() const;

  // Frozen sections interface
  QList<int> frozenSections() const;

  void setSectionFrozen(int logicalIndex, bool frozen);
  bool isSectionFrozen(int logicalIndex) const;


  virtual void setModel(QAbstractItemModel* model);
  virtual void setRootIndex(const QModelIndex &index);

  virtual QMenu* menuForSection(int logicalIndex);

signals:
  void sortIndicatorsChanged();

  void filteredSectionsChanged();

  void frozenSectionsChanged();

public slots:
  void showAllSections();

  virtual void edit(const QModelIndex &index);
  virtual bool edit(int logicalIndex);

protected:
  // Отрисовка
  virtual void paintEvent(QPaintEvent *e);

  virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;

  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

  void mouseDoubleClickEvent(QMouseEvent *event);

  void contextMenuEvent(QContextMenuEvent *event);

  void checkOneSection();

protected slots:
  void hideSectionTriggered();
  void showHideActionToggled(bool checked);
  void showHideMenuAboutToShow();

  void setMovableTriggered(bool movable);
  void setHighlightSectionsTriggered(bool highlight);

  void sortAscTriggered();
  void sortDescTriggered();
  void removeSortTriggered();

  void commitData(QWidget* editor);
  void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
  void editorDestroyed(QObject* object);

private:
  Qt::SortOrder sortIndicatorOrder() const;
  int sortIndicatorSection() const;

  void releaseEditor(QWidget* editor);
};

#endif // HEADERVIEW_H
