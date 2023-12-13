#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QMessageBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TABULATION 0
#define BINSEARCH 1

const char FILEPATH[] = "/Users/Shared/Log.txt";


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void extracted(QVector<pid_t> &w, QVector<int> &status, int &i);
    void HandleExit();

    void ShowProcesses();

    int CalculateTime();

    std::string GetProcessNameFromPid(pid_t pid);

    bool OpenLogFile();

    bool WriteLogFile(std::string Message);

    std::string GetDateTime();

private slots:
    void on_CreateButton_clicked();

    void on_CommitButton_clicked();

    void on_ForceUpdateButton_clicked();

    void on_AccessButton_clicked();

private:
    Ui::MainWindow *ui;

    QVector<pid_t> ActiveProcesses;
    QVector<QString> APName;
    QVector<int> APStatus;

    QMap<pid_t, int> APTime;

    QTimer* ExitTimer;

    int dLogFile;

    bool bDenyAccess;
};
