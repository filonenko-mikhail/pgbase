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

  // Контейнер для путевок
  this.hboxwidget = new QWidget();
  this.hboxwidget.objectName = "hboxwidget";


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
                                         ,"weather_type", //"car_id"
                                         ,"result_fuel"
                                         ,"result_fuel_cost"]);
  this.waybillsModel.sort("id", Qt.AscendingOrder, true);

  this.waybillsModel.addRelation("master_id", "catalog_masters", "id", "_name");
  this.waybillsModel.addRelation("car_id", "catalog_cars", "id", "_name || ' ' || _number");
  this.waybillsModel.setDefault("weather_type", 0);

  this.waybillsView.setModel(this.waybillsModel);
  this.waybillsView.hideColumns(["id", "_date",
                                 , "general_worked_time"
                                 , "comments"
                                 //, "start_mileage"
                                 //, "end_mileage"
                                 , "mileage"
                                 , "car_id"
                                 , "motorage"
                                 , "mileage_out"
                                 , "mileage_tech"
                                 , "mileage_total"
                                 , "weather_type"
                                 , "motorage_fuel"
                                 , "mileage_fuel"
                                 , "mileage_out_fuel"
                                 , "mileage_tech_fuel"
                                 , "result_fuel"
                                 //, "_number"
                                 , "start_fuel"
                                 , "end_fuel"]);
  this.waybillsView.tabKeyNavigation = false;
  this.waybillsView.horizontalHeader().cascadingSectionResizes = true;
  this.waybillsView.horizontalHeader().setResizeMode(QHeaderView.Stretch);
  this.hboxwidget.setFocusProxy(this.waybillsView);

  this.waybillsView2 = new PGTableView(this);
  this.waybillsView2.objectName = "waybillsView2";
  this.waybillsView2.setModel(this.waybillsModel);
  this.waybillsView2.hideColumns(["id", "_date", 
                                  , "general_worked_time"
                                  , "comments"
                                  , "master_id"
                                  , "start_mileage"
                                  , "end_mileage"
                                  //, "mileage"
                                  , "car_id"
                                  //, "motorage"
                                  //, "mileage_out"
                                  //, "mileage_tech"
                                  //, "mileage_total"
                                  , "weather_type"
                                  ]);
  this.waybillsView2.focusPolicy = Qt.NoFocus;
  this.waybillsView2.tabKeyNavigation = false;
  this.waybillsView2.horizontalHeader().cascadingSectionResizes = true;
  this.waybillsView2.horizontalHeader().setResizeMode(QHeaderView.Stretch);
  this.waybillsView2.verticalHeader().hide();

  //connect the headers and scrollbars of both tableviews together
  this.waybillsView.verticalScrollBar().valueChanged.connect(this.waybillsView2.verticalScrollBar(), this.waybillsView2.verticalScrollBar().setValue);
  this.waybillsView2.verticalScrollBar().valueChanged.connect(this.waybillsView.verticalScrollBar(), this.waybillsView.verticalScrollBar().setValue);

  this.hboxwidget2 = new QWidget();
  this.hboxwidget2.objectName = "hboxwidget2";

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
  this.createdLessonsView.tabKeyNavigation = false;
  this.hboxwidget2.setFocusProxy(this.createdLessonsView);

  this.createdLessonsView2 = new PGTableView(this);
  this.createdLessonsView2.objectName = "practicalLessonsView2";
  this.createdLessonsView2.setModel(this.createdLessonsModel);
  this.createdLessonsView2.hideColumns([
  "id"
  ,"waybill_id"
  ,"cursant_id"
  ,"exersize_id"
  ,"mark"
  //,"worked_time"
  ,"comments"
  //,"motorage"
  ,"mileage"
  ,"mileage_out"
  //,"mileage_tech"
  //,"motorage_fuel"
  //,"mileage_fuel"
  //,"mileage_out_fuel"
  //,"mileage_tech_fuel"
  ]);

  this.createdLessonsView2.tabKeyNavigation = false;

  this.createdLessonsView2.horizontalHeader().cascadingSectionResizes = true;
  this.createdLessonsView2.horizontalHeader().setResizeMode(QHeaderView.Stretch);
  this.createdLessonsView2.verticalHeader().hide();


  this.createdLessonsView.verticalScrollBar().valueChanged.connect(this.createdLessonsView2.verticalScrollBar(), this.createdLessonsView2.verticalScrollBar().setValue);
  this.createdLessonsView2.verticalScrollBar().valueChanged.connect(this.createdLessonsView.verticalScrollBar(), this.createdLessonsView.verticalScrollBar().setValue);


  // Нарушения
  this.violationsView = new PGTableView(this);
  this.violationsView.objectName = "violationsView";
  this.violationsModel = new PGTableModel(this);
  this.violationsModel.setTable("journal_violations", database);
  this.violationsModel.addRelation("violation_id", "catalog_violations", "id", "_name");
  this.violationsModel.sort("id", Qt.AscendingOrder);

  this.violationsView.setModel(this.violationsModel);
  this.violationsView.hideColumns(["id","master_id","_date","from_id"]);
  this.violationsView.tabKeyNavigation = false;

  // Заправка
  this.fuelView = new PGTableView(this);
  this.fuelView.objectName = "fuelView";
  this.fuelModel = new PGTableModel(this);
  this.fuelModel.setTable("journal_fuel", database);
  this.fuelModel.sort("id", Qt.AscendingOrder);

  this.fuelView.setModel(this.fuelModel);
  this.fuelView.hideColumns(["id","waybill_id"]);
  this.fuelView.tabKeyNavigation = false;


  this.splitter1 = new QSplitter(Qt.Vertical, this);
  this.splitter1.objectName = "splitter1";

  // Раскладываем
  this.vboxlayout.addWidget(this.waybillDateLabel, 0, 0);
  this.vboxlayout.addWidget(this.waybillDate, 0, 1);
  this.vboxlayout.addWidget(this.waybillWeatherType, 0, 2);
  this.vboxlayout.addItem(new QSpacerItem(40, 2, QSizePolicy.Expanding, QSizePolicy.Fixed), 0, 2, 1, 1, 0);
  this.vboxlayout.addWidget(this.splitter1, 1, 0, 1, 3);

  this.hboxlayout = new QHBoxLayout();
  this.hboxlayout.spacing = 0;
  this.hboxlayout.addWidget(this.waybillsView, 7, 0);
  this.hboxlayout.addWidget(this.waybillsView2, 10, 0);

  this.hboxwidget.setLayout(this.hboxlayout);

  this.hboxlayout2 = new QHBoxLayout();
  this.hboxlayout2.spacing = 0;
  this.hboxlayout2.addWidget(this.createdLessonsView, 7, 0);
  this.hboxlayout2.addWidget(this.createdLessonsView2, 10, 0);

  this.hboxwidget2.setLayout(this.hboxlayout2);

  this.splitter1.addWidget(this.hboxwidget);
  this.splitter1.addWidget(this.fuelView);
  this.splitter1.addWidget(this.hboxwidget2);
  this.splitter1.addWidget(this.violationsView);


  QWidget.setTabOrder(this.waybillDate, this.waybillWeatherType);
  QWidget.setTabOrder(this.waybillWeatherType, this.waybillsView);
  QWidget.setTabOrder(this.waybillsView, this.fuelView); // a to b
  QWidget.setTabOrder(this.fuelView, this.createdLessonsView); // a to b
  QWidget.setTabOrder(this.createdLessonsView, this.violationsView); // a to b to c

  //this.setTabOrder(c, d); // a to b to c to d

  // create delegates
  // Делегаты
  this.mastersDelegateModel = new PGQueryModel(this);
  this.mastersDelegateModel.setQuery("SELECT id, _name FROM catalog_masters WHERE dismissed = false order by _name"
                                     , database);
  this.mastersDelegateModel.refresh();
  this.mastersDelegate = new TableDelegate(this);
  this.mastersDelegate.addModel(this.mastersDelegateModel, "Работают");
  this.mastersDelegate.endEditHint = QAbstractItemDelegate.EditNextItem;
  this.mastersDelegate.createDataEditor.connect(this, this.mastersDataEditor);
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
  this.violationsDelegate.endEditHint = QAbstractItemDelegate.EditNextItem;
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
  this.exersizesDelegateModelAll.setQuery("select id, _name as \"Упр.\" from catalog_exersizes where _type = 2 order by _order", database);
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

  var query = new QSqlQuery(database);
  query.prepare("select max(weather_type) from journal_waybills where _date = ?");
  query.addBindValue(this.waybillDate.date);
  query.exec();
  query.next();
  this.waybillWeatherType.setChecked(query.value(0));
  this.waybillsRowChanged(selectionModel.currentIndex());
};

