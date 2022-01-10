#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "utilities.h"

const char BOTNAME[] = "EmbaBy85 Robot";

void toggleMusic(GtkWidget *widget, int x){
    if (backgroundMusic) {
        backgroundMusic = 0;
        gtk_label_set_text(displayMessage, "Turning off music... please wait!");
    } else {
        backgroundMusic = 1;
        gtk_label_set_text(displayMessage, "");
        pthread_create(&musicThread, NULL, play_music, 1);
        GtkWidget *image = gtk_image_new_from_file("Images/logo.png");
        gtk_button_set_image(logo, image);
    }
}

int cont = 0;
void Menu_Button_Click(GtkWidget *widget, gpointer data){
    if (gtk_widget_is_visible(frame_new)) gtk_widget_hide(frame_new);
    if (gtk_widget_is_visible(frame_load)) gtk_widget_hide(frame_load);
    if (gtk_widget_is_visible(frame_save)) gtk_widget_hide(frame_save);
    if (gtk_widget_is_visible(frame_top)) gtk_widget_hide(frame_top);
    if (gtk_widget_is_visible(frame_game)) gtk_widget_hide(frame_game);

    if (data == 0 && !cont){
        gtk_widget_show(frame_new);
        logging("INFO", "User clicked on NEW GAME button.");
        stopGame();
    } else if (data == 1){
        gtk_widget_show(frame_load);
        logging("INFO", "User clicked on LOAD GAME button.");
        if (game_in_progress){
            gtk_button_set_label(GTK_BUTTON(btn_new), "CONTINUE");
            cont = 1;
        }
    } else if (data == 2){
        gtk_widget_show(frame_save);
        logging("INFO", "User clicked on SAVE GAME button.");
        if (game_in_progress){
            gtk_button_set_label(GTK_BUTTON(btn_new), "CONTINUE");
            cont = 1;
        }
    } else if (data == 3){
        logging("INFO", "User clicked on TOP 10 button.");
        sqlite3_open(DBNAME, &db);
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS scores(name text, score int);", 0, 0, 0);
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT * FROM scores ORDER BY score DESC;", -1, &stmt, NULL);
        for(int i=2; i<=11; i++){
            sqlite3_step(stmt);
            if (sqlite3_column_int(stmt, 1) == 0) break;
            char player_name[200];
            sprintf(player_name, "%s", sqlite3_column_text(stmt, 0));
            gtk_label_set_text(top10_names[i-2], player_name);
            char scorex[10];
            sprintf(scorex, "%d", sqlite3_column_int(stmt, 1));
            gtk_label_set_text(top10_scores[i-2], scorex);
        }
        sqlite3_finalize(stmt);
        gtk_widget_show(frame_top);
        logging("DEBUG", "TOP 10 list successfully retrieved");
        if (game_in_progress){
            gtk_button_set_label(GTK_BUTTON(btn_new), "CONTINUE");
            cont = 1;
        }
    } else if (data == 4){
        char row[1000];
        if (!loading){
            gchar *player1_name; gchar *player2_name;
            player1_name = gtk_entry_get_text(GTK_ENTRY(textbox_p1));
            player2_name = gtk_entry_get_text(GTK_ENTRY(textbox_p2));
            createPlayer(player1_name, player2_name);
            sprintf(row, "Game started: %s VS %s", player1_name, player2_name);
        } else sprintf(row, "Game loaded: %s VS %s", player1.name, player2.name);
        logging("INFO", row);
        updateGameInfo(0);
        gtk_widget_show(frame_game);
        gtk_button_set_label(GTK_BUTTON(btn_new), "NEW GAME");
    } else if (cont){
        gtk_widget_show(frame_game);
        gtk_button_set_label(GTK_BUTTON(btn_new), "NEW GAME");
        cont = 0;
    }
}

