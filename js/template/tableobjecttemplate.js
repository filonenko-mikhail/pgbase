(function(tableName)
{
  var tableForm = function(parent) {
    FormWidget.call(this, parent);
    this.tableName = arguments.callee.tableName;
    this.objectName = arguments.callee.objectName;

    this.layout = new QVBoxLayout(this);

    this.tableView = new PGTableView(this);
    this.tableView.objectName = this.tableName + "TableView";
    this.tableModel = new PGTableModel(this);
    this.tableModel.objectName = this.tableName + "TableModel";
    this.tableModel.setTable(this.tableName, database);
    this.tableView.setModel(this.tableModel);

    var pIndex = database.primaryIndex(this.tableName);
    var order = Qt.AscendingOrder;
    for (var i = 0; i < pIndex.count(); ++i) {
      if (pIndex.isDescending(i))
        order = Qt.DescendingOrder;
      else
        order = Qt.AscendingOrder;
      this.tableModel.sort(this.tableModel.record().indexOf(pIndex.fieldName(i)), order, false);
    }
    this.tableModel.refresh();


    this.layout.addWidget(this.tableView, 0, 0);
    this.setLayout(this.layout);
  };

  tableForm.prototype = new FormWidget();

  tableForm.prototype.tableName = tableName;

  tableForm.objectName = tableName;

  tableForm.tableName = tableName;
  // filed by configuration.js
  tableForm.path = "/Таблицы/" + tableName;
  tableForm.icon = new QIcon("images:32x32/table/table.png");
  var query = new QSqlQuery(database);
  query.exec("select description from pg_description where objsubid = 0 and objoid = '"
             + tableName + "'::regclass");
  if (query.next())
    tableForm.description = query.value(0);

  tableForm.title = "Таблица " + tableForm.description;

  tableForm.actionCategory = "Таблицы";

  tableForm.toString = function() {return this.objectName;};

  return tableForm;
});
