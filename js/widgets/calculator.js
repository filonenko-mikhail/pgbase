NumPad = function(parent)
{
  QWidget.call(this, parent);
  this.windowTitle = qsTr("NumPad");
  var windowIcon = new QIcon(":/images/32x32/calculator/calculator.png");
  this.windowIcon = windowIcon;
  this.focusPolicy = Qt.NoFocus;


  this.setWindowFlags(Qt.WindowStaysOnTopHint);
  this.setAttribute(Qt.WA_ShowWithoutActivating, true);

  this.gridlayout = new QGridLayout(this);

  this.gridlayout.horizontalSpacing = 0;
  this.gridlayout.verticalSpacing = 0;
  this.gridlayout.spacing = 0;
  this.gridlayout.setContentsMargins(0,0,0,0);

  for (var i = 9; i >= 0; --i) {
    this["button" + i] = new QToolButton(this);
    this["button" + i].objectName = "button" + i;
    this["button" + i].text = i;
    this["button" + i].autoRepeat = true;
    var boldFont = new QFont();
    boldFont.setBold(true);
    boldFont.setPixelSize(12);
    this["button" + i].font = boldFont;
    this["button" + i].minimumSize = new QSize(42, 42);
    this["button" + i].focusPolicy = Qt.NoFocus;
    this.gridlayout.addWidget(this["button" + i], Math.floor( Math.abs(i - 9) / 3) + 1
                              , Math.abs(i?(i - 1):i)%3 + 2, 0);
    this["button" + i].clicked.connect(this.digitClicked.bind(this, i));
  }

  this.decimalPoint = new QToolButton(this);
  this.decimalPoint.objectName = "decimalPoint";
  this.decimalPoint.text = String.fromCharCode(QLocale.system().decimalPoint());
  this.decimalPoint.autoRepeat = false;
  this.decimalPoint.minimumSize = new QSize(42, 42);
  this.decimalPoint.focusPolicy = Qt.NoFocus;
  this.decimalPoint.clicked.connect(this, this.decimalPointClicked);
  this.gridlayout.addWidget(this.decimalPoint, 3 + 1, 1 + 2, 0);

  this.backspace = new QToolButton(this);
  this.backspace.objectName = "backspace";
  this.backspace.arrowType = Qt.LeftArrow;
  this.backspace.autoRepeat = false;
  this.backspace.minimumSize = new QSize(42, 42);
  this.backspace.focusPolicy = Qt.NoFocus;
  this.backspace.clicked.connect(this, this.backspaceClicked);
  this.gridlayout.addWidget(this.backspace, 3 + 1, 2 + 2,  0);

  this.enter = new QPushButton(this);
  this.enter.objectName = "enter";
  this.enter.text = qsTr("Enter");
  this.enter.autoRepeat = false;
  this.enter.minimumSize = new QSize(42, 42);
  this.enter.focusPolicy = Qt.NoFocus;
  this.enter.clicked.connect(this, this.enterClicked);
  this.gridlayout.addWidget(this.enter, 4 + 1, 0 + 2, 1, 3, 0);

  this.lineEdit = new QLineEdit(this);
  this.lineEdit.objectName = "calculatorLineEdit";
  this.lineEdit.alignment = Qt.AlignRight;
  this.lineEdit.dragEnabled = true;
  this.gridlayout.addWidget(this.lineEdit, 0, 0, 1, 5, 0);


  this.leftBracket = new QToolButton(this);
  this.leftBracket.objectName = "leftBracket";
  this.leftBracket.text = "(";
  this.leftBracket.autoRepeat = false;
  this.leftBracket.minimumSize = new QSize(42, 42);
  this.leftBracket.focusPolicy = Qt.NoFocus;
  this.leftBracket.clicked.connect(this, this.leftBracketClicked);
  this.gridlayout.addWidget(this.leftBracket, 1, 0, 1, 1, 0);

  this.rightBracket = new QToolButton(this);
  this.rightBracket.objectName = "rightBracket";
  this.rightBracket.text = ")";
  this.rightBracket.autoRepeat = false;
  this.rightBracket.minimumSize = new QSize(42, 42);
  this.rightBracket.focusPolicy = Qt.NoFocus;
  this.rightBracket.clicked.connect(this, this.rightBracketClicked);
  this.gridlayout.addWidget(this.rightBracket, 1, 1, 1, 1, 0);

  this.addition = new QToolButton(this);
  this.addition.objectName = "addition";
  this.addition.text = "+";
  this.addition.autoRepeat = false;
  this.addition.minimumSize = new QSize(42, 42);
  this.addition.focusPolicy = Qt.NoFocus;
  this.addition.clicked.connect(this, this.additionClicked);
  this.gridlayout.addWidget(this.addition, 2, 0, 1, 1, 0);

  this.substraction = new QToolButton(this);
  this.substraction.objectName = "substraction";
  this.substraction.text = "-";
  this.substraction.autoRepeat = false;
  this.substraction.minimumSize = new QSize(42, 42);
  this.substraction.focusPolicy = Qt.NoFocus;
  this.substraction.clicked.connect(this, this.substractionClicked);
  this.gridlayout.addWidget(this.substraction, 2, 1, 1, 1, 0);

  this.multiplication = new QToolButton(this);
  this.multiplication.objectName = "multiplication";
  this.multiplication.text = "*";
  this.multiplication.autoRepeat = false;
  this.multiplication.minimumSize = new QSize(42, 42);
  this.multiplication.focusPolicy = Qt.NoFocus;
  this.multiplication.clicked.connect(this, this.multiplicationClicked);
  this.gridlayout.addWidget(this.multiplication, 3, 0, 1, 1, 0);


  this.division = new QToolButton(this);
  this.division.objectName = "division";
  this.division.text = "/";
  this.division.autoRepeat = false;
  this.division.minimumSize = new QSize(42, 42);
  this.division.focusPolicy = Qt.NoFocus;
  this.division.clicked.connect(this, this.divisionClicked);
  this.gridlayout.addWidget(this.division, 3, 1, 1, 1, 0);

  this.notes = new QTextEdit(this);
  this.notes.objectName = "notes";
  this.notes.plainText = database.settings.value("storage");
  var notesFont = new QFont("Times", 14, QFont.Bold);
  this.notes.document().defaultFont = notesFont;
  this.gridlayout.addWidget(this.notes, 4 + 2, 0, 1, 5, 0);

  this.firstShow = true;
};
NumPad.prototype = new QWidget();

