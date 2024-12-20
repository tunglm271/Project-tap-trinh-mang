#include "app_function.h"
#include "utils.h"
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "../server/data/rooms.h"

#define PORT 8080
#define BUFFER_SIZE 8192


int number_rooms; 
int sock;
struct sockaddr_in serv_addr;
char buffer[BUFFER_SIZE] = {0};
static guint countdown_timeout_id = 0;
GtkWidget *window;
GtkWidget *main_box;
Room rooms[MAX_ROOMS];
char user_name[1024];
int client_id;
int g_room_id = -1;
int isLost = 0;

GameData user_game_data;

const char *money_labels[] = {
    "150.000.000", "85.000.000", "60.000.000", 
    "40.000.000", "30.000.000", "22.000.000", 
    "14.000.000", "10.000.000", "6.000.000", 
    "3.000.000", "2.000.000", "1.000.000", 
    "600.000", "400.000", "200.000", "0"
};

void restart_game_data() {
    user_game_data.current_point = 15;
    user_game_data.is_first_question = TRUE;
    user_game_data.is_hot_seat = FALSE;
    user_game_data.is_50_50_used = FALSE;
    user_game_data.is_call_friend_used = FALSE;
    user_game_data.is_ask_people_used = FALSE;
}

void render_welcome_page(const gchar *username, int roomId);
void render_rooms();
void render_summary_page(bool isGiveUp);
void render_question(GtkButton *button);
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void handle_time_up(GtkDialog *dialog, gint response_id, gpointer user_data);
void handle_give_up(GtkButton *button);
void handle_50_50(GtkWidget *widget, gpointer data);
void handle_call_friend(GtkWidget *widget);
void on_endgame_dialog_response(GtkDialog *dialog, gint response_id, gpointer endgame_data);
void on_eliminate_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void handle_give_up(GtkButton *button) {
    // xu li khi nguoi choi bo cuoc
    render_summary_page(TRUE);
}


void handle_time_up(GtkDialog *dialog, gint response_id, gpointer user_data) {
    gtk_widget_destroy(GTK_WIDGET(dialog));
    //Xu li het thoi gian o day
    render_summary_page(FALSE);
}

gboolean update_countdown(gpointer user_data) {
    CountdownData *data = (CountdownData *)user_data;
    time_t now = time(NULL);

    int time_left = (int)difftime(data->end_time, now);
    if (time_left >= 0) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2d", time_left);
        gtk_label_set_text(data->label, buffer);
        return TRUE; // Continue the timeout
    } else {
        gtk_label_set_text(data->label, "00");
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                        GTK_MESSAGE_ERROR, 
                        GTK_BUTTONS_OK, 
                        NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gchar *label_text = g_strdup_printf("Time's up!\nYou won %s$", money_labels[user_game_data.current_point+1]);
        if(user_game_data.is_first_question) {
            label_text = g_strdup_printf("Time's up!\nYou lost!");
        }
        
        GtkWidget *dialog_label = gtk_label_new(label_text);
        g_free(label_text);
        gtk_widget_set_name(dialog_label, "dialog-text");
        gtk_container_add(GTK_CONTAINER(content_area), dialog_label);
        gtk_widget_show(dialog_label);      
        g_signal_connect(dialog, "response", G_CALLBACK(handle_time_up), NULL);
        gtk_dialog_run(GTK_DIALOG(dialog));
        if(user_game_data.is_first_question) {
            render_rooms();
        }
        return FALSE; // Stop the timeout
    }
}


void on_endgame_dialog_response(GtkDialog *dialog, gint response_id, gpointer data) {
    g_print("You won %s", money_labels[user_game_data.current_point]);
    gtk_widget_destroy(GTK_WIDGET(dialog));
    render_summary_page(FALSE);
    user_game_data.current_point = 14;
}


void on_eliminate_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    g_print("OK button clicked\n");
    gtk_widget_destroy(GTK_WIDGET(dialog));
    render_rooms();
}


void handle_call_friend(GtkWidget *widget) {
    gtk_widget_hide(widget);
    user_game_data.is_call_friend_used = TRUE;
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x11;
    send(sock, buffer, BUFFER_SIZE, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    if(buffer[0] == 0x12) printf("%s\n", buffer+1);
    char correct_answer = buffer[1] - '0' + 'A';
    
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
                    GTK_DIALOG_DESTROY_WITH_PARENT, 
                    GTK_MESSAGE_INFO, 
                    GTK_BUTTONS_OK, 
                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));\
    gchar *dialog_text = g_strdup_printf("I think the answer is %c", correct_answer);
    GtkWidget *dialog_label = gtk_label_new(dialog_text);
    gtk_widget_set_name(dialog_label, "dialog-text");
    gtk_container_add(GTK_CONTAINER(content_area), dialog_label);
    gtk_widget_show(dialog_label);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_dialog_run(GTK_DIALOG(dialog));
}


