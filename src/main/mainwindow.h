#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtGui/QStringListModel>

class QSettings;
class QEvent;
class QCloseEvent;
class QScriptEngine;
class QScriptEngineDebugger;


namespace Ui
{
  class mainWindow;
}


/**
* @brief Класс главного окна приложения
*
* @class MainWindow
*/
class MainWindowPrivate;
class MainWindow : public QMainWindow
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(MainWindow);

protected:
  MainWindowPrivate * const d_ptr;

public:
  explicit MainWindow(QWidget *parent = 0);
  virtual ~MainWindow();

protected slots:
  void tabCloseRequested(int index);

private:
  Ui::mainWindow * ui;
};

#endif // MAINWINDOW_H
