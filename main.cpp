/*
  This file is part of qt-flutter-embedder.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "src/Embedder.h"
#include "src/FlutterWindow.h"

#include "flutter/encodable_value.h"

#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QCommandLineParser>

#include <string>

using namespace KDAB;

/// Before we have QApplication we need to decide if we want GLES or not
/// as the shared context is created when QGuiApplication is created
static Embedder::Features defaultFeatures(int argc, char **argv)
{
    Embedder::Features features = {};

    bool gl = false;
    bool gles = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-gl") == 0)
            gl = true;
        if (strcmp(argv[i], "-gles") == 0)
            gles = true;
    }

    if (gl && gles) {
        qFatal("Don't specify both -gl and -gles");
    } else if (gles) {
        features |= Embedder::Feature::GLES;
    } else if (gl) {
        features |= Embedder::Feature::GL;
    }

    return features;
}

#ifdef DEVELOPER_BUILD
// for testing
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
void registerDummyMethodChannel(Embedder &embedder)
{
    flutter::MethodChannel<> channel(
        embedder.binaryMessenger(), "qtembedder.kdab.com/testPlatformChannel",
        &flutter::StandardMethodCodec::GetInstance());
    channel.SetMethodCallHandler(
        [](const flutter::MethodCall<> &call,
           std::unique_ptr<flutter::MethodResult<>> result) {
            qWarning() << "method call handler!";
            result->Success(flutter::EncodableValue(42));
            // result->Error("UNAVAILABLE", "Not available.");
        });
}

#endif

int main(int argc, char **argv)
{
    qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");

    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity); // TODO: Needed ?

    // default format for shared context
    QSurfaceFormat::setDefaultFormat(Embedder::surfaceFormat(defaultFeatures(argc, argv)));

    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Embedder example");
    QCommandLineOption enableMultiWindowOpt = { { "m", "multiwindow" }, "Enable multi-window mode" };
    QCommandLineOption enableTextureGLContextOpt = { { "t", "textureGLContext" }, "Enable GL context for texture uploads (broken still)" };
    QCommandLineOption useGLESopt = { { "e", "gles" }, "Explicitly request GLES instead of the default" };
    QCommandLineOption useGLopt = { { "g", "gl" }, "Explicitly request GL instead of the default" };
    parser.addOption(enableMultiWindowOpt);
    parser.addOption(enableTextureGLContextOpt);
    parser.addOption(useGLESopt);
    parser.addOption(useGLopt);
    parser.addPositionalArgument("project", "the root of the flutter project directory");
    parser.addHelpOption();
    parser.process(app);

    if (parser.positionalArguments().isEmpty()) {
        qInfo().noquote() << parser.helpText() << "\n";
        return 1;
    }

    qDebug() << "Qt thread is" << QThread::currentThreadId();

    const QString projectPath = parser.positionalArguments().constFirst();
    Embedder::Features features = {};
    if (parser.isSet(enableMultiWindowOpt))
        features |= Embedder::Feature::MultiWindow;

    if (parser.isSet(enableTextureGLContextOpt))
        features |= Embedder::Feature::TextureGLContext;

    if (parser.isSet(useGLESopt))
        features |= Embedder::Feature::GLES;

    if (parser.isSet(useGLopt))
        features |= Embedder::Feature::GL;

    Embedder embedder(features);
    registerDummyMethodChannel(embedder);

    const auto icuPath = std::string(FLUTTER_ICUDTL_DIR) + std::string("/icudtl.dat");

    if (!embedder.runFlutter(argc, argv, projectPath.toStdString(), icuPath)) {
        return -1;
    }

    if (embedder.isMultiWindowMode()) {
        qDebug() << "Adding Window";
        embedder.addWindow();
    }
    return QApplication::exec();
}
