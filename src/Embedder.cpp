/*
  This file is part of qt-flutter-embedder.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Embedder.h"
#include "FlutterWindow.h"
#include "flutter_embedder.h"

#include <QOpenGLContext>
#include <QGuiApplication>
#include <QThread>
#include <QDebug>
#include <QLoggingCategory>
#include <QColorSpace>
#include <QOffscreenSurface>
#include <QtGui/private/qopenglcontext_p.h>

#include <iostream>
#include <filesystem>

// TODO: In the future we won't need to link to GL directly
// only in a developer build where we call gl functions directly
#ifndef NO_LINK_EGL
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#endif

Q_LOGGING_CATEGORY(qtembedder, "qtembedder")

using namespace KDAB;
namespace fs = std::filesystem;

static bool isAOT()
{
#ifdef FLUTTER_AOT
    return true;
#endif
    return false;
}

Embedder::Embedder(Features features)
    : m_features(features)
{
    auto window = new FlutterWindow(*this);
    window->show();
    window->resize(1000, 1000);
    m_windows << window;
}

Embedder::~Embedder()
{
    delete m_offscreenSurface;
}

_FlutterEngine *Embedder::engine() const
{
    return m_flutterEngine;
}

FlutterWindow &Embedder::mainWindow() const
{
    return *m_windows.first();
}

bool Embedder::runFlutter(int argc, char **argv, const std::string &project_path, const std::string &icudtl_path)
{
    if (!fs::exists(project_path)) {
        std::cerr << "Project not found: " << project_path << "\n";
        std::abort();
    }

    if (!fs::exists(icudtl_path)) {
        std::cerr << "icudtl not found: " << icudtl_path << "\n";
        std::abort();
    }

    FlutterRendererConfig config = {};
    config.type = kOpenGL;
    config.open_gl.struct_size = sizeof(config.open_gl);
    config.open_gl.make_current = [](void *userdata) -> bool {
        qCInfo(qtembedder) << "make_current:";

        auto embedder = reinterpret_cast<Embedder *>(userdata);
        const bool isFirstCall = !embedder->glContext();
        if (isFirstCall) {
            // only run once.
            embedder->createGLContext();
        }

        const bool result = embedder->mainWindow().makeCurrent();
        if (!result)
            qCWarning(qtembedder) << "Failed to make context current; thread=" << QThread::currentThreadId() << "; currentContext=" << QOpenGLContext::currentContext();

        if (isFirstCall) {
            // only dump noisy debug once
            embedder->dumpGLInfo(/*printExtensions=*/true);
        }

        return result;
    };

    config.open_gl.clear_current = [](void *) -> bool {
        qCInfo(qtembedder) << "clear_current:";
        QOpenGLContext::currentContext()->doneCurrent();
        return true;
    };

    config.open_gl.present_with_info = [](void *userdata, const FlutterPresentInfo *) -> bool {
        qCInfo(qtembedder) << "present_with_info:";

        auto embedder = reinterpret_cast<Embedder *>(userdata);
        Q_ASSERT(!embedder->isMultiWindowMode());

        auto &window = embedder->mainWindow();
        embedder->glContext()->swapBuffers(&window);
        return true;
    };

    config.open_gl.fbo_callback = [](void *) -> uint32_t {
        qCInfo(qtembedder) << "fbo_callback:";
        return 0;
    };

    config.open_gl.gl_proc_resolver = [](void *userdata, const char *name) -> void * {
        auto embedder = reinterpret_cast<Embedder *>(userdata);

        auto currentContext = QOpenGLContext::currentContext();
        if (!currentContext) {
            qCWarning(qtembedder) << "gl_proc_resolver: gl context is null. name=" << name;
            return nullptr;
        }

        return reinterpret_cast<void *>(currentContext->getProcAddress(name));
    };

    if (m_features & Feature::TextureGLContext) {
        config.open_gl.make_resource_current = [](void *userdata) {
            qCInfo(qtembedder) << "make_resource_current: thread=" << QThread::currentThreadId();

            auto embedder = reinterpret_cast<Embedder *>(userdata);
            if (!embedder->textureGlContext())
                embedder->createTextureGLContext();

            embedder->textureGlContext()->makeCurrent(embedder->offscreenSurfaceForTextureUploads());

            return true;
        };
    }
    // This directory is generated by `flutter build bundle`. Used both in AOT and JIT mode
    const std::string assets_path = project_path + "/build/flutter_assets";

    FlutterEngineAOTData aot_data = nullptr;
    if (isAOT()) {
        // flutter build linux --release , for AOT mode
        const std::string aot_elf = project_path + "/build/linux/x64/release/bundle/lib/libapp.so";
        std::cout << "embedder: Using aot_elf=" << aot_elf << std::endl;

        const FlutterEngineAOTDataSource source = { .type = kFlutterEngineAOTDataSourceTypeElfPath, .elf_path = aot_elf.c_str() };
        auto res = FlutterEngineCreateAOTData(&source, &aot_data);
        if (res != kSuccess || !aot_data) {
            std::cerr << "Could not prepare AOT data result=" << int(res) << "\n";
            return false;
        }
    }

    m_flutterCompositor = {};
    // In multiwindow mode we need a compositor
    if (isMultiWindowMode()) {
        m_flutterCompositor.struct_size = sizeof(m_flutterCompositor);
        m_flutterCompositor.user_data = this;
        m_flutterCompositor.present_view_callback = [](const FlutterPresentViewInfo *info) {
            qCInfo(qtembedder) << "compositor.present_view_callback: view=" << info->view_id;
            auto embedder = reinterpret_cast<Embedder *>(info->user_data);
            auto window = embedder->windowForId(info->view_id);
            embedder->glContext()->swapBuffers(window);
            return true;
        };

        m_flutterCompositor.collect_backing_store_callback = [](const FlutterBackingStore *, void *) {
            qCInfo(qtembedder) << "compositor.collect_backing_store_callback:";
            return true;
        };

        m_flutterCompositor.create_backing_store_callback = [](const FlutterBackingStoreConfig *config,
                                                               FlutterBackingStore *backing_store_out,
                                                               void *user_data) {
            qCInfo(qtembedder) << "create_backing_store: view=" << config->view_id << "; size=" << config->size.width
                               << "x" << config->size.height;

            auto embedder = reinterpret_cast<Embedder *>(user_data);
            auto window = embedder->windowForId(config->view_id);
            Q_ASSERT(window); // TODO: Create it and actually assert if it existed already

            FlutterOpenGLSurface glSurface;
            glSurface.struct_size = sizeof(FlutterOpenGLSurface);
            glSurface.user_data = window;
            glSurface.make_current_callback = [](void *user_data, bool *state_changed) {
                *state_changed = false;
                auto window = reinterpret_cast<FlutterWindow *>(user_data);
                qCInfo(qtembedder) << "glSurface.make_current_callback id=" << window->id();

                const bool result = window->makeCurrent();
                if (!result) {
                    qCWarning(qtembedder) << "Failed to make context current for window" << window->id();
                    return false;
                }

                return true;
            };

            glSurface.clear_current_callback = [](void *user_data, bool *state_changed) {
                *state_changed = false;
                // TODO: Clear color
                auto window = reinterpret_cast<FlutterWindow *>(user_data);
                qCInfo(qtembedder) << "glSurface.clear_current_callback id=" << window->id();
                return true;
            };

            glSurface.destruction_callback = [](void *user_data) {
                auto window = reinterpret_cast<FlutterWindow *>(user_data);
                qCInfo(qtembedder) << "glSurface.destruction_callback id=" << window->id();
            };

            FlutterOpenGLBackingStore glStore;
            glStore.type = kFlutterOpenGLTargetTypeSurface;
            glStore.surface = glSurface;

            backing_store_out->struct_size = sizeof(FlutterBackingStore);
            backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
            backing_store_out->user_data = window;
            // backing_store_out->did_update TODO
            backing_store_out->open_gl = glStore;

            return true;
        };

        m_flutterCompositor.avoid_backing_store_cache = true;
    }

    FlutterProjectArgs args = {
        .struct_size = sizeof(FlutterProjectArgs),
        .assets_path = assets_path.c_str(),
        .icu_data_path = icudtl_path.c_str(),
        .command_line_argc = argc,
        .command_line_argv = argv,
        .compositor = isMultiWindowMode() ? &m_flutterCompositor : nullptr,
        .aot_data = aot_data,
        .dart_entrypoint_argc = argc,
        .dart_entrypoint_argv = argv,
    };

    FlutterEngineResult result =
        FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config,
                         &args, this, &m_flutterEngine);


    if (result != kSuccess) {
        qCWarning(qtembedder) << "Could not run the flutter engine.";
        return false;
    }

    return true;
}