void handle_ask_people_in_studio(GtkWidget *widget) {
    gtk_widget_hide(widget);
    user_game_data.is_ask_people_used = TRUE;

    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x11;
    send(sock, buffer, BUFFER_SIZE, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    if(buffer[0] == 0x12) printf("%s\n", buffer+1);
    int correct_answer = buffer[1] - '0';

    int votes[4] = {0, 0, 0, 0};
    int remaining_votes = 100;
    votes[correct_answer] = rand() % (remaining_votes - 3 * 10) + 40; 
    remaining_votes -= votes[correct_answer];

    for (int i = 0; i < 4; i++) {
        if (i != correct_answer) {
            votes[i] = rand() % (remaining_votes - (2 - i) * 10) + 10; 
            remaining_votes -= votes[i];
        }
    }
    votes[3] += remaining_votes;
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
                    GTK_DIALOG_DESTROY_WITH_PARENT, 
                    GTK_MESSAGE_INFO, 
                    GTK_BUTTONS_OK, 
                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    for (int i = 0; i < 4; i++) {
        gchar *label_text = g_strdup_printf("Option %c: %d%%", 'A' + i, votes[i]);
        GtkWidget *label = gtk_label_new(label_text);
        add_css_class_to_widget(label, "dialog-label");
        GtkWidget *progress_bar = gtk_progress_bar_new();
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), votes[i] / 100.0);
        gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), progress_bar, 1, i, 1, 1);
        g_free(label_text);
    }

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_dialog_run(GTK_DIALOG(dialog));
}

void handle_answer(GtkButton *button, gpointer answerData) {

    if(countdown_timeout_id) {
        g_source_remove(countdown_timeout_id);
        countdown_timeout_id = 0;
    }

    gpointer *data = (gpointer *)answerData;
    int answer = GPOINTER_TO_INT(data[0]);
    printf("Answer chosen: %d\n", answer);
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x08;
    sprintf(buffer + 1, "%d", answer);
    send(sock, buffer, BUFFER_SIZE, 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);

    if (buffer[0] == 0x09) {
        user_game_data.is_first_question = FALSE;
        if(user_game_data.current_point == 0) {
        render_question(NULL);
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                        GTK_MESSAGE_INFO, 
                        GTK_BUTTONS_OK, 
                        NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *dialog_label = gtk_label_new("Congratulations! \nYou are a millionaire!");
        gtk_widget_set_name(dialog_label, "dialog-text");
        gtk_container_add(GTK_CONTAINER(content_area), dialog_label);
        gtk_widget_show(dialog_label);
        g_signal_connect(dialog, "response", G_CALLBACK(on_endgame_dialog_response), NULL);
        gtk_dialog_run(GTK_DIALOG(dialog));
       } else {
        render_question(NULL);
        gtk_widget_set_name(GTK_WIDGET(button), "right-answer");
        g_usleep(200000);
        user_game_data.current_point--;
       }
    } else {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), 
                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                        GTK_MESSAGE_ERROR, 
                        GTK_BUTTONS_OK, 
                        NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *dialog_label;
        if(user_game_data.is_first_question == TRUE) {
            dialog_label = gtk_label_new("Incorrect answer!\nYou're out of the game!");
            g_signal_connect(dialog, "response", G_CALLBACK(on_eliminate_dialog_response), NULL);
        } else {
            dialog_label = gtk_label_new("Incorrect answer!");
            g_signal_connect(dialog, "response", G_CALLBACK(on_endgame_dialog_response), NULL);
        }
        gtk_widget_set_name(dialog_label, "dialog-text");
        gtk_container_add(GTK_CONTAINER(content_area), dialog_label);
        gtk_widget_show(dialog_label);      
        gtk_dialog_run(GTK_DIALOG(dialog));
    }
}


