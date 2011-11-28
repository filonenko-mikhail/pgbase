var form = function(parent) {
  FormWidget.call(this, parent);

  this.vboxlayout = new QGridLayout(this);

  // Дата
  var currentDate = new Date();
  currentDate.setMilliseconds(0);

  this.waybillDateLabel = new QLabel("Дата:", this);

  this.waybillDate = new QDateEdit(currentDate, this);

  //this.waybillDate.minimumSize = new QSize(100, this.waybillDate.sizeHint.height);
  this.waybillDate.calendarPopup = true;
  this.waybillDate.calendarWidget().firstDayOfWeek = Qt.Monday;
  this.waybillDate.dateChanged.connect(this, this.dateChanged);

  this.waybillWeatherType = new QCheckBox("Зима", this);
  this.waybillWeatherType.stateChanged.connect(this, this.weatherChanged);

  // Путевой лист
  this.waybillsView = new PGTableView(this);
  this.waybillsView.objectName = "waybillsView";
  this.waybillsModel = new PGTableModel(this);
  this.waybillsModel.setTable("journal_waybills", database);
  this.waybillsModel.setColumnsReadOnly(["id"
                                         //,"_date"
                                         ,"general_worked_time"
                                         ,"motorage" ,"mileage","mileage_out"//, "mileage_tech"
                                         ,"motorage_fuel" ,"mileage_fuel","mileage_out_fuel"//, "mileage_tech_fuel"
                                         ,"fuel_cost"
                                         ,"weather_type", "car_id"//, "result_fuel"
                                         , "result_fuel_cost"]);
  this.waybillsModel.sort("id", Qt.AscendingOrder, true);

  this.waybillsModel.addRelation("master_id", "catalog_masters", "id", "_name");
  this.waybillsModel.addRelation("car_id", "catalog_cars", "id", "_name || ' ' || _number");
  this.waybillsModel.setDefault("weather_type", 0);

  this.waybillsView.setModel(this.waybillsModel);
  this.waybillsView.hideColumns(["id", "_date"]);

  // Нарушения
  this.violationsView = new PGTableView(this);
  this.violationsView.objectName = "violationsView";
  this.violationsModel = new PGTableModel(this);
  this.violationsModel.setTable("journal_violations", database);
  this.violationsModel.addRelation("violation_id", "catalog_violations", "id", "_name");
  this.violationsModel.sort("id", Qt.AscendingOrder);

  this.violationsView.setModel(this.violationsModel);
  this.violationsView.hideColumns(["id","master_id","_date","from_id"]);

  // Выполненные задания
  this.createdLessonsView = new PGTableView(this);
  this.createdLessonsView.objectName = "practicalLessonsView";
  this.createdLessonsModel = new PGTableModel(this);
  this.createdLessonsModel.setTable("journal_practical_lessons", database);
  this.createdLessonsModel.addRelation("cursant_id", "catalog_cursants", "id", "_name");
  this.createdLessonsModel.addRelation("exersize_id", "catalog_exersizes", "id", "_name");
  this.createdLessonsModel.addRelation("mark", "catalog_marks", "id", "_name");
  this.createdLessonsModel.sort("id", Qt.AscendingOrder);
  this.createdLessonsModel.setColumnsReadOnly(["waybill_id","motorage_fuel" ,"mileage_fuel","mileage_out_fuel", "mileage_tech_fuel"
                                               , "result_fuel", "result_fuel_cost"]);

  this.createdLessonsView.setModel(this.createdLessonsModel);
  this.createdLessonsView.hideColumns(["id","waybill_id"]);

  // Виджеты контйнеры
  this.tabWidget1 = new QTabWidget(this);
  this.tabWidget1.objectName = "tabWidget1";
  this.tabWidget1.addTab(this.violationsView, "Нарушения");

  this.splitter1 = new QSplitter(Qt.Vertical, this);
  this.splitter1.objectName = "splitter1";

  this.tabWidget2 = new QTabWidget(this);
  this.tabWidget2.objectName = "tabWidget2";
  this.tabWidget2.addTab(this.createdLessonsView, "Выполненные задания");

  // Раскладываем
  this.vboxlayout.addWidget(this.waybillDateLabel, 0, 0);
  this.vboxlayout.addWidget(this.waybillDate, 0, 1);
  this.vboxlayout.addWidget(this.waybillWeatherType, 0, 2);
  this.vboxlayout.addItem(new QSpacerItem(40, 2, QSizePolicy.Expanding, QSizePolicy.Fixed), 0, 2, 1, 1, 0);
  this.vboxlayout.addWidget(this.splitter1, 1, 0, 1, 3);
  this.splitter1.addWidget(this.waybillsView);
  this.splitter1.addWidget(this.tabWidget2);
  this.splitter1.addWidget(this.tabWidget1);

  // create delegates
  // Делегаты
  this.mastersDelegateModel = new PGQueryModel(this);
  this.mastersDelegateModel.setQuery("SELECT id, _name FROM catalog_masters WHERE dismissed = false order by _name"
                                     , database);
  this.mastersDelegateModel.refresh();
  this.mastersDelegate = new TableDelegate(this);
  this.mastersDelegate.addModel(this.mastersDelegateModel, "Работают");
  this.mastersDelegate.endEditHint = QAbstractItemDelegate.EditNextItem;
  this.waybillsView.setItemDelegateForColumn("master_id", this.mastersDelegate);

  this.carDelegateModel = new PGQueryModel(this);
  this.carDelegateModel.setQuery("SELECT id, _name || ' ' || _number FROM catalog_cars WHERE not dismissed order by id"
                                 , database);
  this.carDelegateModel.refresh();

  this.carDelegate = new TableDelegate(this);
  this.carDelegate.addModel(this.carDelegateModel, "Все");

  this.waybillsView.setItemDelegateForColumn("car_id", this.carDelegate);

  // create delegates
  this.violationsDelegateModel = new PGQueryModel(this);

  this.violationsDelegateModel.setQuery("SELECT id, _name FROM catalog_violations where not hide order by id", database);

  this.violationsDelegateModel.refresh();
  this.violationsDelegate = new TableDelegate(this);
  this.violationsDelegate.addModel(this.violationsDelegateModel, "Нарушения");
  this.violationsView.setItemDelegateForColumn("violation_id", this.violationsDelegate);

  this.cursantsDelegateModel = new PGQueryModel(this);
  this.cursantsDelegateModelAll = new PGQueryModel(this);
  this.cursantsDelegateModelAll.setQuery("select a.id, b._name as \"Группа\", a._name as \"Учащийся\" "
                                         + " , a.program_worked_time - a.worked_time as \"Осталось\""
                                         + " , a.comments as \"Комментарий\""
                                         + " from catalog_cursants a "
                                         + " join catalog_groups b on b.id = a.group_id and not a._released and length(a._name)>1"
                                         + " order by b.id, a._name", database);
  this.cursantsDelegateModelAll.refresh();

  this.cursantsDelegate = new TableDelegate(this);
  this.cursantsDelegate.hideHorizontalHeader = false;
  this.cursantsDelegate.addModel(this.cursantsDelegateModel, "МПОУМТС");
  this.cursantsDelegate.addModel(this.cursantsDelegateModelAll, "Все");
  this.cursantsDelegate.endEditHint = QAbstractItemDelegate.EditNextItem;
  this.cursantsDelegate.createDataEditor.connect(this, this.cursantsDataEditor);

  this.cursantsDelegate.createDataEditor.connect(this.cursantsDelegateModelAll, this.cursantsDelegateModelAll.refresh);
  this.createdLessonsView.setItemDelegateForColumn("cursant_id", this.cursantsDelegate);

  this.exersizesDelegateModel = new PGQueryModel(this);

  this.exersizesDelegateModelAll = new PGQueryModel(this);
  this.exersizesDelegateModelAll.setQuery("select id, _name as \"Упр.\" from catalog_exersizes where _type = 2 order by id", database);
  this.exersizesDelegateModelAll.refresh();
  this.exersizesDelegate = new TableDelegate(this);
  this.exersizesDelegate.addModel(this.exersizesDelegateModel, "Текущие");
  this.exersizesDelegate.addModel(this.exersizesDelegateModelAll, "Все");
  this.exersizesDelegate.endEditHint = QAbstractItemDelegate.EditNextItem;
  this.exersizesDelegate.hideHorizontalHeader = false;
  this.exersizesDelegate.createDataEditor.connect(this, this.createdLessonsDataEditor);
  this.createdLessonsView.setItemDelegateForColumn("exersize_id", this.exersizesDelegate);

  this.marksDelegateModel = new PGQueryModel(this);
  this.marksDelegateModel.setQuery("select id, _name as \"Оценка\" from catalog_marks order by _name", database);
  this.marksDelegateModel.refresh();

  this.marksDelegate = new TableDelegate(this);
  this.marksDelegate.addModel(this.marksDelegateModel);
  this.marksDelegate.hideHorizontalHeader = false;
  this.marksDelegate.endEditHint = QAbstractItemDelegate.EditNextItem;

  var selectionModel = this.waybillsView.selectionModel();

  selectionModel.currentRowChanged.connect(this, this.waybillsRowChanged);

  this.waybillsModel.dataChanged.connect(this, this.waybillDataChanged);
  this.createdLessonsModel.dataChanged.connect(this, this.createdLessonsDataChanged);

  this.dateChanged(this.waybillDate.date);

  this.waybillsRowChanged(QInvalidModelIndex);
};

