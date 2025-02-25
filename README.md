# Introduction

A video frame extractor that extract selected frame and produce 2 images (colored and grayscale). This program is written for assignment 6 of COMP 4800 (Dr.Chen).

# Team member

Ngoc Doanh Tran - tran97@uwindsor.ca - 110092702
Hai Nguyen - nguyen4e@uwindsor.ca - 110092844

# #Demo

At the 1000th frame of the video, we have a image of pooh bear standing in front of a mirror.

![Image](https://github.com/user-attachments/assets/123b4460-394e-4977-b3a3-3d95a7dd9277)

After running the program, we have 2 version images of the frame, one is colored, one is grayscale.

![Image](https://github.com/user-attachments/assets/fbeb7333-5394-46ae-9067-1ff18bce7288)

# File Structure

main.c ---------- main program

dppgm.c --------- pgm image displayer

dpppm.c --------- ppm image displayer

# Input file structure

The input file is MP4 video with any name.

# Output file structure

The out file is pgm and ppm image, both have the name of "input.\*" since they are inputs for the display program.

# Compiling and Run

To start the program, run the following command:

```
gcc $(pkg-config --cflags gtk4 glib-2.0 ) -o main main.c $(pkg-config --libs gtk4 glib-2.0 libavcodec libavfilter libavformat libavdevice libavutil libswresample libswscale) -lm -lpthread

gcc $(pkg-config --cflags gtk4 glib-2.0 ) -o dppgm dppgm.c $(pkg-config --libs gtk4 glib-2.0 libavcodec libavfilter libavformat libavdevice libavutil libswresample libswscale) -lm -lpthread

gcc $(pkg-config --cflags gtk4 glib-2.0 ) -o dpppm dpppm.c $(pkg-config --libs gtk4 glib-2.0 libavcodec libavfilter libavformat libavdevice libavutil libswresample libswscale) -lm -lpthread

./main
```

Or run Automaiton Script file at run.sh (the input is default of input.mp4 - video, 1000th frame and 0.3 0.3 0.3 as coefficient)

```
./run.sh
```

# Dependencies

GTK -------- https://download.gnome.org/sources/gtk/

GLib -------- https://download.gnome.org/sources/glib/

Pango -------- https://download.gnome.org/sources/pango/

Gdk-pixbuf -------- https://download.gnome.org/sources/gdk-pixbuf/

ATK -------- https://download.gnome.org/sources/atk/

GObject-Introspection -------- https://download.gnome.org/sources/gobject-introspection/

Epoxy -------- https://download.gnome.org/sources/libepoxy/

FFmpeg -------- https://ffmpeg.org/download.html

# Installation

For Linux
| Distribution | Binary package | Development package | Additional packages |
| :---: | :---: | :---: | :---: |
| Arch | gtk4 | - |- |
| Debian/Ubuntu | libgtk-4-1 | libgtk-4-dev | gtk-4-examples |
| Fedora | gtk4 | gtk4-devel | - |

For furthermore:
https://www.gtk.org/docs/installations/linux

# References

https://www.gnu.org/software/libiconv/#TOCdownloading

https://www.cairographics.org

https://en.wikipedia.org/wiki/K-means_clustering
