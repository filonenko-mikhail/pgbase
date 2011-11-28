// -------------------- Sql -------------------------
QSqlDatabase.prototype.formatValue = function(field, trimstrings) {
  if (field instanceof QSqlField)
    return this.driver().formatValue(field, trimstrings);
  return "";
};

QSqlDatabase.prototype.escapeIdentifier = function(identifier, type) {
  return this.driver().escapeIdentifier(identifier, type);
};

QSqlDatabase.prototype.sqlStatement = function(type, tableName, record, preparedStatement) {
  return this.driver().sqlStatement(type, tableName, record, preparedStatement);
};

stringListModelFromSql = function(queryString, parent) {
  var sqlQuery = new QSqlQuery(database);
  var stringList = [];
  var i = 0;
  if (sqlQuery.exec(queryString)) {
    while(sqlQuery.next()) {
      stringList[i] = sqlQuery.value(0).toString();
      ++i;
    }
  }

  var resultModel = new QStringListModel(parent);
  resultModel.setStringList(stringList);

  return resultModel;
};
