#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDateTime>
#include <iostream>

#include <unicode/udat.h>
#include <unicode/dtitvfmt.h> // Though not directly used for day names, good to have for general date/time
#include <unicode/locid.h>      // For Locale
#include <unicode/ustream.h>    // For easy output of UnicodeString
#include <unicode/dtfmtsym.h>   // For DateFormatSymbols
#include <unicode/reldatefmt.h> // For RelativeDateTimeFormatter
#include <unicode/smpdtfmt.h> // SimpleDateFormat
#include <unicode/locdspnm.h> // LocaleDisplayNames
#include <unicode/listformatter.h> // For icu::ListFormatter

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    localeChanged(u"zh_CN");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::localeChanged(QStringView localeCode)
{
    QLocale locale(localeCode);
    icu::Locale icuLocale(localeCode.toLocal8Bit().constData());

    updateDayMonthName(locale, icuLocale);
    updateDateLabels(locale, icuLocale);
    updateRegionName(icuLocale);
    updateListLabel(icuLocale);
}

icu::UnicodeString fromQString(const QString& qstr) {
    return icu::UnicodeString(qstr.utf16(), qstr.length());
}

QString toQString(const icu::UnicodeString& icuString) {
    // Get a pointer to the internal UTF-16 buffer of the icu::UnicodeString.
    // The buffer is not necessarily null-terminated, so we also need the length.
    const UChar* ucharData = icuString.getBuffer();
    int32_t length = icuString.length();

    // QString has a constructor that takes a const QChar* and a length.
    // UChar is typically a 16-bit unsigned integer, which is compatible with QChar.
    // Static_cast is used here for explicit type conversion, though often
    // UChar and QChar are typedefs to the same underlying type (e.g., unsigned short).
    return QString(reinterpret_cast<const QChar*>(ucharData), length);
}

void MainWindow::updateDayMonthName(QLocale & locale, icu::Locale & icuLocale)
{
    // QLocale
    QStringList names;
    for (int i = 1; i <= 7; i++) {
        names << QString("%1\t%2\t%3").arg(locale.standaloneDayName(i, QLocale::NarrowFormat), locale.standaloneDayName(i, QLocale::ShortFormat), locale.standaloneDayName(i, QLocale::LongFormat));
    }
    for (int i = 1; i <= 12; i++) {
        names << QString("%1\t%2\t%3").arg(locale.standaloneMonthName(i, QLocale::NarrowFormat), locale.standaloneMonthName(i, QLocale::ShortFormat), locale.standaloneMonthName(i, QLocale::LongFormat));
    }
    ui->qlocaleDWNames->setText(names.join('\n'));

    // ICU
    using namespace icu;
    UErrorCode status = U_ZERO_ERROR;
    DateFormatSymbols symbols(icuLocale, status);
    if (U_FAILURE(status)) {
        std::cerr << "Error creating DateFormatSymbols for English: " << u_errorName(status) << std::endl;
        return;
    }
    int32_t count = 0;
    const UnicodeString* weekdays = symbols.getWeekdays(count);
    const UnicodeString* sweekdays = symbols.getShortWeekdays(count);
    const UnicodeString* nweekdays = symbols.getWeekdays(count, DateFormatSymbols::DtContextType::STANDALONE, DateFormatSymbols::DtWidthType::NARROW);
    QStringList icuNames;
    for (int i = UCAL_SUNDAY; i <= UCAL_SATURDAY; ++i) {
        icuNames << QString("%1\t%2\t%3").arg(toQString(nweekdays[i]), toQString(sweekdays[i]), toQString(weekdays[i]));
    }
    const UnicodeString* months = symbols.getMonths(count);
    const UnicodeString* smonths = symbols.getShortMonths(count);
    const UnicodeString* nmonths = symbols.getMonths(count, DateFormatSymbols::DtContextType::STANDALONE, DateFormatSymbols::DtWidthType::NARROW);
    for (int i = UCAL_JANUARY; i <= UCAL_DECEMBER; ++i) {
        icuNames << QString("%1\t%2\t%3").arg(toQString(nmonths[i]), toQString(smonths[i]), toQString(months[i]));
    }
    ui->icuDWNames->setText(icuNames.join('\n'));
}

