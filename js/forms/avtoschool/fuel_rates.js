
form = function(parent) {
  FormWidget.call(this, parent);
  this.tableName = "journal_fuel_rates";

  this.objectName = this.tableName;

  this.layout = new QVBoxLayout(this);

  this.tableView = new PGTableView(this);
  this.tableView.objectName = "tableView";
  this.tableModel = new PGTableModel(this);
  this.tableModel.objectName = "tableModel";
  this.tableModel.setTable(this.tableName, database);
  this.tableModel.addRelation("car_id", "catalog_cars", "id", "_name || ' ' || _number");
  this.tableView.setModel(this.tableModel);

  this.carDelegateModel = new PGQueryModel(this);
  this.carDelegateModel.setQuery("SELECT id, _name || ' ' || _number FROM catalog_cars WHERE not dismissed order by id"
                                 , database);
  this.carDelegateModel.refresh();

  this.carDelegate = new TableDelegate(this);
  this.carDelegate.addModel(this.carDelegateModel, "Все");

  this.tableView.setItemDelegateForColumn("car_id", this.carDelegate);

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

form.objectName = "fuel_rates";
form.path = "/Редактирование/Журналы/Нормы топлива";
form.icon = new QIcon();
form.title = "Журнал 'Нормы топлива'";
form.actionCategory = "Журналы";
form.toString = function() {return this.objectName;};

form.prototype = new FormWidget();

form;
