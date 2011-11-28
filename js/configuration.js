({name : "configurationManager"
  , absoluteFilePath : "" // Filled by configuration loader
  , absolutePath : "" // Filled by configuration loader

  , forms : [] // configuration forms
  , tables : [] // confiuration tables

  , QT_OBJECT_LIST_NAME_ITEM_ROLE :  Qt.UserRole + 101
  , QT_OBJECT_NAME_ITEM_ROLE : Qt.UserRole + 102

  , toString : function() { return this.name;}

  , tableObjectTemplate : include("js:template/tableobjecttemplate.js")

      // ---------------------------------------------------------
      , loadObjects : function(directory, deep) {
    if (!deep) return;


    var objectList = [];
    var jsFiles = ["*.js", "*.qs"];
    var fileList = directory.entryList(jsFiles, QDir.Filter(QDir.Files), QDir.SortFlag(QDir.Name));
    var loadedObject;
    var fileName;
    var filePath;
    for (var fileIndex = 0; fileIndex < fileList.length; ++fileIndex) {
      fileName = fileList[fileIndex];
      filePath = directory.filePath(fileName);
      loadedObject = include(filePath);
      if (loadedObject) {
        loadedObject.absoluteFilePath = filePath;
        loadedObject.absolutePath = directory.absolutePath();
        objectList[loadedObject.objectName] = loadedObject;
      }
    }

    var dirList = directory.entryList(QDir.Filters(QDir.Dirs, QDir.NoDotAndDotDot), QDir.SortFlags(QDir.Name));
    var dirName;
    var filePath;
    for (var dirIndex = 0; dirIndex < dirList.length; ++dirIndex) {
      dirName = dirList[dirIndex];
      directory.cd(dirName);
      var sudObjectList = this.loadObjects(directory, deep - 1);
      for (a in sudObjectList)
        objectList[a] = sudObjectList[a];
      directory.cdUp();
    }
    return objectList;
  }

  // ---------------------------------------------------------
  , reportName : function(ioDevice) {
    var xml = new QXmlStreamReader();

    xml.setDevice(ioDevice);
    var tokenType;
    while (!xml.atEnd()) {
      tokenType = xml.readNext();
      if (tokenType == QXmlStreamReader.StartElement) {
        return xml.attributes().value("name");
      }
    }
  }
  // ---------------------------------------------------------
  , loadReports : function(directory, deep) {
    if (!deep) return;

    var objectList = [];
    var ncFiles = ["*.xml", "*.ncr"];

    var fileList = directory.entryList(ncFiles, QDir.Filter(QDir.Files), QDir.SortFlag(QDir.Name));
    var loadedObject;
    var fileName;
    var filePath;
    var file = new QFile();
    var reportName;
    for (var fileIndex = 0; fileIndex < fileList.length; ++fileIndex) {
      fileName = fileList[fileIndex];
      filePath = directory.filePath(fileName);
      file.setFileName(filePath);
      if (file.open(QIODevice.ReadOnly)) {
        loadedObject = {};
        reportName = this.reportName(file);
        loadedObject.objectName = reportName;
        loadedObject.path = "/Отчеты/" +  reportName;
        loadedObject.icon = new QIcon("images:/32x32/report/report.png");
        loadedObject.toString = function() {return this.objectName;};
        // filled when loaded by configuration.js
        loadedObject.absoluteFilePath = filePath;
        loadedObject.absolutePath = directory.absolutePath();

        file.close();
      }

      objectList[loadedObject.objectName] = loadedObject;
    }

    var dirList = directory.entryList(QDir.Filters(QDir.Dirs, QDir.NoDotAndDotDot), QDir.SortFlags(QDir.Name));
    var dirName;
    var filePath;
    for (var dirIndex = 0; dirIndex < dirList.length; ++dirIndex) {
      dirName = dirList[dirIndex];
      directory.cd(dirName);
      var sudObjectList = this.loadObjects(directory, deep - 1);
      for (a in sudObjectList)
        objectList[a] = sudObjectList[a];
      directory.cdUp();
    }
    return objectList;
  }
  // ------------------------------------------------------
  , up : function() {
    var databaseComment = database.databaseName();
    var query = new QSqlQuery(database);
    query.prepare("select description from pg_description join pg_namespace on objoid = oid and nspname = ?");
    query.addBindValue(opts["schema"].value);
    if (query.exec()) {
      if (query.next())
        databaseComment = query.value(0);
    }

    window.windowTitle = ("%1 - %3")
                         .arg(databaseComment)
                         .arg(database.settings.value("windowTitle", window.windowTitle));
    configurationDir = new QDir(opts["argv"][1]?opts["argv"][1]:"js:");

    var formsDir = new QDir("js:");
    if (formsDir.cd("forms")) {
      this.forms = this.loadObjects(formsDir, 40);
      var currentForm;
      var item;
      var parentItem;
      var descriptionItem;
      for (var formName in this.forms) {
        currentForm = this.forms[formName];
        item = database.objectsModel.itemForPath(currentForm.path);
        item.setIcon(currentForm.icon);
        item.setData("forms", this.QT_OBJECT_LIST_NAME_ITEM_ROLE);
        item.setData(formName, this.QT_OBJECT_NAME_ITEM_ROLE);

        descriptionItem = new QStandardItem();
        if (currentForm.description) {
          item.setToolTip(currentForm.description);
          descriptionItem.setText(currentForm.description);
        }


        parentItem = !item.parent()?database.objectsModel.invisibleRootItem():item.parent();
        parentItem.setChild(item.row(), 1, descriptionItem);

        currentForm.__defineGetter__("instance", function() {
          return configuration.activateObject(this);
        });
        currentForm.action = new QAction(currentForm.title, this);
        currentForm.action.icon = currentForm.icon;
        currentForm.action.objectName = currentForm.objectName;
        currentForm.action.object = currentForm;
        currentForm.action.toolTip = currentForm.description;
        currentForm.action.statusTip = currentForm.description;
        currentForm.action.triggered.connect(this.activateObject.bind(this, currentForm));
      }
    }

    var currentTable;
    var tableName;
    var removeSchema = function(tablename) {
      if (tablename.split('.').length > 1)
        return tablename.split('.')[1];
      else
        return tablename.split('.')[0];
    };
    var tables = database.tables().map(removeSchema);
    for (var tableIndex = 0; tableIndex < tables.length; ++tableIndex) {
      tableName = tables[tableIndex];
      currentTable = this.tableObjectTemplate(tableName);
      this.tables[currentTable.objectName] = currentTable;

      item = database.objectsModel.itemForPath(currentTable.path);
      item.setIcon(currentTable.icon);
      item.setData("tables", this.QT_OBJECT_LIST_NAME_ITEM_ROLE);
      item.setData(tableName, this.QT_OBJECT_NAME_ITEM_ROLE);

      descriptionItem = new QStandardItem();
      if (currentTable.description) {
        item.setToolTip(currentTable.description);
        descriptionItem.setText(currentTable.description);
      }


      parentItem = !item.parent()?database.objectsModel.invisibleRootItem():item.parent();
      parentItem.setChild(item.row(), 1, descriptionItem);

      currentTable.__defineGetter__("instance", function() {
        return configuration.activateObject(this);
      });

      currentTable.action = new QAction(currentTable.title, this);
      currentTable.action.icon = currentTable.icon;
      currentTable.action.objectName = currentTable.objectName;
      currentTable.action.object = currentTable;
      currentTable.action.toolTip = currentTable.description;
      currentTable.action.statusTip = currentTable.description;
      currentTable.action.triggered.connect(this.activateObject.bind(this, currentTable));
    }

    // ------------------------------------
    // Sort database objects tree
    // Сортируем построенное дерево
    window.objectsView.header().setSortIndicator(0, Qt.AscendingOrder);
    window.objectsView.doubleClicked.connect(this, this.execObjectTreeItem);
    window.menuView.triggeredIndex.connect(this, this.execObjectTreeItem);
  }

  , down : function()
  {
  }

  , execObjectTreeItem: function(index)
  {
    var currentItem = database.objectsModel.itemFromIndex(index);
    if (currentItem.column > 0);
    if (currentItem.parent())
      currentItem = currentItem.parent().child(currentItem.row());

    var objectListName = currentItem.data(this.QT_OBJECT_LIST_NAME_ITEM_ROLE);
    if (objectListName) {
      var objectName = currentItem.data(this.QT_OBJECT_NAME_ITEM_ROLE);
      var object = this[objectListName][objectName];
      this.activateObject(object);
    }
  }

  , activateObject : function(object)
  {
    if (object["_private_instance"]) {
      window.activateWidget(object["_private_instance"]);
    } else if (object) {
      var widget = new object(window);
      object["_private_instance"] = widget;

      widget.objectName = object.objectName;
      widget.windowIcon = object.icon;
      widget.windowTitle = object.title;
      widget.closed.connect(this.objectClosed.bind(this, object));

      window.addWidget(widget);
    }
    return object["_private_instance"];
  }

  , objectClosed : function(object)
  {
    delete object["_private_instance"];
  }
});
