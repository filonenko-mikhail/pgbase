var form = function(parent) {
  FormWidget.call(this, parent);
  this.objectName = "groupsFormWidget";

  // Размещаем все на форме
  this.tabWidget = new QTabWidget(this);
  this.tabWidget.objectName = "tabWidget";
  this.vboxlayout = new QVBoxLayout(this);
  this.splitter = new QSplitter(Qt.Vertical, this);
  this.splitter.objectName = "splitter";
  this.vboxlayout.addWidget(this.splitter, 0, 0);

  this.groupsView = new PGTableView(this);
  this.groupsView.objectName = "groupsView";
  this.cursantsView = new PGTableView(this.tabWidget);
  this.cursantsView.objectName = "cursantsView";

  this.splitter.addWidget(this.groupsView);
  this.splitter.addWidget(this.tabWidget);
  this.cadetTab = this.tabWidget.addTab(this.cursantsView, new QIcon(), "Учащиеся");

  // Создание настройка моделей.
  this.groupsModel = new PGTableModel(this);
  this.groupsModel.setTable("catalog_groups", database);
  this.groupsModel.objectName = "groupsModel";
  this.groupsModel.sort("id");
  // for sorting apply
  this.groupsModel.refresh();
  this.groupsModel.addRelation("form_id", "catalog_group_forms", "id", "_name");
  this.groupsModel.addRelation("teacher_id", "catalog_teachers", "id", "_name");
  this.groupsModel.addRelation("med_teacher_id", "catalog_teachers", "id", "_name");
  this.groupsModel.addRelation("comments", "catalog_teachers", "id", "_name");

  this.groupsView.setModel(this.groupsModel);

  this.cursantsModel = new PGTableModel(this);
  this.cursantsModel.setTable("catalog_cursants", database);
  this.cursantsModel.addRelation("master_id", "catalog_masters", "id", "_name");
  // ставим свою сортировку
  this.cursantsModel.sort("_name");

  this.cursantsView.setModel(this.cursantsModel);

  // Дополнительная настройка вьюшек
  this.groupsView.hideColumn("id");
  this.groupsView.freezeSqlColumns(["_name"]);
  this.groupsModel.setColumnsReadOnly(["id","cursants_count", "worked_time", "program_worked_time"]);

  this.cursantsView.hideColumns(["id", "group_id"]);
  this.cursantsView.freezeSqlColumns(["_name"]);
  this.cursantsModel.setColumnsReadOnly(["worked_time","id","group_id", "theor_cost", "pract_cost"]);

  this.formDelegateModel = new PGQueryModel(this);
  this.formDelegateModel.setQuery("SELECT id, _name as \"Форма\" FROM catalog_group_forms order by id", database);
  this.formDelegateModel.refresh();
  this.formDelegate = new TableDelegate(this);
  this.formDelegate.hideHorizontalHeader = false;
  this.formDelegate.addModel(this.formDelegateModel, "Все");
  this.formDelegate.createDataEditor.connect(this.formDelegateModel
                                             , this.formDelegateModel.refresh);
  this.groupsView.setItemDelegateForColumn("form_id", this.formDelegate);

  this.teachersDelegateModel = new PGQueryModel(this);
  this.teachersDelegateModel.setQuery("SELECT id, _name as \"ФИО\" "
                                      + " FROM catalog_teachers WHERE dismissed = false order by _name", database);
  this.teachersDelegateModel.refresh();

  this.teachersDelegate = new TableDelegate(this);
  this.teachersDelegate.addModel(this.teachersDelegateModel, "Работают");
  this.teachersDelegate.hideHorizontalHeader = false;
  this.teachersDelegate.createDataEditor.connect(this.teachersDelegateModel
                                                 , this.teachersDelegateModel.refresh);
  this.groupsView.setItemDelegateForColumn("teacher_id", this.teachersDelegate);
  this.groupsView.setItemDelegateForColumn("med_teacher_id", this.teachersDelegate);

  this.mastersDelegateModel = new PGQueryModel(this);
  this.mastersDelegateModel.setQuery("SELECT id, _name as \"ФИО\""
                                      + " FROM catalog_masters WHERE dismissed = false order by _name", database);
  this.mastersDelegateModel.refresh();

  this.mastersDelegate = new TableDelegate(this);
  this.mastersDelegate.hideHorizontalHeader = false;
  this.mastersDelegate.addModel(this.mastersDelegateModel, "Работают");
  this.cursantsView.setItemDelegateForColumn("master_id", this.mastersDelegate);

  var sexes = ["М", "Ж"];
  this.sexDelegateModel = new QStringListModel(this);
  this.sexDelegateModel.setStringList(sexes);

  this.sexDelegate = new TableDelegate(this);
  this.sexDelegate.addModel(this.sexDelegateModel);
  this.sexDelegate.hideKeyColumn = false;
  this.cursantsView.setItemDelegateForColumn("sex", this.sexDelegate);


  // изменение строки в группах
  this.selectionModel = this.groupsView.selectionModel();
  this.selectionModel.currentRowChanged.connect(this, this.groupsCurrentRowChanged);
  this.groupsModel.dataChanged.connect(this, this.groupsDataChanged);

  this.groupsCurrentRowChanged(this.selectionModel.currentIndex());
};

form.objectName = "groups";
form.path = "/Редактирование/Справочники/Группы";
form.icon = new QIcon("images:32x32/group/group.png");
form.title = "Справочник 'Группы'";
form.actionCategory = "Справочники";
form.toString = function() {return this.objectName;};

form.prototype = new FormWidget();

form.prototype.groupsCurrentRowChanged = function(current) {
  var groupRecord = this.groupsModel.record(current.row());
  var filter = "";

  if (groupRecord.isNull("id")){
    filter = "group_id IS NULL";
    this.cursantsView.setEnabled(false);
  } else {
    filter = "group_id = " + database.formatValue(groupRecord.field("id"));
    this.cursantsView.setEnabled(true);
  }

  this.cursantsModel.filter = filter;
  this.cursantsModel.setDefault("group_id", groupRecord.value("id"));
  this.cursantsModel.refresh();

  this.tabWidget.setTabText(this.cadetTab, "Учащиеся группы №%1".arg(groupRecord.value("_name")));
};

form.prototype.groupsDataChanged = function(topLeft, bottomRight) {
  var selectionModel = this.groupsView.selectionModel();
  this.groupsCurrentRowChanged(selectionModel.currentIndex());
};

form;
