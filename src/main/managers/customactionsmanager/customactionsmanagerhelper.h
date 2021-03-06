#ifndef CUSTOMACTIONSMANAGERHELPER_H
#define CUSTOMACTIONSMANAGERHELPER_H

//#include "abstractactionsmanagerhelper.h"

#include <QtCore/QObject>

#include <QtGui/QAction>

class CustomActionsManagerHelper : public QObject// : public AbstractActionsManagerHelper
{
  Q_OBJECT
public:
  explicit CustomActionsManagerHelper(QObject* parent = 0);
  virtual ~CustomActionsManagerHelper();

  virtual unsigned char approach(QWidget *widget) const {return 0;};
  virtual void activateFor(QWidget* widget) = 0;
  virtual void deactivate() = 0;

  virtual QList<QAction*> additionalActions() const {return QList<QAction*>();};

signals:
  void blockFocusChanged(bool block);

};


#define COPYPASTEROWSFORMAT "application/x-pg-modelrows"
#include <QtGui/QItemSelection>

class QAbstractItemView;
class ItemViewCustomActionsManagerHelper : public CustomActionsManagerHelper
{
  Q_OBJECT
public:
  explicit ItemViewCustomActionsManagerHelper(QObject* parent = 0);

  virtual unsigned char approach(QWidget* widget) const;

  virtual void activateFor(QWidget* widget);
  virtual void deactivate();


  virtual bool canCutLines() const ;
  virtual bool canCopyLines() const ;
  virtual bool canPasteLines() const ;
  virtual bool canRemoveLines() const ;

  virtual bool canRefresh() const ;


  QList<QAction*> additionalActions() const;

public slots:
  virtual void cutLines();
  virtual void copyLines();
  virtual void pasteLines();
  virtual void removeLines();

  virtual void refresh();

signals:
  void canCutLinesChanged(bool enable);
  void canCopyLinesChanged(bool enable);
  void canPasteLinesChanged(bool enable);
  void canRemoveLinesChanged(bool enable);

  void canRefreshChanged(bool enable);

protected:
  bool eventFilter(QObject *object, QEvent *event);

private slots:
  void clipboardDataChanged();

  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
private:
  QAbstractItemView *itemView;

  QAction *cutLinesAction;
  QAction *copyLinesAction;
  QAction *pasteLinesAction;
  QAction *removeLinesAction;
  QAction *refreshActionSeparator;
  QAction *refreshAction;
};

#ifdef QT_WEBKIT_LIB

class QMenu;
class QWebView;
class WebViewCustomActionsManagerHelper : public CustomActionsManagerHelper
{
  Q_OBJECT
public:
  explicit WebViewCustomActionsManagerHelper(QObject* parent = 0);
  virtual ~WebViewCustomActionsManagerHelper();

  virtual unsigned char approach(QWidget* widget) const;

  virtual void activateFor(QWidget* widget);
  virtual void deactivate();

  QList<QAction*> additionalActions() const;

private:
  QWebView *webView;
  QMenu *webMenu;
};

#endif //#ifdef QT_WEBKIT_LIB

#endif //#ifdef CUSTOMACTIONSMANAGERHELPER_H
