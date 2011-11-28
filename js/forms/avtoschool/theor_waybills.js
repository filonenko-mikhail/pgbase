var form = function(parent) {
  FormWidget.call(this, parent);

  this.vboxlayout = new QGridLayout(this);

  var currentDate = new Date();
  currentDate.setMilliseconds(0);

  this.waybillDateLabel = new QLabel("Дата:", this);

  this.waybillDate = new QDateEdit(currentDate, this);

  //this.waybillDate.minimumSize = new QSize(100, this.waybillDate.sizeHint.height);
  this.waybillDate.calendarPopup = true;
  this.waybillDate.calendarWidget().firstDayOfWeek = Qt.Monday;
  this.waybillDate.dateChanged.connect(this, this.refresh);
  //this.dateChange(this.waybillDate.date);

  this.teachersLabel = new QLabel("Преподаватель:", this);

  this.teachersCombo = new QComboBox;

  this.teachersModel = new PGQueryModel(this);
  this.teachersModel.setQuery("select id, _name from catalog_teachers where not dismissed order by _name", database);
  this.teachersModel.refresh();

  this.teachersCombo.setModel(this.teachersModel);
  this.teachersCombo.modelColumn = 1;
  this.teachersCombo.currentIndex = -1;
  this.teachersCombo["currentIndexChanged(int)"].connect(this, this.refresh);

  this.groupsLabel = new QLabel("Группа:", this);

  this.groupsComboModel = new PGQueryModel(this);

  this.groupsCombo = new QComboBox(this);
  this.groupsCombo.setModel(this.groupsComboModel);
  this.groupsCombo.modelColumn = 1;

  this.exersizeLabel = new QLabel("Занятие:", this);
  this.exersizeComboModel = new PGQueryModel(this);
  this.exersizeComboModel.setQuery("select id, _name, length from catalog_exersizes where _type = 1 order by _order", database);
  this.exersizeComboModel.refresh();

  this.exersizeCombo = new QComboBox(this);
  this.exersizeCombo.setModel(this.exersizeComboModel);
  this.exersizeCombo.modelColumn = 1;
  this.exersizeCombo["currentIndexChanged(int)"].connect(this, this.refresh);

  this.workedLabel = new QLabel("Кол-во часов:", this);
  this.workedEdit = new QSpinBox(this);
  this.workedEdit.minimum = 6;
  this.workedEdit.maximum = 8;
  this.workedEdit["editingFinished()"].connect(this, this.refresh);

  this.generate = new QPushButton("Создать", this);
  this.generate["clicked()"].connect(this, this.generateClicked);

  // Путевой лист
  this.waybillsView = new PGTableView(this);
  this.waybillsView.objectName = "waybillsView";
  this.waybillsModel = new PGTableModel(this);
  this.waybillsModel.setTable("journal_theor_lessons", database);
  this.waybillsModel.addRelation("teacher_id", "catalog_teachers", "id", "_name");
  this.waybillsModel.addRelation("cursant_id", "catalog_cursants", "id", "_name");
  this.waybillsModel.addRelation("exersize_id", "catalog_exersizes", "id", "_name");

  this.waybillsModel.setColumnsReadOnly(["id","_date", "teacher_id", "worked_time", "_cost"]);

  this.waybillsView.setModel(this.waybillsModel);
  this.waybillsView.hideColumns(["id","teacher_id", "_date", "worked_time", "_cost"]);
  this.waybillsModel.sort("id", Qt.AscendingOrder);

  this.cursantsDelegateModel = new PGQueryModel(this);

  this.cursantsDelegate = new TableDelegate(this);
  this.cursantsDelegate.endEditHint = QAbstractItemDelegate.EditNextItem;
  this.cursantsDelegate.addModel(this.cursantsDelegateModel);
  this.cursantsDelegate.createDataEditor.connect(this, this.createCursantsDataEditor);

  this.exersizesDelegateModel = new PGQueryModel(this);
  this.exersizesDelegateModel.setQuery("select id, _name as \"Упр.\" from catalog_exersizes where _type = 1 order by _order", database);
  this.exersizesDelegateModel.refresh();

  this.exersizesDelegate = new TableDelegate(this);
  this.exersizesDelegate.endEditHint = QAbstractItemDelegate.EditNextItem;
  this.exersizesDelegate.addModel(this.exersizesDelegateModel);
  this.exersizesDelegate.createDataEditor.connect(this, this.createExersizeDataEditor);

  this.waybillsView.setItemDelegateForColumn("cursant_id", this.cursantsDelegate);
  this.waybillsView.setItemDelegateForColumn("exersize_id", this.exersizesDelegate);

  this.vboxlayout.addWidget(this.waybillDateLabel, 0, 0);
  this.vboxlayout.addWidget(this.waybillDate, 0, 1);
  this.vboxlayout.addWidget(this.teachersLabel, 0, 2);
  this.vboxlayout.addWidget(this.teachersCombo, 0, 3);

  this.vboxlayout.addWidget(this.groupsLabel, 0, 4);
  this.vboxlayout.addWidget(this.groupsCombo, 0, 5);

  this.vboxlayout.addWidget(this.exersizeLabel, 0, 6);
  this.vboxlayout.addWidget(this.exersizeCombo, 0, 7);

  this.vboxlayout.addWidget(this.workedLabel, 0, 8);
  this.vboxlayout.addWidget(this.workedEdit, 0, 9);

  this.vboxlayout.addWidget(this.generate, 0, 10);

  this.vboxlayout.addItem(new QSpacerItem(40, 2, QSizePolicy.Expanding, QSizePolicy.Fixed), 0, 11, 1, 1, 0);

  this.vboxlayout.addWidget(this.waybillsView, 1, 0, 1, 12, 0);

  var selectionModel = this.waybillsView.selectionModel();

  this.refresh();
};

