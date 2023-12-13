#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include "CreationID.hpp"
#include "prioritywidget.hpp"
#include <csignal>
#include <sched.h>

#define _GNU_SOURCE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->CreateBox->addItems({"Tabulation", "Binary Search"});
    ui->ActionsBox->addItems({"Suspend/Resume", "Priority", "Affinity", "Terminate"});

    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({ "Process Name", "PID", "Status", "Priority", "Time" });

    ExitTimer = new QTimer(this);
    connect(ExitTimer, &QTimer::timeout, this, &MainWindow::HandleExit);

    ExitTimer->setInterval(2000);
    ExitTimer->start();

    bDenyAccess = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_CreateButton_clicked()
{
    auto ProcessToCreate = static_cast<EPathType>(ui->CreateBox->currentIndex());

    pid_t p;
    p = fork();
    if (p == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (p == 0) {
        if (execl(MPathMap[ProcessToCreate].c_str(),
                  MProcessNameMap[ProcessToCreate].c_str(),
                  (char*)NULL) == -1)
        //        if (execl("/usr/bin/open", "open", "-a", "Terminal.app", MPathMap[ProcessToCreate].c_str(),
        //                  (char*)NULL)
        //            == -1)
        {
            perror("execl");
            exit(EXIT_FAILURE);
        }
    } else {
        APTime[p] = -2;
        ActiveProcesses.append(p);
        APStatus.append(EStatus::Running);
        APName.append(MProcessNameMap[ProcessToCreate].c_str());
        std::string LogMessage = "\t" + GetDateTime() + "PID: "
                                 + std::to_string(static_cast<int>(ActiveProcesses.last()))
                                 + "\nProcess Name: " + APName.last().toStdString() + "\n\n";
        WriteLogFile(LogMessage);
    }
}

void MainWindow::HandleExit()
{
    if (!ActiveProcesses.isEmpty()) {
        QVector<pid_t> w(ActiveProcesses.size());
        QVector<int> status(ActiveProcesses.size());
        bool bRunning = false;

        for (int i = ActiveProcesses.size() - 1; i >= 0; --i) {
            if ((w[i] = waitpid(ActiveProcesses[i], &status[i], WNOHANG)) == 0)
                bRunning = true;
            if (w[i] == -1)
                perror("waitpid");
            if (WIFEXITED(status[i]) && !bRunning) {
                ActiveProcesses.removeAt(i);
                APStatus.removeAt(i);
                APName.removeAt(i);
                ui->tableWidget->removeRow(i);
            }
            if (WIFSTOPPED(status[i])) {
                APStatus[i] = EStatus::Stopped;
            }
            if (WIFCONTINUED(status[i])) {
                APStatus[i] = EStatus::Running;
            }
            bRunning = false;
        }
    }
    ShowProcesses();
}

void MainWindow::ShowProcesses()
{
    if (!ActiveProcesses.isEmpty()) {
        ui->tableWidget->setRowCount(ActiveProcesses.size());

        for (int i = 0; i < ActiveProcesses.size(); ++i) {
            QVector<QTableWidgetItem *> table_item;
            table_item.push_back(new QTableWidgetItem(APName[i]));
            table_item.push_back(new QTableWidgetItem(QString::number(ActiveProcesses[i])));

            if (APStatus[i] == EStatus::Running) {
                table_item.push_back(new QTableWidgetItem("R"));
            }
            else{
                table_item.push_back(new QTableWidgetItem("S"));
            }

            table_item.push_back(new QTableWidgetItem(
                QString::number(getpriority(PRIO_PROCESS, ActiveProcesses[i]))));

            if (APStatus[i] == EStatus::Running) {
                APTime[ActiveProcesses[i]] += 2;
            }
            table_item.push_back(new QTableWidgetItem(QString::number(APTime[ActiveProcesses[i]])));

            for (int j = 0; j < 5; ++j)
            {
                ui->tableWidget->setItem(i, j, table_item[j]);
            }
        }
    }
}

std::string MainWindow::GetProcessNameFromPid(pid_t pid)
{
    std::string processName;

    // Construct the path to the cmdline file for the process
    std::string cmdlinePath = "/proc/" + std::to_string(pid) + "/cmdline";

    // Open the cmdline file for reading
    std::ifstream cmdlineFile(cmdlinePath);
    if (cmdlineFile.is_open()) {
        // Read the contents of the cmdline file, which contains the process name
        getline(cmdlineFile, processName);
        cmdlineFile.close();
    } else {
        QMessageBox::information(this, "Error!", "Error opening /proc/cmdline");
    }

    return processName;
}

void MainWindow::on_CommitButton_clicked()
{
    bool ok;
    pid_t CurrentPID = ui->PIDLineEdit->text().toInt(&ok);
    int Index = -1;
    for (int i = 0; i < ActiveProcesses.size(); ++i) {
        if (CurrentPID == ActiveProcesses[i] && ok)
            Index = i;
    }

    if (Index == -1) {
        QMessageBox::information(this, "Error!", "Given PID does not exist!");
        return;
    }

    PriorityWidget setPriorityWidget(this);

    int Action = ui->ActionsBox->currentIndex();
    switch (Action) {
    case EAction::Suspend:
        if (APStatus[Index] == EStatus::Running) {
            kill(CurrentPID, SIGSTOP);
            APStatus[Index] = EStatus::Stopped;
        } else {
            kill(CurrentPID, SIGCONT);
            APStatus[Index] = EStatus::Running;
        }
        break;

    case EAction::Priority:
        if (setPriorityWidget.exec() == QDialog::Accepted) {
            if (setpriority(PRIO_PROCESS, CurrentPID, setPriorityWidget.GetNewPriority()) == -1)
                perror("setpriority");
        }
        break;

    case EAction::Affinity:
        //        cpu_set_t cpuset;
        //        CPU_ZERO(&cpuset);
        //        CPU_SET(0, &cpuset);
        //        CPU_SET(1, &cpuset);
        //        CPU_SET(2, &cpuset);
        //
        //        if (sched_setaffinity(CurrentPID, sizeof(cpuset), &cpuset) == -1) {
        //            perror("sched_setaffinity");
        //            return 1;
        //        }
        break;

    case EAction::Terminate:
        kill(CurrentPID, SIGKILL);
        break;

    default:
        break;
    }
}

void MainWindow::on_ForceUpdateButton_clicked()
{
    ShowProcesses();
}

bool MainWindow::OpenLogFile()
{
    dLogFile = open(FILEPATH, O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR);
    if (dLogFile == -1) {
        QMessageBox::information(this, "Error", "Failed to open log file!");
        return false;
    }
    return true;
}

bool MainWindow::WriteLogFile(std::string Message)
{
    if (OpenLogFile()) {
        auto bytesWritten = write(dLogFile, Message.c_str(), strlen(Message.c_str()));
        if (bytesWritten == -1) {
            QMessageBox::information(this, "Error!", "Failed to write to log file!");
            ::close(dLogFile);
            return false;
        }
        ::close(dLogFile);
        return true;
    } else {
        return false;
    }
}

std::string MainWindow::GetDateTime()
{
    std::time_t currentTime = std::time(nullptr);

    return std::ctime(&currentTime);
}

void MainWindow::on_AccessButton_clicked()
{
    if (bDenyAccess) {
        if (chmod(FILEPATH, 770) == -1) {
            QMessageBox::information(this, "Error", " Failed to set access mask!");
            return;
        }
        ui->AccessButton->setText("Allow Access");
    } else {
        if (chmod(FILEPATH, 777) == -1) {
            QMessageBox::information(this, "Error", " Failed to set access mask!");
            return;
        }
        ui->AccessButton->setText("Deny Access");
    }
    bDenyAccess = !bDenyAccess;
}

