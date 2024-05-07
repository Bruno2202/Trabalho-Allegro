// FEITO POR Bruno Terribile e Kauyse

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>

const int WIDTH = 800;
const int HEIGHT = 600;
const int PLAYER_SIZE = 40;
const int PLAYER_SPEED = 5;
const int BULLET_SIZE = 10;
const int BULLET_SPEED = 10;
const int MAX_BULLETS = 5;
const int ASTEROID_SIZE = 40;
int ASTEROID_MIN_SPEED = 2; // Velocidade mínima dos asteroides
int ASTEROID_MAX_SPEED = 12; // Velocidade máxima dos asteroides
const int MAX_ASTEROIDS = 20;

class Player {
public:
    Player(float x, float y) : x(x), y(y), moveLeft(false), moveRight(false) {}

    void moveLeftStart() { moveLeft = true; }
    void moveLeftStop() { moveLeft = false; }
    void moveRightStart() { moveRight = true; }
    void moveRightStop() { moveRight = false; }

    void update() {
        if (moveLeft && x > 0) {
            x -= PLAYER_SPEED;
        }
        if (moveRight && x < WIDTH - PLAYER_SIZE) {
            x += PLAYER_SPEED;
        }
    }

    void fireBullet(std::vector<std::pair<float, float>>& bullets) {
        if (bullets.size() < MAX_BULLETS) {
            // Calcular a posição do tiro na ponta superior central da nave
            float bulletX = x + (PLAYER_SIZE - BULLET_SIZE) / 2; // Centralizando o tiro na nave
            float bulletY = y - BULLET_SIZE; // Ajuste para que o tiro saia da ponta superior
            bullets.push_back(std::make_pair(bulletX, bulletY));
        }
    }


    float getX() const { return x; }
    float getY() const { return y; }
    int getSize() const { return PLAYER_SIZE; }

private:
    float x, y;
    bool moveLeft, moveRight;
};

class Asteroid {
public:
    Asteroid(float x, float y, int speed) : x(x), y(y), alive(true), speed(speed) {}

    void move() { y += speed; }
    void setSpeed(int newSpeed) { speed = newSpeed; }
    float getX() const { return x; }
    float getY() const { return y; }
    bool isOffScreen() const { return y > HEIGHT; }
    bool isAlive() const { return alive; }
    void destroy() { alive = false; }

    bool checkCollision(float otherX, float otherY, int otherSize) const {
        float distX = x - std::max(otherX + 10, std::min(x, otherX + otherSize - 10));
        float distY = y - std::max(otherY + 10, std::min(y, otherY + otherSize - 10));
        return (distX * distX + distY * distY) < (ASTEROID_SIZE * ASTEROID_SIZE);
    }

private:
    float x, y;
    bool alive;
    int speed;
};

void drawPlayerShip(float x, float y, float size) {
    // Corpo principal da nave
    al_draw_filled_triangle(x - size / 2, y + size / 2, x, y - size / 2, x + size / 2, y + size / 2, al_map_rgb(0, 0, 255));

    // Parte traseira
    al_draw_filled_rectangle(x - size / 4, y + size / 2, x + size / 4, y + size / 2 + size / 4, al_map_rgb(100, 100, 100));

    // Motor
    al_draw_filled_rectangle(x - size / 8, y + size / 4, x + size / 8, y + size / 2, al_map_rgb(255, 0, 0));
}


void drawStars(int numStars) {
    for (int i = 0; i < numStars; ++i) {
        float x = rand() % WIDTH;
        float y = rand() % HEIGHT;
        al_draw_pixel(x, y, al_map_rgb(255, 255, 255));
    }
}

void drawAsteroid(float x, float y, float size) {
    // Desenho de um asteroide em formato de pentágono irregular
    float radius = size / 2;
    al_draw_filled_triangle(x - radius * 0.5, y - radius, // Vertice 1
        x + radius, y,              // Vertice 2
        x - radius * 0.5, y + radius, // Vertice 3
        al_map_rgb(128, 128, 128));
    al_draw_filled_triangle(x - radius * 0.5, y - radius, // Vertice 1
        x - radius * 1.5, y,        // Vertice 4
        x - radius * 0.5, y + radius, // Vertice 3
        al_map_rgb(128, 128, 128));
    al_draw_filled_triangle(x - radius * 1.5, y,        // Vertice 4
        x + radius, y,              // Vertice 2
        x - radius * 0.5, y + radius, // Vertice 3
        al_map_rgb(128, 128, 128));
}