void handle_50_50(GtkWidget *widget, gpointer data) {
    
    user_game_data.is_50_50_used = TRUE;
    gtk_widget_hide(widget);

    GtkWidget **buttons = (GtkWidget **)data;
    
    const gchar *button_texts[4];
    for (int i = 0; i < 4; i++) {
        button_texts[i] = gtk_button_get_label(GTK_BUTTON(buttons[i]));
        printf("Button %d text: %s\n", i, button_texts[i]);
    }

    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x11;
    send(sock, buffer, BUFFER_SIZE, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    if(buffer[0] == 0x12) printf("%s\n", buffer+1);
    int correct_answer = buffer[1] - '0';
    printf("Correct answer: %d\n", correct_answer);
    int eliminated = 0;
    int eliminated_indices[2] = {-1, -1};
    srand(time(NULL));
    while (eliminated < 2) {
        int rand_index = rand() % 4;
        if (rand_index != correct_answer && rand_index != eliminated_indices[0] && rand_index != eliminated_indices[1]) {
            gtk_button_set_label(GTK_BUTTON(buttons[rand_index]), "");
            gtk_widget_set_sensitive(buttons[rand_index], FALSE);
            eliminated_indices[eliminated] = rand_index;
            eliminated++;
        }
    }
}

void render_question(GtkButton *button) {
    
    remove_all_children(GTK_CONTAINER(main_box));

    if(countdown_timeout_id) {
        g_source_remove(countdown_timeout_id);
        countdown_timeout_id = 0;
    }

    gtk_orientable_set_orientation(GTK_ORIENTABLE(main_box), GTK_ORIENTATION_HORIZONTAL);

    GtkWidget *question_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(question_section, "question_section");

    GtkWidget *money_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(money_section, 200, -1);
    gtk_widget_set_name(money_section, "money_section");

    GtkWidget *wrapper_grid = gtk_grid_new();
    gtk_widget_set_size_request(wrapper_grid, 100, -1); 

    // Create the countdown label
    GtkWidget *countdown_label = gtk_label_new("30");
    gtk_widget_set_name(countdown_label, "countdown-label");

    // Add the label to the grid
    gtk_grid_attach(GTK_GRID(wrapper_grid), countdown_label, 0, 0, 1, 1);

    // Add the wrapper grid to the question section
    gtk_box_pack_start(GTK_BOX(question_section), wrapper_grid, FALSE, FALSE, 10);

    // Initialize countdown data
    CountdownData *countdown_data = g_malloc(sizeof(CountdownData));
    countdown_data->label = GTK_LABEL(countdown_label);
    countdown_data->end_time = time(NULL) + 30;
    // Start the countdown
    countdown_timeout_id = g_timeout_add(1000, (GSourceFunc)update_countdown, countdown_data);
    
    
    
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x07;
    send(sock, buffer, BUFFER_SIZE, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);

    
    char *question = strtok(buffer, "\n"); 
    char *options[4];
    for (int i = 0; i < 4; i++) {
        options[i] = strtok(NULL, "\n");
    }

    // Create a new label with a question
    GtkWidget *question_label = gtk_label_new(question ? question : "No question received");
    gtk_widget_set_name(question_label, "question-label");

    // Create a grid for the answer buttons
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(grid, FALSE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 30);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 70);

    GtkWidget *buttons[4];
    for (int i = 0; i < 4; i++) {
        buttons[i] = gtk_button_new_with_label(options[i]);
        gtk_widget_set_size_request(buttons[i], 150, -1);
        gpointer *answer_data = g_new(gpointer, 1);
        answer_data[0] = GINT_TO_POINTER(i);
        g_signal_connect(buttons[i], "clicked", G_CALLBACK(handle_answer), answer_data);
        gtk_grid_attach(GTK_GRID(grid), buttons[i], i % 2, i / 2, 1, 1);
    }

    // Pack the question label and grid into the question box
    gtk_box_pack_start(GTK_BOX(question_section), question_label, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(question_section), grid, FALSE, FALSE, 10);
    // Create the give up button
    if (user_game_data.is_first_question == FALSE) {
        GtkWidget *give_up_button = gtk_button_new_with_label("Give Up");
        gtk_widget_set_size_request(give_up_button, 100, -1);
        gtk_widget_set_name(give_up_button, "give-up-btn");
        gtk_widget_set_size_request(give_up_button, 150, -1);
        g_signal_connect(give_up_button, "clicked", G_CALLBACK(handle_give_up), NULL);
        gtk_box_pack_start(GTK_BOX(question_section), give_up_button, FALSE, FALSE, 10);
    }

    // =============================
    // Add content to the money list box
    // =============================

    GtkWidget* helpButton = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(helpButton), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(helpButton), 10);
    gtk_widget_set_margin_start(helpButton, 15);  
    gtk_widget_set_margin_end(helpButton, 15);   
    gtk_widget_set_margin_top(helpButton, 10);    
    gtk_widget_set_margin_bottom(helpButton, 10);
    
    GtkWidget* callFriend =  gtk_button_new_with_label("");
    GtkWidget* help_50 =  gtk_button_new_with_label("50:50");
    GtkWidget* askPeople =  gtk_button_new_with_label("");

    gtk_widget_set_name(callFriend,"callFriend");
    gtk_widget_set_name(askPeople,"askPeople");
    add_css_class_to_widget(help_50,"helpOption");

    gpointer *button_data = g_new(gpointer, 4);
    for (int i = 0; i < 4; i++) {
        button_data[i] = buttons[i];
    }
    g_signal_connect(help_50, "clicked", G_CALLBACK(handle_50_50), button_data);
    g_signal_connect(callFriend, "clicked", G_CALLBACK(handle_call_friend), NULL);
    g_signal_connect(askPeople, "clicked", G_CALLBACK(handle_ask_people_in_studio), NULL);

    if(user_game_data.is_50_50_used == FALSE) {
        gtk_grid_attach(GTK_GRID(helpButton), help_50, 0, 0, 1, 1);
    }

    if(user_game_data.is_call_friend_used == FALSE) {
        gtk_grid_attach(GTK_GRID(helpButton), callFriend, 1, 0, 1, 1);
    }

    if(user_game_data.is_ask_people_used == FALSE) {
        gtk_grid_attach(GTK_GRID(helpButton), askPeople, 2, 0, 1, 1);
    }
    gtk_box_pack_start(GTK_BOX(money_section), helpButton, FALSE, FALSE, 5);

    // Create and pack the labels into the box
    for (int i = 0; i < 15; i++) {
        GtkWidget *money_label = gtk_label_new(money_labels[i]);
        
        // Add CSS class to specific labels
        if (i == 0 || i == 5 || i == 10) {
            add_css_class_to_widget(money_label, "milestone");
        }

        if( i == (user_game_data.current_point -1) && user_game_data.is_first_question == FALSE) {
            gtk_widget_set_name(money_label, "current-point");
        }

        // Pack the label into the money section
        gtk_box_pack_start(GTK_BOX(money_section), money_label, FALSE, FALSE, 5);
    }

    gtk_box_pack_start(GTK_BOX(main_box), question_section, TRUE, TRUE, 10);
    if(user_game_data.is_first_question == FALSE) {
        gtk_box_pack_start(GTK_BOX(main_box), money_section, FALSE, FALSE, 0);
    }
    gtk_widget_show_all(main_box);
    
}

