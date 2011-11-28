#include "searchwidget.h"

#include <QtGui/QMenu>
#include <QtGui/QPainter>

namespace Qt {
  enum MatchFlagEx {
    MatchBackward = 0x8000
                  };
}

SearchWidget::SearchWidget(QWidget *parent, Qt::WindowFlags flags)
  : QWidget(parent, flags) {
  init();
}

SearchWidget::~SearchWidget(){}

void SearchWidget::init()
{
  setupUi(this);
  setObjectName("searchWidget");
  //setWindowTitle(tr("Search"));
  setFocusProxy(findLineEdit);


  findPreviousButton->setShortcut(QKeySequence::FindPrevious);
  findNextButton->setShortcut(QKeySequence::FindNext);
  replaceNextButton->setShortcut(QKeySequence::Replace);


  //findLineEdit->setButtonPosition(QwwButtonLineEdit::RightInside);

  QMenu *menu = new QMenu(this);

  findLineEdit->setMenu(menu);
  findLineEdit->setPixmap(QIcon(":/images/magnifier.png").pixmap(24));
  setFindHintString(tr("<Type find string>"));
  findLineEdit->setSide(FancyLineEdit::Right);

  menu->addAction(wholeWordsAction);
  menu->addAction(regExpAction);
  menu->addAction(caseSensitiveAction);
  menu->addAction(recursiveAction);

  findActions[WholeWords] = wholeWordsAction;
  findActions[RegExp] = regExpAction;
  findActions[CaseSensitive] = caseSensitiveAction;
  findActions[Recursive] = recursiveAction;

  connect(wholeWordsAction, SIGNAL(triggered())
          , this, SLOT(actionTriggered()));
  connect(regExpAction, SIGNAL(triggered())
          , this, SLOT(actionTriggered()));
  connect(caseSensitiveAction, SIGNAL(triggered())
          , this, SLOT(actionTriggered()));
  connect(recursiveAction, SIGNAL(triggered())
          , this, SLOT(actionTriggered()));

//	connect(findLineEdit, SIGNAL(textChanged(QString))
//					, this, SIGNAL(findStringChanged(QString)));
//	connect(replaceLineEdit, SIGNAL(textChanged(QString))
//					, this, SIGNAL(replaceStringChanged(QString)));

  connect(findLineEdit, SIGNAL(textEdited(QString))
          , this, SLOT(findStringEditedSlot(QString)));
  connect(replaceLineEdit, SIGNAL(textEdited(QString))
          , this, SLOT(replaceStringEditedSlot(QString)));

  connect(findNextButton, SIGNAL(clicked())
          , this, SLOT(findNextClicked()));
  connect(findPreviousButton, SIGNAL(clicked())
          , this, SLOT(findPreviousClicked()));

  connect(replaceNextButton, SIGNAL(clicked())
          , this, SLOT(replaceNextClicked()));
  connect(replacePreviousButton, SIGNAL(clicked())
          , this, SLOT(replacePreviousClicked()));

  connect(replaceAllButton, SIGNAL(clicked())
          , this, SLOT(replaceAllClicked()));

  connect(closeButton, SIGNAL(clicked())
          , this, SIGNAL(closeRequested()));
}

QString SearchWidget::findString()
{
  return findLineEdit->typedText();
}

QString SearchWidget::replaceString()
{
  return replaceLineEdit->text();
}

bool SearchWidget::isWholeWordsSearch() const
{
  return wholeWordsAction->isChecked();
}
void SearchWidget::setWholeWordsSearch(bool yes)
{
  wholeWordsAction->setChecked(yes);
}

bool SearchWidget::isRegExpSearch() const
{
  return regExpAction->isChecked();
}
void SearchWidget::setRegExpSearch(bool yes)
{
  regExpAction->setChecked(yes);
}

bool SearchWidget::isCaseSensitiveSearch() const
{
  return caseSensitiveAction->isChecked();
}
void SearchWidget::setCaseSensitiveSearch(bool yes)
{
  caseSensitiveAction->setChecked(yes);
}


bool SearchWidget::isRecursive() const
{
  return recursiveAction->isChecked();
}
void SearchWidget::setRecursive(bool yes)
{
  recursiveAction->setChecked(yes);;
}

