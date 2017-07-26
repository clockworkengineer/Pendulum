#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QtCore>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(const QString& connectionName, QWidget *parent = 0);
    ~ConnectionDialog();


private:
    Ui::ConnectionDialog *ui;

    QString connectionName;

    QProcess pendulum;

public slots:
    void processOutput();
    void processError();
    void processFinished(int exitCode);

protected:
    void closeEvent(QCloseEvent *);

};

#endif // CONNECTIONDIALOG_H