void on_server_message(GIOChannel *source, GIOCondition condition, gpointer data) {
    if(condition & G_IO_IN) {  
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        if(buffer[0] == 0x14) {
            render_rooms();
        }
        if (buffer[0] == 0x16) {
            printf("Hello\n");
            render_question(NULL);
        }
        if (buffer[0] == 0x20) {
            isLost = 1;
            render_summary_page(FALSE);
        }
    }
}

void handle_go_back() {
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x24;
    sprintf(buffer+1, "%d", g_room_id);
    printf("%d\n", g_room_id);
    send(sock, buffer, BUFFER_SIZE, 0);
    g_room_id = - 1;
}

void render_loading(GtkButton *button) {
    GtkWidget *spinner;
    GtkWidget *label;

    remove_all_children(GTK_CONTAINER(main_box));
    label = gtk_label_new("Waiting for other players...");
    gtk_box_pack_start(GTK_BOX(main_box), label, TRUE, TRUE, 0);

    GtkWidget *go_back_button = gtk_button_new_with_label("GO BACK");
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), go_back_button, 0, 0, 1, 1);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_box), grid, FALSE, FALSE, 0);
    gtk_widget_set_name(go_back_button, "go-back-btn");
    
    g_signal_connect(go_back_button, "clicked", G_CALLBACK(handle_go_back), NULL);
    
    // Create a spinner
    spinner = gtk_spinner_new();
    gtk_widget_set_name(spinner, "spinner");
    gtk_box_pack_start(GTK_BOX(main_box), spinner, TRUE, TRUE, 0);

    // Start the spinner
    gtk_spinner_start(GTK_SPINNER(spinner));

    // Show all widgets in the window
    gtk_widget_show_all(main_box);
}

