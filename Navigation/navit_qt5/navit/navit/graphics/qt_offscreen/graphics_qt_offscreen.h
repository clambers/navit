/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2008 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */
#ifndef __GRAPHICS_QT_OFFSCREEN_H
#define __GRAPHICS_QT_OFFSCREEN_H

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "navit/point.h"
#include "navit/item.h"
#include "navit/graphics.h"
#include "navit/color.h"
#include "navit/debug.h"
#include "navit/plugin.h"
#include "navit/callback.h"
#include "navit/event.h"
#include "navit/window.h"
#include "navit/keys.h"
#include "navit/navit.h"

#include <qglobal.h>

#ifndef QT_QPAINTER_USE_FREETYPE
#define QT_QPAINTER_USE_FREETYPE 1
#endif

#ifdef QT_QPAINTER_USE_FREETYPE
#include "navit/font/freetype/font_freetype.h"
#endif

#include <QResizeEvent>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPixmap>
#include <QWidget>
#include <QPolygonF>
#include <QPixmapCache>
#include <QtGui>
#include <QGLWidget>

/* Use qt events instead of glib */
#ifndef QT_QPAINTER_USE_EVENT_QT
#define QT_QPAINTER_USE_EVENT_QT 1
#endif
struct graphics_gc_priv {
    QPen* pen;
    QBrush* brush;
    struct color c;
};

//##############################################################################################################
//# Description:
//# Comment:
//# Authors: Martin Schaller (04/2008)
//##############################################################################################################
struct graphics_priv {
    QApplication* app;
    QPainter* painter;
    struct graphics_gc_priv* background_gc;
    unsigned char rgba[4];
    enum draw_mode_num mode;
    struct graphics_priv* parent, *overlays, *next;
    struct point p, pclean;
    int cleanup;
    int overlay_disable;
    int wraparound;
#ifdef QT_QPAINTER_USE_FREETYPE
    struct font_priv* (*font_freetype_new)(void* meth);
    struct font_freetype_methods freetype_methods;
#endif
    int w, h, flags;
    struct navit* nav;
    char* window_title;
};

void qt_offscreen_draw(struct graphics_priv* gr, const QRect* r, int paintev);
struct event_watch {
    QSocketNotifier* sn;
    struct callback* cb;
    int fd;
};

void event_qt_remove_timeout(struct event_timeout* ev);

#endif /* __GRAPHICS_QT_OFFSCREEN_H */
