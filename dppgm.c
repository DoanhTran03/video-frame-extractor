#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define IMAGE_FILE "input.pgm"  // Change this to your PGM file

// Function to load a PGM file and return grayscale buffer
static uint8_t *load_pgm(const char *filename, int *width, int *height) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        g_printerr("❌ Failed to open file: %s\n", filename);
        return NULL;
    }

    // Read the header
    char magic[3];
    int max_val;
    if (fscanf(file, "%2s\n%d %d\n%d\n", magic, width, height, &max_val) != 4 || strcmp(magic, "P5") != 0) {
        g_printerr("❌ Invalid or unsupported PGM format\n");
        fclose(file);
        return NULL;
    }

    // Allocate memory for grayscale data
    uint8_t *gray_data = (uint8_t *)malloc((*width) * (*height));
    if (!gray_data) {
        g_printerr("❌ Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read grayscale pixel data
    fread(gray_data, 1, (*width) * (*height), file);
    fclose(file);
    return gray_data;
}

// Function to convert grayscale buffer to RGB (GdkPixbuf requires RGB format)
static uint8_t *convert_grayscale_to_rgb(uint8_t *gray_data, int width, int height) {
    uint8_t *rgb_data = (uint8_t *)malloc(width * height * 3);
    if (!rgb_data) {
        g_printerr("❌ Memory allocation for RGB data failed\n");
        return NULL;
    }

    // Convert grayscale (1 channel) to RGB (3 channels)
    for (int i = 0; i < width * height; i++) {
        rgb_data[i * 3] = gray_data[i];     // R
        rgb_data[i * 3 + 1] = gray_data[i]; // G
        rgb_data[i * 3 + 2] = gray_data[i]; // B
    }

    return rgb_data;
}

// Function to create a scaled GdkTexture from the image data
static GdkTexture *create_scaled_texture(const char *filename, int target_width, int target_height) {
    int width, height;
    uint8_t *gray_data = load_pgm(filename, &width, &height);
    if (!gray_data) return NULL;

    uint8_t *rgb_data = convert_grayscale_to_rgb(gray_data, width, height);
    free(gray_data);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(rgb_data, GDK_COLORSPACE_RGB, FALSE, 8,
                                                 width, height, width * 3, free, NULL);
    if (!pixbuf) {
        g_printerr("❌ Failed to create GdkPixbuf\n");
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

static void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *picture;
    GdkTexture *texture;
    int window_width = 800, window_height = 600;

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PGM Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), window_width, window_height);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // Load and scale the image
    texture = create_scaled_texture(IMAGE_FILE, window_width, window_height);
    if (!texture) {
        g_printerr("❌ Failed to load and scale image\n");
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

    app = gtk_application_new("com.example.pgmviewer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
