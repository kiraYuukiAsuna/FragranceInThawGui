#ifndef DEVELOPMODEWINDOW_H
#define DEVELOPMODEWINDOW_H

#include <QDialog>
#include "src/configmanager.h"

namespace Ui {
    class DevelopModeWindow;
}

class DevelopModeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DevelopModeWindow(QWidget *parent = nullptr);
    ~DevelopModeWindow();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    ConfigFile mConfig;

    ConfigManager configManager;

private:
    void setUISettings();
    void getUISettings();

private:
    Ui::DevelopModeWindow *ui;
};

#endif // DEVELOPMODEWINDOW_H
