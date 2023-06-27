/*
Spodaj napisan program je namenjen za projektno nalogo pri predmetu Programiranje mikrokrmilnikov na Fakulteti za elektrotehniko na Univerzi Ljubljana.
S projektom sem naredil igro Space Invaders, ki je originalno izšla na Atari konzoli. Moj namen je bil narediti podobno igro z ASCII znaki, vmes sem si
pa dovolil nekatere stvari spremeniti, kot je generacija ovir, namreč sem hotel v nadaljevanju naresti še posamezne levele, vendar zaradi urnika na faksu,
kolokvijev in drugih obštudijskih projektov, ni uspelo. Zaradi tega je program napisan tako, da se mu na lahek način spreminja število ovir in sovražnikov.
V primeru, da mi bo čez poletje dolgčas, se bom s programom še kaj igral.

Princip delovanja igre temelji na enem while loopu, v katerem kličem funkcije, dokler igralec ne zmaga ali je ubit. Te funkcije kontrolirajo premik
sovražnikov, igralca, prihod podatkov in število objektov na igralni površini, poleg tega, ostale funkcijo priskrbijo za pravilno inicializacijo igralca,
ovir in sovražnikov. Vse logične spremenljivke sem zapisal v obliki structov medtem, ko sem pa zapis konstant zapisal kot posamezne globalne spremenljivke.

Ko se bom spet podal v podoben projekt, bom izkoristil raje class namesto structa, saj bi mi dovolil uporabo metod, ki bi olajšale delo. Poleg tega me
je začel zanimati tudi multi-threading, ker trenutno uporabljam metodo busy wait, ki temelji na linearnem izvajanju in čakanju na naslednjo izvedbo loopa.
*/

//Deklaracija knjižnic
#include <iostream>
#include <conio.h>
#include <windows.h>
//Deklaracija le nekatrih funkcij std
using std::cout;
using std::cin;
using std::endl;

//Globalne spremenljivke, večinoma konstantne vrednosti. Če se bo program nadgrajeval v prihodnjosti in dodalo različne levele, bodo spremenljivke, ki določajo število sovražnikov in ovir, premaknjene v main(), kjer se bo njihovo število inicializiranih spreminjalo.
const unsigned int WIDTH = 40;
const unsigned int HEIGHT = 20;
const unsigned int ENEMY_START_X = 1;
const unsigned int ENEMY_START_Y = 1;
const unsigned int ENEMY_SPACING = 4;
const unsigned int OBSTACLE_START_X = 1;
const unsigned int OBSTACLE_START_Y = HEIGHT - 5;
const unsigned int OBSTACLE_SPACING = 1;
const unsigned int numOfEnemies = 5;
const unsigned int enemyRows = 3;
const unsigned int totalNumOfEn = enemyRows * numOfEnemies;
const unsigned int numOfObstacles = WIDTH - 2;
const unsigned int obstacleRows = 2;
const unsigned int totalNumOfOb = obstacleRows * numOfObstacles;
bool gameOver = false;
int unsigned score = 0;

//Definirana struktura igralca 
struct player {
    unsigned int playerX = WIDTH / 2; //X os igralca
    unsigned int playerY = HEIGHT - 1; //Y os igralca
    unsigned int bulletX = playerX; // X os metka
    unsigned int bulletY = playerY - 1; // Y os metka
    bool isFiring = false; //Stanje streljanja
};

//Definirana struktura sovražnikov (glej strukturo igralca)
struct enemy {
    int unsigned enemyX = 0;
    int unsigned enemyY = 0;
    int unsigned enemyBulletX = 0;
    int unsigned enemyBulletY = 0;
    bool enemyIsFiring = false;
    bool isAlive = true; //Stanje sovražnika - mrtev ali živ
    bool enemyBelow = false; //Stanje sovražnika pod trenutnim - mrtev ali ne / ali sploh je
};

//Definirana struktura ovire (glej strukturo igralca)
struct obstacle {
    unsigned int obstacleX = 0;
    unsigned int obstacleY = 0;
    bool isThere = false;
};

