#include "connectiondialog.h"
#include "ui_connectiondialog.h"

ConnectionDialog::ConnectionDialog(const QString& connectionName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog),  connectionName(connectionName)
{
    ui->setupUi(this);

    QStringList args;

    this->setWindowTitle(this->connectionName);

    ui->connectionOutput->setReadOnly(true);

    QSettings pendulumSettings;

    pendulumSettings.beginGroup(this->connectionName);

    restoreGeometry(pendulumSettings.value("geometry").toByteArray());

    args << "--server" <<  pendulumSettings.value("server").toString() <<
            "--user" <<  pendulumSettings.value("user").toString() <<
            "--password" <<  pendulumSettings.value("password").toString() <<
            "--mailbox" <<  pendulumSettings.value("mailbox").toString() <<
            "--destination" <<  pendulumSettings.value("destination").toString() <<
            "--ignore" << pendulumSettings.value("ignore").toString() <<
            "--poll" << pendulumSettings.value("poll").toString() <<
            "--retry" << pendulumSettings.value("retry").toString();

    if ( pendulumSettings.value("updates").toBool()) {
        args << "--updates";

    }

    if ( pendulumSettings.value("all").toBool()) {
        args <<  "--all";
    }

    pendulumSettings.endGroup();

    connect (&this->pendulum, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
    connect (&this->pendulum, SIGNAL(readyReadStandardError()), this, SLOT(processError()));
    connect (&this->pendulum, SIGNAL(finished(int)), this, SLOT(processFinished(int)));

    pendulum.start("/home/robt/Projects/NetBeansProjects/Pendulum/dist/Debug/GNU-Linux/pendulum", args);

}

ConnectionDialog::~ConnectionDialog()
{
    this->pendulum.close();
    this->pendulum.kill();
    delete ui;
}

void ConnectionDialog::processOutput()
{

    QByteArray byteArray = this->pendulum.readAllStandardOutput();
    QStringList strLines = QString(byteArray).split("\n");

    foreach (QString line, strLines){
        if (!line.isEmpty())  {
            ui->connectionOutput->append(line);
        }
    }

}

void ConnectionDialog::processError()
{

    QByteArray byteArray = this->pendulum. readAllStandardError();
    QStringList strLines = QString(byteArray).split("\n");

    foreach (QString line, strLines){
        if (!line.isEmpty()) {
            if (line.endsWith("\r"))   line.chop(1);
            ui->connectionOutput->append(line);
        }
    }
}

void ConnectionDialog::processFinished(int exitCode)
{

    Q_UNUSED(exitCode);

    this->close();

}

void ConnectionDialog::closeEvent(QCloseEvent *event)
{
    QSettings pendulumSettings;

    pendulumSettings.beginGroup(this->connectionName);
    pendulumSettings.setValue("geometry", saveGeometry());
    pendulumSettings.endGroup();

    QDialog::closeEvent(event);

}
