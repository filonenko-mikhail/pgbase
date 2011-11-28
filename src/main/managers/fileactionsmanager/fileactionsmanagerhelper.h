#ifndef FILEACTIONSMANAGERHELPER_H
#define FILEACTIONSMANAGERHELPER_H

#include <QtCore/QObject>

#include <QtGui/QAction>

class FileActionsManagerHelper : public QObject
{
  Q_OBJECT
public:
  explicit FileActionsManagerHelper(QObject* parent = 0);
  virtual ~FileActionsManagerHelper();

  virtual unsigned char approach(QWidget *widget) const = 0;
  virtual void activateFor(QWidget* widget) = 0;
  virtual void deactivate() = 0;


  virtual bool canNew() const {return false;};
  virtual bool canOpen() const {return false;};
  virtual bool canOpenRecent() const {return false;};


  virtual bool canSave() const {return false;};
  virtual bool canSaveAll() const {return false;};

  virtual bool canClose() const {return false;};

  virtual QStringList recentFiles() const {return QStringList();};

  virtual QList<QAction*> additionalActions() const {return QList<QAction*>();};

public slots:
  virtual void newAction(){};
  virtual void open(){};
  virtual void openRecent(const QString& fileName){};

  virtual void save(){};
  virtual void saveAll(){};

  virtual void close(){};

signals:
  void canNewChanged(bool enable);

  void canOpenChanged(bool enable);
  void canOpenRecentChanged(bool enable);

  void canSaveChanged(bool enable);
  void canSaveAllChanged(bool enable);

  void canCloseChanged(bool enable);

  void blockFocusChanged(bool block);

};

class QTextEdit;
class QPlainTextEdit;
class TextEditFileActionsManagerHelper : public FileActionsManagerHelper
{
  Q_OBJECT
public:
  explicit TextEditFileActionsManagerHelper(QObject* parent = 0);

  virtual unsigned char approach(QWidget* widget) const;

  virtual void activateFor(QWidget* widget);
  virtual void deactivate();

public slots:

private:
  QTextEdit *textFile;
  QPlainTextEdit *plainTextEdit;
};


#define COPYPASTEROWSFORMAT "application/x-pg-modelrows"
#include <QtGui/QItemSelection>

class QAbstractItemView;
class ItemViewFileActionsManagerHelper : public FileActionsManagerHelper
{
  Q_OBJECT
public:
  explicit ItemViewFileActionsManagerHelper(QObject* parent = 0);

  virtual unsigned char approach(QWidget* widget) const;

  virtual void activateFor(QWidget* widget);
  virtual void deactivate();

public slots:

private:
  QAbstractItemView *itemView;
};

#ifdef QT_WEBKIT_LIB

class QMenu;
class QWebView;
class WebViewFileActionsManagerHelper : public FileActionsManagerHelper
{
  Q_OBJECT
public:
  explicit WebViewFileActionsManagerHelper(QObject* parent = 0);
  virtual ~WebViewFileActionsManagerHelper();

  virtual unsigned char approach(QWidget* widget) const;

  virtual void activateFor(QWidget* widget);
  virtual void deactivate();

private:
  QWebView *webView;
};

#endif //#ifdef QT_WEBKIT_LIB

#endif //#ifdef FILEACTIONSMANAGERHELPER_H
