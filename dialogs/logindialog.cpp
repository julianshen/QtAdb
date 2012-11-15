#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(Accept()));
    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(Decline()));
    ui->lblMessage->setText("Please supply proxy details");
}

void LoginDialog::Accept()
{
    this->hide();
}

void LoginDialog::Decline()
{
    this->hide();
}

LoginDialog::~LoginDialog()
{
    DELETE_IF_NOT_NULL(ui);
}

void LoginDialog::setMessage(const QString proxy, const QString type)
{
    if (type == "")
    {
        ui->lblMessage->setText("Please supply proxy details for "+proxy+":");
    }else
    {
        ui->lblMessage->setText("Please supply proxy details for "+proxy+" ("+type+"):");
    }
}

QString LoginDialog::user() { return ui->txtUsername->text();}
QString LoginDialog::password(){ return ui->txtPassword->text(); }