form.objectName = "waybills";
form.path = "/Редактирование/Журналы/Путевые листы";
form.icon = new QIcon("images:/32x32/calendar/calendar_waybills.png");
form.title = "Журнал 'Путевые листы'";
form.description = "Добавление/редактирование/удаление путевого листа из системы";
form.actionCategory = "Журналы";
form.toString = function() {return this.objectName;};
// filled when loaded by configuration.js
form.absoluteFilePath = "";
form.absolutePath = "";

form.prototype = new FormWidget();

form.prototype.dateChanged = function(date) {
  var dateField = this.waybillsModel.record().field("_date");
  dateField.setValue(date);
  var filter1 = '"_date" = ' + database.formatValue(dateField);
  this.waybillsModel.filter = filter1;
  this.waybillsModel.setDefault("_date", date);
  this.waybillsModel.refresh();
  var selectionModel = this.waybillsView.selectionModel();

  this.waybillsRowChanged(selectionModel.currentIndex());
};

form.prototype.weatherChanged = function(state) {
  var intState = (state != 0)*1;
  this.waybillsModel.setDefault("weather_type", intState);
};

form.prototype.waybillsRowChanged = function(currentIndex) {
  var waybillRecord = this.waybillsModel.record(currentIndex.row());

  if (!currentIndex.isValid() || waybillRecord.isNull("id")) {
    this.violationsView.setEnabled(false);
    this.createdLessonsView.setEnabled(false);
    filter1 = "waybill_id is null";
    filter2 = "_date is null and master_id is null";
  } else {
    this.violationsView.setEnabled(false);
    this.createdLessonsView.setEnabled(false);

    filter1 = "waybill_id is null";
    filter2 = "_date is null and master_id is null";

    this.violationsView.setEnabled(true);
    this.createdLessonsView.setEnabled(true);

    var field = waybillRecord.field("id");
    var dateField = waybillRecord.field("_date");
    var masterIdField = waybillRecord.field("master_id");
    var filter1 = "";
    var filter2 = "";

    filter1 = "waybill_id = " + database.formatValue(waybillRecord.field("id"));

    filter2 = "_date = " + database.formatValue(waybillRecord.field("_date"))
              + " AND master_id = " + database.formatValue(waybillRecord.field("master_id"));
  }

  this.createdLessonsModel.filter = filter1;
  this.createdLessonsModel.setDefault("waybill_id", waybillRecord.value("id"));
  this.createdLessonsModel.refresh();

  this.violationsModel.filter = filter2;
  this.violationsModel.setDefault("_date", waybillRecord.value("_date"));
  this.violationsModel.setDefault("master_id", waybillRecord.value("master_id"));
  this.violationsModel.refresh();
};

