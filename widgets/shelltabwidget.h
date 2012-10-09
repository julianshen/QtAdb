#ifndef SHELLTABWIDGET_H
#define SHELLTABWIDGET_H

#include <QWidget>

namespace Ui {
class ShellTabWidget;
}

class ShellTabWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ShellTabWidget(QWidget *parent = 0);
    ~ShellTabWidget();
    
private slots:
    void closeTab(int);
    void tabChanged(int);
    void tabAlert(int);
    void uiUpdated(QWidget*);
    void addNewTab();
private:
    Ui::ShellTabWidget *ui;

};

#endif // SHELLTABWIDGET_H