void submit_name(GtkButton *button, gpointer user_data) {
    g_print("submit_name\n");
    gpointer *data = (gpointer *)user_data;
    GtkEntry *name_input = (GtkEntry *)data[0];
    GtkEntry *password_input = (GtkEntry *)data[1];
    const gchar *username = gtk_entry_get_text(name_input);
    const gchar *password = gtk_entry_get_text(password_input);
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x01;
    sprintf(buffer+1, "username:%s;password:%s", username, password);
    g_print("%s\n",buffer);
    send(sock, buffer, BUFFER_SIZE, 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    if(buffer[0] == 0x02) {
        printf("dang nhap thanh cong\n");
        sscanf(buffer + 1, "%[^;];%d", user_name, &client_id);
        printf("%s\n", user_name);
        printf("%d\n", client_id);
        // render_welcome_page(username);
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        int received_number_rooms;
        if(buffer[0] == 0x14) {
            memcpy(&received_number_rooms, buffer + 1, sizeof(int));
            memcpy(rooms, buffer + 1 + sizeof(int), sizeof(rooms));
            printf("%d\n", received_number_rooms); 
            number_rooms = received_number_rooms;
            render_rooms();
        }
        g_free(data);
    } else {
        printf("%X\n", buffer[0]);
        remove_child_by_name(GTK_CONTAINER(main_box), "error-label");
        GtkWidget *error_label = gtk_label_new("username or password is incorrect!");
        gtk_widget_set_name(error_label, "error-label");
        gtk_box_pack_start(GTK_BOX(main_box), error_label, FALSE, FALSE, 0);
        gtk_widget_show_all(main_box);
    }
}

void render_question_wrapper(GtkWidget *widget, gpointer data) {
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x19;
    gpointer *newData = (gpointer *)data;
    int roomId = GPOINTER_TO_INT(newData[0]);
    sprintf(buffer+1, "%d", roomId);
    send(sock, buffer, BUFFER_SIZE, 0);
    render_question(NULL);
}

void render_welcome_page(const gchar *text, int roomId) {
    GtkWidget *welcome_text = gtk_label_new(NULL);
    GtkWidget *start_btn = gtk_button_new_with_label("Start game");
    gtk_label_set_text(GTK_LABEL(welcome_text), g_strdup_printf("%s", text));
    gtk_widget_set_name(welcome_text, "welcome-text");
    remove_all_children(GTK_CONTAINER(main_box));

    gtk_box_pack_start(GTK_BOX(main_box), welcome_text, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(main_box), start_btn, TRUE, FALSE, 0);

    gpointer *data = g_new(gpointer, 1);
    data[0] = GINT_TO_POINTER(roomId);
    g_signal_connect(start_btn, "clicked", G_CALLBACK(render_question_wrapper), data);
    // g_signal_connect(start_btn, "clicked", G_CALLBACK(render_question), NULL);
    // g_signal_connect(start_btn, "clicked", G_CALLBACK(render_rooms), NULL);
    // g_signal_connect(start_btn, "clicked", G_CALLBACK(render_summary_page), NULL);
    gtk_widget_show_all(GTK_WIDGET(main_box));  
}


void submit_register(GtkButton *button, gpointer register_data) {
    gpointer *data = (gpointer *)register_data;
    GtkWidget *box = (GtkWidget *)data[0];
    GtkEntry *username_entry = (GtkEntry *)data[1];
    GtkEntry *password_entry = (GtkEntry *)data[2];
    GtkEntry *confirm_password_entry = (GtkEntry *)data[3];
    
    const gchar *username = gtk_entry_get_text(username_entry);
    const gchar *password = gtk_entry_get_text(password_entry);
    const gchar *confirm_password = gtk_entry_get_text(confirm_password_entry);
    
    if (strcmp(password, confirm_password) != 0) {
        GtkWidget *error_label = gtk_label_new("Passwords do not match!");
        gtk_box_pack_start(GTK_BOX(box), error_label, FALSE, FALSE, 0);
        gtk_widget_show_all(box);
    } else { 
        memset(buffer, 0, BUFFER_SIZE);
        buffer[0] = 0x04;
        sprintf(buffer+1, "username:%s;password:%s", username, password);
        send(sock, buffer, BUFFER_SIZE, 0);
        
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        if(buffer[0] == 0x05) {
            printf("dang ky thanh cong\n");
            render_rooms();
        } else printf("dang ky khong thanh cong\n");
    }

    g_free(data);
}


void render_login(GtkButton *button) {
    remove_all_children(GTK_CONTAINER(main_box));

    GtkWidget *labelName = gtk_label_new("Enter username:");
    GtkWidget *labelPassword = gtk_label_new("Enter password");
    GtkWidget *NameInput = gtk_entry_new();
    GtkWidget *PasswordInput = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(PasswordInput), FALSE);
    GtkWidget *SubmitBtn = gtk_button_new_with_label("Login");

    gpointer *user_data = g_new(gpointer, 2);
    user_data[0] = NameInput;
    user_data[1] = PasswordInput;


    // Connect the submit button click event to the on_submit_clicked callback
    g_signal_connect(SubmitBtn, "clicked", G_CALLBACK(submit_name), user_data);
    g_signal_connect(PasswordInput, "activate", G_CALLBACK(submit_name), user_data);


    gtk_box_pack_start(GTK_BOX(main_box), labelName, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(main_box), NameInput, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(main_box), labelPassword, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(main_box), PasswordInput, FALSE, FALSE, 0);  
    gtk_box_pack_start(GTK_BOX(main_box), SubmitBtn, FALSE, FALSE, 0); 
    
    gtk_widget_show_all(main_box);  
}

void render_register(GtkButton *button) {
    remove_all_children(GTK_CONTAINER(main_box));

    GtkWidget *labelUsername = gtk_label_new("Enter username:");
    GtkWidget *labelPassword = gtk_label_new("Enter password:");
    GtkWidget *labelConfirmPassword = gtk_label_new("Confirm password:");
    GtkWidget *username_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE); // Make the text invisible
    GtkWidget *confirm_password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(confirm_password_entry), FALSE); // Make the text invisible
    GtkWidget *submit_btn = gtk_button_new_with_label("Register");

    gpointer *register_data = g_new(gpointer, 4);
    register_data[0] = main_box;
    register_data[1] = username_entry;
    register_data[2] = password_entry;
    register_data[3] = confirm_password_entry;

    g_signal_connect(submit_btn, "clicked", G_CALLBACK(submit_register), register_data);
    g_signal_connect(confirm_password_entry, "activate", G_CALLBACK(submit_register), register_data);

    gtk_box_pack_start(GTK_BOX(main_box), labelUsername, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), username_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), labelPassword, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), password_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), labelConfirmPassword, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), confirm_password_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), submit_btn, FALSE, FALSE, 0);

    gtk_widget_show_all(main_box);  
}