void SaveLoad_Button_Click(GtkWidget *widget, gpointer data){
    char log[70];
    if (data < 4){ // SAVE
        if (!game_in_progress){
            gtk_label_set_text(displayMessage, "Start a game first!");
            return;
        }
        char array[300];
        int first = 1;
        for (int i=0; i<boxesY*2+1; i++){
            for (int j=0; j<boxesY+1; j++){
                if (!first) sprintf(array, "%s %d ", array, game_bits[i][j]);
                else {
                    sprintf(array, "%d ", game_bits[i][j]);
                    first = 0;
                }
            }
        }
        saveGame(data, game_level, array, nowPlayingBIT);
        sprintf(log, "Saving game at slot #%d", data);
        Menu_Button_Click(btn_new, 0);
    } else { // LOAD
        if (loadGame(data - 3)){
            updateGameInfo(0);
            if (player2.name == BOTNAME)
                game_mode = 1;
            sprintf(log, "Loading game from slot #%d", data-3);
        } else {
            sprintf(log, "No game saved at slot #%d!", data-3);
        }
    }

    gtk_label_set_text(displayMessage, log);
    logging("INFO", log);
}


void Selection_Click(GtkWidget *widget,gpointer data){
    if (data == 0){
        gtk_widget_set_name(btn_beginner, "NEW_TAB_SELECTED");
        gtk_widget_set_name(btn_expert, "NEW_TAB");
        game_level = 2;
    } else if (data == 1){
        gtk_widget_set_name(btn_beginner, "NEW_TAB");
        gtk_widget_set_name(btn_expert, "NEW_TAB_SELECTED");
        game_level = 5;
    } else if (data == 2){
        gtk_widget_set_name(btn_single, "NEW_TAB_SELECTED");
        gtk_widget_set_name(btn_multiplayer, "NEW_TAB");
        gtk_entry_set_text(textbox_p2, BOTNAME);
        gtk_widget_set_sensitive(textbox_p2, FALSE);
        game_mode = 1;
    } else if (data == 3){
        gtk_widget_set_name(btn_single, "NEW_TAB");
        gtk_widget_set_name(btn_multiplayer, "NEW_TAB_SELECTED");
        gtk_entry_set_text(textbox_p2, "");
        gtk_widget_set_sensitive(textbox_p2, TRUE);
        game_mode = 2;
    }
}

void startTimer(){
    if (game_in_progress)
        game_time += 1;
    int mins = game_time / 60;
    int secs = game_time - (mins * 60);
    char time[10];
    sprintf(time, "%d : %d", mins, secs);
    gtk_label_set_text(lbl_time, time);
}

