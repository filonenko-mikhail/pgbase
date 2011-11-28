include("widgets/calculator.js");

QsMainWindow = function(parent) {
  MainWindow.call(this, parent);

  //this.windowModality = Qt.ApplicationModal;

  this.toolsMenu = new QMenu(qsTr("&Tools"), this);
  this.toolsMenu.objectName = "toolsMenu";
  this.mainMenuBar.insertMenu(this.windowMenu.menuAction(), this.toolsMenu);

  // ---------------------------------------------------
  // ScriptMenu
  this.scriptMenu = new QMenu(qsTr("&Script"), this);
  this.scriptMenu.objectName = "scriptMenu";
  this.mainMenuBar.insertMenu(this.windowMenu.menuAction(), this.scriptMenu);
  this.importExtension = this.scriptMenu.addAction(new QIcon(), qsTr("&Import extension..."));
  this.importExtension.triggered.connect(this, this.scriptImportExtension);

  this.evaulateFile = this.scriptMenu.addAction(new QIcon(":/images/script/script_go.png")
                                                , qsTr("&Evaluate file..."));
  this.evaulateFile.triggered.connect(this, this.scriptEvaluateFile);

  this.evaulateFileWithDebugger = this.scriptMenu.addAction(new QIcon(":/images/bug/bug_go.png")
                                , qsTr("Evaluate file with &debugger..."));
  this.evaulateFileWithDebugger.triggered.connect(this, this.scriptEvaluateFileIndebugger);

  this.showDebugger = this.scriptMenu.addAction(new QIcon(), qsTr("S&how script debugger..."));
  this.showDebugger.triggered.connect(this, this.scriptShowDebugger);

  this.restartEngine = this.scriptMenu.addAction(new QIcon(), qsTr("Restart s&cript engine"));

  this.restartEngine.triggered.connect(this, this.scriptRestartEngine);

  // ----------------------------------------------------
  // Help menu
  this.helpMenu = this.mainMenuBar.addMenu(qsTr("&Help"));
  this.helpMenu.objectName = "helpMenu";
  this.aboutQtAction = this.helpMenu.addAction(qsTr("About &Qt..."));
  this.aboutQtAction.triggered.connect(application, application.aboutQt);

  this.aboutApplicationAction = this.helpMenu.addAction(qsTr("&About..."));
  this.aboutApplicationAction.triggered.connect(this, this.aboutApplication);

  // -------------------------------------------------------------
  // Database objects in tree view
  this.dockWidget1 = new QDockWidget(qsTr("Database objects"), this, 0);
  this.dockWidget1.objectName = "dockWidget1";

  // -----------------------------------
  // Database menu view
  this.menuView = new QtMenuView(qsTr("&Database objects"), this);
  this.menuView.objectName = "menuView";
  this.mainMenuBar.insertMenu(this.windowMenu.menuAction(), this.menuView);


  var icon = new QIcon(":/images/32x32/application/application_side_tree.png");
  this.dockWidget1.windowIcon = icon;
  this.dockWidget1.floating = false;
  this.dockWidget1.toggleViewAction().icon = icon;

  this.dockWidgetContents1 = new QWidget();
  this.dockWidgetContents1.objectName = "dockWidgetContents1";

  this.verticalLayout1 = new QVBoxLayout(this.dockWidgetContents1);
  this.verticalLayout1.spacing = 0;
  this.verticalLayout1.setContentsMargins(0, 0, 0, 0);
  this.verticalLayout1.objectName = "verticalLayout1";

  this.objectsView = new QTreeView(this.dockWidgetContents1);
  this.objectsView.objectName = "objectsView";
  this.objectsView.editTriggers = QAbstractItemView.NoEditTriggers;
  this.objectsView.indentation = 10;
  this.objectsView.uniformRowHeights = true;
  this.objectsView.sortingEnabled = true;
  this.objectsView.header().showSortIndicator = true;

  this.objectsViewHeader = new QtHeaderView(Qt.Horizontal, this.objectsView);
  this.objectsViewHeader.objectName = "objectsViewHeader";
  this.objectsViewHeader.stretchLastSection = true;
  this.objectsViewHeader.enableFilter = false;
  this.objectsViewHeader.enableFreeze = false;
  this.objectsViewHeader.sortFlag = QtHeaderView.NoSort;
  this.objectsView.setHeader(this.objectsViewHeader);
  this.verticalLayout1.addWidget(this.objectsView, 0, 0);

  this.dockWidget1.setWidget(this.dockWidgetContents1);
  this.addDockWidget(Qt.LeftDockWidgetArea, this.dockWidget1);

  // ---------------------------
  // NumPad
  this.dockWidget2 = new QDockWidget(qsTr("NumPad"), this, 0);
  this.dockWidget2.objectName = "dockWidget2";

  this.dockWidget2.floating = false;

  this.dockWidgetContents2 = new QWidget();
  this.dockWidgetContents2.objectName = "dockWidgetContents2";

  this.gridLayout2 = new QGridLayout(this.dockWidgetContents2);
  this.gridLayout2.objectName = "gridLayout2";
  this.gridLayout2.spacing = 0;
  this.gridLayout2.setContentsMargins(0, 0, 0, 0);
  this.gridLayout2.addItem(new QSpacerItem(40, 2, QSizePolicy.Expanding, QSizePolicy.Preferred)
                           , 0, 0, 1, 1, 0);
  this.gridLayout2.addItem(new QSpacerItem(2, 40, QSizePolicy.Preferred, QSizePolicy.Expanding)
                           , 1, 0, 1, 1, 0);

  this.numPad = new NumPad(this.dockWidgetContents2);
  this.numPad.objectName = "numPad";
  this.gridLayout2.addWidget(this.numPad, 0, 1);
  this.dockWidget2.windowIcon = this.numPad.windowIcon;
  this.dockWidget2.toggleViewAction().icon = this.numPad.windowIcon;

  this.dockWidget2.setWidget(this.dockWidgetContents2);
  this.addDockWidget(Qt.LeftDockWidgetArea, this.dockWidget2);


  // ---------------------------
  // LogView
  this.dockWidget3 = new QDockWidget(qsTr("Log"), this, 0);
  this.dockWidget3.objectName = "dockWidget3";

  this.dockWidget3.floating = false;

  this.dockWidgetContents3 = new QWidget();
  this.dockWidgetContents3.objectName = "dockWidgetContents3";

  this.verticalLayout2 = new QVBoxLayout(this.dockWidgetContents3);
  this.verticalLayout2.spacing = 0;
  this.verticalLayout2.setContentsMargins(0, 0, 0, 0);
  this.verticalLayout2.objectName = "verticalLayout2";


  this.logEdit = new QPlainTextEdit(this.dockWidgetContents3);
  this.logEdit.minimumSize = new QSize(0, 20);
  this.verticalLayout2.addWidget(this.logEdit, 0, 0);

  this.dockWidget3.setWidget(this.dockWidgetContents3);
  this.addDockWidget(Qt.BottomDockWidgetArea, this.dockWidget3);

  signalLogger.appended.connect(this, this.appendLogMessage);

  // -------------------------------------
  // Database objects model
  var labels = new Array;
  labels[0] = qsTr("Item");
  labels[1] = qsTr("Description");
  database.objectsModel = new StandardTreeModel(this);
  database.objectsModel.setHorizontalHeaderLabels(labels);

  this.objectsView.setModel(database.objectsModel);
  this.menuView.setModel(database.objectsModel, true);

  //this.logPlainTextEdit = this.coreDockWidget1.contents1.logPlainTextEdit;

  this.restoreSettings();
};

