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

void ConnectionDetailsDialog::on_pushButton_clicked()
{
    // Save

    this->connectionName = ui->lineEdit_6->text();
    this->serverURL = ui->lineEdit->text();
    this->userName = ui->lineEdit_2->text();
    this->password = ui->lineEdit_3->text();
    this->mailBoxList = ui->lineEdit_4->text();
    this->destinationFolder = ui->lineEdit_5->text();
    this->upates = (ui->checkBox->checkState() == Qt::Checked) ? true : false;
    this->allMailBoxes = (ui->checkBox_2->checkState() == Qt::Checked) ? true : false;
    this->pollTime = ui->spinBox->value();
    this->retryCount = ui->spinBox_2->value();
    this->mailBoxIgnoreList = ui->lineEdit_7->text();

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

void ConnectionDetailsDialog::on_pushButton_2_clicked()
{
    // Cancel

    this->close();
}

void ConnectionDetailsDialog::showEvent( QShowEvent* event ) {
    QWidget::showEvent( event );

    if (!this->connectionName.isEmpty()) {
        ui->lineEdit_6->setEnabled(false);
    }

    ui->lineEdit_6->setText(this->connectionName);
    ui->lineEdit->setText(this->serverURL);
    ui->lineEdit_2->setText(this->userName);
    ui->lineEdit_3->setText(this->password);
    ui->lineEdit_4->setText(this->mailBoxList);
    ui->lineEdit_5->setText(this->destinationFolder);
    ui->checkBox->setChecked(this->upates);
    ui->checkBox_2->setChecked(this->allMailBoxes);
    ui->spinBox->setValue(this->pollTime);
    ui->spinBox_2->setValue(this->retryCount);
    ui->lineEdit_7->setText(this->mailBoxIgnoreList);

}
