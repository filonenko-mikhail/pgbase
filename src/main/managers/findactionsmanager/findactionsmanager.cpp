#include "findactionsmanager.h"
#include "private/findactionsmanager_p.h"
#include "searchwidget.h"

#include "findactionsmanagerhelper.h"

#include <QtDebug>
#include <QtCore/QMetaMethod>

FindActionsManager::FindActionsManager(FindActionsManagerPrivate& dd, QWidget* parent)
  : QObject(parent)
  , d_ptr(&dd)
{
  init(parent);
}

FindActionsManager::FindActionsManager(QWidget* parent)
  : QObject(parent)
  , d_ptr(new FindActionsManagerPrivate)
{
  init(parent);
}

FindActionsManager::~FindActionsManager()
{
  delete d_ptr;
}

void FindActionsManager::init(QWidget* parent)
{
  Q_D(FindActionsManager);
  setObjectName("findActionsManager");

  d->parent = parent;

  addStandartHelpers();

  connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*))
          , this, SLOT(focusChanged(QWidget*, QWidget*)));
}

void FindActionsManager::setSearchWidget(SearchWidget* searchWidget)
{
  Q_D(FindActionsManager);

  disconnectFromSearchWidget(d->searchWidget);
  connectToSearchWidget(searchWidget);

  d->searchWidget = searchWidget;

  if (!d->currentHelper) {
    d->searchWidget->canFindChanged(false);
    d->searchWidget->canFindChanged(false);
  } else {
    d->searchWidget->canFindChanged(d->currentHelper->canFind());
    d->searchWidget->canFindChanged(d->currentHelper->canReplace());
  }
}

void FindActionsManager::focusChanged(QWidget */*old*/, QWidget* now)
{
  Q_D(FindActionsManager);
  FindActionsManagerHelper* helper;

  if (!d->parent->isAncestorOf(now)) return;

  // Если фокусный виджет есть
  helper = helperFor(now);
  // Если фокусный виджет обладает поисковым помощником
  if (helper) {
    if (d->searchableFocusWidget)
      searchableWidgetDestroyed();
    // Подключаемся к деструктору искабельного виджета
    connect(now, SIGNAL(destroyed()), this, SLOT(searchableWidgetDestroyed()));
    // Активируем помощника для данного виджета
    helper->activateFor(now);
    // Подключаемся к помщнику
    connectToHelper(helper);
    // Устанавливаем текущий искабельный виджет
    d->searchableFocusWidget = now;
    d->currentHelper = helper;
  }
}

void FindActionsManager::searchableWidgetDestroyed()
{
  Q_D(FindActionsManager);

  // Отключаемся от все сигналов данного виджета
  disconnect(d->searchableFocusWidget, 0, this, 0);

  // Деактивируем помощника
  d->currentHelper->deactivate();
  // Отключаемся от помощника
  disconnectFromHelper(d->currentHelper);

  // Текущий искабельный виджет равен 0
  d->searchableFocusWidget = 0;
  d->currentHelper = 0;
}

SearchWidget* FindActionsManager::searchWidget() const
{
  Q_D(const FindActionsManager);
  return d->searchWidget;
}

FindActionsManagerHelper* FindActionsManager::helperFor(QWidget* widget) const
{
  Q_D(const FindActionsManager);
  // Находим поискового помощника для виджета
  QSet<FindActionsManagerHelper*>::ConstIterator it = d->helpers.constBegin();
  QSet<FindActionsManagerHelper*>::ConstIterator end = d->helpers.constEnd();

  FindActionsManagerHelper *findedHelper = 0;
  int maxApproach = 0;
  int approach;
  while (it != end) {
    approach = (*it)->approach(widget);
    if (approach > maxApproach) {
      findedHelper = (*it);
      maxApproach = approach;
    }
    ++it;
  }
  return findedHelper;
}

void FindActionsManager::addStandartHelpers()
{
  addHelper(new ItemViewFindActionsManagerHelper(this));
}

