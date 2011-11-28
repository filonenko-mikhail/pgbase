var form = function(parent) {
  FormWidget.call(this, parent);
  this.objectName = "cursant_statisticsFormWidget";

  // Размещаем все на форме
  this.vboxlayout = new QVBoxLayout(this);
  this.splitter = new QSplitter(Qt.Horizontal, this);
  this.splitter.objectName = "splitter1";
  this.vboxlayout.addWidget(this.splitter, 0, 0);

  this.groupsView = new PGQueryView(this);
  this.groupsView.objectName = "groupsView";
  this.cursantsView = new PGQueryView(this);
  this.cursantsView.objectName = "cursantsView";
  this.lessonsView = new PGQueryView(this);
  this.lessonsView.objectName = "lessonsView";

  this.splitter.addWidget(this.groupsView);
  this.splitter.addWidget(this.cursantsView);
  this.splitter.addWidget(this.lessonsView);

  // Создание настройка моделей.
  this.groupsModel = new PGQueryModel(this);
  this.groupsModel.setQuery("select id, _name from catalog_groups order by complete_for nulls first, id", database);
  this.groupsModel.objectName = "groupsModel";
  this.groupsModel.sort("id");
  // for sorting apply
  this.groupsModel.refresh();

  this.groupsView.setModel(this.groupsModel);

  this.cursantsModel = new PGQueryModel(this);
  this.cursantsModel.setQuery("select id, group_id, _name, master_id from catalog_cursants", database);
  this.cursantsModel.refresh();
  this.cursantsModel.addRelation("master_id", "catalog_masters", "id", "_name");
  // ставим свою сортировку
  this.cursantsModel.sort("_name");

  this.cursantsView.setModel(this.cursantsModel);

  this.lessonsModel = new PGQueryModel(this);
  // ставим свою сортировку

  this.lessonsView.setModel(this.lessonsModel);

  // Дополнительная настройка вьюшек
  this.groupsView.hideColumn("id");

  this.cursantsView.hideColumns(["id", "group_id"]);

  // изменение строки в группах
  this.selectionModel = this.groupsView.selectionModel();
  this.selectionModel.currentRowChanged.connect(this, this.groupsCurrentRowChanged);
  this.cursantsView.selectionModel().currentRowChanged.connect(this, this.cursantsCurrentRowChanged);

  this.groupsCurrentRowChanged(this.selectionModel.currentIndex());
};

form.objectName = "cursant_statistics";
form.path = "/Отчеты/Учащиеся";
form.icon = new QIcon();
form.title = "Журнал обучения";
form.actionCategory = "Отчеты";
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
  this.cursantsModel.refresh();
};

form.prototype.cursantsCurrentRowChanged = function(current) {
  var cursantRecord = this.cursantsModel.record(current.row());

  this.lessonsModel.setQuery('select _date as "Дата", master_name as "МПОУМТС", ' +
                             ' exersize_name as "№", worked_time as "Ч", mark as "Оц",' +
                             ' motorage as "М/ч", mileage as "Км гор", mileage_out as "Км заг",' + // mileage_tech as "Км тех",' +
                             ' motorage_fuel as "Л м/ч", mileage_fuel as "Л гор", mileage_out_fuel as "Л загор",' +  //mileage_tech_fuel as "Л тех",' +
                             ' result_fuel as "Л всего", fuel_cost as "Л цена", result_fuel_cost as "Т стоим", ' +
                             ' _cost as "Стоим"' +
                             ' from cursant_lessons'
                             + " where cursant_id = %1".arg(cursantRecord.field("id").value()), database);
  this.lessonsModel.refresh();
  this.lessonsView.horizontalHeader().setResizeMode(QHeaderView.ResizeToContents);
};

form;
