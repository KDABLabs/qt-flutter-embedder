/*
  This file is part of qt-flutter-embedder.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "src/Embedder.h"
#include "src/FlutterWindow.h"

#include <iostream>
#include <string>

#include <QtWidgets>

#include <EGL/egl.h>
#include <GLES3/gl3.h>


using namespace KDAB;

void print_gl_info()
{
    int extensionCnt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCnt);

    const char *extensions = ( const char * )glGetString(GL_EXTENSIONS);
    Q_ASSERT(eglGetCurrentDisplay() != EGL_NO_DISPLAY);
    const GLubyte *renderer = glGetString(GL_RENDERER); // Get renderer string
    const GLubyte *vendor = glGetString(GL_VENDOR); // Get vendor string
    const GLubyte *version = glGetString(GL_VERSION); // Version as a string
    const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); // GLSL version string

    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "Vendor: " << vendor << std::endl;
    std::cout << "OpenGL Version: " << version << std::endl;
    std::cout << "GLSL Version: " << glslVersion << std::endl;

    for (int i = 0; i < extensionCnt; ++i) {
        const char *ext = ( const char * )glGetStringi(GL_EXTENSIONS, i);
        qDebug() << ext;
    }
}

int main(int argc, char **argv)
{
    qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity); // TODO

    Embedder embedder;

    const QString projectPath = app.arguments()[1];
    const auto icuPath = std::string(FLUTTER_ICUDTL_DIR) + std::string("/icudtl.dat");

    if (!embedder.initFlutter(argc, argv, projectPath.toStdString(), icuPath)) {
        return -1;
    }

    // embedder.addWindow();

    // window.createContext();
    // QTimer::singleShot(1000, [] {
    //     s_window->m_context->makeCurrent(s_window);
    //     Q_ASSERT(QOpenGLContext::currentContext()->isOpenGLES());
    //     qDebug() << s_window->m_context->format();
    // });

    return app.exec();
}
