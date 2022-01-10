#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

sqlite3 *db;

struct player {
    int currentScore;
    int bestScore;
    char name[200];
    int moves;
} player1, player2, winner;

int getScore(char *name){

    sqlite3_open("scores.db", &db);
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

void updateScore(char *name, int score){
    sqlite3_open("scores.db", &db);
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

void createPlayer(char *player1_name, char *player2_name){
    char p1name[200], p2name[200];
    sprintf(p1name, "%s", player1_name);
    sprintf(p2name, "%s", player2_name);
    for(int i=0; i<=200; i++){
        player1.name[i] = p1name[i];
        if (player1.name[i] == '\0') break;
    }
    for(int i=0; i<=200; i++){
        player2.name[i] = p2name[i];
        if (player2.name[i] == '\0') break;
    }
    player1.currentScore = 0;
    player1.bestScore = getScore(player1_name);

    player2.currentScore = 0;
    player2.bestScore = getScore(player2_name);
}
