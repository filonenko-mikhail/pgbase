// Created by trolls instead of __qt_sender__
Function.prototype.bind = function() {
  var func = this;
  var thisObject = arguments[0];
  var args = Array.prototype.slice.call(arguments, 1);
  return function() {
    return func.apply(thisObject, args);
  };
};

// ---------------------------------------------
// Date -> QDate*
Date.prototype.toQDate = function() {
  return new QDate(this.getFullYear(), this.getMonth(), this.getDay());
};

Date.prototype.toQTime = function() {
  return new QTime(this.getHours(), this.getMinutes(), this.getSeconds(), this.getMilliseconds());
};

Date.prototype.toQDateTime = function() {
  return new QDateTime(this.toDate() , this.toTime());
};


// ---------------------------------------------
// Static objects

QInvalidModelIndex = new QModelIndex();

staticUiLoader = new QUiLoader;

QEmptyPixmap = new QPixmap();

if (!Array.prototype.map)
{
  Array.prototype.map = function(fun /*, thisp*/)
  {
    var len = this.length;
    if (typeof fun != "function")
      throw new TypeError();

    var res = new Array(len);
    var thisp = arguments[1];
    for (var i = 0; i < len; i++)
    {
      if (i in this)
        res[i] = fun.call(thisp, this[i], i, this);
    }

    return res;
  };
}
