#ifndef PENDULUMMAINWINDOW_H
#define PENDULUMMAINWINDOW_H

#include <QMainWindow>
#include <QtCore>

namespace Ui {
class PendulumMainWindow;
}

class PendulumMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PendulumMainWindow(QWidget *parent = 0);
    ~PendulumMainWindow();

    QString getConnectionName(void) const;
    void setConnectionName(const QString& connectionName);

private slots:
    void on_newConnection_clicked();
    void on_editConnection_clicked();
    void on_deleteConnection_clicked();
    void on_connect_clicked();
    void on_connectionList_clicked(const QModelIndex &index);
    void on_connectionList_doubleClicked(const QModelIndex &index);

private:

    Ui::PendulumMainWindow *ui;

    QStringList connectionList;

    void populateConnectionList(void);
    void loadConnectionList(void);
    void saveConnectionList(void);

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // PENDULUMMAINWINDOW_H
