/*
 * vim:ts=4:sw=4:expandtab
 *
 * © 2010 Michael Stapelberg
 *
 * See LICENSE for licensing information
 *
 */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <xcb/xcb.h>
#include <ev.h>
#include <cairo.h>
#include <cairo/cairo-xcb.h>
#include <X11/Xlib.h>

#include "i3lock.h"
#include "xcb.h"
#include "unlock_indicator.h"
#include "xinerama.h"
#include "blur.h"

#define BUTTON_RADIUS 90
#define BUTTON_SPACE (BUTTON_RADIUS + 5)
#define BUTTON_CENTER (BUTTON_RADIUS + 5)
#define BUTTON_DIAMETER (2 * BUTTON_SPACE)

/*******************************************************************************
 * Variables defined in i3lock.c.
 ******************************************************************************/

extern bool debug_mode;

/* The current position in the input buffer. Useful to determine if any
 * characters of the password have already been entered or not. */
int input_position;

/* The lock window. */
extern xcb_window_t win;

/* The current resolution of the X11 root window. */
extern uint32_t last_resolution[2];
uint32_t button_diameter_physical;

/* Whether the unlock indicator is enabled (defaults to true). */
extern bool unlock_indicator;

/* List of pressed modifiers, or NULL if none are pressed. */
extern char *modifier_string;

/* A Cairo surface containing the specified image (-i), if any. */
extern cairo_surface_t *img;
extern bool dpms_capable;
/* Whether the image should be tiled. */
extern bool tile;
/* Whether to use fuzzy mode. */
extern bool fuzzy;
extern int blur_radius;
extern float blur_sigma;
/* The background color to use (in hex). */
extern char color[7];
extern Display *display;

/* Whether the failed attempts should be displayed. */
extern bool show_failed_attempts;
/* Number of failed unlock attempts. */
extern int failed_attempts;

/*******************************************************************************
 * Variables defined in xcb.c.
 ******************************************************************************/

/* The root screen, to determine the DPI. */
extern xcb_screen_t *screen;

/*******************************************************************************
 * Local variables.
 ******************************************************************************/

/* Cache the screen’s visual, necessary for creating a Cairo context. */
static xcb_visualtype_t *vistype;

/* Maintain the current unlock/PAM state to draw the appropriate unlock
 * indicator. */
unlock_state_t unlock_state;
pam_state_t pam_state;

/* A surface for the unlock indicator */
static cairo_surface_t *unlock_indicator_surface = NULL;

/*
 * Returns the scaling factor of the current screen. E.g., on a 227 DPI MacBook
 * Pro 13" Retina screen, the scaling factor is 227/96 = 2.36.
 *
 */
static double scaling_factor(void) {
    const int dpi = (double)screen->height_in_pixels * 25.4 /
                    (double)screen->height_in_millimeters;
    return (dpi / 96.0);
}

