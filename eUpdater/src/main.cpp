#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtGui/QPalette>
#include <QtWidgets/QApplication>
#include "updater.h"

static void setDarkTheme() {
    QApplication::setStyle("Fusion");

    QPalette dark_palette;
    QColor baseColor(31, 31, 31);
    QColor textColor(Qt::white);
    QColor highlightColor(0, 136, 57);
    QColor disabledTextColor(Qt::darkGray);

    dark_palette.setColor(QPalette::Window, baseColor);
    dark_palette.setColor(QPalette::WindowText, textColor);
    dark_palette.setColor(QPalette::Base, baseColor.darker(160));
    dark_palette.setColor(QPalette::AlternateBase, baseColor);
    dark_palette.setColor(QPalette::ToolTipBase, baseColor.darker(120));
    dark_palette.setColor(QPalette::ToolTipText, textColor);
    dark_palette.setColor(QPalette::Text, textColor);
    dark_palette.setColor(QPalette::Button, baseColor);
    dark_palette.setColor(QPalette::ButtonText, textColor);
    dark_palette.setColor(QPalette::BrightText, Qt::red);
    dark_palette.setColor(QPalette::Link, highlightColor);
    dark_palette.setColor(QPalette::Highlight, highlightColor);
    dark_palette.setColor(QPalette::HighlightedText, Qt::white);
    dark_palette.setColor(QPalette::Active, QPalette::Button, baseColor);
    dark_palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                          disabledTextColor);
    dark_palette.setColor(QPalette::Disabled, QPalette::WindowText,
                          disabledTextColor);
    dark_palette.setColor(QPalette::Disabled, QPalette::Text,
                          disabledTextColor);
    dark_palette.setColor(QPalette::Disabled, QPalette::Light, baseColor);
    QApplication::setPalette(dark_palette);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("eUpdater");
    app.setApplicationVersion("1.0.0");

    // Apply dark theme
    setDarkTheme();

    QCommandLineParser parser;
    parser.setApplicationDescription("eUpdater - Qt-based updater utility");
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption manifestOpt({"m", "manifest-url"},
                                         "Remote manifest JSON URL.", "url");
    const QCommandLineOption apiOpt({"a", "release-api-url"},
                                    "GitHub releases API URL.", "url");
    const QCommandLineOption tplOpt({"t", "installer-template"},
                                    "Installer URL template with %1 = version.",
                                    "template");
    const QCommandLineOption pkgOpt({"p", "package-name"},
                                    "Package name for asset matching.", "name");

    parser.addOption(manifestOpt);
    parser.addOption(apiOpt);
    parser.addOption(tplOpt);
    parser.addOption(pkgOpt);

    parser.process(app);

    const QString manifestUrl = parser.value(manifestOpt);
    const QString apiUrl = parser.value(apiOpt);
    const QString installerTpl = parser.value(tplOpt);
    const QString packageName = parser.value(pkgOpt);

    eUpdater updater;
    updater.checkForUpdates(manifestUrl, apiUrl, installerTpl, packageName);

    return 0;
}