NumPad.prototype.hideEvent = function(event) {
  database.settings.setValue("storage", this.notes.plainText);
};


NumPad.prototype.digitClicked = function(digitValue)
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt["Key_"+digitValue]
                             , Qt.NoModifier, "" + digitValue, true, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt["Key_"+digitValue]
                             , Qt.NoModifier, "" + digitValue, true, 1));
};

NumPad.prototype.decimalPointClicked = function()
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, 0
                             , Qt.NoModifier, String.fromCharCode(QLocale.system().decimalPoint())
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, 0
                             , Qt.NoModifier, String.fromCharCode(QLocale.system().decimalPoint())
                             , false, 1));
};

NumPad.prototype.backspaceClicked = function()
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt.Key_Backspace
                             , Qt.NoModifier, null
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt.Key_Backspace
                             , Qt.NoModifier, null
                             , false, 1));
};

NumPad.prototype.leftBracketClicked = function()
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt.Key_BracketLeft
                             , Qt.NoModifier, "("
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt.Key_BracketLeft
                             , Qt.NoModifier, "("
                             , false, 1));
};

NumPad.prototype.rightBracketClicked = function()
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt.Key_BracketRight
                             , Qt.NoModifier, ")"
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt.Key_BracketRight
                             , Qt.NoModifier, ")"
                             , false, 1));
};

NumPad.prototype.additionClicked = function()
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt.Key_Plus
                             , Qt.NoModifier, "+"
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt.Key_Plus
                             , Qt.NoModifier, "+"
                             , false, 1));
};

NumPad.prototype.substractionClicked = function()
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt.Key_Minus
                             , Qt.NoModifier, "-"
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt.Key_Minus
                             , Qt.NoModifier, "-"
                             , false, 1));
};

NumPad.prototype.multiplicationClicked = function()
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt.Key_Asterisk
                             , Qt.NoModifier, "*"
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt.Key_Asterisk
                             , Qt.NoModifier, "*"
                             , false, 1));
};

NumPad.prototype.divisionClicked = function()
{
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt.Key_Slash
                             , Qt.NoModifier, "/"
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt.Key_Slash
                             , Qt.NoModifier, "/"
                             , false, 1));
};

NumPad.prototype.enterClicked = function()
{
  var focusWidget = QApplication.focusWidget();
  if (focusWidget.text) {
    var decimalPoint = String.fromCharCode(QLocale.system().decimalPoint());
    var result = focusWidget.text;

    while (result.indexOf(decimalPoint, 0) != -1) {
      result = result.replace(decimalPoint, ".");
    }

    //print(result);
    var success = true;
    try {
      result = eval(result);
      //print(result);
    } catch(e) {
      success = false;
    }
    if (success) {
      result = result.toString();
      while (result.indexOf(".", 0) != -1) {
        result = result.replace(".", decimalPoint);
      }
      focusWidget.text = result;
    }
  }
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyPress, Qt.Key_Return
                             , Qt.NoModifier, null
                             , false, 1));
  QCoreApplication.sendEvent(QApplication.focusWidget()
                             , new QKeyEvent(QEvent.KeyRelease, Qt.Key_Return
                             , Qt.NoModifier, null
                             , false, 1));
};
