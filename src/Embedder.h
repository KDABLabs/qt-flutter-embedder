/*
  This file is part of qt-flutter-embedder.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QVector>

#include <flutter_embedder.h>

class QOpenGLContext;
class QSurfaceFormat;

namespace KDAB {
class FlutterWindow;
class Embedder
{
public:
    Embedder();
    bool initFlutter(int argc, char **argv, const std::string &project_path, const std::string &icudtl_path);

    _FlutterEngine *engine() const;

    /// Returns the 1st view
    FlutterWindow &mainWindow() const;

    /// Creates the GL context if it wasn't created yet
    void maybeCreateGLContext();

    QOpenGLContext *glContext() const;

    /// Adds a new window
    FlutterWindow *addWindow();

    /// Our desired surface format
    static QSurfaceFormat surfaceFormat();

private:
    bool runFlutter(const FlutterRendererConfig &config, const FlutterProjectArgs &args);
    QOpenGLContext *m_glContext = nullptr;
    _FlutterEngine *m_flutterEngine = nullptr;
    QVector<FlutterWindow *> m_windows;
};
}
