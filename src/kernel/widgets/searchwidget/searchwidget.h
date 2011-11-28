#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "ui_searchwidget.h"

/*!
  \ingroup widgets
  Виджет а-ля виджет для локального поиска в текущем файле QtCreator.
* @class SearchWidget searchwidget.h "src/kernel/widgets/searchwidget/searchwidget.h"
*/
class SearchWidget : public QWidget, private Ui::SearchWidget {
    Q_OBJECT

    Q_PROPERTY (QString findString READ findString WRITE setFindString)
    Q_PROPERTY (QString replaceString READ replaceString WRITE setReplaceString)

    Q_PROPERTY (bool wholeWordsSearch READ isWholeWordsSearch WRITE setWholeWordsSearch)
    Q_PROPERTY (bool regExpSearch READ isRegExpSearch WRITE setRegExpSearch)
    Q_PROPERTY (bool caseSensitiveSearch READ isCaseSensitiveSearch WRITE setCaseSensitiveSearch)
    Q_PROPERTY (bool recursiveSearch READ isRecursive WRITE setRecursive)

public:
    enum Actions {
      WholeWords,
      RegExp,
      CaseSensitive,
      AutoHighlight,
      Recursive,
    };

    //SearchWidget(QWidget *parent = 0);
    SearchWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~SearchWidget();

    QString findString();
    QString replaceString();

    bool isWholeWordsSearch() const;
    void setWholeWordsSearch(bool yes);

    bool isRegExpSearch() const;
    void setRegExpSearch(bool yes);

    bool isCaseSensitiveSearch() const;
    void setCaseSensitiveSearch(bool yes);

    bool isRecursive() const;
    void setRecursive(bool yes);

    QAction* findAction(Actions type);

signals:
    void findNext(const QVariant &value, Qt::MatchFlags flags);
    void findPrevious(const QVariant &find, Qt::MatchFlags flags);

    void replaceNext(const QVariant &find, const QString& replace, Qt::MatchFlags flags);
    void replacePrevious(const QVariant &find, const QString& replace, Qt::MatchFlags flags);
    void replaceAll(const QVariant &find, const QString& replace, Qt::MatchFlags flags);

    void findEdited(const QVariant &value, Qt::MatchFlags flags);
    void replaceEdited(const QVariant &find, const QString& replace, Qt::MatchFlags flags);

    void closeRequested();

public slots:
    void setFindString(const QString& findString);
    void setFindHintString(const QString& hint);
    void setReplaceString(const QString& replaceString);

    void canFindChanged(bool enabled = true);
    void canReplaceChanged(bool enabled = true);

protected slots:
    void updateFindIcons();

    void findNextClicked();
    void findPreviousClicked();
    void replaceNextClicked();
    void replacePreviousClicked();
    void replaceAllClicked();

    void findStringEditedSlot(const QString& string);
    void replaceStringEditedSlot(const QString& string);

    void actionTriggered();

protected:
    void init();

    QVariant value();
    Qt::MatchFlags flags();

  private:
    QHash <Actions, QAction *> findActions;
};

#endif // SEARCHWIDGET_H