void join_room(GtkWidget *widget, gpointer data) {
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x18;
    send(sock, buffer, BUFFER_SIZE, 0);

    gpointer *newData = (gpointer *)data;
    int roomId = GPOINTER_TO_INT(newData[0]);
    g_room_id = roomId;
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x15;
    sprintf(buffer+1, "%d;%s", roomId, user_name);
    send(sock, buffer, BUFFER_SIZE, 0);
    render_loading(NULL);
    g_print("You have joined the room: %d\n", roomId);

}

void create_room(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog, *content_area, *entry, *create_button, *cancel_button;
    GtkWidget *box;

    // Tạo hộp thoại popup
    dialog = gtk_dialog_new_with_buttons("Create New Room", GTK_WINDOW(window), 
                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Create", GTK_RESPONSE_ACCEPT,
                                        NULL);

    // Thay đổi kích thước hộp thoại
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 100);  // Kích thước 400x200 px

    // Lấy vùng nội dung của hộp thoại
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // Tạo hộp chứa phần tử
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);
    
    // Tạo ô nhập liệu cho tên phòng
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter room name");

    // Thêm ô nhập liệu vào hộp thoại
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 5);

    // Hiển thị hộp thoại và chờ người dùng nhập liệu
    gtk_widget_show_all(dialog);

    // Xử lý sự kiện khi người dùng nhấn "Create" hoặc "Cancel"
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_ACCEPT) {
        // Lấy tên phòng từ ô nhập liệu
        const gchar *room_name = gtk_entry_get_text(GTK_ENTRY(entry));
        printf("Room created: %s\n", room_name);  // In ra tên phòng đã tạo
        memset(buffer, 0, BUFFER_SIZE);
        buffer[0] = 0x13;
        sprintf(buffer+1, "%s\n", room_name);
        send(sock, buffer, BUFFER_SIZE, 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        int roomId = atoi(buffer);

        render_welcome_page("Wait for other people!", roomId);
        
    } else {
        printf("Room creation cancelled.\n");  // In ra nếu người dùng hủy
    }
    

    // Đóng hộp thoại
    gtk_widget_destroy(dialog);
}


void render_play_game() {
    
}

void render_history() {
    remove_all_children(GTK_CONTAINER(main_box));

    GtkWidget *history_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(history_box), 20);

    GtkWidget *title_label = gtk_label_new("Game History");
    gtk_widget_set_name(title_label, "history-title");
    gtk_box_pack_start(GTK_BOX(history_box), title_label, FALSE, FALSE, 10);

    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x22;
    send(sock, buffer, BUFFER_SIZE, 0);

    recv(sock, buffer, BUFFER_SIZE, 0);
    if (buffer[0] == 0x23) {
        GtkWidget *no_history_label = gtk_label_new("No game history available.");
        gtk_box_pack_start(GTK_BOX(history_box), no_history_label, FALSE, FALSE, 5);
    } else {
        GtkWidget *grid = gtk_grid_new();
        gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

        char *line = strtok(buffer, "\n");
        int row = 0;
        while (line != NULL) {
            char username[50];
            char result[50];
            sscanf(line, "%[^;];%s", username, result);
            GtkWidget *username_label = gtk_label_new(username);
            GtkWidget *result_label = gtk_label_new(result);
            gtk_grid_attach(GTK_GRID(grid), username_label, 0, row, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), result_label, 1, row, 1, 1);
            line = strtok(NULL, "\n");
            row++;
        }
        gtk_box_pack_start(GTK_BOX(history_box), grid, FALSE, FALSE, 5);
    }
    // char *history_entry = strtok(buffer, "\n");
    // while (history_entry != NULL) {
    //     GtkWidget *entry_label = gtk_label_new(history_entry);
    //     gtk_widget_set_name(entry_label, "history-entry");
    //     gtk_box_pack_start(GTK_BOX(history_box), entry_label, FALSE, FALSE, 5);
    //     history_entry = strtok(NULL, "\n");
    // }

    // for (int i = 0; i < 5; i++) {
    //     gchar *room_text = g_strdup_printf("Room %d", i + 1);
    //     GtkWidget *room_label = gtk_label_new(room_text);
    //     g_free(room_text);

    //     GtkWidget *money_label = gtk_label_new(money_labels[i]);

    //     gtk_grid_attach(GTK_GRID(grid), room_label, 0, i, 1, 1);
    //     gtk_grid_attach(GTK_GRID(grid), money_label, 1, i, 1, 1);
    // }


    GtkWidget *back_button = gtk_button_new_with_label("Back");
    g_signal_connect(back_button, "clicked", G_CALLBACK(render_rooms), NULL);
    gtk_box_pack_start(GTK_BOX(history_box), back_button, FALSE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(main_box), history_box, TRUE, TRUE, 0);
    gtk_widget_show_all(main_box);
}


