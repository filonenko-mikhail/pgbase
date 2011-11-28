#ifndef FILEACTIONSMANAGER_H
#define FILEACTIONSMANAGER_H

#include <QtCore/QObject>

class QAction;
class QUndoView;
//class SearchWidget;

class FileActionsManagerPrivate;
class FileActionsManagerHelper;
class FileActionsManager : public QObject//: public AbstractActionsManager
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(FileActionsManager);
protected:
  FileActionsManagerPrivate * const d_ptr;
public:
  FileActionsManager(QWidget* parent);
  virtual ~FileActionsManager();

  void addWidget(QWidget* widget, QAction* before = 0);
  void removeWidget(QWidget* widget);

  void addHelper(FileActionsManagerHelper* helper);
  void removeHelper(FileActionsManagerHelper* helper);
  QSet<FileActionsManagerHelper*> helpers() const;

signals:
  void canNewChanged(bool enable);

  void canOpenChanged(bool enable);
  void canOpenRecentChanged(bool enable);

  void canSaveChanged(bool enable);
  void canSaveAllChanged(bool enable);

  void canCloseChanged(bool enable);

signals:
  void newAction();

  void open();

  void save();
  void saveAll();

  void close();


protected slots:
  virtual void focusChanged(QWidget* old, QWidget* now);

  void blockFocusChanged(bool blocked);

protected:
  FileActionsManagerHelper* helperFor(QWidget* widget)const;
  void connectToHelper(FileActionsManagerHelper* helper);
  void disconnectFromHelper(FileActionsManagerHelper* helper);
  QList<QAction*> actions() const;

private:
  void addStandardHelpers();
};

#endif // FILEACTIONSMANAGER_H
