var form = function(parent) {
  FormWidget.call(this, parent);

  this.showForms = arguments.callee.showForms;
  this.showTables = arguments.callee.showTables;
  this.showReports = arguments.callee.showReports;

  this.vboxlayout = new QVBoxLayout(this);

  this.formsGroupBox = new QGroupBox("Формы", this);
  this.formsGroupBox.layout = new QVBoxLayout(this.formsGroupBox);
  this.vboxlayout.addWidget(this.formsGroupBox, 0, 0);
  this.reportsGroupBox = new QGroupBox("Отчеты", this);
  this.reportsGroupBox.layout = new QVBoxLayout(this.reportsGroupBox);
  this.vboxlayout.addWidget(this.reportsGroupBox, 0, 0);
  this.tablesGroupBox = new QGroupBox("Таблицы", this);
  this.tablesGroupBox.layout = new QVBoxLayout(this.tablesGroupBox);
  this.vboxlayout.addWidget(this.tablesGroupBox, 0, 0);

  var currentForm;
  var currentButton;
  for (var formIndex in this.showForms) {
    currentForm = configuration.forms[this.showForms[formIndex]];

    currentButton = new QPushButton(currentForm.action.text, this.formsGroupBox);
    currentButton.objectName = currentForm.action.objectName + "Button";
    currentButton.icon = currentForm.action.icon;
    currentButton.clicked.connect(currentForm.action, currentForm.action.triggered);

    this.formsGroupBox[currentButton.objectName] = currentButton;

    this.formsGroupBox.layout.addWidget(currentButton, 0, 0);
  }

  var currentTable;
  var currentButton;
  for (var tableIndex in this.showTables) {
    currentTable = configuration.tables[this.showTables[tableIndex]];

    //select description from pg_description where objsubid = 0 and objoid = 'journal_currency_exchange'::regclass
    var tableDescription = currentTable.action.toolTip;
    currentButton = new QPushButton(tableDescription, this.tablesGroupBox);
    currentButton.objectName = currentTable.action.objectName + "Button";
    currentButton.icon = currentTable.action.icon;
    currentButton.clicked.connect(currentTable.action, currentTable.action.triggered);

    this.tablesGroupBox[currentButton.objectName] = currentButton;

    this.tablesGroupBox.layout.addWidget(currentButton, 0, 0);
  }

  this.vboxlayout.addStretch(1);
};

form.path = "/Главное меню";
form.icon = new QIcon("images:/32x32/application/application_home.png");
form.title = "Главное меню";
form.description = "Главное меню, предоставляющее доступ к основным функция системы";
form.actionCategory = "Основные";
form.objectName = "mainform";
form.toString = function() {return this.objectName;};
// filled when loaded by configuration.js
form.absoluteFilePath = "";
form.absolutePath = "";

form.prototype = new FormWidget();

// return
form;
