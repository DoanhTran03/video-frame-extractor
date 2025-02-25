#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define IMAGE_FILE "input.ppm"

// Function to load a PPM (P6) file and return an RGB buffer
static uint8_t *load_ppm(const char *filename, int *width, int *height) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        g_printerr("Failed to open file: %s\n", filename);
        return NULL;
    }

    // Read the header
    char magic[3];
    int max_val;
    if (fscanf(file, "%2s\n%d %d\n%d\n", magic, width, height, &max_val) != 4 || strcmp(magic, "P6") != 0) {
        g_printerr("Invalid or unsupported PPM format\n");
        fclose(file);
        return NULL;
    }

    // Allocate memory for RGB pixel data
    int rowstride = (*width) * 3;  // 3 bytes per pixel (RGB)
    uint8_t *rgb_data = (uint8_t *)malloc(rowstride * (*height));
    if (!rgb_data) {
        g_printerr("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read pixel data
    fread(rgb_data, 1, rowstride * (*height), file);
    fclose(file);
    return rgb_data;
}

// Function to create a scaled GdkTexture from the image data
static GdkTexture *create_scaled_texture(const char *filename, int target_width, int target_height) {
    int width, height;
    uint8_t *rgb_data = load_ppm(filename, &width, &height);
    if (!rgb_data) return NULL;

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(rgb_data, GDK_COLORSPACE_RGB, FALSE, 8,
                                                 width, height, width * 3, free, NULL);
    if (!pixbuf) {
        g_printerr("Failed to create GdkPixbuf\n");
        return NULL;
    }

    // Scale the image to fit the window while keeping aspect ratio
    GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, target_width, target_height, GDK_INTERP_BILINEAR);
    g_object_unref(pixbuf);  // Free original pixbuf

    // Convert GdkPixbuf to GdkTexture for GTK4 compatibility
    GdkTexture *texture = gdk_texture_new_for_pixbuf(scaled_pixbuf);
    g_object_unref(scaled_pixbuf);  // Free scaled pixbuf

    return texture;
}

// GTK Activate Callback
static void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *picture;
    GdkTexture *texture;
    int window_width = 800, window_height = 600;

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PPM Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), window_width, window_height);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // Load and scale the image
    texture = create_scaled_texture(IMAGE_FILE, window_width, window_height);
    if (!texture) {
        g_printerr("Failed to load and scale image\n");
        return;
    }

    // Display the image using GtkPicture
    picture = gtk_picture_new_for_paintable(GDK_PAINTABLE(texture));
    g_object_unref(texture);  // Free texture after setting it to GtkPicture
    gtk_window_set_child(GTK_WINDOW(window), picture);

    gtk_widget_show(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.ppmviewer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