void Embedder::createGLContext()
{
    if (m_glContext) {
        Q_ASSERT(false);
        return;
    }

    m_glContext = new QOpenGLContext();
    m_glContext->setFormat(surfaceFormat());
    m_glContext->setShareContext(qt_gl_global_share_context());
    m_glContext->setObjectName("m_glContext");

    if (!m_glContext->create())
        qFatal("Could not create opengl context");
}

void Embedder::createTextureGLContext()
{
    if (m_textureGlContext) {
        Q_ASSERT(false);
        return;
    }
    m_offscreenSurface = new QOffscreenSurface();
    m_offscreenSurface->setFormat(surfaceFormat());
    m_offscreenSurface->create();

    m_textureGlContext = new QOpenGLContext();
    m_textureGlContext->setFormat(surfaceFormat());
    m_textureGlContext->setShareContext(qt_gl_global_share_context());
    m_textureGlContext->setObjectName("m_textureGlContext");
    if (!m_textureGlContext->create())
        qFatal("Could not create opengl context");
}

QSurfaceFormat Embedder::surfaceFormat() const
{
    return Embedder::surfaceFormat(m_features);
}

/** static*/
QSurfaceFormat Embedder::surfaceFormat(Features features)
{
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(8);
    fmt.setStencilBufferSize(8);
    fmt.setAlphaBufferSize(8);
    fmt.setSamples(8);

    // stuff we probably don't need:
    // fmt.setColorSpace(QColorSpace::SRgb);
    // fmt.setProfile(QSurfaceFormat::CoreProfile);

    if (features & Feature::GLES)
        fmt.setRenderableType(QSurfaceFormat::OpenGLES);
    else if (features & Feature::GL)
        fmt.setRenderableType(QSurfaceFormat::OpenGL);

    return fmt;
}

