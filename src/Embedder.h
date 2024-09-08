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
    explicit Embedder(bool multiWindowMode = false);
    bool initFlutter(int argc, char **argv, const std::string &project_path, const std::string &icudtl_path);

    _FlutterEngine *engine() const;

    /// Returns the 1st view
    FlutterWindow &mainWindow() const;

    /// Returns the window having said id, nullptr otherwise
    FlutterWindow *windowForId(FlutterViewId) const;

    /// Creates the GL context if it wasn't created yet
    void maybeCreateGLContext();

    /// Returns the OpenGL context
    QOpenGLContext *glContext() const;

    /// Adds a new window
    FlutterWindow *addWindow();

    /// Prints OpenGL format, for debug purposes
    void dumpGLInfo();

    /// Returns whether this embedder supports multi-window
    bool isMultiWindowMode() const;

    /// Our desired surface format
    static QSurfaceFormat surfaceFormat();

private:
    bool runFlutter(const FlutterRendererConfig &config, const FlutterProjectArgs &args);
    QOpenGLContext *m_glContext = nullptr;
    _FlutterEngine *m_flutterEngine = nullptr;
    FlutterCompositor m_flutterCompositor;
    QVector<FlutterWindow *> m_windows;

    const bool m_multiWindowMode = false;
};
}
