#ifndef FINDACTIONSMANAGER_H
#define FINDACTIONSMANAGER_H

//#include "abstractactionsmanager.h"

class FindActionsManagerHelper;

#include <QtCore/QObject>
class SearchWidget;
/*!
  \ingroup managers
* @class FindActionsManager findactionsmanager.h "src/main/managers/findactionsmanager.h"
*/
class FindActionsManagerPrivate;
class FindActionsManager : public QObject
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(FindActionsManager)

  void init(QWidget* parent);
protected:
  FindActionsManager(FindActionsManagerPrivate &dd, QWidget* parent);

  FindActionsManagerPrivate * const d_ptr;
public:
  explicit FindActionsManager(QWidget *parent);
  virtual ~FindActionsManager();

  void setSearchWidget(SearchWidget* searchWidget);
  SearchWidget *searchWidget() const;

  void addHelper(FindActionsManagerHelper* helper);
  void removeHelper(FindActionsManagerHelper* helper);
  QSet<FindActionsManagerHelper*> helpers() const;

signals:
  void canFindChanged(bool enable);
  void canReplaceChanged(bool enable);

  void findNext(const QVariant& value, Qt::MatchFlags flags);
  void findPrevious(const QVariant& value, Qt::MatchFlags flags);

  void replaceNext(const QVariant& find, const QString& replace, Qt::MatchFlags flags);
  void replacePrevious(const QVariant& find, const QString& replace, Qt::MatchFlags flags);

  void replaceAll(const QVariant& find, const QString& replace, Qt::MatchFlags flags);

  void findEdited(const QVariant& value, Qt::MatchFlags flags);

protected slots:
  virtual void focusChanged(QWidget* old, QWidget* now);

protected:
  FindActionsManagerHelper* helperFor(QWidget* widget)const;
  void connectToHelper(FindActionsManagerHelper* helper);
  void disconnectFromHelper(FindActionsManagerHelper* helper);

private slots:
  void searchableWidgetDestroyed();

private:
  void addStandartHelpers();
  void connectToSearchWidget(SearchWidget* searchWidget);
  void disconnectFromSearchWidget(SearchWidget* searchWidget);
};

#endif // FINDACTIONSMANAGER_H
