#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "private/mainwindow_p.h"

#include "globalscriptengine.h"

// Menus
#include "qtwindowmenu.h"

#include "fileactionsmanager.h"
#include "editactionsmanager.h"
#include "customactionsmanager.h"
#include "findactionsmanager.h"
#include "navigateactionsmanager.h"
#include "printactionsmanager.h"

// Components

#include <QtCore/QStringList>
#include <QtCore/QDebug>

#include <QtGui/QStandardItem>
#include <QtGui/QToolBar>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QTreeView>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QUndoView>

#include <QtScript/QScriptEngine>
#include <QtScriptTools/QScriptEngineDebugger>

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , d_ptr(new MainWindowPrivate())
  , ui(new Ui::mainWindow)
{
  ui->setupUi(this);
  // additional setup ui
  Q_D(MainWindow);
  //qDebug() << "main windows";
  d->q_ptr = this;

  // MINI HACK
  // Creating QMdiArea TabBar more closeable and more nice.
  if (ui->mdiArea->viewMode() == QMdiArea::TabbedView) {
    QTabBar* tabBar = ui->mdiArea->findChild<QTabBar*>();
    // Мало ли
    if (tabBar) {
      tabBar->setExpanding(false);
      tabBar->setTabsClosable(true);
      connect(tabBar, SIGNAL(tabCloseRequested(int))
              , this, SLOT(tabCloseRequested(int)));
    }
  }

  // MINI HACK
  // set policy to noFocus for QUndoView;
  ui->undoView->setFocusPolicy(Qt::NoFocus);

  // ---------------------------------
  // File actions Print actions
  d->fileActionsManager = new FileActionsManager(this);
  d->printActionsManager = new PrintActionsManager(this);

  d->fileMenu = new QMenu(tr("&File"), this);
  d->fileMenu->setObjectName("fileMenu");
  d->printActionsSeparator = d->fileMenu->addSeparator();
  d->exitActionSeparator = d->fileMenu->addSeparator();
  d->exitAction = d->fileMenu->addAction(QIcon::fromTheme("application-exit"
    , QIcon(":/images/32x32/application/application-exit.png"))
                                         , tr("&Exit"), qApp, SLOT(closeAllWindows()), QKeySequence::Quit);
  d->exitAction->setStatusTip(tr("Exit the application"));
  d->exitAction->setToolTip(d->exitAction->statusTip());

  ui->mainMenuBar->addMenu(d->fileMenu);

  //d->fileToolBar = addToolBar(tr("File"));
  //d->fileToolBar->setObjectName("fileToolBar");

  d->printToolBar = addToolBar(tr("Print"));
  d->printToolBar->setObjectName("printToolBar");

  // -----------------------------------
  // Edit actions
  d->editActionsManager = new EditActionsManager(this);
  // Edit menu
  d->editMenu = new QMenu(tr("&Edit"), this);
  d->editMenu->setObjectName("editMenu");
  ui->mainMenuBar->addMenu(d->editMenu);

  d->editToolBar = addToolBar(tr("Edit"));
  d->editToolBar->setObjectName("editToolBar");

  // -----------------------------------
  // Additional actions
  d->customActionsManager = new CustomActionsManager(this);
  d->customMenu = new QMenu(tr("&Additional"), this);
  d->customMenu->setObjectName("additionalMenu");
  ui->mainMenuBar->addMenu(d->customMenu);

  d->findActionsManager = new FindActionsManager(this);

  // -----------------------------------
  // Navigate actions
  d->navigateActionsManager = new NavigateActionsManager(this);

  d->navigateMenu = new QMenu(tr("&Navigate"), this);
  d->navigateMenu->setObjectName("navigateMenu");
  ui->mainMenuBar->addMenu(d->navigateMenu);

  d->customToolBar = addToolBar(tr("Additional"));
  d->customToolBar->setObjectName("customToolBar");

  d->navigateToolBar = addToolBar(tr("Navigate"));
  d->navigateToolBar->setObjectName("navigateToolBar");

  // ------------------------------------
  // Window menu actions
  d->windowMenu = new QtWindowMenu(this);
  d->windowMenu->setObjectName("windowMenu");
  ui->mainMenuBar->addMenu(d->windowMenu);

  QAction *editToolBarSeparator = d->editToolBar->addSeparator();
  QAction *editMenuSeparator = d->editMenu->addSeparator();

  d->searchWidgetAction = d->editMenu->addAction(
      QIcon::fromTheme("edit-find-replace"
                       , QIcon(":/images/32x32/edit/edit-find-replace.png"))
      , tr("&Find/replace"));
  d->searchWidgetAction->setShortcut(QKeySequence::Find);
  d->searchWidgetAction->setObjectName("searchWidgetAction");
  d->editToolBar->addAction(d->searchWidgetAction);

  d->editToolBar->addSeparator();
  d->editMenu->addSeparator();

  d->searchWidgetAction->setCheckable(true);
  connect(d->searchWidgetAction, SIGNAL(triggered(bool))
          , ui->searchWidget, SLOT(setVisible(bool)));
  connect(ui->searchWidget, SIGNAL(closeRequested())
          , d->searchWidgetAction, SLOT(trigger()));
  ui->searchWidget->setVisible(false);

  d->configureToolBarsAction = d->editMenu->addAction(
      QIcon(":/images/32x32/brick/bricks.png")
      , tr("&Configure Toolbars..."), this
      , SLOT(configureToolBars()));
  d->configureToolBarsAction->setObjectName("configureToolBarsAction");
  d->configureToolBarsAction->setStatusTip(tr("Configure toolbars"));
  d->editToolBar->addAction(d->configureToolBarsAction);

  //d->fileActionsManager->addWidget(d->fileMenu, d->printActionsSeparator);
  //d->fileActionsManager->addWidget(d->fileToolBar);

  d->printActionsManager->addWidget(d->fileMenu, d->exitActionSeparator);
  d->printActionsManager->addWidget(d->printToolBar);

  d->editActionsManager->addWidget(d->editMenu, editMenuSeparator);
  d->editActionsManager->addWidget(d->editToolBar, editToolBarSeparator);
  d->editActionsManager->addUndoView(ui->undoView);

  d->customActionsManager->addWidget(d->customMenu);
  d->customActionsManager->addWidget(d->customToolBar);

  d->findActionsManager->setSearchWidget(ui->searchWidget);

  d->navigateActionsManager->addWidget(d->navigateMenu);
  d->navigateActionsManager->addWidget(d->navigateToolBar);

}

MainWindow::~MainWindow()
{
  delete d_ptr;
  delete ui;
}

void MainWindow::tabCloseRequested(int index)
{
  ui->mdiArea->subWindowList().at(index)->close();
}

MainWindowPrivate::MainWindowPrivate(){}

MainWindowPrivate::~MainWindowPrivate(){}
