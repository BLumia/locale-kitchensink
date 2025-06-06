#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <unicode/locid.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void localeChanged(QStringView localeCode);

    void updateDayMonthName(QLocale &locale, icu::Locale &icuLocale);
    void updateDateLabels(QLocale &locale, icu_77::Locale &icuLocale);
    void updateRegionName(icu::Locale &sysLocale);
    void updateListLabel(icu::Locale &icuLocale);

private slots:
    void on_comboBox_textActivated(const QString &arg1);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
