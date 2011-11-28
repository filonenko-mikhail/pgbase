#ifndef MAINWINDOW_P_H
#define MAINWINDOW_P_H

#include "mainwindow.h"

#include <QtCore/QTimer>

#include <QtSql/QSqlDatabase>

class QtWindowMenu;
class QtToolBarManager;
class QUndoView;

class FileActionsManager;
class EditActionsManager;
class CustomActionsManager;
class FindActionsManager;
class NavigateActionsManager;
class PrintActionsManager;

class MainWindowPrivate {
  Q_DECLARE_PUBLIC(MainWindow);
  MainWindow * q_ptr;
public:
  MainWindowPrivate();
  virtual ~MainWindowPrivate();

  QMenu *fileMenu;
  QAction *printActionsSeparator;
  QAction *exitActionSeparator;
  QAction *exitAction;

  QMenu *editMenu;
  QMenu *customMenu;
  QMenu *navigateMenu;

  QtWindowMenu *windowMenu;

  QMenu *aboutMenu;

  QMenu *scriptEngineMenu;
  QAction *evaluateFile;
  QAction *evaulateFileWithDebugger;
  QAction *restartEngine;
  QAction *showDebugger;

  QToolBar *fileToolBar;
  QToolBar *printToolBar;
  QToolBar *editToolBar;
  QToolBar *customToolBar;
  QToolBar *navigateToolBar;

  QtToolBarManager* toolBarManager;

  QAction *searchWidgetAction;
  QAction *configureToolBarsAction;

  QUndoView *undoView;

  FileActionsManager *fileActionsManager;

  EditActionsManager *editActionsManager;

  CustomActionsManager *customActionsManager;

  FindActionsManager *findActionsManager;

  NavigateActionsManager *navigateActionsManager;

  PrintActionsManager *printActionsManager;
};

#endif // MAINWINDOW_P_H
