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
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

    void on_listWidget_clicked(const QModelIndex &index);

    void on_listWidget_doubleClicked(const QModelIndex &index);

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
