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

    ui->connectionList->addItems(this->connectionList);

    ui->editConnection->setEnabled(false);
    ui->deleteConnection->setEnabled(false);
    ui->connect->setEnabled(false);

    QSettings pendulumSettings;
    restoreGeometry(pendulumSettings.value("geometry").toByteArray());
    restoreState(pendulumSettings.value("windowState").toByteArray());

}

PendulumMainWindow::~PendulumMainWindow()
{
    delete ui;
}

void PendulumMainWindow::on_newConnection_clicked()
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

void PendulumMainWindow::on_editConnection_clicked()
{
    // Edit

    QListWidgetItem *connectionToEdit= ui->connectionList->currentItem();

    if(connectionToEdit != nullptr) {

        ConnectionDetailsDialog connectionDetails(connectionToEdit->text());

        connectionDetails.exec();

    }


}

void PendulumMainWindow::on_deleteConnection_clicked()
{
    // Delete

    QListWidgetItem *connectionToDelete = ui->connectionList->currentItem();

    if(connectionToDelete != nullptr) {
        QString connectionName = connectionToDelete->text();
        int connectionIndex = this->connectionList.indexOf(connectionName);
        if (connectionIndex != -1) {
            this->connectionList.removeAt(connectionIndex);
            this->populateConnectionList();
        }
        QSettings pendulumSettings;
        pendulumSettings.beginGroup(connectionName);
        pendulumSettings.remove("");
        pendulumSettings.endGroup();

    }


}

void PendulumMainWindow::populateConnectionList()
{

    ui->connectionList->clear();
    ui->connectionList->addItems(this->connectionList);
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

void PendulumMainWindow::on_connect_clicked()
{
    // Connect

    QListWidgetItem *connectionToRun= ui->connectionList->currentItem();

    if(connectionToRun != nullptr) {
        ConnectionDialog connection(connectionToRun->text(), this);
        connection.exec();
    }
}


void PendulumMainWindow::on_connectionList_clicked(const QModelIndex &index)
{

    Q_UNUSED(index);

    ui->editConnection->setEnabled(true);
    ui->deleteConnection->setEnabled(true);
    ui->connect->setEnabled(true);

}

void PendulumMainWindow::on_connectionList_doubleClicked(const QModelIndex &index)
{

    Q_UNUSED(index);

    on_connect_clicked();

}