void render_rooms() {
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x17;
    send(sock, buffer, BUFFER_SIZE, 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    int received_number_rooms;
    memcpy(&received_number_rooms, buffer + 1, sizeof(int));
    memcpy(rooms, buffer + 1 + sizeof(int), sizeof(rooms));
    printf("%d\n", received_number_rooms); 
    number_rooms = received_number_rooms;

    remove_all_children(GTK_CONTAINER(main_box));
    if(countdown_timeout_id) {
        g_source_remove(countdown_timeout_id);
        countdown_timeout_id = 0;
    }



    gtk_orientable_set_orientation(GTK_ORIENTABLE(main_box), GTK_ORIENTATION_VERTICAL);
    restart_game_data();

    GtkWidget *rooms_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *room_box_left, *room_box_right;
    GtkWidget *room_label;
    GtkWidget *join_btn;
    GtkWidget *people_label;
    GtkWidget *create_room_btn;
    GtkWidget *view_history_btn;

    // Xóa tất cả các phần tử con hiện tại trong container chính
    remove_all_children(GTK_CONTAINER(main_box));

    // Tạo các box con để chia layout thành 2 cột
    room_box_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);  // Cột bên trái
    room_box_right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // Cột bên phải

    // Duyệt qua mảng các phòng và tạo các phần tử
    for (int i = 0; i < number_rooms; i++) {
        if (rooms[i].playing == 1) {
            continue;
        }
        GtkWidget *room_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);  // Hộp mỗi phòng
        gtk_style_context_add_class(gtk_widget_get_style_context(room_box), "room-box");  // Thêm class cho room_box

        const gchar *room_name = rooms[i].name;
        int people_count = rooms[i].num_users ? rooms[i].num_users : 0;  // Lấy số người trong phòng
        
        // Tạo nhãn tên phòng
        room_label = gtk_label_new(room_name);
        gtk_style_context_add_class(gtk_widget_get_style_context(room_label), "room-label");

        // Tạo nhãn hiển thị số lượng người
        gchar *people_text = g_strdup_printf("People: %d/10", people_count);
        people_label = gtk_label_new(people_text);
        g_free(people_text);
        gtk_style_context_add_class(gtk_widget_get_style_context(people_label), "room-people");

        // Tạo nút tham gia phòng
        join_btn = gtk_button_new_with_label("Join room");
        gtk_style_context_add_class(gtk_widget_get_style_context(join_btn), "join-btn");
        
        gpointer *data = g_new(gpointer, 1);
        data[0] = GINT_TO_POINTER(rooms[i].id);
        // Gắn tín hiệu khi nhấn nút để tham gia phòng
        g_signal_connect(join_btn, "clicked", G_CALLBACK(join_room), data);

        // Thêm các phần tử vào hộp mỗi phòng
        gtk_box_pack_start(GTK_BOX(room_box), room_label, TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(room_box), people_label, TRUE, TRUE, 5);
        gtk_box_pack_start(GTK_BOX(room_box), join_btn, TRUE, TRUE, 5);

        // Chia phòng vào 2 cột, bên trái hoặc bên phải
        if (i % 2 == 0) {
            gtk_box_pack_start(GTK_BOX(room_box_left), room_box, FALSE, FALSE, 10);  // Cột trái
        } else {
            gtk_box_pack_start(GTK_BOX(room_box_right), room_box, FALSE, FALSE, 10); // Cột phải
        }
    }

    // Tạo box chứa 2 cột
    GtkWidget *two_columns_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10); // Chia thành 2 cột
    gtk_box_pack_start(GTK_BOX(two_columns_box), room_box_left, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(two_columns_box), room_box_right, TRUE, TRUE, 10);

    // Thêm vào main_box
    gtk_box_pack_start(GTK_BOX(main_box), two_columns_box, TRUE, TRUE, 10);

    // Tạo nút "Create Room" ở dưới danh sách phòng
    create_room_btn = gtk_button_new_with_label("Create Room");
    gtk_style_context_add_class(gtk_widget_get_style_context(create_room_btn), "create-room-btn");

    // Gắn tín hiệu khi nhấn nút để tạo phòng mới
    g_signal_connect(create_room_btn, "clicked", G_CALLBACK(create_room), NULL);

    view_history_btn = gtk_button_new_with_label("View History");

    // Thêm nút vào main_box dưới 2 cột
    gtk_box_pack_start(GTK_BOX(main_box), create_room_btn, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(main_box), view_history_btn, FALSE, FALSE, 10);
    gtk_widget_set_name(view_history_btn, "view-history-btn");
    g_signal_connect(view_history_btn, "clicked", G_CALLBACK(render_history), NULL);

    gtk_widget_show_all(GTK_WIDGET(main_box));

    GIOChannel *channel = g_io_channel_unix_new(sock);
    g_io_add_watch(channel, G_IO_IN, (GIOFunc)on_server_message, NULL);
}

