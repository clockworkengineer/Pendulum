#include "pendulummainwindow.h"
#include "ui_pendulummainwindow.h"
#include "connectiondetailsdialog.h"
#include "connectiondialog.h"

PendulumMainWindow::PendulumMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PendulumMainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationName("ClockWorkEngineer");
    QCoreApplication::setApplicationName("Pendulum");

    this->loadConnectionList();

    ui->listWidget->addItems(this->connectionList);

    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);

    QSettings pendulumSettings;
    restoreGeometry(pendulumSettings.value("geometry").toByteArray());
    restoreState(pendulumSettings.value("windowState").toByteArray());

}

PendulumMainWindow::~PendulumMainWindow()
{
    delete ui;
}

void PendulumMainWindow::on_pushButton_clicked()
{
    // New

    ConnectionDetailsDialog connectionDetails("");

    connectionDetails.exec();

    QString connectionName = connectionDetails.getConnectionName();
    if (!connectionName.isEmpty()) {
        this->connectionList.append(connectionName);
        this->populateConnectionList();
    }

}

void PendulumMainWindow::on_pushButton_2_clicked()
{
    // Edit

    QListWidgetItem *connectionToEdit= ui->listWidget->currentItem();

    if(connectionToEdit != nullptr) {

        ConnectionDetailsDialog connectionDetails(connectionToEdit->text());

        connectionDetails.exec();

    }


}

void PendulumMainWindow::on_pushButton_3_clicked()
{
    // Delete

    QListWidgetItem *connectionToDelete = ui->listWidget->currentItem();

    if(connectionToDelete != nullptr) {
        QString connectionName = connectionToDelete->text();
        int connectionIndex = this->connectionList.indexOf(connectionName);
        if (connectionIndex != -1) {
            this->connectionList.removeAt(connectionIndex);
            this->populateConnectionList();
        }
    }


}

void PendulumMainWindow::populateConnectionList()
{

    ui->listWidget->clear();
    ui->listWidget->addItems(this->connectionList);
    this->saveConnectionList();

}

void PendulumMainWindow::saveConnectionList()
{
    //Save

    QSettings pendulumSettings;
    pendulumSettings.beginGroup("ConnectionList");
    pendulumSettings.setValue("connections",this->connectionList);
    pendulumSettings.endGroup();

}

void PendulumMainWindow::closeEvent(QCloseEvent *event)
{
    QSettings pendulumSettings;
    pendulumSettings.setValue("geometry", saveGeometry());
    pendulumSettings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);

}

void PendulumMainWindow::loadConnectionList()
{
    //Load

    QSettings pendulumSettings;
    pendulumSettings.beginGroup("ConnectionList");
    this->connectionList = pendulumSettings.value("connections").toStringList();
    pendulumSettings.endGroup();

}

void PendulumMainWindow::on_pushButton_4_clicked()
{
    // Connect

    QListWidgetItem *connectionToRun= ui->listWidget->currentItem();

    if(connectionToRun != nullptr) {
        ConnectionDialog connection(connectionToRun->text(), this);
        connection.exec();
    }
}


void PendulumMainWindow::on_listWidget_clicked(const QModelIndex &index)
{

    ui->pushButton_2->setEnabled(true);
    ui->pushButton_3->setEnabled(true);
    ui->pushButton_4->setEnabled(true);

}

void PendulumMainWindow::on_listWidget_doubleClicked(const QModelIndex &index)
{
    on_pushButton_4_clicked();

}
