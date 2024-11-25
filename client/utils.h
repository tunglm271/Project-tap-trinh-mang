#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>
#include <arpa/inet.h>

void create_app_socket(int *sock, struct sockaddr_in *serv_addr);
void load_css(GtkWidget *widget);
void remove_all_children(GtkContainer *container);
void add_css_class_to_widget(GtkWidget *widget, const gchar *css_class);
void play_sound_effect(const char *file_path);
void remove_child_by_name(GtkContainer *container, const gchar *name);
#endif // UTILS_H