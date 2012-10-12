#include "shelltabwidget.h"
#include "ui_shelltabwidget.h"
#include "settingswidget.h"
#include "shellwidget.h"
#include "QPushButton"

ShellTabWidget::ShellTabWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShellTabWidget)
{
    ui->setupUi(this);
    connect(ui->tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int)));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));

    QPushButton *pbtmp =new QPushButton(QIcon(":icons/install.png"),"",this);
    pbtmp->setToolTip("Add a new tab");

    ui->tabWidget->setCornerWidget(pbtmp,Qt::BottomLeftCorner);
    connect(pbtmp,SIGNAL(clicked()),this,SLOT(addNewTab()));
    addNewTab();
}

ShellTabWidget::~ShellTabWidget()
{
    delete ui;
}

void ShellTabWidget::closeTab(int tab)
{
    delete ui->tabWidget->widget(tab);
    //ui->tabWidget->removeTab(tab);
    if (ui->tabWidget->count() == 0)
        addNewTab();
    else
    {
        for (int tabpos = 0; tabpos < ui->tabWidget->count(); ++tabpos)
        {
            ((ShellWidget*)ui->tabWidget->widget(tabpos))->setTabPosition(tabpos);
        }
    }

}

void ShellTabWidget::tabAlert(int tab)
{
    if (ui->tabWidget->currentIndex() != tab)
    {
        ui->tabWidget->setTabText(tab,"*Shell*");
    }
    ui->tabWidget->setTabEnabled(tab,((ShellWidget*)ui->tabWidget->widget(tab))->isReadOnly() == false);
}

void ShellTabWidget::tabChanged(int tab)
{

    ui->tabWidget->setTabText(tab,"Shell");
}

void ShellTabWidget::addNewTab()
{
    ShellWidget *tmp = new ShellWidget(this);
    int position  = ui->tabWidget->addTab(tmp,"Shell");
    ui->tabWidget->setCurrentIndex(position);
    tmp->setTabPosition(position);
    connect(tmp,SIGNAL(closed(int)),this,SLOT(closeTab(int)));
    connect(tmp,SIGNAL(alert(int)),this,SLOT(tabAlert(int)));
    tmp->setFocus();
    tmp->setFont(this->font());
    ui->tabWidget->setTabIcon(position,QIcon(":icons/shell.png"));
}


void ShellTabWidget::uiUpdated(QWidget*what)
{
    if (what == this)
    {
        ui->tabWidget->currentWidget()->setFocus();
    }
}
