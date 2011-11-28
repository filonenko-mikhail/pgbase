#include "fileactionsmanagerhelper.h"
//#include "private/fileactionsmanagerhelper_p.h"

#include <QtDebug>

#include <QtCore/QMimeData>

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QAction>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>

FileActionsManagerHelper::FileActionsManagerHelper(QObject *parent)
  :QObject(parent) {}

FileActionsManagerHelper::~FileActionsManagerHelper(){}


#include <QtGui/QTextEdit>
#include <QtGui/QPlainTextEdit>

// -------------------------------------------------
// Text File
TextEditFileActionsManagerHelper::TextEditFileActionsManagerHelper(QObject *parent)
  :FileActionsManagerHelper(parent)
  , textFile(0)
  , plainTextEdit(0)
{
}

unsigned char TextEditFileActionsManagerHelper::approach(QWidget *widget) const
{
  return qobject_cast<QTextEdit*>(widget) || qobject_cast<QPlainTextEdit*>(widget) ? 10 : 0;
}

void TextEditFileActionsManagerHelper::activateFor(QWidget* widget)
{
  if ((textFile = qobject_cast<QTextEdit*>(widget))) {
  } else if ((plainTextEdit = qobject_cast<QPlainTextEdit*>(widget))) {
  }
}

void TextEditFileActionsManagerHelper::deactivate()
{
  if (textFile) {
    textFile = 0;
  } else if (plainTextEdit) {
    plainTextEdit = 0;
  }
}

#include <QtGui/QAbstractItemView>
// ---------------------------------------------
// ItemView
ItemViewFileActionsManagerHelper::ItemViewFileActionsManagerHelper(QObject *parent)
  :FileActionsManagerHelper(parent)
  , itemView(0){}

unsigned char ItemViewFileActionsManagerHelper::approach(QWidget *widget) const
{
  return qobject_cast<QAbstractItemView *>(widget) ? 10 : 0;
}

void ItemViewFileActionsManagerHelper::activateFor(QWidget *widget)
{
  if ((itemView = qobject_cast<QAbstractItemView*>(widget))) {

  }
}

void ItemViewFileActionsManagerHelper::deactivate()
{
  itemView = 0;
}


#ifdef QT_WEBKIT_LIB

#include <QtGui/QMenu>

#include <QtWebKit/QWebView>
#include <QtWebKit/QWebPage>

WebViewFileActionsManagerHelper::WebViewFileActionsManagerHelper(QObject *parent)
  :FileActionsManagerHelper(parent)
{
  webMenu = new QMenu(tr("Web view"));
  connect(webMenu, SIGNAL(aboutToShow())
          , this, SLOT(aboutToShow()));
  connect(webMenu, SIGNAL(aboutToHide())
          , this, SLOT(aboutToHide()));
}

WebViewFileActionsManagerHelper::~WebViewFileActionsManagerHelper()
{
  delete webMenu;
}

unsigned char WebViewFileActionsManagerHelper::approach(QWidget *widget) const
{
  return qobject_cast<QWebView*>(widget) ? 10 : 0;
}

void WebViewFileActionsManagerHelper::activateFor(QWidget *widget)
{
  if ((webView = qobject_cast<QWebView*>(widget))) {

  }
}

void WebViewFileActionsManagerHelper::deactivate()
{
  webView = 0;
}

#endif //#ifdef QT_WEBKIT_LIB
