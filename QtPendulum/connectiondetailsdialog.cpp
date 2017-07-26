#include "connectiondetailsdialog.h"
#include "ui_connectiondetailsdialog.h"

ConnectionDetailsDialog::ConnectionDetailsDialog(const QString& connectionName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDetailsDialog), connectionName(connectionName)
{
    ui->setupUi(this);

    if (!connectionName.isEmpty()) {
        QSettings pendulumSettings;
        pendulumSettings.beginGroup(connectionName);
        this->serverURL = pendulumSettings.value("server").toString();
        this->userName = pendulumSettings.value("user").toString();
        this->password = pendulumSettings.value("password").toString();
        this->mailBoxList = pendulumSettings.value("mailbox").toString();
        this->destinationFolder = pendulumSettings.value("destination").toString();
        this->upates = pendulumSettings.value("updates").toBool();
        this->allMailBoxes = pendulumSettings.value("all").toBool();
        this->pollTime = pendulumSettings.value("poll").toInt();
        this->retryCount = pendulumSettings.value("retry").toInt();
        this->mailBoxIgnoreList = pendulumSettings.value("ignore").toString();
        pendulumSettings.endGroup();
    }


}

ConnectionDetailsDialog::~ConnectionDetailsDialog()
{
    delete ui;
}

QString ConnectionDetailsDialog::getConnectionName() const
{
    return(this->connectionName);
}

void ConnectionDetailsDialog::setConnectionName(const QString &connectionName)
{
    this->connectionName = connectionName;
}

void ConnectionDetailsDialog::on_saveButton_clicked()
{
    // Save

    this->connectionName = ui->connectionName->text();
    this->serverURL = ui->serverURL->text();
    this->userName = ui->userName->text();
    this->password = ui->password->text();
    this->mailBoxList = ui->mailBoxList->text();
    this->destinationFolder = ui->destinationFolder->text();
    this->upates = (ui->updates->checkState() == Qt::Checked) ? true : false;
    this->allMailBoxes = (ui->all->checkState() == Qt::Checked) ? true : false;
    this->pollTime = ui->pollTime->value();
    this->retryCount = ui->retryCount->value();
    this->mailBoxIgnoreList = ui->ignoreList->text();

    QSettings pendulumSettings;

    pendulumSettings.beginGroup(connectionName);
    pendulumSettings.setValue("server",this->serverURL);
    pendulumSettings.setValue("user",this->userName);
    pendulumSettings.setValue("password", this->password);
    pendulumSettings.setValue("mailbox", this->mailBoxList);
    pendulumSettings.setValue("destination", this->destinationFolder);
    pendulumSettings.setValue("all", this->allMailBoxes);
    pendulumSettings.setValue("updates", this->upates);
    pendulumSettings.setValue("poll",this->pollTime);
    pendulumSettings.setValue("retry", this->retryCount);
    pendulumSettings.setValue("ignore", this->mailBoxIgnoreList);
    pendulumSettings.endGroup();

    this->close();
}

void ConnectionDetailsDialog::on_cancelButton_clicked()
{
    // Cancel

    this->close();

}

void ConnectionDetailsDialog::showEvent( QShowEvent* event ) {
    QWidget::showEvent( event );

    if (!this->connectionName.isEmpty()) {
        ui->connectionName->setEnabled(false);
    }

    ui->connectionName->setText(this->connectionName);
    ui->serverURL->setText(this->serverURL);
    ui->userName->setText(this->userName);
    ui->password->setText(this->password);
    ui->mailBoxList->setText(this->mailBoxList);
    ui->destinationFolder->setText(this->destinationFolder);
    ui->updates->setChecked(this->upates);
    ui->all->setChecked(this->allMailBoxes);
    ui->pollTime->setValue(this->pollTime);
    ui->retryCount->setValue(this->retryCount);
    ui->ignoreList->setText(this->mailBoxIgnoreList);

}