//Funkcija za izrisevanje igralne podlage, ki se jo generira preko cout funkcije (C++ ekvivalent printf(), ki deluje preko overflowanja <<)
void drawBoard(player p1, enemy enemies[], obstacle obstacles[]) {
    //čiščenje cmdja
    COORD coord;
    coord.X = 0;
    coord.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    //Izriše zgornji del
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    //Itiracija skozi stolpce
    for (int i = 0; i < HEIGHT; i++) {
        cout << "|";
        //Iteracija skozi vrstice
        for (int j = 0; j < WIDTH; j++) {
            bool isObstacle = false; //Ali je ovira tam ali ne. Če je, izriše #
            for (int m = 0; m < totalNumOfOb; m++) {
                if (i == obstacles[m].obstacleY && j == obstacles[m].obstacleX && obstacles[m].isThere) {
                    cout << "#";
                    isObstacle = true;
                    break;
                }
            }
            //Če ni ovire, pogleda če je tam igralec ali sovražnik ali metek
            if (!isObstacle) {
                if (i == p1.playerY && j == p1.playerX) {
                    cout << "A";
                }
                else if (i == p1.bulletY && j == p1.bulletX) {
                    cout << "*";
                }
                else {
                    bool enemyAlive = false; //Ali je sovražnik živ. Če je, izpiše M, drugače pa presledek
                    for (int k = 0; k < totalNumOfEn; k++) {
                        if (i == enemies[k].enemyY && j == enemies[k].enemyX) {
                            cout << "M";
                            enemyAlive = true;
                            break;
                        }
                        else if (i == enemies[k].enemyBulletY && j == enemies[k].enemyBulletX) {
                            cout << "*";
                            enemyAlive = true;
                            break;
                        }
                    }
                    if (!enemyAlive) {
                        cout << " ";
                    }
                }
            }
        }
        //Okvir podlage
        cout << "|" << endl;
    }
    //Spodnji del podlage
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    //Izpiše rezultat
    cout << "Score: " << score << endl;
}

//Funckija inicalizacije igralca
void playerInit(player& p1) {
    p1.playerX = WIDTH / 2;
    p1.playerY = HEIGHT - 1;
    p1.isFiring = false;
}

//Funckija inicalizacije sovražnikov. Pozicija je določena glede na njihovo število po vrsticah 
void enemyInit(enemy enemies[]) {
    for (int j = 0; j < enemyRows; j++) {
        for (int i = 0; i < numOfEnemies; i++) {
            enemies[j * numOfEnemies + i].enemyX = ENEMY_START_X + i * ENEMY_SPACING;
            enemies[j * numOfEnemies + i].enemyY = ENEMY_START_Y + j;
            enemies[j * numOfEnemies + i].enemyIsFiring = false;
            enemies[j * numOfEnemies + i].isAlive = true;
        }
    }
}

//Funckija inicalizacije ovir, ki poteka podobno kot pri sovražnik le, da imajo določeno možnost generiranja.
void obstacleInit(obstacle obstacles[]) {
    srand(time(NULL));
    unsigned int obstacleChance; //Možnost generiranja
    for (int j = 0; j < obstacleRows; j++) {
        for (int i = 0; i < numOfObstacles; i++) {
            obstacleChance = rand() % 2;
            if (obstacleChance == 0) { //Se pojavijo
                obstacles[j * numOfObstacles + i].obstacleX = OBSTACLE_START_X + i * OBSTACLE_SPACING;
                obstacles[j * numOfObstacles + i].obstacleY = OBSTACLE_START_Y + j;
                obstacles[j * numOfObstacles + i].isThere = true;
            }
            else { //Se ne pojavijo
                obstacles[j * numOfObstacles + i].obstacleX = OBSTACLE_START_X + i * OBSTACLE_SPACING;
                obstacles[j * numOfObstacles + i].obstacleY = OBSTACLE_START_Y + j;
                obstacles[j * numOfObstacles + i].isThere = false;
            }
        }
    }

}

