#ifndef CONNECTIONDETAILSDIALOG_H
#define CONNECTIONDETAILSDIALOG_H

#include <QDialog>
#include <QtCore>

namespace Ui {
class ConnectionDetailsDialog;
}

class ConnectionDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDetailsDialog(const QString& connectionName, QWidget *parent = 0);
    ~ConnectionDetailsDialog();

    QString getConnectionName(void) const;
    void setConnectionName(const QString& connectionName);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:

    Ui::ConnectionDetailsDialog *ui;

    QString connectionName;
    QString serverURL;
    QString userName;
    QString password;
    QString mailBoxList;
    QString destinationFolder;
    qint16 pollTime=1;
    qint16 retryCount=5;
    bool upates=false;
    bool allMailBoxes=false;
    QString mailBoxIgnoreList;

protected:
    void showEvent( QShowEvent* event );


};

#endif // CONNECTIONDETAILSDIALOG_H
