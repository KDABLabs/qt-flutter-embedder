/*
  This file is part of qt-flutter-embedder.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "FlutterWindow.h"
#include "Embedder.h"

#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLContext>

using namespace KDAB;

static FlutterViewId s_nextId = 0;

FlutterWindow::FlutterWindow(Embedder &embedder)
    : m_embedder(embedder)
    , m_id(s_nextId++)
{
    setFormat(embedder.surfaceFormat());
    setSurfaceType(QWindow::OpenGLSurface);
}

void FlutterWindow::resizeEvent(QResizeEvent *ev)
{
    const float pixelRatio = devicePixelRatio();

    FlutterWindowMetricsEvent event = {};
    event.struct_size = sizeof(event);

    // Flutter needs to know in physical pixels
    event.width = size_t(ev->size().width() * pixelRatio);
    event.height = size_t(ev->size().height() * pixelRatio);
    event.view_id = m_id;

    event.pixel_ratio = pixelRatio;
    FlutterEngineSendWindowMetricsEvent(m_embedder.engine(), &event);

    return QWindow::resizeEvent(ev);
}

void FlutterWindow::keyPressEvent(QKeyEvent *ev)
{
    sendKeyEventToFlutter(ev, FlutterKeyEventType::kFlutterKeyEventTypeDown);
}

void FlutterWindow::keyReleaseEvent(QKeyEvent *ev)
{
    sendKeyEventToFlutter(ev, FlutterKeyEventType::kFlutterKeyEventTypeUp);
}

void FlutterWindow::mousePressEvent(QMouseEvent *ev)
{
    auto pos = ev->pos();
    sendMouseEventToFlutter(FlutterPointerPhase::kDown, pos.x(), pos.y());
}

void FlutterWindow::mouseReleaseEvent(QMouseEvent *ev)
{
    auto pos = ev->pos();
    sendMouseEventToFlutter(FlutterPointerPhase::kUp, pos.x(), pos.y());
}

void FlutterWindow::mouseDoubleClickEvent(QMouseEvent *)
{
}

void FlutterWindow::mouseMoveEvent(QMouseEvent *ev)
{
    // auto pos = ev->pos();
    // sendMouseEventToFlutter(FlutterPointerPhase::kMove, pos.x(), pos.y());
}

void FlutterWindow::sendMouseEventToFlutter(FlutterPointerPhase phase, double x, double y)
{
    FlutterPointerEvent event = {};
    event.struct_size = sizeof(event);
    event.phase = phase;

    // flutter needs the event in physical pixels
    event.x = x * devicePixelRatio();
    event.y = y * devicePixelRatio();

    event.view_id = m_id;

    event.timestamp =
        size_t(std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch())
                   .count());
    FlutterEngineSendPointerEvent(m_embedder.engine(), &event, 1);
}

void FlutterWindow::sendKeyEventToFlutter(QKeyEvent *ev, FlutterKeyEventType type)
{
    FlutterKeyEvent event;
    FlutterKeyEventCallback callback = [](bool handled, void *userdata) {};
    event.struct_size = sizeof(event);
    event.synthesized = false;
    event.timestamp = ev->timestamp();
    event.type = type;
    event.device_type = kFlutterKeyEventDeviceTypeKeyboard;
    std::string text = ev->text().toStdString();
    event.character = text.c_str();
    event.logical = ev->key();
    event.physical = ev->nativeScanCode();

    FlutterEngineSendKeyEvent(m_embedder.engine(), &event,
                              callback, nullptr);
}

void FlutterWindow::closeEvent(QCloseEvent *ev)
{
    FlutterEngineShutdown(m_embedder.engine());
    ev->accept();
}

FlutterViewId FlutterWindow::id() const
{
    return m_id;
}
