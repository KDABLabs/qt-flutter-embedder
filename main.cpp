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
    bool vulkan = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--gl") == 0 || strcmp(argv[i], "-g") == 0)
            gl = true;
        if (strcmp(argv[i], "--gles") == 0 || strcmp(argv[i], "-e") == 0)
            gles = true;
        if (strcmp(argv[i], "--vulkan") == 0 || strcmp(argv[i], "-v") == 0)
            vulkan = true;
    }

    if (gl && !vulkan && !gles) {
        features |= Embedder::Feature::GL;
    } else if (gles && !vulkan && !gl) {
        features |= Embedder::Feature::GLES;
    } else if (vulkan && !gl && !gles) {
        features |= Embedder::Feature::Vulkan;
    } else if (gles || gl || vulkan) {
        qFatal("-gl, -gles, -vulkan are mutually exclusive");
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
    QCommandLineOption useVulkanOpt = { { "v", "vulkan" }, "Explicitly request Vulkan instead of the default" };
    QCommandLineOption useImpellerOpt = { { "i", "enable-impeller" }, "Enable Impeller rendering backend" };

    parser.addOption(enableMultiWindowOpt);
    parser.addOption(enableTextureGLContextOpt);
    parser.addOption(useGLESopt);
    parser.addOption(useGLopt);
    parser.addOption(useVulkanOpt);
    parser.addOption(useImpellerOpt);
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

    if (parser.isSet(useImpellerOpt))
        features |= Embedder::Feature::Impeller;

    if (parser.isSet(useVulkanOpt)) {
        features |= Embedder::Feature::Vulkan;
        qFatal("Vulkan is not supported yet");
    }

    Embedder embedder(features);
#ifdef DEVELOPER_BUILD
    registerDummyMethodChannel(embedder);
#endif

    const auto icuPath = std::string(FLUTTER_ICUDTL_DIR) + std::string("/icudtl.dat");

    std::vector<char *> modifiedArgv;
    modifiedArgv.reserve(argc + 1);
    for (int i = 0; i < argc; ++i) {
        modifiedArgv.push_back(argv[i]);
    }

    if (parser.isSet(useImpellerOpt)) {
        // TODO: impeller docs mention this, but no idea if it works

        const char *impellerFlag = "--enable-impeller=true";
        auto impellerArg = new char[strlen(impellerFlag) + 1];
        strcpy(impellerArg, impellerFlag);
        modifiedArgv.push_back(impellerArg);

        const char *impellerBackendFlag = "--impeller-backend=vulkan";
        auto impellerBackendArg = new char[strlen(impellerBackendFlag) + 1];
        strcpy(impellerBackendArg, impellerBackendFlag);
        modifiedArgv.push_back(impellerBackendArg);
    }

    if (!embedder.runFlutter(modifiedArgv.size(), modifiedArgv.data(), projectPath.toStdString(), icuPath)) {
        return -1;
    }

    if (embedder.isMultiWindowMode()) {
        qDebug() << "Adding Window";
        embedder.addWindow();
    }
    return QApplication::exec();
}
