#ifndef FILEACTIONSMANAGER_P_H
#define FILEACTIONSMANAGER_P_H

#include "fileactionsmanager.h"

#include <QtCore/QSet>

class QMenu;
class QWidget;
class FileActionsManagerPrivate
{
public:
  FileActionsManagerPrivate();
  virtual ~FileActionsManagerPrivate();

  QWidget *parent;

  bool isFocusChangedBlocked;

  QHash<QWidget*, QAction*> widgets;

  QSet<FileActionsManagerHelper*> helpers;
  FileActionsManagerHelper *currentHelper;

  // general undo/redo framework
  QAction *newAction;
  QAction *openAction;

  QMenu *openRecentMenu;
  QAction *openRecentAction;

  QAction *saveAction;
  QAction *saveAsAction;
  QAction *saveAllActions;

  QAction *closeAction;
  QAction *closeAllAction;

  QAction *pasteAction;
  QAction *clearAction;

  QAction *selectSeparator;
  QAction *selectAllAction;

  QAction *columnSeparator;

  QAction *additionalActionsSeparator;
};

#endif // FILEACTIONSMANAGER_P_H
