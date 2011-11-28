#ifndef CUSTOMACTIONSMANAGER_H
#define CUSTOMACTIONSMANAGER_H

#include <QtCore/QObject>

class QAction;

class CustomActionsManagerPrivate;
class CustomActionsManagerHelper;
class CustomActionsManager : public QObject
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(CustomActionsManager);
protected:
  CustomActionsManagerPrivate * const d_ptr;
public:
  CustomActionsManager(QWidget* parent);
  virtual ~CustomActionsManager();

  void addWidget(QWidget* widget, QAction* before = 0);
  void removeWidget(QWidget* widget);

  void addHelper(CustomActionsManagerHelper* helper);
  void removeHelper(CustomActionsManagerHelper* helper);
  QSet<CustomActionsManagerHelper*> helpers() const;

protected slots:
  virtual void focusChanged(QWidget* old, QWidget* now);

  void blockFocusChanged(bool blocked);

protected:
  CustomActionsManagerHelper* helperFor(QWidget* widget)const;
  void connectToHelper(CustomActionsManagerHelper* helper);
  void disconnectFromHelper(CustomActionsManagerHelper* helper);
  QList<QAction*> actions() const;

private slots:

private:
  void addStandardHelpers();
};

#endif // CUSTOMACTIONSMANAGER_H
