// WORKAROUND findChildren and findChild become empty after qt.core loading
// i'm using children() and recursive call save State
saveState = function(settings, widget) {
  if (widget.objectName) {
    settings.remove(widget.objectName);
    settings.beginGroup(widget.objectName); //1
    var widgetList = widget.children();
    for( var i = 0; i < widgetList.length; ++i) {
      var view = widgetList[i];
      if (view instanceof QTableView && view.objectName != null) {
        saveTableViewState(view, settings);
      } else if (view instanceof QTreeView && view.objectName != null) {
        saveTreeViewState(view, settings);
      }  else if (view instanceof QSplitter && view.objectName != null) {
        saveSplitterState(view, settings);
        saveState(settings, view);
      } else if (view instanceof QWidget)
        saveState(settings, view);
    }
    settings.endGroup(); //1
  }
};

restoreState = function(settings, widget) {
  if (widget.objectName) {
    settings.beginGroup(widget.objectName); //1
    var widgetList = widget.children();
    for( var i = 0; i < widgetList.length; ++i) {
      var view = widgetList[i];
      if (view instanceof QTableView) {
        restoreTableViewState(view, settings);
      } else if (view instanceof QTreeView) {
        restoreTreeViewState(view, settings);
      } else if (view instanceof QSplitter) {
        restoreSplitterState(view, settings);
        restoreState(settings, view);
      } else if (view instanceof QWidget)
        restoreState(settings, view);
    }
    settings.endGroup(); //1
  }
};

saveHeaderViewState = function(view, settings) {
  if (!view.objectName)
    return;
  settings.setValue("movable", view.isMovable());
  settings.setValue("highlightSections", view.highlightSections);
  settings.beginWriteArray("sections");
  var model = view.model();
  if (model) {
    for (var i = 0; i < view.count(); ++i) {
      settings.setArrayIndex(i);
      //print(i + model.headerData(i, view.orientation()));
      settings.setValue("visualIndex", view.visualIndex(i));
      settings.setValue("hidden", view.isSectionHidden(i));
      settings.setValue("data", model.headerData(i, view.orientation()));
      var sectionSize = view.isSectionHidden(i)?view.defaultSectionSize:view.sectionSize(i);
      settings.setValue("width", sectionSize);
    }
  }
  settings.endArray();
};


// WORKAROUND Because ecma script toBoolean convert empty string to false, non-empty string to true, and settings::value returns QVariant which may be string.
restoreHeaderViewState = function(view, settings) {
  if (!view.objectName)
    return;
  var movable;
  movable = typeof(settings.value("movable")) == "string"
            ? settings.value("movable") == "true"
            : settings.value("movable");
  view.setMovable(true);
  var highlightSections;
  highlightSections = typeof(settings.value("highlightSections")) == "string"
                      ? settings.value("highlightSections") == "true"
                      : settings.value("highlightSections");
  view.highlightSections = highlightSections;
  var count = settings.beginReadArray("sections");
  var model = view.model();
  //print ('-----------');
  if (model) {
    for (var i = 0; i < count; ++i) {
      settings.setArrayIndex(i);
      //print(i + " " + model.headerData(i, view.orientation()) + " OLD " + view.visualIndex(i) + " NEW " + settings.value("visualIndex") );
      var hidden = typeof(settings.value("hidden")) == "string" ? settings.value("hidden") == "true" : settings.value("hidden");
      view.setSectionHidden(i, hidden);
      view.moveSection(view.visualIndex(i), settings.value("visualIndex"));
      view.resizeSection(i, settings.value("width", view.defaultSectionSize));
      model.setHeaderData(i, view.orientation(), settings.value("data", model.headerData(i, view.orientation())));
    }
  }
  view.setMovable(movable);
  settings.endArray();
};

saveTableViewState = function(view, settings) {
  if (!view.objectName)
    return;
  settings.beginGroup(view.objectName);
  var model = view.model();
  if (model) {
    saveHeaderViewState(view.horizontalHeader(), settings);
    var parentIndex = view.currentIndex();
    var deep = 0;
    settings.remove("lastIndex");
    settings.beginWriteArray("lastIndex");
    while (parentIndex.isValid()) {
      settings.setArrayIndex(deep);
      settings.setValue("x", parentIndex.column());
      settings.setValue("y", parentIndex.row());
      parentIndex = parentIndex.parent();
      ++deep;
    }
    settings.endArray();
  }
  settings.endGroup();
};

restoreTableViewState = function(view, settings) {
  if (!view.objectName)
    return;
  settings.beginGroup(view.objectName);
  var model = view.model();
  if (model) {
    restoreHeaderViewState(view.horizontalHeader(), settings);
    var parentIndex = QInvalidModelIndex;
    var x;
    var y;
    var deep = settings.beginReadArray("lastIndex");
    for (var i = deep - 1; i >= 0; --i) {
      settings.setArrayIndex(i);
      x = settings.value("x", 0);
      y = settings.value("y", 0);
      parentIndex = model.index(y, x, parentIndex);
    }
    settings.endArray();
    view.setCurrentIndex(parentIndex);
    view.scrollTo(parentIndex);
  }
  settings.endGroup();
};

saveTreeViewState = function(view, settings) {
  if (!view.objectName)
    return;
  settings.beginGroup(view.objectName);
  var model = view.model();
  if (model) {
    saveHeaderViewState(view.header(), settings);
    var parentIndex = view.currentIndex();
    var deep = 0;
    settings.remove("lastIndex");
    settings.beginWriteArray("lastIndex");
    while (parentIndex.isValid()) {
      settings.setArrayIndex(deep);
      settings.setValue("x", parentIndex.column());
      settings.setValue("y", parentIndex.row());
      parentIndex = parentIndex.parent();
      ++deep;
    }
    settings.endArray();
  }
  settings.endGroup();
};


restoreTreeViewState = function(view, settings) {
  if (!view.objectName)
    return;
  settings.beginGroup(view.objectName);
  var model = view.model();
  if (model) {
    restoreHeaderViewState(view.header(), settings);
    var parentIndex = QInvalidModelIndex;
    var x;
    var y;
    var deep = settings.beginReadArray("lastIndex");
    for (var i = deep - 1; i >= 0; --i) {
      settings.setArrayIndex(i);
      x = settings.value("x", 0);
      y = settings.value("y", 0);
      parentIndex = model.index(y, x, parentIndex);
    }
    settings.endArray();
    view.setCurrentIndex(parentIndex);
  }
  settings.endGroup();
};

saveSplitterState = function(view, settings) {
  if (!view.objectName) {
    return;
  }
  //settings.beginGroup(view.objectName);
  var state = view.saveState();
  settings.setValue(view.objectName + "state", state);
  //settings.endGroup();
};

restoreSplitterState = function(view, settings) {
  if (!view.objectName) {
    return;
  }
  //settings.beginGroup(view.objectName);
  if (settings.contains(view.objectName + "state")) {
    var state = settings.value(view.objectName + "state");
    view.restoreState(state);
  }
  //settings.endGroup();
};
