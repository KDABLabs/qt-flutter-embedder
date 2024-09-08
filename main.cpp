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

using namespace KDAB;

int main(int argc, char **argv)
{
    qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity); // TODO
    app.setAttribute(Qt::AA_ShareOpenGLContexts);

    Embedder embedder(/*multiWindowMode=*/app.arguments().contains("-m"));

    const QString projectPath = app.arguments()[1];
    const auto icuPath = std::string(FLUTTER_ICUDTL_DIR) + std::string("/icudtl.dat");

    if (!embedder.initFlutter(argc, argv, projectPath.toStdString(), icuPath)) {
        return -1;
    }

    return app.exec();
}
