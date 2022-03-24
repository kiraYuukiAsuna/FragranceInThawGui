#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QDialog>
#include "src/configmanager.h"

namespace Ui {
    class WelcomeWindow;
}

class WelcomeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);
    ~WelcomeWindow();

private:
    Ui::WelcomeWindow *ui;

private:
    ConfigFile mConfig;

    ConfigManager mConfigManager;

public:
    bool checkIfRequirementSatisfied();

};

#endif // WELCOMEWINDOW_H