form.prototype.cursantsDataEditor = function() {
  var masterField = this.waybillsModel.record(this.waybillsView.currentIndex().row()).field("master_id");
  var dateField = this.waybillsModel.record(this.waybillsView.currentIndex().row()).field("_date");

  this.cursantsDelegateModel.setQuery("select a.id, b._name as \"Группа\", a._name as \"Учащийся\" "
                                      + " , a.program_worked_time - a.worked_time as \"Необходимо\""
                                      + " , a.comments as \"Комментарий\""
                                      + " from catalog_cursants a"
                                      + " join catalog_groups b on b.id = a.group_id and not a._released and length(a._name) > 1"
                                      + "    and not b._released "
                                      + " where a.master_id = %1 "
                                      .arg(database.formatValue(masterField))
                                      + " order by a._name", database);
  this.cursantsDelegateModel.refresh();
};

form.prototype.createdLessonsDataEditor = function() {
  var cursantField = this.createdLessonsModel.record(this.createdLessonsView.currentIndex().row()).field("cursant_id");
  this.exersizesDelegateModel.setQuery('select b.id as id, b._name as \"Упр.\" '
                                       + ' from catalog_exersizes b where b._type = 2 '
                                       + ' and (not (exists(select a.id from journal_practical_lessons a where cursant_id = %1 and a.exersize_id = b.id and mark > 2)) or b.optional) '.arg(database.formatValue(cursantField))
                                       + ' order by id'
                                       , database);
  this.exersizesDelegateModel.refresh();
};

form.prototype.waybillDataChanged = function(topLeft, bottomRight) {
  var selectionModel = this.waybillsView.selectionModel();
  this.waybillsRowChanged(selectionModel.currentIndex());
};

form.prototype.createdLessonsDataChanged = function(topLeft, bottomRight) {
  var selectionModel = this.createdLessonsView.selectionModel();
};

form;