int main(){
    logging("DEBUG", "Initializing game window");
    gtk_init(NULL, NULL);
    load_css();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete_event", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
    gtk_window_set_title(window, "Dots & Boxes Game");
    gtk_window_set_resizable(window, FALSE);
    gtk_window_set_icon_from_file(window, "Images/icon.png", NULL);

    // MENU BAR
    table = gtk_table_new(4, 4, 0);
    btn_new = gtk_button_new_with_mnemonic("NEW GAME");
    btn_load = gtk_button_new_with_mnemonic("LOAD GAME");
    btn_save = gtk_button_new_with_mnemonic("SAVE GAME");
    btn_top10 = gtk_button_new_with_mnemonic("TOP 10");

    gtk_widget_set_name(btn_new, "MENU");
    gtk_widget_set_name(btn_load, "MENU");
    gtk_widget_set_name(btn_save, "MENU");
    gtk_widget_set_name(btn_top10, "MENU");
    logo = gtk_button_new();
    GtkWidget *image = gtk_image_new_from_file ("images/logo.png");
    gtk_button_set_image(logo, image);
    gtk_widget_set_name(logo, "LOGO");
    g_signal_connect(logo, "clicked", G_CALLBACK(toggleMusic), "");
    gtk_table_attach(GTK_TABLE(table), logo, 0, 4, 0, 1, 1, 4, GTK_FILL, 0);
    gtk_table_attach(GTK_TABLE(table), btn_new, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), btn_load, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), btn_save, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), btn_top10, 3, 4, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_container_add(window, table);
    frame_new = gtk_frame_new("");
    frame_load = gtk_frame_new("");
    frame_save = gtk_frame_new("");
    frame_top = gtk_frame_new("");
    frame_game = gtk_frame_new("");
    gtk_table_attach(GTK_TABLE(table), frame_new, 0, 4, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), frame_load, 0, 4, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), frame_save, 0, 4, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), frame_top, 0, 4, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), frame_game, 0, 4, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_set_col_spacings(table, 5);
    gtk_table_set_row_spacing(table, 0, 25);

    // ERROR LABEL;
    displayMessage = gtk_label_new("");
    gtk_widget_set_name(displayMessage, "error");
    gtk_table_attach(GTK_TABLE(table), displayMessage, 0, 4, 2, 3, 1, 4, GTK_FILL, 0);

    // "SAVE GAME" TAB
    GtkWidget *table_save, *label_title_save, *btn_save1, *btn_save2, *btn_save3;
    table_save = gtk_table_new(4, 1, 0);
    label_title_save = gtk_label_new("CHOOSE A SLOT TO SAVE THE GAME");
    btn_save1 = gtk_button_new_with_mnemonic("Slot 1");
    btn_save2 = gtk_button_new_with_mnemonic("Slot 2");
    btn_save3 = gtk_button_new_with_mnemonic("Slot 3");

    gtk_widget_set_name(label_title_save, "SAVELoad");
    gtk_widget_set_name(btn_save1, "SAVELoad");
    gtk_widget_set_name(btn_save2, "SAVELoad");
    gtk_widget_set_name(btn_save3, "SAVELoad");

    gtk_table_attach(GTK_TABLE(table_save), label_title_save, 0, 4, 0, 1, 1, 4, GTK_FILL, 0);
    gtk_table_attach(GTK_TABLE(table_save), btn_save1, 0, 4, 1, 2, 1, 4, GTK_FILL, 0);
    gtk_table_attach(GTK_TABLE(table_save), btn_save2, 0, 4, 2, 3, 1, 4, GTK_FILL, 0);
    gtk_table_attach(GTK_TABLE(table_save), btn_save3, 0, 4, 3, 4, 1, 4, GTK_FILL, 0);
    gtk_container_add(frame_save, table_save);

    // "LOAD GAME" TAB
    GtkWidget *table_load, *label_title_load, *btn_load1, *btn_load2, *btn_load3;
    table_load = gtk_table_new(4, 1, 0);
    label_title_load = gtk_label_new("CHOOSE A SLOT TO LOAD A GAME");
    btn_load1 = gtk_button_new_with_mnemonic("Slot 1");
    btn_load2 = gtk_button_new_with_mnemonic("Slot 2");
    btn_load3 = gtk_button_new_with_mnemonic("Slot 3");

    gtk_widget_set_name(label_title_load, "SAVELoad");
    gtk_widget_set_name(btn_load1, "SAVELoad");
    gtk_widget_set_name(btn_load2, "SAVELoad");
    gtk_widget_set_name(btn_load3, "SAVELoad");

    gtk_table_attach(GTK_TABLE(table_load), label_title_load, 0, 4, 0, 1, 1, 4, GTK_FILL, 0);
    gtk_table_attach(GTK_TABLE(table_load), btn_load1, 0, 4, 1, 2, 1, 4, GTK_FILL, 0);
    gtk_table_attach(GTK_TABLE(table_load), btn_load2, 0, 4, 2, 3, 1, 4, GTK_FILL, 0);
    gtk_table_attach(GTK_TABLE(table_load), btn_load3, 0, 4, 3, 4, 1, 4, GTK_FILL, 0);
    gtk_container_add(frame_load, table_load);

    // LOAD / SAVE BUTTONS
    g_signal_connect(btn_save1, "clicked", G_CALLBACK(SaveLoad_Button_Click), 1);
    g_signal_connect(btn_save2, "clicked", G_CALLBACK(SaveLoad_Button_Click), 2);
    g_signal_connect(btn_save3, "clicked", G_CALLBACK(SaveLoad_Button_Click), 3);
    g_signal_connect(btn_load1, "clicked", G_CALLBACK(SaveLoad_Button_Click), 4);
    g_signal_connect(btn_load2, "clicked", G_CALLBACK(SaveLoad_Button_Click), 5);
    g_signal_connect(btn_load3, "clicked", G_CALLBACK(SaveLoad_Button_Click), 6);


    // "NEW GAME" TAB
    frame_new_table = gtk_table_new(5, 3, 0);
    label_level = gtk_label_new("Game Level:");
    label_mode = gtk_label_new("Game Mode:");
    label_p1 = gtk_label_new("Player 1 Name:");
    label_p2 = gtk_label_new("Player 2 Name:");
    btn_beginner = gtk_button_new_with_mnemonic("Beginner");
    btn_expert = gtk_button_new_with_mnemonic("Expert");
    btn_single = gtk_button_new_with_mnemonic("Single Player");
    btn_multiplayer = gtk_button_new_with_mnemonic("Multiplayer");
    btn_start = gtk_button_new_with_mnemonic("START GAME");
    textbox_p1 = gtk_entry_new();
    textbox_p2 = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(frame_new_table), label_level, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), label_mode, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), label_p1, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), label_p2, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), textbox_p1, 1, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), textbox_p2, 1, 3, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), btn_beginner, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), btn_expert, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), btn_single, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), btn_multiplayer, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(frame_new_table), btn_start, 0, 3, 4, 5, 5, GTK_FILL, 0, 0);

    gtk_widget_set_name(label_level, "NEW_TAB");
    gtk_widget_set_name(label_mode, "NEW_TAB");
    gtk_widget_set_name(label_p1, "NEW_TAB");
    gtk_widget_set_name(label_p2, "NEW_TAB");
    gtk_widget_set_name(btn_start, "STARTGAME");
    gtk_widget_set_name(textbox_p1, "NEW_TAB");
    gtk_widget_set_name(textbox_p2, "NEW_TAB");
    gtk_widget_set_name(btn_expert, "NEW_TAB");
    gtk_widget_set_name(btn_single, "NEW_TAB");
    gtk_widget_set_name(btn_multiplayer, "NEW_TAB");
    gtk_widget_set_name(btn_beginner, "NEW_TAB");

    gtk_container_add(frame_new, frame_new_table);

    // GAME TAB
    game_main_table = gtk_table_new(1, 3, 0);

    game_ltable = gtk_table_new(11, 1, 0);
    game_rtable = gtk_table_new(55, 55, 0);
    game_lframe = gtk_frame_new("");
    game_rframe = gtk_frame_new("");
    gtk_widget_set_vexpand(game_lframe, TRUE); gtk_widget_set_hexpand(game_lframe, TRUE);
    gtk_widget_set_name(game_rframe, "GameTable");
    gtk_table_attach(GTK_TABLE(game_main_table), game_lframe, 0, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(game_main_table), game_rframe, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_container_add(frame_game, game_main_table);
    gtk_container_add(game_lframe, game_ltable);
    gtk_container_add(game_rframe, game_rtable);
    gtk_widget_set_halign(game_rframe, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(game_rframe, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(game_rtable, TRUE); gtk_widget_set_hexpand(game_rtable, TRUE);
    lbl_NAME1 = gtk_label_new("PLAYER NAME 1");
    lbl_VS = gtk_label_new("VS");
    lbl_NAME2 = gtk_label_new("PLAYER NAME 2");
    lbl_SCORE1 = gtk_label_new("SCORE 1 (BEST: 0)\nMoves: 0");
    lbl_SCORE2 = gtk_label_new("SCORE 2 (BEST: 0)\nMoves: 0");
    gtk_label_set_justify(GTK_LABEL(lbl_SCORE1),GTK_JUSTIFY_CENTER);
    gtk_label_set_justify(GTK_LABEL(lbl_SCORE2),GTK_JUSTIFY_CENTER);
    lbl_turn_title = gtk_label_new("Now Playing:");
    lbl_turn = gtk_label_new("PLAYER 1");
    lbl_remaining_bricks = gtk_label_new("0 Lines Remaining");
    lbl_time = gtk_label_new("00:00");
    btn_undo = gtk_button_new_with_mnemonic("UNDO");
    btn_redo = gtk_button_new_with_mnemonic("REDO");

    gtk_widget_set_name(lbl_NAME1, "GAME_TAB_NAME_RED");
    gtk_widget_set_name(lbl_NAME2, "GAME_TAB_NAME_YELLOW");
    gtk_widget_set_name(lbl_SCORE1, "GAME_TAB_SCORE");
    gtk_widget_set_name(lbl_SCORE2, "GAME_TAB_SCORE");
    gtk_widget_set_name(lbl_turn, "GAME_TAB_NAME");
    gtk_widget_set_name(btn_undo, "GAME_TAB");
    gtk_widget_set_name(btn_redo, "GAME_TAB");
    gtk_widget_set_name(lbl_VS, "GAME_TAB_VS");
    gtk_widget_set_name(lbl_turn_title, "NOW_PLAYING");
    gtk_widget_set_name(lbl_time, "TIME");

    gtk_table_attach(GTK_TABLE(game_ltable), lbl_time,              0, 1, 0, 1, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), lbl_remaining_bricks,  0, 1, 1, 2, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), lbl_NAME1,             0, 1, 2, 3, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), lbl_SCORE1,            0, 1, 3, 4, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), lbl_VS,                0, 1, 4, 5, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), lbl_NAME2,             0, 1, 5, 6, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), lbl_SCORE2,            0, 1, 6, 7, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), lbl_turn_title,        0, 1, 7, 8, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), lbl_turn,              0, 1, 8, 9, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), btn_undo,              0, 1, 9, 10, 1, 4, 0, 0);
    gtk_table_attach(GTK_TABLE(game_ltable), btn_redo,              0, 1, 10, 11, 1, 4, 0, 0);

    // TOP 10 TAB
    GtkWidget *top10_table;

    top10_table = gtk_table_new(10, 3, 0);
    gtk_container_add(frame_top, top10_table);

    for (int i=0; i<10; i++){
        GtkWidget *rank;
        top10_names[i] = gtk_label_new("");
        top10_scores[i] = gtk_label_new("");
        if (i != 9){
            char r[5];
            sprintf(r, "%c", i+1+'0');
            rank = gtk_label_new(r);
        } else {
            rank = gtk_label_new("10");
        }
        gtk_widget_set_name(top10_names[i], "SCORE");
        gtk_widget_set_name(top10_scores[i], "SCORE");
        gtk_widget_set_name(rank, "SCORE");
        gtk_table_attach(GTK_TABLE(top10_table), rank, 0, 1, i+2, i+3, 1, 4, 0, 0);
        gtk_table_attach(GTK_TABLE(top10_table), top10_names[i], 1, 2, i+2, i+3, 1, 4, 0, 0);
        gtk_table_attach(GTK_TABLE(top10_table), top10_scores[i], 2, 3, i+2, i+3, 1, 4, 0, 0);
    }

    // BUTTON ASSIGNMENTS
    g_signal_connect(btn_new, "clicked", G_CALLBACK(Menu_Button_Click), 0);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(Menu_Button_Click), 1);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(Menu_Button_Click), 2);
    g_signal_connect(btn_top10, "clicked", G_CALLBACK(Menu_Button_Click), 3);

    g_signal_connect(btn_beginner, "clicked", G_CALLBACK(Selection_Click), 0);
    g_signal_connect(btn_expert, "clicked", G_CALLBACK(Selection_Click), 1);
    g_signal_connect(btn_single, "clicked", G_CALLBACK(Selection_Click), 2);
    g_signal_connect(btn_multiplayer, "clicked", G_CALLBACK(Selection_Click), 3);

    g_signal_connect(btn_start, "clicked", G_CALLBACK(createGame), NULL);

    g_signal_connect(btn_undo, "clicked", G_CALLBACK(undoRedo), 1);
    g_signal_connect(btn_redo, "clicked", G_CALLBACK(undoRedo), 0);

    gtk_widget_show_all(window);
    gtk_widget_set_vexpand(frame_new, TRUE);
    gtk_widget_hide(frame_load); gtk_widget_set_vexpand(frame_load, TRUE);
    gtk_widget_hide(frame_save); gtk_widget_set_vexpand(frame_save, TRUE);
    gtk_widget_hide(frame_top); gtk_widget_set_vexpand(frame_top, TRUE);
    gtk_widget_hide(frame_game); gtk_widget_set_vexpand(frame_game, TRUE);

    GdkCursor* c = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_TARGET);
    gdk_window_set_cursor(gtk_widget_get_window(window), c);

    logging("INFO", "GUI successfully created.");
    pthread_create(&musicThread, NULL, play_music, 1);
    g_timeout_add(1000, startTimer, NULL);
    gtk_main();
    return 0;
}