//Funkcija premika igralca, ki spreminja X koordinato igralca, in njegovo stanje streljanja. Dodana je tudi funkcija exit(0) za prekinitev igre 
void movePlayer(char input, player& p1) {
    switch (input) {
    case 'a':
        if (p1.playerX > 0) {
            p1.playerX--;
        }
        break;
    case 'd':
        if (p1.playerX < WIDTH - 1) {
            p1.playerX++;
        }
        break;
    case ' ':
        if (!p1.isFiring) {
            p1.bulletX = p1.playerX;
            p1.bulletY = p1.playerY + 1;
            p1.isFiring = true;
        }
        break;
    case 'q':
        exit(0);
        break;
    }
}

//Funkcija za premik in stanje streljanja sovražnikov
void moveEnemies(enemy enemies[], player& p1) {
    static int enemyDirection = 1; //Smer premikanja
    for (int i = 0; i < totalNumOfEn; i++) {
        if (enemies[i].isAlive) { //Ali je konkreten sovražnik živ, drugače se ga ignorira
            if (enemies[i].enemyX <= 0) { // Če zadane rob, spremeni smer in iterira eno vrstico dol
                enemyDirection = 1;
                for (int j = 0; j < enemyRows; j++) {
                    for (int i = 0; i < numOfEnemies; i++) {
                        enemies[j * numOfEnemies + i].enemyY++;
                    }
                }
                break;
            }
            else if (enemies[i].enemyX == WIDTH - 1) { // Če zadane rob, spremeni smer in iterira eno vrstico dol
                enemyDirection = -1;
                for (int j = 0; j < enemyRows; j++) {
                    for (int i = 0; i < numOfEnemies; i++) {
                        enemies[j * numOfEnemies + i].enemyY++;
                    }
                }
                break;
            }
            //Pripis vrednosti, če je sovražnik pod sovražnikom
            for (int j = i + numOfEnemies; j < totalNumOfEn; j += numOfEnemies) {
                if (enemies[j].isAlive) {
                    enemies[j].enemyBelow = true;

                }
            }
            //Če ni sovražnika pod sovražnikom in če sovražnik ne strela, sovražnik lahko strelja
            if (enemies[i].enemyIsFiring == false && !enemies[i].enemyBelow) {
                srand(time(NULL));
                int chance = rand() % 5;
                if (chance == 0) { //sovražnik strelja, če je vrednost chance deljiva s 5
                    enemies[i].enemyIsFiring = true;
                    enemies[i].enemyBulletX = enemies[i].enemyX;
                    enemies[i].enemyBulletY = enemies[i].enemyY + 1;
                }
            }
            //Če sovražnik pride do iste vrstice kot je igralec je igre konec
            if (enemies[i].enemyY == p1.playerY) {
                gameOver = true;
            }
        }
    }
    //Pripis smeri premikanja h koordinatam igralca
    for (int i = 0; i < totalNumOfEn; i++) {
        enemies[i].enemyX += enemyDirection;
    }
}