form.path = "/Редактирование/Журналы/Теор. занятия";
form.title = "Журнал \"Теоретические занятия\"";
form.icon = new QIcon();
form.objectName = "theor_waybills";
form.description = "Ведение учета теоретических занятий";
form.actionCategory = "Редактирование";
form.toString = function() {return this.objectName;};

form.prototype = new FormWidget();

form.prototype.refresh = function() {
  var dateField = this.waybillsModel.record().field("_date");
  dateField.setValue(this.waybillDate.date);
  var filter1 = '"_date" = ' + database.formatValue(dateField);
  var teacherField = this.teachersModel.record(this.teachersCombo.currentIndex).field("id");
  var exersizeField = this.exersizeComboModel.record(this.exersizeCombo.currentIndex).field("id");
  var lengthField = this.exersizeComboModel.record(this.exersizeCombo.currentIndex).field("length");
  //print(teacherField.value());
  filter1 += " AND \"teacher_id\" = " + database.formatValue(teacherField);

  if(this.workedEdit.text.length == 0) {
    this.workedEdit.text = lengthField.value();
  }

  this.waybillsModel.filter = filter1;
  this.waybillsModel.setDefault("_date", dateField.value());
  this.waybillsModel.setDefault("teacher_id"
                                , teacherField.value());
  this.waybillsModel.setDefault("exersize_id"
                                , exersizeField.value());
  this.waybillsModel.setDefault("worked_time"
                                , this.workedEdit.value + ':00');

  this.waybillsModel.refresh();

  if (this.lastTeacher != this.teachersCombo.currentIndex) {
    this.groupsComboModel.setQuery("select id, _name as \"Группа\" from catalog_groups where (teacher_id = %1 or med_teacher_id = %1) order by _name desc"
                                   .arg(database.formatValue(teacherField))
                                   , database);

    this.groupsComboModel.refresh();
    this.groupsCombo.modelColumn = 1;
    this.lastTeacher = this.teachersCombo.currentIndex;
  }
};

form.prototype.createCursantsDataEditor = function() {
  var teacherField = this.teachersModel.record(this.teachersCombo.currentIndex).field("id");
  var groupField = this.groupsComboModel.record(this.groupsCombo.currentIndex).field("id");

  this.cursantsDelegateModel.setQuery("select a.id"
                                      + " , a._name as \"ФИО\" from catalog_cursants as a "
                                      + " join catalog_groups b on b.id = a.group_id and b.id = %1 and b.teacher_id = %2"
                                      .arg(database.formatValue(groupField))
                                      .arg(database.formatValue(teacherField))
                                      + " where not exists (select * from journal_theor_lessons g where a.id = g.cursant_id and exersize_id in (select id from catalog_exersizes where final_exam and _type = 1 )) "
                                      + " order by b.complete_for nulls first, a._name "
                                      , database);
  this.cursantsDelegateModel.refresh();
};


form.prototype.createExersizeDataEditor = function() {
  //print('create data editor');
  var cursantField = this.waybillsModel.record(this.waybillsView.currentIndex().row()).field("cursant_id");
  //print(cursantField);

  this.exersizesDelegateModel.setQuery("select id, _name as \"Упр.\" from catalog_exersizes where _type = 1 and id not in (select exersize_id from journal_theor_lessons where cursant_id = %1 and mark > 2) order by id"
                                       .arg(database.formatValue(cursantField))
                                       ,database);
  this.exersizesDelegateModel.refresh();
};

form.prototype.generateClicked = function() {
  var dateField = this.waybillsModel.record().field("_date");
  dateField.setValue(this.waybillDate.date);
  var teacherField = this.teachersModel.record(this.teachersCombo.currentIndex).field("id");
  var exersizeField = this.exersizeComboModel.record(this.exersizeCombo.currentIndex).field("id");
  var lengthField = this.exersizeComboModel.record(this.exersizeCombo.currentIndex).field("length");
  var groupField = this.groupsComboModel.record(this.groupsCombo.currentIndex).field("id");

  var insertQuery = "insert into journal_theor_lessons " +
    " (teacher_id, cursant_id, exersize_id, _date, worked_time) " +
    " select " + teacherField.value() + " , a.id, " + exersizeField.value() +
    ", " + database.formatValue(dateField) + "::date, " +
    "'" + this.workedEdit.value +":00'::interval from catalog_cursants a " +
    "  where a.group_id = " + database.formatValue(groupField);

  var query = new QSqlQuery(insertQuery, database);
  if (query.lastError().isValid()) {
    print(insertQuery);
    print(query.lastError().text());
  }

  this.refresh();
};

//return
form;
