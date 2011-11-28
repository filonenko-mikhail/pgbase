
var form = function(parent) {
  FormWidget.call(this, parent);
  this.objectName = "planningGroupsFormWidget";

  // Размещаем все на форме
  this.tabWidget = new QTabWidget(this);
  this.tabWidget.objectName = "tabWidget";
  this.vboxlayout = new QVBoxLayout(this);
  this.splitter = new QSplitter(Qt.Vertical, this);
  this.splitter.objectName = "splitter";
  this.vboxlayout.addWidget(this.splitter, 0, 0);

  this.groupsView = new PGQueryView(this);
  this.groupsView.objectName = "groupsView";
  this.cursantsView = new PGTableView(this.tabWidget);
  this.cursantsView.objectName = "cursantsView";

  this.splitter.addWidget(this.groupsView);
  this.splitter.addWidget(this.tabWidget);
  this.cadetTab = this.tabWidget.addTab(this.cursantsView, new QIcon(), "Учащиеся");

  // Создание настройка моделей.
  this.groupsModel = new OddColorModel(this);

  this.groupsModel.setQuery('select a.group_id as "id", group_name as "Группа", complete_for as "Завершить", group_cursant_count as "Кол-во", master_name as "МПОУМТС", cursants_count as "Кол-во курсантов", needed_hour as "Необходимо", total_haved_time as "Имеется", total_stored as "Запас", ' +
                            'floor(total_stored/40) as "Может обучаться", ' +
                            'b.total_store_sum as "Всего может" ' +
                            'from masters_plan_new a ' +
                            'join (select group_id, sum(floor(total_stored/40)) as total_store_sum from masters_plan_new group by group_id) b '+
                            '  on a.group_id = b.group_id ' +
                            ' order by complete_for nulls first, a.group_id, master_name', database);
  this.groupsModel.objectName = "groupsModel";
  // for sorting apply
  this.groupsModel.refresh();

  this.groupsView.setModel(this.groupsModel);

  this.cursantsModel = new PGTableModel(this);
  this.cursantsModel.setTable("catalog_cursants", database);
  this.cursantsModel.addRelation("master_id", "catalog_masters", "id", "_name");
  // ставим свою сортировку
  this.cursantsModel.sort("_name");

  this.cursantsView.setModel(this.cursantsModel);

  // Дополнительная настройка вьюшек
  this.groupsView.hideColumn("id");

  this.cursantsView.hideColumns(["id", "group_id"]);
  this.cursantsView.freezeSqlColumns(["_name"]);
  this.cursantsModel.setColumnsReadOnly(["worked_time","id","group_id"]);


  this.formDelegateModel = new PGQueryModel(this);
  this.formDelegateModel.setQuery("SELECT id, _name as \"Форма\" FROM catalog_group_forms order by id", database);
  //this.formDelegateModel.refresh();
  this.formDelegate = new TableDelegate(this);
  this.formDelegate.hideHorizontalHeader = false;
  this.formDelegate.addModel(this.formDelegateModel, "Все");
  this.formDelegate.createDataEditor.connect(this.formDelegateModel
                                             , this.formDelegateModel.refresh);
  //this.groupsView.setItemDelegateForColumn("form_id", this.formDelegate);

  this.teachersDelegateModel = new PGQueryModel(this);
  this.teachersDelegateModel.setQuery("SELECT id, _name as \"ФИО\" "
                                      + " FROM catalog_teachers WHERE dismissed = false order by _name", database);
  //this.teachersDelegateModel.refresh();

  this.teachersDelegate = new TableDelegate(this);
  this.teachersDelegate.addModel(this.teachersDelegateModel, "Работают");
  this.teachersDelegate.hideHorizontalHeader = false;
  this.teachersDelegate.createDataEditor.connect(this.teachersDelegateModel
                                                 , this.teachersDelegateModel.refresh);
  //this.groupsView.setItemDelegateForColumn("teacher_id", this.teachersDelegate);
  //this.groupsView.setItemDelegateForColumn("med_teacher_id", this.teachersDelegate);

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

form.objectName = "planning_groups";
form.path = "/Редактирование/Справочники/Планирование";
form.icon = new QIcon();
form.title = "Распределение";
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

  this.tabWidget.setTabText(this.cadetTab, "Учащиеся группы №%1".arg(groupRecord.value("Группа")));
};

form.prototype.groupsDataChanged = function(topLeft, bottomRight) {
  var selectionModel = this.groupsView.selectionModel();
  this.groupsCurrentRowChanged(selectionModel.currentIndex());
};

form;
