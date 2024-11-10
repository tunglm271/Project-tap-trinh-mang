#include <gtk/gtk.h>

void on_button_clicked(GtkButton *button, gpointer user_data) {
    g_print("Button clicked!\n");
}

int main(int argc, char *argv[]) {
    GtkBuilder *builder;
    GtkWidget *window;

    gtk_init(&argc, &argv);

    // Tạo đối tượng GtkBuilder và tải tệp Glade
    builder = gtk_builder_new_from_file("client/homepage.glade");

    // Lấy đối tượng window từ Glade
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_builder_connect_signals(builder, NULL);

    // Hiển thị cửa sổ
    gtk_widget_show_all(window);

    // Chạy vòng lặp chính của GTK
    gtk_main();

    return 0;
}