int main() {
    ALLEGRO_DISPLAY* display = nullptr;
    ALLEGRO_EVENT_QUEUE* event_queue = nullptr;
    ALLEGRO_TIMER* timer = nullptr;

    if (!al_init()) {
        return -1;
    }

    display = al_create_display(WIDTH, HEIGHT);
    if (!display) {
        return -1;
    }

    al_init_primitives_addon();
    al_init_font_addon();

    al_install_keyboard();

    event_queue = al_create_event_queue();
    timer = al_create_timer(1.0 / 60);

    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_display_event_source(display));

    Player player(WIDTH / 2 - PLAYER_SIZE / 2, HEIGHT - PLAYER_SIZE - 10);
    std::vector<std::pair<float, float>> bullets;
    std::vector<Asteroid> asteroids;

    srand(time(NULL));

    int destroyedAsteroids = 0;

    al_start_timer(timer);
    bool done = false;
    while (!done) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (asteroids.size() < MAX_ASTEROIDS) {
                float x = rand() % WIDTH;
                float y = -ASTEROID_SIZE;
                int speed = ASTEROID_MIN_SPEED + rand() % (ASTEROID_MAX_SPEED - ASTEROID_MIN_SPEED + 1); // Velocidade aleatória entre ASTEROID_MIN_SPEED e ASTEROID_MAX_SPEED
                asteroids.push_back(Asteroid(x, y, speed));
            }

            for (auto& asteroid : asteroids) {
                asteroid.move();
            }

            for (size_t i = 0; i < asteroids.size(); ++i) {
                if (asteroids[i].isOffScreen() || !asteroids[i].isAlive()) {
                    asteroids.erase(asteroids.begin() + i);
                    --i;
                }
                else if (asteroids[i].checkCollision(player.getX(), player.getY(), player.getSize())) {
                    done = true;
                    break;
                }
            }

            for (size_t i = 0; i < bullets.size(); ++i) {
                bullets[i].second -= BULLET_SPEED;
                if (bullets[i].second < 0) {
                    bullets.erase(bullets.begin() + i);
                    --i;
                }
                else {
                    for (size_t j = 0; j < asteroids.size(); ++j) {
                        if (asteroids[j].checkCollision(bullets[i].first, bullets[i].second, BULLET_SIZE)) {
                            asteroids[j].destroy();
                            bullets.erase(bullets.begin() + i);
                            --i;
                            destroyedAsteroids++;
                            break;
                        }
                    }
                }
            }

            player.update();
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
            case ALLEGRO_KEY_LEFT:
                player.moveLeftStart();
                break;
            case ALLEGRO_KEY_RIGHT:
                player.moveRightStart();
                break;
            case ALLEGRO_KEY_SPACE:
                player.fireBullet(bullets);
                break;
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch (ev.keyboard.keycode) {
            case ALLEGRO_KEY_LEFT:
                player.moveLeftStop();
                break;
            case ALLEGRO_KEY_RIGHT:
                player.moveRightStop();
                break;
            }
        }

        al_clear_to_color(al_map_rgb(0, 0, 0));

        // Desenha o fundo do espaço com estrelas
        drawStars(20); // Agora com apenas 20 estrelas

        // Desenha a nave do jogador
        drawPlayerShip(player.getX(), player.getY(), PLAYER_SIZE);

        for (const auto& bullet : bullets) {
            al_draw_filled_rectangle(bullet.first, bullet.second, bullet.first + BULLET_SIZE, bullet.second + BULLET_SIZE, al_map_rgb(255, 255, 0));
        }

        for (const auto& asteroid : asteroids) {
            if (asteroid.isAlive()) {
                drawAsteroid(asteroid.getX(), asteroid.getY(), ASTEROID_SIZE);
            }
        }

        al_draw_textf(al_create_builtin_font(), al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "Asteroides destruidos: %d", destroyedAsteroids);

        al_flip_display();
    }

    // Tela de GAME OVER
    al_clear_to_color(al_map_rgb(0, 0, 0)); // Preenche a tela com preto
    ALLEGRO_FONT* font = al_create_builtin_font();
    al_draw_textf(font, al_map_rgb(255, 255, 255), WIDTH / 2, HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "SUA NAVE FOI DESTRUIDA POR UM ASTEROIDE");
    al_draw_textf(font, al_map_rgb(255, 255, 255), WIDTH / 2, HEIGHT / 2 + 50, ALLEGRO_ALIGN_CENTER, "Asteroides destruidos: %d", destroyedAsteroids);
    al_flip_display();
    al_rest(2); // Aguarda por 2 segundos antes de sair do jogo

    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);

    return 0;
}