void render_summary_page(bool isGiveUp) {
    if(countdown_timeout_id) {
        g_source_remove(countdown_timeout_id);
        countdown_timeout_id = 0;
    }
    GtkWidget *summary_box, *amount_label, *home_button;

    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = 0x21;
    sprintf(buffer+1, "%s", money_labels[user_game_data.current_point + 1]);
    send(sock, buffer, BUFFER_SIZE, 0);

    // Tạo container chính cho màn hình tổng kết
    summary_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(summary_box), 20);

    gchar *amount_text;
    if(isGiveUp) {
        amount_text = g_strdup_printf("Congratulations! You earned %s!", money_labels[user_game_data.current_point + 1]);
    } else {
        int mark = ((15 - user_game_data.current_point) / 5 ) * 5;
        g_print("mark: %d\n", mark);
        if(mark == 0) {
            amount_text = g_strdup_printf("Sorry you lost! You earned 0$");
        } else {
            amount_text = g_strdup_printf("Congratulations! You earned %s!", money_labels[15 - mark]);
        }

        if (isLost) {
            amount_text = g_strdup_printf("You lost! Someone answered before you.");
            isLost = 0;
        }
    }
    amount_label = gtk_label_new(amount_text);
    gtk_widget_set_name(amount_label, "amount-label"); // Gán tên để định kiểu bằng CSS

    // Áp dụng CSS cho label
    GtkCssProvider *css_provider = gtk_css_provider_new();
    const gchar *css_data =
        "#amount-label {"
        "    background-color: yellow;"
        "    color: black;"
        "    padding: 10px;"
        "    border-radius: 5px;"
        "    font-size: 18px;"
        "}";
    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(amount_label);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Tạo nút "Quay lại trang chủ"
    home_button = gtk_button_new_with_label("Home");
    g_signal_connect(home_button, "clicked", G_CALLBACK(render_rooms), window); // Chuyển về trang chủ

    // Thêm các widget vào hộp chính
    gtk_box_pack_start(GTK_BOX(summary_box), amount_label, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(summary_box), home_button, FALSE, FALSE, 10);

    // Xóa các phần tử cũ khỏi cửa sổ chính (main_box)
    remove_all_children(GTK_CONTAINER(main_box));

    // Thêm hộp tổng kết vào cửa sổ chính
    gtk_box_pack_start(GTK_BOX(main_box), summary_box, TRUE, TRUE, 0);

    // Hiển thị mọi thứ
    gtk_widget_show_all(main_box);
}


void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *header_bar;
    GtkWidget *title_label;

    GtkWidget *loginButton;
    GtkWidget *signUpButton;

    // Create a new window with the application
    window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_window_close), NULL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Custom Window Title");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    title_label = gtk_label_new("Ai Là Triệu Phú");
    gtk_widget_set_name(title_label, "title");
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), NULL);
    gtk_container_add(GTK_CONTAINER(header_bar), title_label);
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    load_css(window);


    // Create a box container (vertical or horizontal)
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(main_box, "content");

    loginButton = gtk_button_new_with_label("Login");
    signUpButton = gtk_button_new_with_label("Create new account");
  
    // Connect the submit button click event to the on_submit_clicked callback
    g_signal_connect(loginButton, "clicked", G_CALLBACK(render_login), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), loginButton, FALSE, FALSE, 0);  // Add the "Enter your name" label
    gtk_box_pack_start(GTK_BOX(main_box), signUpButton, FALSE, FALSE, 0);  // Add the text entry field
    g_signal_connect(signUpButton, "clicked", G_CALLBACK(render_register), NULL);
    // Add the box container to the window
    gtk_container_add(GTK_CONTAINER(window), main_box);
    // Show all widgets in the window
    gtk_widget_show_all(window);
    create_app_socket(&sock, &serv_addr);

    char cwd[PATH_MAX];  // Store current working directory
    char sound_path[PATH_MAX * 2];  // To store the full path of the sound file
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        g_print("Current working directory: %s\n", cwd);
        snprintf(sound_path, sizeof(sound_path), "%s/%s", cwd, "client/assets/intro.ogg");
        play_sound_effect(sound_path);
    }
}