QsMainWindow.prototype = new QMainWindow();

QsMainWindow.prototype.addWidget = function(widget) {
  var subWindow = this.centralWidget.mdiArea.addSubWindow(widget);
  subWindow.windowIcon = widget.windowIcon;
  subWindow.windowTitle = widget.windowTitle;
  subWindow.setOption(QMdiSubWindow.RubberBandMove);
  subWindow.setOption(QMdiSubWindow.RubberBandResize);
  //subWindow.setAttribute(Qt.WA_DeleteOnClose);
  subWindow.show();
};

QsMainWindow.prototype.activateWidget = function(widget) {
  this.centralWidget.mdiArea.setActiveSubWindow(widget.parentWidget());
};

QsMainWindow.prototype.removeWidget = function(widget) {
  this.centralWidget.mdiArea.removeSubWindow(widget.parentWidget());
};


QsMainWindow.prototype.scriptRestartEngine = function() {
  var result;
  application.aboutToQuit();
  if ((result = reloadScriptEngine()))
    QCoreApplication.exit(result);
};

QsMainWindow.prototype.restoreSettings = function(event) {
  // ----------------------------------------------------
  // Restore state of main window
  database.settings.beginGroup(this.objectName);
  state = QByteArray.fromHex(database.settings.value("windowState"));
  this.restoreState(state);
  state = QByteArray.fromHex(database.settings.value("windowGeometry"));
  this.restoreGeometry(state);

  database.settings.beginGroup(this.objectsViewHeader.objectName);
  restoreHeaderViewState(this.objectsViewHeader, database.settings);
  database.settings.endGroup();

  database.settings.endGroup();
  // ----------------------------------------------------

};