static void draw_unlock_indicator() {
    /* Initialise the surface if not yet done */
    if (unlock_indicator_surface == NULL) {
        button_diameter_physical = ceil(scaling_factor() * BUTTON_DIAMETER);
        DEBUG("scaling_factor is %.f, physical diameter is %d px\n",
                scaling_factor(), button_diameter_physical);
        unlock_indicator_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, button_diameter_physical, button_diameter_physical);
    }

    cairo_t *ctx = cairo_create(unlock_indicator_surface);
    /* clear the surface */
    cairo_save(ctx);
    cairo_set_operator(ctx, CAIRO_OPERATOR_CLEAR);
    cairo_paint(ctx);
    cairo_restore(ctx);

    if (unlock_state >= STATE_KEY_PRESSED && unlock_indicator) {
        cairo_scale(ctx, scaling_factor(), scaling_factor());
        /* Draw a (centered) circle with transparent background. */
        cairo_set_line_width(ctx, 10.0);
        cairo_arc(ctx,
                  BUTTON_CENTER /* x */,
                  BUTTON_CENTER /* y */,
                  BUTTON_RADIUS /* radius */,
                  0 /* start */,
                  2 * M_PI /* end */);

        /* Use the appropriate color for the different PAM states
         * (currently verifying, wrong password, or default) */
        switch (pam_state) {
            case STATE_PAM_VERIFY:
                cairo_set_source_rgba(ctx, 0, 114.0 / 255, 255.0 / 255, 0.75);
                break;
            case STATE_PAM_WRONG:
                cairo_set_source_rgba(ctx, 250.0 / 255, 0, 0, 0.75);
                break;
            default:
                cairo_set_source_rgba(ctx, 0, 0, 0, 0.75);
                break;
        }
        cairo_fill_preserve(ctx);

        switch (pam_state) {
            case STATE_PAM_VERIFY:
                cairo_set_source_rgb(ctx, 51.0 / 255, 0, 250.0 / 255);
                break;
            case STATE_PAM_WRONG:
                cairo_set_source_rgb(ctx, 125.0 / 255, 51.0 / 255, 0);
                break;
            case STATE_PAM_IDLE:
                cairo_set_source_rgb(ctx, 51.0 / 255, 125.0 / 255, 0);
                break;
        }
        cairo_stroke(ctx);

        /* Draw an inner seperator line. */
        cairo_set_source_rgb(ctx, 0, 0, 0);
        cairo_set_line_width(ctx, 2.0);
        cairo_arc(ctx,
                  BUTTON_CENTER /* x */,
                  BUTTON_CENTER /* y */,
                  BUTTON_RADIUS - 5 /* radius */,
                  0,
                  2 * M_PI);
        cairo_stroke(ctx);

        cairo_set_line_width(ctx, 10.0);

        /* Display a (centered) text of the current PAM state. */
        char *text = NULL;
        /* We don't want to show more than a 3-digit number. */
        char buf[4];

        cairo_set_source_rgb(ctx, 0, 0, 0);
        cairo_set_font_size(ctx, 28.0);
        switch (pam_state) {
            case STATE_PAM_VERIFY:
                text = "verifying…";
                break;
            case STATE_PAM_WRONG:
                text = "wrong!";
                break;
            default:
                if (show_failed_attempts && failed_attempts > 0) {
                    if (failed_attempts > 999) {
                        text = "> 999";
                    } else {
                        snprintf(buf, sizeof(buf), "%d", failed_attempts);
                        text = buf;
                    }
                    cairo_set_source_rgb(ctx, 1, 0, 0);
                    cairo_set_font_size(ctx, 32.0);
                }
                break;
        }

        if (text) {
            cairo_text_extents_t extents;
            double x, y;

            cairo_text_extents(ctx, text, &extents);
            x = BUTTON_CENTER - ((extents.width / 2) + extents.x_bearing);
            y = BUTTON_CENTER - ((extents.height / 2) + extents.y_bearing);

            cairo_move_to(ctx, x, y);
            cairo_show_text(ctx, text);
            cairo_close_path(ctx);
        }

        if (pam_state == STATE_PAM_WRONG && (modifier_string != NULL)) {
            cairo_text_extents_t extents;
            double x, y;

            cairo_set_font_size(ctx, 14.0);

            cairo_text_extents(ctx, modifier_string, &extents);
            x = BUTTON_CENTER - ((extents.width / 2) + extents.x_bearing);
            y = BUTTON_CENTER - ((extents.height / 2) + extents.y_bearing) + 28.0;

            cairo_move_to(ctx, x, y);
            cairo_show_text(ctx, modifier_string);
            cairo_close_path(ctx);
        }

        /* After the user pressed any valid key or the backspace key, we
         * highlight a random part of the unlock indicator to confirm this
         * keypress. */
        if (unlock_state == STATE_KEY_ACTIVE ||
            unlock_state == STATE_BACKSPACE_ACTIVE) {
            cairo_new_sub_path(ctx);
            double highlight_start = (rand() % (int)(2 * M_PI * 100)) / 100.0;
            cairo_arc(ctx,
                      BUTTON_CENTER /* x */,
                      BUTTON_CENTER /* y */,
                      BUTTON_RADIUS /* radius */,
                      highlight_start,
                      highlight_start + (M_PI / 3.0));
            if (unlock_state == STATE_KEY_ACTIVE) {
                /* For normal keys, we use a lighter green. */
                cairo_set_source_rgb(ctx, 51.0 / 255, 219.0 / 255, 0);
            } else {
                /* For backspace, we use red. */
                cairo_set_source_rgb(ctx, 219.0 / 255, 51.0 / 255, 0);
            }
            cairo_stroke(ctx);

            /* Draw two little separators for the highlighted part of the
             * unlock indicator. */
            cairo_set_source_rgb(ctx, 0, 0, 0);
            cairo_arc(ctx,
                      BUTTON_CENTER /* x */,
                      BUTTON_CENTER /* y */,
                      BUTTON_RADIUS /* radius */,
                      highlight_start /* start */,
                      highlight_start + (M_PI / 128.0) /* end */);
            cairo_stroke(ctx);
            cairo_arc(ctx,
                      BUTTON_CENTER /* x */,
                      BUTTON_CENTER /* y */,
                      BUTTON_RADIUS /* radius */,
                      highlight_start + (M_PI / 3.0) /* start */,
                      (highlight_start + (M_PI / 3.0)) + (M_PI / 128.0) /* end */);
            cairo_stroke(ctx);
        }
    }

    cairo_destroy(ctx);
}

/*
 * Draws global image with fill color onto a pixmap with the given
 * resolution and returns it.
 *
 */
