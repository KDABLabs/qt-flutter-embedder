/*
  This file is part of qt-flutter-embedder.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "src/Embedder.h"
#include "src/FlutterWindow.h"

#include <string>

#include <QApplication>
#include <QCommandLineParser>

using namespace KDAB;

int main(int argc, char **argv)
{
    qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");

    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity); // TODO
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Embedder example");
    QCommandLineOption enableMultiWindowOpt = { { "m", "multiwindow" }, "Enable multi-window mode" };
    parser.addOption(enableMultiWindowOpt);
    parser.addPositionalArgument("project", "the root of the flutter project directory");
    parser.addHelpOption();
    parser.process(app);

    if (parser.positionalArguments().isEmpty()) {
        qInfo().noquote() << parser.helpText() << "\n";
        return 1;
    }

    const QString projectPath = parser.positionalArguments().constFirst();
    Embedder embedder(parser.isSet(enableMultiWindowOpt));

    const auto icuPath = std::string(FLUTTER_ICUDTL_DIR) + std::string("/icudtl.dat");

    if (!embedder.initFlutter(argc, argv, projectPath.toStdString(), icuPath)) {
        return -1;
    }

    return app.exec();
}