QsMainWindow.prototype.closeEvent = function(event) {
  this.writeSettings();

  signalLogger.appended.disconnect(this, this.appendLogMessage);
  this.centralWidget.mdiArea.closeAllSubWindows();
  event.accept();
};

QsMainWindow.prototype.writeSettings = function(event) {
  // ----------------------------------------------------
  // Save state of main window
  database.settings.beginGroup(window.objectName);
  database.settings.setValue("windowState", window.saveState().toHex());
  database.settings.setValue("windowGeometry", window.saveGeometry().toHex());

  database.settings.beginGroup(this.objectsViewHeader.objectName);
  saveHeaderViewState(this.objectsViewHeader, database.settings);
  database.settings.endGroup();
  //database.settings.setValue("windowTitle", window.windowTitle);
  database.settings.endGroup();
  // ----------------------------------------------------
};

QsMainWindow.prototype.scriptImportExtension = function() {
  var extension = QInputDialog.getText(this
                                       , qsTr("Import extension")
                                       , qsTr("Enter extension name (e.g. \"qt.webkit\"):")
                                       , QLineEdit.Normal, "", 0);
  //getText(QWidget parent, String title, String label, EchoMode echo, String text, WindowFlags flags)
  if (extension) {
    if (importExtension(extension)) {
      print(qsTr("Failed to import extension %1").arg(extension));
    }
  }
};

QsMainWindow.prototype.scriptEvaluateFile = function() {
  var fileName = QFileDialog.getOpenFileName(this
                                             , qsTr("Open qt script file")
                                             , "script"
                                             , qsTr("Script files (*.js);;All files(*.*)"));
  if (fileName)
    include(fileName);
};

QsMainWindow.prototype.scriptEvaluateFileIndebugger = function() {
  var fileName = QFileDialog.getOpenFileName(this
                                             , qsTr("Open qt script file in debugger")
                                             , "script"
                                             , qsTr("Script files (*.js);;All files(*.*)"));
  if (fileName) {
    debugger;
    include(fileName);
  }
};

QsMainWindow.prototype.scriptShowDebugger = function() {
  debugger;
};

QsMainWindow.prototype.scriptRestartEngine = function() {
  var result;
  application.aboutToQuit();
  if ((result = reloadScriptEngine()))
    QCoreApplication.exit(result);
};

QsMainWindow.prototype.appendLogMessage = function(message) {
  this.logEdit.appendHtml(("<pre>%1</pre>").arg(message));
  this.logEdit.textCursor().movePosition(QTextCursor.End);
};

//QsMainWindow.prototype.numPadTriggered = function() {
//  numPad = new NumPad(0);
//  numPad.show();
//};

QsMainWindow.prototype.confSelectorActionTriggered = function() {
  var dialog = new SelectorDialog(this, 0);
  dialog.exec();
};

QsMainWindow.prototype.aboutApplication = function() {
  QMessageBox.about(this, qsTr("About")
                    , qsTr("Simple program"));
};