xcb_pixmap_t draw_image(uint32_t *resolution) {
    xcb_pixmap_t bg_pixmap = XCB_NONE;

    if (!vistype)
        vistype = get_root_visual_type(screen);
    if (fuzzy) {
        bg_pixmap = create_fg_pixmap(conn, screen, resolution);
    }
    else {
        bg_pixmap = create_bg_pixmap(conn, screen, resolution, color);
    }
    /* Initialize cairo: Create one in-memory surface to render the unlock
     * indicator on, create one XCB surface to actually draw (one or more,
     * depending on the amount of screens) unlock indicators on. */

    cairo_surface_t *xcb_output = cairo_xcb_surface_create(conn, bg_pixmap, vistype, resolution[0], resolution[1]);
    cairo_t *xcb_ctx = cairo_create(xcb_output);

    if (img||fuzzy) {
        if (fuzzy) {
            blur_image_gl(0, bg_pixmap, last_resolution[0],last_resolution[1], blur_radius, blur_sigma);
            cairo_surface_t * tmp = cairo_xcb_surface_create(conn, bg_pixmap, get_root_visual_type(screen), last_resolution[0], last_resolution[1]);
            cairo_set_source_surface(xcb_ctx, tmp, 0, 0);
            cairo_paint(xcb_ctx);
            cairo_surface_destroy(tmp);
        }
        else if (!tile) {
            cairo_set_source_surface(xcb_ctx, img, 0, 0);
            cairo_paint(xcb_ctx);
        } else {
            /* create a pattern and fill a rectangle as big as the screen */
            cairo_pattern_t *pattern;
            pattern = cairo_pattern_create_for_surface(img);
            cairo_set_source(xcb_ctx, pattern);
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
            cairo_rectangle(xcb_ctx, 0, 0, resolution[0], resolution[1]);
            cairo_fill(xcb_ctx);
            cairo_pattern_destroy(pattern);
        }
    } else {
        char strgroups[3][3] = {{color[0], color[1], '\0'},
                                {color[2], color[3], '\0'},
                                {color[4], color[5], '\0'}};
        uint32_t rgb16[3] = {(strtol(strgroups[0], NULL, 16)),
                             (strtol(strgroups[1], NULL, 16)),
                             (strtol(strgroups[2], NULL, 16))};
        cairo_set_source_rgb(xcb_ctx, rgb16[0] / 255.0, rgb16[1] / 255.0, rgb16[2] / 255.0);
        cairo_rectangle(xcb_ctx, 0, 0, resolution[0], resolution[1]);
        cairo_fill(xcb_ctx);
    }


    if (xr_screens > 0) {
        /* Composite the unlock indicator in the middle of each screen. */
        for (int screen = 0; screen < xr_screens; screen++) {
            int x = (xr_resolutions[screen].x + ((xr_resolutions[screen].width / 2) - (button_diameter_physical / 2)));
            int y = (xr_resolutions[screen].y + ((xr_resolutions[screen].height / 2) - (button_diameter_physical / 2)));
            cairo_set_source_surface(xcb_ctx, unlock_indicator_surface, x, y);
            cairo_rectangle(xcb_ctx, x, y, button_diameter_physical, button_diameter_physical);
            cairo_fill(xcb_ctx);
        }
    } else {
        /* We have no information about the screen sizes/positions, so we just
         * place the unlock indicator in the middle of the X root window and
         * hope for the best. */
        int x = (last_resolution[0] / 2) - (button_diameter_physical / 2);
        int y = (last_resolution[1] / 2) - (button_diameter_physical / 2);
        cairo_set_source_surface(xcb_ctx, unlock_indicator_surface, x, y);
        cairo_rectangle(xcb_ctx, x, y, button_diameter_physical, button_diameter_physical);
        cairo_fill(xcb_ctx);
    }

    cairo_surface_destroy(xcb_output);
    cairo_destroy(xcb_ctx);
    return bg_pixmap;
}

/*
 * Calls draw_image on a new pixmap and swaps that with the current pixmap
 *
 */
void redraw_screen(void) {

    /* avoid drawing if monitor state is not on */
    if (dpms_capable) {
        xcb_dpms_info_reply_t *dpms_info =
            xcb_dpms_info_reply(conn,xcb_dpms_info(conn), NULL);
        if (dpms_info) {
            /* monitor is off when DPMS state is enabled and power level is not
             * DPMS_MODE_ON */
            uint8_t monitor_off = dpms_info->state
                && dpms_info->power_level != XCB_DPMS_DPMS_MODE_ON;
            free(dpms_info);
            if (monitor_off)
                return;
        }
    }

    xcb_pixmap_t bg_pixmap = draw_image(last_resolution);
    xcb_change_window_attributes(conn, win, XCB_CW_BACK_PIXMAP, (uint32_t[1]){bg_pixmap});
    /* XXX: Possible optimization: Only update the area in the middle of the
     * screen instead of the whole screen. */
    xcb_clear_area(conn, 0, win, 0, 0, last_resolution[0], last_resolution[1]);
    xcb_free_pixmap(conn, bg_pixmap);
    xcb_flush(conn);
}

/* 
 * Redraws screen and also redraws unlock indicator
 *
 */
void redraw_unlock_indicator(void) {
    draw_unlock_indicator();
    redraw_screen();
}

/*
 * Hides the unlock indicator completely when there is no content in the
 * password buffer.
 *
 */
void clear_indicator(void) {
    if (input_position == 0) {
        unlock_state = STATE_STARTED;
    } else 
        unlock_state = STATE_KEY_PRESSED;
    redraw_unlock_indicator();
}

/*
 * Clears the old unlock indicator surface so
 */

void resize_screen(void) {
    cairo_surface_destroy(unlock_indicator_surface);
    unlock_indicator_surface = NULL;
}