void MainWindow::updateDateLabels(QLocale & locale, icu::Locale &icuLocale)
{
    using namespace icu;
    UErrorCode status = U_ZERO_ERROR;
    RelativeDateTimeFormatter formatter1(
        icuLocale,
        nullptr, // Use default NumberFormat
        UDAT_STYLE_LONG,
        UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE,
        status
    );
    if (U_FAILURE(status)) {
        std::cerr << "Error creating RelativeDateTimeFormatter 1: " << u_errorName(status) << std::endl;
        return;
    }
    SimpleDateFormat* fmt = new SimpleDateFormat("h:mm", icuLocale, status);
    UDate now = icu::Calendar::getNow();
    UnicodeString result1, result2, result3, result4, result5, result6;
    UnicodeString sampleTime, combinedString;
    fmt->format(now, sampleTime, status);
    QStringList results;
    formatter1.format(UDAT_DIRECTION_PLAIN, UDAT_ABSOLUTE_NOW, result1, status);
    formatter1.format(5, UDAT_DIRECTION_LAST, UDAT_RELATIVE_MINUTES, result2, status);
    formatter1.format(1, UDAT_DIRECTION_LAST, UDAT_RELATIVE_HOURS, result3, status);
    formatter1.format(2, UDAT_DIRECTION_LAST, UDAT_RELATIVE_HOURS, result4, status);
    formatter1.format(UDAT_DIRECTION_LAST, UDAT_ABSOLUTE_DAY, result5, status);
    formatter1.format(UDAT_DIRECTION_LAST, UDAT_ABSOLUTE_SUNDAY, result6, status);
    formatter1.combineDateAndTime(result5, sampleTime, combinedString, status);
    if (U_FAILURE(status)) {
        std::cerr << "Error formatting 2: " << u_errorName(status) << std::endl;
    } else {
        results << toQString(result1) << toQString(result2) << toQString(result3) << toQString(result4) << toQString(combinedString) << toQString(result6);

        results << QDateTime::currentDateTime().toString(locale.dateTimeFormat(QLocale::NarrowFormat)) + locale.dateTimeFormat(QLocale::NarrowFormat);
        results << QDateTime::currentDateTime().toString(locale.dateFormat(QLocale::NarrowFormat)) + locale.dateFormat(QLocale::NarrowFormat);
        results << QDateTime::currentDateTime().toString(locale.dateFormat(QLocale::ShortFormat)) + locale.dateFormat(QLocale::ShortFormat);
        results << QDateTime::currentDateTime().toString(locale.dateFormat(QLocale::LongFormat)) + locale.dateFormat(QLocale::LongFormat);
        ui->relLabel->setText(results.join('\n'));
    }
}

void MainWindow::updateRegionName(icu_77::Locale &sysLocale)
{
    using namespace icu;
    UErrorCode status = U_ZERO_ERROR;
    std::vector<std::string> localeCodes = {"zh_CN", "en_US", "ja", "ko", "zh_HK", "zh_TW", "kk_KZ", "es_ES", "kab", "krl", "ar"};
    icu::LocaleDisplayNames * displayNames = icu::LocaleDisplayNames::createInstance(sysLocale, ULDN_DIALECT_NAMES);
    QStringList results;
    for (const std::string& localeCode : localeCodes) {
        icu::UnicodeString dialectName;
        icu::Locale currentLocale(localeCode.c_str());

        icu::UnicodeString originalLanguageName;
        currentLocale.getDisplayName(currentLocale, originalLanguageName);

        displayNames->localeDisplayName(currentLocale, dialectName);
        results << QString("%1:\t%2 - %3").arg(localeCode, toQString(originalLanguageName), toQString(dialectName));
    }
    ui->regLabel->setText(results.join('\n'));
}

void MainWindow::updateListLabel(icu_77::Locale &icuLocale)
{
    using namespace icu;
    UErrorCode status = U_ZERO_ERROR;

    icu::LocaleDisplayNames * displayNames = icu::LocaleDisplayNames::createInstance(icuLocale, ULDN_DIALECT_NAMES);
    std::vector<std::string> localeCodes = {"zh", "en", "ja", "ko"};
    std::vector<UnicodeString> languageNames;
    for (const std::string & localeCode : std::as_const(localeCodes)) {
        UnicodeString name;
        displayNames->languageDisplayName(localeCode.c_str(), name);
        languageNames.push_back(name);
    }

    icu::UnicodeString formattedListStr1, formattedListStr2;

    icu::ListFormatter* lf_and = icu::ListFormatter::createInstance(
        icuLocale,
        ULISTFMT_TYPE_AND,
        ULISTFMT_WIDTH_WIDE,
        status
        );
    if (U_FAILURE(status)) {
        std::cerr << "Error creating ListFormatter (en, and): " << u_errorName(status) << std::endl;
        return;
    }

    icu::ListFormatter* lf_or = icu::ListFormatter::createInstance(
        icuLocale,
        ULISTFMT_TYPE_OR,
        ULISTFMT_WIDTH_WIDE,
        status
        );
    if (U_FAILURE(status)) {
        std::cerr << "Error creating ListFormatter (en, and): " << u_errorName(status) << std::endl;
        return;
    }

    lf_and->format(languageNames.data(), languageNames.size(), formattedListStr1, status);
    lf_or->format(languageNames.data(), languageNames.size(), formattedListStr2, status);
    QStringList result {
        toQString(formattedListStr1), toQString(formattedListStr2),
    };
    ui->cmbListLabel->setText(result.join('\n'));
}

void MainWindow::on_comboBox_textActivated(const QString &arg1)
{
    localeChanged(arg1);
}