form.prototype.weatherChanged = function(state) {
  var intState = (state != 0)*1;
  var query = new QSqlQuery(database);
  query.prepare("update journal_waybills set weather_type = ? where _date = ?");
  query.addBindValue(intState);
  var dateField = this.waybillsModel.record().field("_date");
  dateField.setValue(this.waybillDate.date);
  query.addBindValue(database.formatValue(dateField));
  query.exec();
  this.waybillsModel.refresh();
  this.waybillsModel.setDefault("weather_type", intState);
};

form.prototype.waybillsRowChanged = function(currentIndex) {
  var waybillRecord = this.waybillsModel.record(currentIndex.row());

  if (!currentIndex.isValid() || waybillRecord.isNull("id")) {
    this.violationsView.setEnabled(false);
    this.fuelView.setEnabled(false);
    this.createdLessonsView.setEnabled(false);
    this.createdLessonsView2.setEnabled(false);
    filter1 = "waybill_id is null";
    filter2 = "_date is null and master_id is null";
  } else {
    this.violationsView.setEnabled(false);
    this.fuelView.setEnabled(false);
    this.createdLessonsView.setEnabled(false);
    this.createdLessonsView2.setEnabled(false);

    filter1 = "waybill_id is null";
    filter2 = "_date is null and master_id is null";

    this.violationsView.setEnabled(true);
    this.fuelView.setEnabled(true);
    this.createdLessonsView.setEnabled(true);
    this.createdLessonsView2.setEnabled(true);

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

  this.fuelModel.filter = filter1;
  this.fuelModel.setDefault("waybill_id", waybillRecord.value("id"));
  this.fuelModel.refresh();

  this.violationsModel.filter = filter2;
  this.violationsModel.setDefault("_date", waybillRecord.value("_date"));
  this.violationsModel.setDefault("master_id", waybillRecord.value("master_id"));
  this.violationsModel.refresh();
};

form.prototype.mastersDataEditor = function() {
  var dateField = this.waybillsModel.record(this.waybillsView.currentIndex().row()).field("_date");

  this.mastersDelegateModel.setQuery("SELECT a.id, a._name FROM catalog_masters a where a.dismissed = false "
                                     +" and not exists(select id from journal_waybills b where b._date = %1 and a.id= b.master_id)"
                                     .arg(database.formatValue(dateField))
                                     + " order by a._name"
                                     , database);
  this.mastersDelegateModel.refresh();

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
                                       + ' order by _order'
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