void SearchWidget::setFindString(const QString& findString)
{
  findLineEdit->setText(findString);
}

void SearchWidget::setFindHintString(const QString& hint)
{
  findLineEdit->setHintText(hint);
}

void SearchWidget::setReplaceString(const QString& replaceString)
{
  replaceLineEdit->setText(replaceString);
}


void SearchWidget::canReplaceChanged(bool enable)
{
  if (replaceLabel->isEnabled() != enable) {
    replaceLabel->setEnabled(enable);;
    replaceLineEdit->setEnabled(enable);
    replacePreviousButton->setEnabled(enable);
    replaceNextButton->setEnabled(enable);
    replaceAllButton->setEnabled(enable);
  }
}

void SearchWidget::canFindChanged(bool enabled)
{
  QList<QWidget *> children = findChildren<QWidget*>();
  foreach (QWidget* child, children)
    if (child->objectName().compare("closeButton"))
      child->setEnabled(enabled);
}

QAction* SearchWidget::findAction(Actions type)
{
  return findActions.value(type);
}

void SearchWidget::updateFindIcons()
{
  bool casesensitive = caseSensitiveAction->isChecked();
  bool wholewords = wholeWordsAction->isChecked();
  bool regexp = regExpAction->isChecked();

  QPixmap pixmap(17, 17);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  int x = 16;

  if (casesensitive) {
    painter.drawPixmap(x - 10, 0, caseSensitiveAction->icon().pixmap(16));
    x -= 6;
  }
  if (wholewords) {
    painter.drawPixmap(x - 10, 0, wholeWordsAction->icon().pixmap(16));
    x -= 6;
  }
  if (regexp) {
    painter.drawPixmap(x - 10, 0, regExpAction->icon().pixmap(16));
  }
  if (!casesensitive && !wholewords && !regexp) {
    QPixmap mag(":/images/find/magnifier.png");
    painter.drawPixmap(0, (pixmap.height() - mag.height()) / 2, mag);
  }
  findLineEdit->setPixmap(pixmap);
}

void SearchWidget::findStringEditedSlot(const QString& /*string*/)
{
  emit findEdited(value(), flags());
}

void SearchWidget::replaceStringEditedSlot(const QString& string)
{
  emit replaceEdited(value(), string, flags());
}

void SearchWidget::findNextClicked()
{
  emit findNext(value(), flags());
}

void SearchWidget::findPreviousClicked()
{
  Qt::MatchFlags matchFlags = flags();
  matchFlags.operator |=(QFlag(Qt::MatchBackward));
  emit findPrevious(value(), matchFlags);
}

void SearchWidget::replaceNextClicked()
{
  emit replaceNext(value(), replaceLineEdit->text(), flags());
}

void SearchWidget::replacePreviousClicked()
{
  Qt::MatchFlags matchFlags = flags();
  matchFlags.operator |=(QFlag(Qt::MatchBackward));
  emit replacePrevious(value(), replaceLineEdit->text(), matchFlags);
}

void SearchWidget::replaceAllClicked()
{
  emit replaceAll(value(), replaceLineEdit->text(), flags());
}

QVariant SearchWidget::value()
{
  if (isRegExpSearch())
    return QRegExp(findLineEdit->typedText());
  else
    return findLineEdit->typedText();
}

Qt::MatchFlags SearchWidget::flags()
{
  Qt::MatchFlags matchFlags;
  if (isRegExpSearch()) {
    matchFlags = Qt::MatchRegExp;
    if (isCaseSensitiveSearch())
      matchFlags |= Qt::MatchCaseSensitive;
  } else {
    if (!isWholeWordsSearch())
      matchFlags |= Qt::MatchContains;
    else
      matchFlags |= Qt::MatchFixedString;
    if (isCaseSensitiveSearch())
      matchFlags |= Qt::MatchCaseSensitive;
  }
  if (isRecursive())
    matchFlags |= Qt::MatchRecursive;

  return matchFlags;
}

void SearchWidget::actionTriggered()
{
  emit findEdited(value(), flags());
}
