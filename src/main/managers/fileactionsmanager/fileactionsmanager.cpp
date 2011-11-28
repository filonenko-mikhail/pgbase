#include "fileactionsmanager.h"
#include "private/fileactionsmanager_p.h"

#include "fileactionsmanagerhelper.h"

#include <QtCore/QMetaMethod>

#include <QtGui/QApplication>
#include <QtGui/QAction>
#include <QtGui/QMenu>

FileActionsManager::~FileActionsManager()
{
  delete d_ptr;
}


FileActionsManager::FileActionsManager(QWidget* parent)
  : QObject(parent)
  , d_ptr(new FileActionsManagerPrivate)
{
  Q_D(FileActionsManager);
  setObjectName("fileActionsManager");

  d->parent = parent;

  QAction* a;

  a = d->newAction = new QAction(QIcon::fromTheme("document-new"
                                 , QIcon(":/images/document/document-new.png")),
                                 tr("&New"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::New);
  a->setObjectName("newAction");
  a->setToolTip(tr("Create new document"));
  a->setStatusTip(tr("Create new document"));

  a = d->openAction = new QAction(QIcon::fromTheme("document-open"
                                              , QIcon(":/images/folder/folder.png"))
                              , tr("&Open"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Open);
  a->setObjectName("openAction");
  a->setToolTip(tr("Open document"));
  a->setStatusTip(tr("Open document"));

  QMenu *m = d->openRecentMenu = new QMenu(tr("Open recent"));
  m->setIcon(QIcon::fromTheme("document-open-recent"));
  d->openRecentAction = d->openRecentMenu->menuAction();

  a = d->saveAction = new QAction(QIcon::fromTheme("document-save"
                                              , QIcon(":/images/disk/disk.png"))
                              , tr("&Save"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Open);
  a->setObjectName("saveAction");
  a->setToolTip(tr("Save document"));
  a->setStatusTip(tr("Save document"));

  a = d->saveAsAction = new QAction(QIcon::fromTheme("document-save-as")
                              , tr("Save as"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Open);
  a->setObjectName("saveAsAction");
  a->setToolTip(tr("Save document as "));
  a->setStatusTip(tr("Save document as"));

  a = d->saveAllActions = new QAction(QIcon(":/images/disk/disk_multiple.png")
                              , tr("Save &All"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Open);
  a->setObjectName("saveAllAction");
  a->setToolTip(tr("Save all documents"));
  a->setStatusTip(tr("Save all documents"));

  a = d->closeAction = new QAction(tr("Close"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Open);
  a->setObjectName("closeAction");
  a->setToolTip(tr("Close document"));
  a->setStatusTip(tr("Close document"));

  a = d->closeAllAction = new QAction(tr("Close all"), this);
  a->setPriority(QAction::LowPriority);
  a->setShortcut(QKeySequence::Open);
  a->setObjectName("closeAllAction");
  a->setToolTip(tr("Close all documents"));
  a->setStatusTip(tr("Close all documents"));

  foreach(QAction * action, actions())
    action->setEnabled(false);

  addStandardHelpers();

  connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*))
          , this, SLOT(focusChanged(QWidget*, QWidget*)));
  focusChanged(0, QApplication::focusWidget());
}

void FileActionsManager::addStandardHelpers()
{
  addHelper(new TextEditFileActionsManagerHelper(this));
  addHelper(new ItemViewFileActionsManagerHelper(this));
}

QList<QAction*> FileActionsManager::actions() const
{
  Q_D(const FileActionsManager);
  QList<QAction*> result;
  result << d->newAction
         << d->openAction
         << d->openRecentAction
         << d->saveAction
         << d->saveAsAction
         << d->saveAllActions
         << d->closeAction
         << d->closeAllAction;

  return result;
}


void FileActionsManager::addWidget(QWidget* widget, QAction* before)
{
  Q_D(FileActionsManager);
  d->widgets.insert(widget, before);

  widget->insertActions(before, actions());
}

void FileActionsManager::removeWidget(QWidget* widget)
{
  Q_D(FileActionsManager);

  foreach(QAction * action, actions())
  widget->removeAction(action);

  d->widgets.remove(widget);
}


void FileActionsManager::focusChanged(QWidget* old, QWidget* now)
{
  Q_D(FileActionsManager);
  if (!d->isFocusChangedBlocked) {
    if (d->currentHelper) {
      disconnectFromHelper(d->currentHelper);
      d->currentHelper->deactivate();
      d->currentHelper = 0;
    }

    if (!d->parent->isAncestorOf(now)) return;

    if ((d->currentHelper = helperFor(now))) {
      d->currentHelper->activateFor(now);
      connectToHelper(d->currentHelper);
    }
  }
}

void FileActionsManager::blockFocusChanged(bool blocked)
{
  Q_D(FileActionsManager);
  d->isFocusChangedBlocked = blocked;
}

void FileActionsManager::addHelper(FileActionsManagerHelper* helper)
{
  Q_D(FileActionsManager);
  d->helpers.insert(helper);
}

void FileActionsManager::removeHelper(FileActionsManagerHelper* helper)
{
  Q_D(FileActionsManager);
  if (d->currentHelper == helper) {
    disconnectFromHelper(helper);
    d->currentHelper = 0;
  }
  d->helpers.remove(helper);
}

QSet<FileActionsManagerHelper*> FileActionsManager::helpers() const
{
  Q_D(const FileActionsManager);
  return d->helpers;
}

FileActionsManagerHelper* FileActionsManager::helperFor(QWidget* widget) const
{
  Q_D(const FileActionsManager);
  QSet<FileActionsManagerHelper*>::ConstIterator it = d->helpers.constBegin();
  QSet<FileActionsManagerHelper*>::ConstIterator end = d->helpers.constEnd();
  FileActionsManagerHelper *findedHelper = 0;
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

void FileActionsManager::connectToHelper(FileActionsManagerHelper* helper)
{
  Q_D(FileActionsManager);

  QList<QAction*> additionalActions = helper->additionalActions();
  if (!additionalActions.isEmpty()) {
    QHash<QWidget*, QAction*>::ConstIterator it = d->widgets.constBegin();
    QHash<QWidget*, QAction*>::ConstIterator end = d->widgets.constEnd();
    for (; it != end; ++it) {
      it.key()->insertAction(it.value(), d->additionalActionsSeparator);
      it.key()->insertActions(it.value(), additionalActions);
    }
  }

  QMetaMethod metaMethod;

  for (int i = helper->metaObject()->methodOffset(); i < helper->metaObject()->methodCount(); ++i) {
    metaMethod = helper->metaObject()->method(i);
    if (metaMethod.methodType() == QMetaMethod::Slot
        && (metaObject()->indexOfSignal(metaMethod.signature()) != -1)) {
      connect(this, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data()
              , helper, QString("%1%2").arg(QSLOT_CODE).arg(metaMethod.signature()).toLatin1().data());
    }
  }

  for (int i = FileActionsManagerHelper::staticMetaObject.methodOffset(); i < helper->metaObject()->methodCount(); ++i) {
    metaMethod = helper->metaObject()->method(i);
    if (metaMethod.methodType() == QMetaMethod::Signal
        && (metaObject()->indexOfSignal(metaMethod.signature()) != -1)) {
      connect(helper, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data()
              , this, QString("%1%2").arg(QSIGNAL_CODE).arg(metaMethod.signature()).toLatin1().data());
    }
  }

  connect(helper, SIGNAL(blockFocusChanged(bool))
          , this, SLOT(blockFocusChanged(bool)));
}

void FileActionsManager::disconnectFromHelper(FileActionsManagerHelper* helper)
{
  Q_D(FileActionsManager);

  QList<QAction*> additionalActions = helper->additionalActions();
  if (!additionalActions.isEmpty()) {
    int additionalActionsCount = additionalActions.count();
    QHash<QWidget*, QAction*>::ConstIterator it = d->widgets.constBegin();
    QHash<QWidget*, QAction*>::ConstIterator end = d->widgets.constEnd();
    for (; it != end; ++it) {
      it.key()->removeAction(d->additionalActionsSeparator);
      for (int i = 0; i < additionalActionsCount; ++i)
        it.key()->removeAction(additionalActions.at(i));
    }
  }

  foreach(QAction * action, actions())
  action->setEnabled(false);

  disconnect(helper, 0
             , this, 0);

  disconnect(this, 0
             , helper, 0);
}

FileActionsManagerPrivate::FileActionsManagerPrivate()
  : currentHelper(0) {}
FileActionsManagerPrivate::~FileActionsManagerPrivate() {}
