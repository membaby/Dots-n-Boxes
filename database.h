#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <ctype.h>
#include "variables.h"

// Initalize a STRUCTURE to store data of the players and the winner
struct player {
    int currentScore;
    int bestScore;
    char name[200];
    int moves;
} player1, player2, winner;

// Input: Player Name (Case Insensitive)
// Output: Returns INT score of player from database
int getScore(char *name){
    sqlite3_open(DBNAME, &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS scores(name text, score int);", 0, 0, 0);
    char query[200];
    sqlite3_stmt *stmt;
    sprintf(query, "SELECT score FROM scores WHERE name='%s'", name);
    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    sqlite3_step(stmt);
    int score = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return score;
}

// Input: Player Name & Score
// Output: Updates score of player in database if new score is higher than best score.
void updateScore(char *name, int score){
    sqlite3_open(DBNAME, &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS scores(name text, score int);", 0, 0, 0);
    char query[200];
    sprintf(query, "SELECT * FROM scores WHERE name='%s'", name);
    sqlite3_exec(db, query, 0, 0, 0);

    if (getScore(name) != score && score != 0){
        if (getScore(name) == 0){
            sprintf(query, "INSERT INTO scores VALUES ('%s', %d);", name, score);
            sqlite3_exec(db, query, 0, 0, 0);
        } else {
            sprintf(query, "UPDATE scores SET score=%d WHERE name='%s'", score, name);
            sqlite3_exec(db, query, 0, 0, 0);
        }
        logging("DEBUG", query);
    }
    sqlite3_exec(db, "END TRANSACTION;", 0, 0, 0);
}

// Input: Slot ID and Game Settings
// Output: Game is saved to database.
void saveGame(int id, int gamelevel, char array[], int nowPlaying){
    sqlite3_open(DBNAME, &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS `game` (id INT NOT NULL, difficulity INT, player1 TEXT, player2 TEXT, array TEXT, playingBIT INT, gametime INT, PRIMARY KEY (`id`));", 0, 0, 0);
    sqlite3_exec(db, "INSERT INTO `game` VALUES (1, 0, '0', '0', '0', 0, 0)", 0, 0, 0);
    sqlite3_exec(db, "INSERT INTO `game` VALUES (2, 0, '0', '0', '0', 0, 0)", 0, 0, 0);
    sqlite3_exec(db, "INSERT INTO `game` VALUES (3, 0, '0', '0', '0', 0, 0)", 0, 0, 0);
    char query[500];
    sprintf(query, "UPDATE `game` SET difficulity=%d, player1='%s', player2='%s', array='%s', playingBIT=%d, gametime=%d WHERE id=%d",
            gamelevel, player1.name, player2.name, array, nowPlaying, game_time, id);
    logging("DEBUG", query);
    sqlite3_exec(db, query, 0, 0, 0);
    sqlite3_exec(db, "END TRANSACTION;", 0, 0, 0);
    logging("DEBUG", "Game has been saved!");
}

// Input: Slot ID
// Output: Loads and starts game saved in slot
int loadGame(int id){
    sqlite3_open(DBNAME, &db);
    char query[200];
    sqlite3_stmt *stmt;
    sprintf(query, "SELECT * FROM `game` WHERE id=%d", id);
    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    sqlite3_step(stmt);
    if (!sqlite3_column_int(stmt, 1)) return 0;
    
    loading = 1;
    redoundo = 1;
    stopGame();

    game_level = sqlite3_column_int(stmt, 1);
    game_time = sqlite3_column_int(stmt, 6);
    createPlayer(sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3));
    createGame();
    int ii = 0;
    for (int i=0; i<game_level*2+1; i++){
        for (int j=0; j<game_level+1; j++){
            while (1){
                int XYZ;
                if (sqlite3_column_text(stmt, 4)[ii] == 48) XYZ = 0;
                else if (sqlite3_column_text(stmt, 4)[ii] == 49) XYZ = 1;
                else if (sqlite3_column_text(stmt, 4)[ii] == 50) XYZ = 2;
                else if (sqlite3_column_text(stmt, 4)[ii] == 32) {
                    ii += 1;
                    continue;
                }

                if (XYZ == 1 || XYZ == 2){

                    if (XYZ == 1) nowPlayingBIT = 0;
                    else if (XYZ == 2) nowPlayingBIT = 1;

                    if (i == 0 || i % 2 == 0){
                        if (game[i][j*2+1]){
                            g_object_set_data(G_OBJECT(game[i][j*2+1]), "y", GINT_TO_POINTER(i));
                            toggleBrick(game[i][j*2+1], j);
                        }
                    } else {
                        if (game[i][j*2]){
                            g_object_set_data(G_OBJECT(game[i][j*2]), "y", GINT_TO_POINTER(i));
                            toggleBrick(game[i][j*2], j);
                        }
                    }
                }
                ii += 1;
                break;
            }
        }
    }
    nowPlayingBIT = sqlite3_column_int(stmt, 5);
    sqlite3_finalize(stmt);
    loading = 0;
    redoundo = 0;
    logging("DEBUG", "Game has been loaded!");
    return 1;
}

// Input: Player Name
// Output: Player name with title case to be used in sql queries to stop case sensitivity.
void title_case(char *s)
{
    if (s == NULL) return;
    for (char *p = s; *p != '\0'; ++p)
        *p = (p == s || *(p-1) == ' ') ? toupper(*p) : tolower(*p);
}

// Updates data in `player` sruct to match current game settings.
// Input: Players Names
// Output: Player1 and Player2 are loaded with new data.
void createPlayer(char *player1_name, char *player2_name){
    char p1name[200], p2name[200];
    sprintf(p1name, "%s", player1_name);
    sprintf(p2name, "%s", player2_name);
    title_case(p1name);
    title_case(p2name);

    for(int i=0; i<=200; i++){
        player1.name[i] = p1name[i];
        if (player1.name[i] == '\0') break;
    }
    for(int i=0; i<=200; i++){
        player2.name[i] = p2name[i];
        if (player2.name[i] == '\0') break;
    }
    player1.currentScore = 0;
    player1.bestScore = getScore(p1name);
    player1.moves = 0;

    player2.currentScore = 0;
    player2.bestScore = getScore(p2name);
    player2.moves = 0;
}