void FindActionsManager::connectToHelper(FindActionsManagerHelper* helper)
{
  Q_D(FindActionsManager);

  d->searchWidget->canFindChanged(helper->canFind());
  d->searchWidget->canReplaceChanged(helper->canReplace());

  // Подключаемся к поисковому помощнику
  QMetaMethod metaMethod;
  for (int i = helper->metaObject()->methodOffset(); i < helper->metaObject()->methodCount(); ++i) {
    metaMethod = helper->metaObject()->method(i);
    if (metaMethod.methodType() == QMetaMethod::Slot
        && (metaObject()->indexOfSignal(metaMethod.signature()) != -1)) {
      connect(this, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data()
              , helper, QString("%1%2").arg(QSLOT_CODE).arg(metaMethod.signature()).toLatin1().data());
    }
  }

  for (int i = FindActionsManagerHelper::staticMetaObject.methodOffset(); i < helper->metaObject()->methodCount(); ++i) {
    metaMethod = helper->metaObject()->method(i);
    if (metaMethod.methodType() == QMetaMethod::Signal
        && (metaObject()->indexOfSignal(metaMethod.signature()) != -1)) {
      connect(helper, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data()
              , this, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data());
    }
  }

  d->searchWidget->setFindString(helper->initialFindString());
  d->searchWidget->setFindHintString(helper->initialFindHintString());
}

void FindActionsManager::disconnectFromHelper(FindActionsManagerHelper* helper)
{
  Q_D(FindActionsManager);

  // Отключаемся от поискового помощника
  d->searchWidget->setFindHintString(QString::null);
  d->searchWidget->setFindHintString(QString::null);

  disconnect(this, 0, helper, 0);

  disconnect(helper, 0, this, 0);

  d->searchWidget->canFindChanged(false);
  d->searchWidget->canReplaceChanged(false);
}

void FindActionsManager::connectToSearchWidget(SearchWidget* searchWidget)
{
  if (!searchWidget) return;

  // Подлючаемся к окну поиска
  QMetaMethod metaMethod;
  for (int i = searchWidget->metaObject()->methodOffset(); i < searchWidget->metaObject()->methodCount(); ++i) {
    metaMethod = searchWidget->metaObject()->method(i);
    if (metaMethod.methodType() == QMetaMethod::Slot
        && (metaObject()->indexOfSignal(metaMethod.signature()) != -1)) {
      connect(this, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data()
              , searchWidget, QString("%1%2").arg(QSLOT_CODE).arg(metaMethod.signature()).toLatin1().data());
    }
  }

  for (int i = SearchWidget::staticMetaObject.methodOffset(); i < searchWidget->metaObject()->methodCount(); ++i) {
    metaMethod = searchWidget->metaObject()->method(i);
    //qDebug() << metaMethod.signature();
    if (metaMethod.methodType() == QMetaMethod::Signal
        && (metaObject()->indexOfSignal(metaMethod.signature()) != -1)) {
      //qDebug() << metaMethod.signature();
      connect(searchWidget, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data()
              , this, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data());
    }
  }
}

void FindActionsManager::disconnectFromSearchWidget(SearchWidget* searchWidget)
{
  if (!searchWidget) return;

  // Отключаемся от окна поиска
  disconnect(searchWidget, 0, this, 0);
  disconnect(this, 0, searchWidget, 0);
}

void FindActionsManager::addHelper(FindActionsManagerHelper* helper)
{
  Q_D(FindActionsManager);
  d->helpers.insert(helper);
}

void FindActionsManager::removeHelper(FindActionsManagerHelper* helper)
{
  Q_D(FindActionsManager);
  // Если установлен искабельный виджет
  if (d->searchableFocusWidget)
    // Если для данного виджета помощник равен удаляемому
    if (d->currentHelper == helper)
      // Как будто искабельный виджет удалился
      searchableWidgetDestroyed();

  d->helpers.remove(helper);
}

QSet<FindActionsManagerHelper*> FindActionsManager::helpers() const
{
  Q_D(const FindActionsManager);
  return d->helpers;
}

FindActionsManagerPrivate::FindActionsManagerPrivate()
  : searchWidget(0), searchableFocusWidget(0), currentHelper(0) {}

FindActionsManagerPrivate::~FindActionsManagerPrivate() {}