//Funkcija premikanja metkov
void moveBullets(player& p1, enemy enemies[], obstacle obstacles[]) {
    if (p1.isFiring) {
        p1.bulletY--; //Premik metka navzgor
        if (p1.bulletY <= 0) {
            p1.isFiring = false; //Igralec ne strelja več, ker so metki prišli do zgornje limite
            p1.bulletX = 100; //X koordinato nastavi na 100, kar ga efektivno zbriše
            p1.bulletY = 100; //Y koordinato nastavi na 100, kar ga efektivno zbriše
        }
        //Igralec zadane sovražnika
        for (int i = 0; i < totalNumOfEn; i++) {
            if (p1.bulletY == enemies[i].enemyY && p1.bulletX == enemies[i].enemyX) {
                p1.isFiring = false;
                enemies[i].enemyX = 100;
                enemies[i].enemyY = 100;
                p1.bulletX = 100;
                p1.bulletY = 100;
                score += 10; //Poveča točke
                enemies[i].isAlive = false; //Sovražnik je umrl
            }
        }
    }
    //Premik sovražnikovega metka
    for (int i = 0; i < totalNumOfEn; i++) {
        if (enemies[i].enemyIsFiring) {
            enemies[i].enemyBulletY++; //Premik sovražnikovega metka navzdol
            if (enemies[i].enemyBulletY > HEIGHT) {
                enemies[i].enemyIsFiring = false; //Sovražnik ne strelja več, ker je so metki prišli do spodnje limite
            }
            if (enemies[i].enemyBulletY == p1.playerY && enemies[i].enemyBulletX == p1.playerX) { //V primeru, da sovražnik zadane igralca, igralec zgubi
                gameOver = true;
                break;
            }
        }
    }
    for (int i = 0; i < totalNumOfOb; i++) {
        //Igralec zadane oviro
        if (p1.bulletY == obstacles[i].obstacleY && p1.bulletX == obstacles[i].obstacleX && obstacles[i].isThere) {
            p1.isFiring = false;
            obstacles[i].obstacleX = 100;
            obstacles[i].obstacleY = 100;
            p1.bulletX = 100;
            p1.bulletY = 100;
            score += 1;
            obstacles[i].isThere = false;
        }
        //Sovražnik zadane oviro
        for (int j = 0; j < totalNumOfEn; j++) {
            if (enemies[j].enemyBulletY == obstacles[i].obstacleY && enemies[j].enemyBulletX == obstacles[i].obstacleX && obstacles[i].isThere) {
                obstacles[i].obstacleX = 100;
                obstacles[i].obstacleY = 100;
                enemies[j].enemyBulletX = 100;
                enemies[j].enemyBulletY = 100;
                enemies[j].enemyIsFiring = false;
                obstacles[i].isThere = false;
            }
        }
    }
}

//Funkcija, ki vrne število živih sovražnikov
int getCurrentEnemies(enemy enemies[]) {
    int alive = 0;
    for (int i = 0; i < totalNumOfEn; i++) {
        if (enemies[i].isAlive) {
            alive++;
        }
    }
    return alive;
}

int main() {
    srand(time(NULL)); //Funkcija ki prične timer za slučajno generacijo
    struct player player1; //Dekleracija igralca
    struct obstacle obstacles[totalNumOfOb]; //Dekleracija ovir
    struct enemy enemies[totalNumOfEn]; //Dekleracija sovražnikov
    playerInit(player1); //klic funkcije za določitev začetnih vrednosti
    enemyInit(enemies); //klic funkcije za določitev začetnih vrednosti
    obstacleInit(obstacles); //klic funkcije za določitev začetnih vrednosti
    char input; //Dekleracija podatkov za premik
    unsigned int timer = 0; //Inicializacija timerja, pomemben za časovni premik
    int numOfAlive; //Dekleracija vrednosti živih sovražnikov
    while (!gameOver) { //Dokler igre ni konec - konec je podan znotraj funkcij
        numOfAlive = getCurrentEnemies(enemies);
        //Nastavitev hitrosti premika sovražnikov
        if (numOfAlive < 5) {
            if (timer % 2 == 0) {
                moveEnemies(enemies, player1);
            }
        }
        //Nastavitev hitrosti premika sovražnikov
        else {
            if (timer % 5 == 0) {
                moveEnemies(enemies, player1);
            }
        }
        moveBullets(player1, enemies, obstacles);
        drawBoard(player1, enemies, obstacles);
        if (_kbhit()) { //Proži se, če je tipkovnica zazna spremembno stanja
            input = _getch(); //Pripiše vrednost udarjane tipke v input
            movePlayer(input, player1);
        }
        //Tukaj je napisan konec pogoj za zmago
        if (numOfAlive == 0) { //Zmaga se, pod pogojem, da je število sovražnikov enako nič
            cout << "YOU HAVE WON";
            Sleep(3000);
            exit(0); //Konča program
        }
        timer++; //Povečanje vrednosti timerja
        Sleep(10); //Uravnavanje hitrosti igre/Busy wait
    }
    cout << "GAME OVER";
    Sleep(3000);
    return 0;
}  