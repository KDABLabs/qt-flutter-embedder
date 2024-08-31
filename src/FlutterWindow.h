/*
  This file is part of qt-flutter-embedder.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QWindow>

#include <flutter_embedder.h>

class QOpenGLContext;

namespace KDAB {
class Embedder;
class FlutterWindow : public QWindow
{
public:
    explicit FlutterWindow(Embedder &);

    FlutterViewId id() const;

protected:
    void resizeEvent(QResizeEvent *ev) override;

    void keyPressEvent(QKeyEvent *ev) override;

    void keyReleaseEvent(QKeyEvent *ev) override;

    void mousePressEvent(QMouseEvent *ev) override;

    void mouseReleaseEvent(QMouseEvent *ev) override;

    void mouseDoubleClickEvent(QMouseEvent *) override;

    void mouseMoveEvent(QMouseEvent *ev) override;

    void sendMouseEventToFlutter(FlutterPointerPhase phase, double x, double y);
    void sendKeyEventToFlutter(QKeyEvent *ev, FlutterKeyEventType type);

    void closeEvent(QCloseEvent *ev) override;

private:
    Embedder &m_embedder;
    const FlutterViewId m_id;
};
}
