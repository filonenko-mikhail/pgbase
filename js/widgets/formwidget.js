FormWidget = function(parent) {
  QWidget.call(this, parent);
  this.firstShow = true;

  this.closed = signal();
};

FormWidget.prototype = new QWidget();

FormWidget.prototype.showEvent = function(event) {
  if (this.firstShow) {
    this.firstShow = false;
    restoreState(database.settings, this);
  }
};

FormWidget.prototype.closeEvent = function(event) {
  saveState(database.settings, this);

  event.accept();

  this.closed.emit();
};
