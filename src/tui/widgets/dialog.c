/**************************************************************************
 **
 ** sngrep - SIP Messages flow viewer
 **
 ** Copyright (C) 2013-2019 Ivan Alonso (Kaian)
 ** Copyright (C) 2013-2019 Irontec SL. All rights reserved.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
/**
 * @file dialog.h
 * @author Ivan Alonso [aka Kaian] <kaian@irontec.com>
 *
 * @brief
 *
 */
#include "config.h"
#include <glib-unix.h>
#include "glib-extra/glib.h"
#include "glib-extra/glib_enum_types.h"
#include "tui/tui.h"
#include "tui/widgets/dialog.h"

enum
{
    PROP_TYPE = 1,
    PROP_BUTTONS,
    PROP_MESSAGE,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

// Class definition
G_DEFINE_TYPE(SngDialog, sng_dialog, SNG_TYPE_WINDOW)

SngWidget *
sng_dialog_new(SngDialogType type, SngDialogButtons buttons, const gchar *title, const gchar *message)
{
    return g_object_new(
        SNG_TYPE_DIALOG,
        "type", type,
        "buttons", buttons,
        "border", TRUE,
        "title", title,
        "message", message,
        NULL
    );
}

SngDialogResponse
sng_dialog_run(SngDialog *dialog)
{
    g_main_loop_run(dialog->loop);
    return dialog->response;
}

static void
sng_dialog_set_response(SngDialog *dialog, SngDialogResponse response)
{
    dialog->response = response;
    g_main_loop_quit(dialog->loop);
}

static void
sng_dialog_set_response_yes(SngDialog *dialog)
{
    sng_dialog_set_response(dialog, SNG_RESPONSE_YES);
}

static void
sng_dialog_set_response_no(SngDialog *dialog)
{
    sng_dialog_set_response(dialog, SNG_RESPONSE_NO);
}

static void
sng_dialog_key_pressed(SngWidget *widget, gint key)
{
    SngDialog *dialog = SNG_DIALOG(widget);

    // Check actions for this key
    KeybindingAction action = ACTION_UNKNOWN;
    while ((action = key_find_action(key, action)) != ACTION_UNKNOWN) {
        // Check if we handle this action
        switch (action) {
            case ACTION_CANCEL:
                dialog->response = SNG_RESPONSE_CANCEL;
                g_main_loop_quit(dialog->loop);
                break;
            default:
                // Parse next action
                continue;
        }

        // We've handled this key, stop checking actions
        break;
    }
}


static void
sng_dialog_constructed(GObject *object)
{
    SngDialog *dialog = SNG_DIALOG(object);

    // Calculate dialog height
    g_auto(GStrv) msg_lines = g_strsplit(dialog->message, "\n", -1);
    gint height =
        g_strv_length(msg_lines)
        + 2 // Space for buttons
        + 2 // Space for borders
        + 2; // Space for title bar

    sng_widget_set_height(
        SNG_WIDGET(dialog),
        MAX(height, SNG_DIALOG_MIN_HEIGHT)
    );

    // Calculate dialog width
    gint width = 0;
    for (guint i = 0; i < g_strv_length(msg_lines); i++) {
        width = MAX(width, sng_label_get_text_len(msg_lines[i]));
    }
    if (dialog->type != SNG_DIALOG_OTHER) {
        width += 5;
    }
    sng_widget_set_width(
        SNG_WIDGET(dialog),
        MAX(width, SNG_DIALOG_MIN_WIDTH)
    );

    SngWidget *lb_message = sng_label_new(dialog->message);
    sng_widget_set_vexpand(lb_message, TRUE);
    sng_container_add(SNG_CONTAINER(dialog), lb_message);

    if (dialog->buttons == SNG_BUTTONS_YES_NO) {

        SngWidget *bn_yes = sng_button_new();
        sng_label_set_text(SNG_LABEL(bn_yes), "[   Yes    ]");
        sng_window_add_button(SNG_WINDOW(dialog), SNG_BUTTON(bn_yes));
        g_signal_connect_swapped(bn_yes, "activate",
                                 G_CALLBACK(sng_dialog_set_response_yes), dialog);

        SngWidget *bn_no = sng_button_new();
        sng_label_set_text(SNG_LABEL(bn_no), "[    No    ]");
        sng_window_add_button(SNG_WINDOW(dialog), SNG_BUTTON(bn_no));
        g_signal_connect_swapped(bn_no, "activate",
                                 G_CALLBACK(sng_dialog_set_response_no), dialog);

        // Set first button as default
        sng_window_set_default_focus(SNG_WINDOW(dialog), bn_yes);
    }
    sng_container_show_all(SNG_CONTAINER(dialog));

    // Chain-up parent constructed
    G_OBJECT_CLASS(sng_dialog_parent_class)->constructed(object);
}

static void
sng_dialog_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    SngDialog *dialog = SNG_DIALOG(object);
    switch (property_id) {
        case PROP_TYPE:
            dialog->type = g_value_get_enum(value);
            break;
        case PROP_BUTTONS:
            dialog->buttons = g_value_get_enum(value);
            break;
        case PROP_MESSAGE:
            dialog->message = g_strdup(g_value_get_string(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
sng_dialog_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    SngDialog *dialog = SNG_DIALOG(object);
    switch (property_id) {
        case PROP_TYPE:
            g_value_set_enum(value, dialog->type);
            break;
        case PROP_BUTTONS:
            g_value_set_enum(value, dialog->buttons);
            break;
        case PROP_MESSAGE:
            g_value_set_string(value, dialog->message);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
sng_dialog_class_init(SngDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->constructed = sng_dialog_constructed;
    object_class->set_property = sng_dialog_set_property;
    object_class->get_property = sng_dialog_get_property;

    SngWidgetClass *widget_class = SNG_WIDGET_CLASS(klass);
    widget_class->key_pressed = sng_dialog_key_pressed;

    obj_properties[PROP_TYPE] =
        g_param_spec_enum("type",
                          "Dialog type",
                          "Dialog type",
                          SNG_TYPE_DIALOG_TYPE,
                          SNG_DIALOG_OTHER,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT
        );

    obj_properties[PROP_BUTTONS] =
        g_param_spec_enum("buttons",
                          "Dialog buttons",
                          "Dialog buttons",
                          SNG_TYPE_DIALOG_BUTTONS,
                          SNG_BUTTONS_NONE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT
        );

    obj_properties[PROP_MESSAGE] =
        g_param_spec_string("message",
                            "Dialog message",
                            "Dialog message",
                            NULL,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        obj_properties
    );
}

static void
sng_dialog_init(SngDialog *dialog)
{
    // Create a main loop for this dialog
    dialog->loop = g_main_loop_new(NULL, FALSE);

    // Source for reading events from stdin
    GSource *source = g_unix_fd_source_new(STDIN_FILENO, G_IO_IN | G_IO_ERR | G_IO_HUP);
    g_source_set_callback(source, (GSourceFunc) G_CALLBACK(tui_read_input), dialog->loop, NULL);
    g_source_attach(source, NULL);

    // Refresh screen every 200 ms
    g_timeout_add(200, (GSourceFunc) tui_refresh_screen, dialog->loop);
}
