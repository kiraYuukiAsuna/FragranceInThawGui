#include "src/mainwindow.h"
#include "src/welcomewindow.h"
#include <QApplication>
#include <QString>
#include <QFontDatabase>
#include <QFont>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // QString executableFilePath = QCoreApplication::applicationDirPath();

    QString fontPath = QString("%1/Resource/Fonts/SourceHanSansCN-Regular.ttf").arg(QCoreApplication::applicationDirPath());
    int loadedFontID = QFontDatabase::addApplicationFont(fontPath);
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if (!loadedFontFamilies.empty())
    {
        QString sansCNFamily = loadedFontFamilies.at(0);
        QFont defaultFont = a.font();
        defaultFont.setFamily(sansCNFamily);
        defaultFont.setPixelSize(16);
        a.setFont(defaultFont);
    }

    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}
