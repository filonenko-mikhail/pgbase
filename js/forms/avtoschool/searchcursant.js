var form = function(parent) {
  FormWidget.call(this, parent);

  this.vboxlayout = new QGridLayout(this);

  this.lineEdit = new QLineEdit(this);
  this.pushButton = new QPushButton("Найти", this);

  // Виджеты контейнеры
  this.tabWidget = new QTabWidget(this);
  this.tabWidget.objectName = "tabWidget";

  this.queryModel = new PGQueryModel(this);

  this.tableView = new PGQueryView(this);
  //this.tableView.windowTitle = "Занятия";
  this.tableView.setModel(this.queryModel);

  this.tabWidget.addTab(this.tableView, "Занятия");
  //this.tabWidget.addTab(new QLabel(qsTr("asdfsad"), this), qsTr("Lable"));

  // Раскладываем
  this.vboxlayout.addWidget(this.lineEdit, 0, 0);
  this.vboxlayout.addWidget(this.pushButton, 0, 1);
  //this.vboxlayout.addItem(new QSpacerItem(40, 2, QSizePolicy.Expanding, QSizePolicy.Fixed), 0, 1, 1, 1, 0);
  //this.vboxlayout.addWidget(this.splitter1, 1, 0, 1, 2);
  this.vboxlayout.addWidget(this.tabWidget, 1, 0, 1, 2);

  this.lineEdit.returnPressed.connect(this.pushButton, this.pushButton.clicked);
  this.pushButton.clicked.connect(this, this.searchButtonClicked);

  //this.lineEdit.setFocus(Qt.OtherFocusReason);
};

form.objectName = "searchcursant";
form.path = "/Поиск/Учащегося";
form.icon = new QIcon("images:/32x32/find/find-cursant.png");
form.title = "Поиск учащегося";
form.description = "Поиск практических занятий учащегося";
form.actionCategory = "Поиск";
form.toString = function() {return this.objectName;};

form.prototype = new FormWidget();

form.prototype.searchButtonClicked = function() {
  this.queryModel.setQuery('select group_name as "Группа", cursant_name as "Учащийся", _date as "Дата", master_name as "МПОУМТС", ' +
                           ' exersize_name as "№", worked_time as "Ч", mark as "Оц",' +
                           ' motorage as "М/ч", mileage as "Км гор", mileage_out as "Км заг", mileage_tech as "Км тех",' +
                           ' motorage_fuel as "Л м/ч", mileage_fuel as "Л гор", mileage_out_fuel as "Л загор", mileage_tech_fuel as "Л тех",' +
                           ' result_fuel as "Л всего", fuel_cost as "Л цена", result_fuel_cost as "Т стоим", ' +
                           ' _cost as "Стоим"' +
                           ' from cursant_lessons'
                           + " where cursant_name like '%%1%'".arg(this.lineEdit.text), database);
  this.queryModel.refresh();

  this.tableView.horizontalHeader().setResizeMode(QHeaderView.ResizeToContents);
  //this.lessonsVi.horizontalHeader().setResizeMode(QHeaderView.ResizeToContents);
};

//return
form;