QOpenGLContext *Embedder::glContext() const
{
    return m_glContext;
}

QOpenGLContext *Embedder::textureGlContext() const
{
    return m_textureGlContext;
}

QOffscreenSurface *Embedder::offscreenSurfaceForTextureUploads() const
{
    return m_offscreenSurface;
}

FlutterWindow *Embedder::addWindow()
{
    auto window = new FlutterWindow(*this);
    window->resize(600, 600);

    FlutterWindowMetricsEvent metrics = {};
    metrics.struct_size = sizeof(metrics);
    metrics.height = 600;
    metrics.width = 600;
    metrics.left = 0;
    metrics.top = 0;
    metrics.pixel_ratio = window->devicePixelRatio();
    metrics.view_id = window->id();

    FlutterAddViewInfo info;
    info.struct_size = sizeof(info);
    info.view_id = window->id();
    info.user_data = window;
    info.view_metrics = &metrics;


    info.add_view_callback = [](const FlutterAddViewResult *result) {
        qCInfo(qtembedder) << "Embedder::addWindow: added=" << result->added;
        if (result->added) {
            auto window = reinterpret_cast<FlutterWindow *>(result->user_data);

            // re-thread
            QMetaObject::invokeMethod(window, [window] {
                window->show();
            });

        } else {
            qCWarning(qtembedder) << "Embedder: Could not add view!";
        }
    };

    auto result = FlutterEngineAddView(m_flutterEngine, &info);
    if (result != kSuccess) {
        qCWarning(qtembedder) << "Embedder: Error adding view";
        delete window;
        return nullptr;
    }

    m_windows << window;
    window->setTitle(QString("Window #%1").arg(m_windows.size()));

    return window;
}

FlutterWindow *Embedder::windowForId(FlutterViewId id) const
{
    auto it = std::find_if(m_windows.cbegin(), m_windows.cend(), [id](auto w) {
        return w->id() == id;
    });

    if (it == m_windows.cend()) {
        Q_ASSERT(nullptr);
        return nullptr;
    }

    return *it;
}

#define GL_CHARPTR(X) ( const char * )glGetString(X)

void Embedder::dumpGLInfo(bool printExtensions)
{
#ifndef NO_LINK_EGL
    Q_ASSERT(m_glContext);
    qCInfo(qtembedder) << "\n\ndumpGLInfo: START";
    qCInfo(qtembedder) << "format=" << m_glContext->format()
                       << "\nGL_VERSION=" << GL_CHARPTR(GL_VERSION)
                       << "\nGL_RENDERER" << GL_CHARPTR(GL_RENDERER)
                       << "\nGL_VENDOR" << GL_CHARPTR(GL_VENDOR)
                       << "\nGL_SHADING_LANGUAGE_VERSION" << GL_CHARPTR(GL_SHADING_LANGUAGE_VERSION)
                       << "\n";

    if (printExtensions) {
        int extensionCnt = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCnt);

        const char *extensions = GL_CHARPTR(GL_EXTENSIONS);
        Q_ASSERT(eglGetCurrentDisplay() != EGL_NO_DISPLAY);

        QStringList extensionList;
        extensionList.reserve(extensionCnt);
        for (int i = 0; i < extensionCnt; ++i)
            extensionList.append(( const char * )glGetStringi(GL_EXTENSIONS, i));

        qCInfo(qtembedder) << extensionList.join("; ");
        qCInfo(qtembedder) << "\ndumpGLInfo: END\n";
    }
#endif
}

bool Embedder::isMultiWindowMode() const
{
    return m_features & Feature::MultiWindow;
}

bool Embedder::isGLES() const
{
    return m_features & Feature::GLES;
}
