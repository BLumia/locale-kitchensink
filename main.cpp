#include "mainwindow.h"

#include <QApplication>
#include <QUrl>
#include <QDir>

#include <iostream>
#include <unicode/translit.h>
#include <unicode/unistr.h>

int main(int argc, char *argv[])
{
    icu::UnicodeString japaneseSentence(u"今日はいい天気ですね。");

    UErrorCode status = U_ZERO_ERROR;
    icu::Transliterator* japaneseTransliterator =
    icu::Transliterator::createInstance("Hiragana-Latin", UTRANS_FORWARD, status);

    japaneseTransliterator->transliterate(japaneseSentence);

    std::string romanizedString;
    japaneseSentence.toUTF8String(romanizedString);
    std::cout << romanizedString << std::endl;

    // 释放转写器资源
    delete japaneseTransliterator;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
