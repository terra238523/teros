#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define WIDTH 20
#define HEIGHT 10
#define MAX_SNAKE 100

typedef struct { int x, y; } Point;

Point snake[MAX_SNAKE];
int snake_len = 3;
Point food;
char dir = 'd';
int game_over = 0;
int score = 0;

struct termios old_tio;
int old_flags = -1;

// Disable terminal buffering
void disable_buffering() {
    // restore terminal attrs (affects tty)
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

    // restore blocking/flags (must restore so parent sees blocking reads)
    if (old_flags != -1) {
        fcntl(STDIN_FILENO, F_SETFL, old_flags);
        old_flags = -1;
    }
}

// Enable non-blocking input

void enable_nonblocking_input() {
    struct termios new_tio;
    // save current terminal attrs
    if (tcgetattr(STDIN_FILENO, &old_tio) == -1) {
        // not a terminal? fail gracefully
        return;
    }
    // save file flags and set non-blocking
    old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (old_flags == -1) old_flags = 0;
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

    new_tio = old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    atexit(disable_buffering); // ensure restore on exit
}

// Place food at a random empty position
void place_food() {
    int valid;
    do {
        valid = 1;
        food.x = rand() % WIDTH;
        food.y = rand() % HEIGHT;
        for (int i = 0; i < snake_len; i++) {
            if (snake[i].x == food.x && snake[i].y == food.y) valid = 0;
        }
    } while (!valid);
}

// Initialize snake in top-left corner
void init_snake() {
    for (int i = 0; i < snake_len; i++) {
        snake[i].x = snake_len - i - 1;
        snake[i].y = 0;
    }
}

// Draw board
void draw_board() {
    printf("\033[H\033[J"); // clear screen
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int printed = 0;
            if (food.x == x && food.y == y) {
                printf("F"); printed = 1;
            }
            for (int i = 0; i < snake_len; i++) {
                if (snake[i].x == x && snake[i].y == y) { printf("O"); printed = 1; }
            }
            if (!printed) printf(".");
        }
        printf("\n");
    }
    printf("Score: %d\n", score);
    printf("Use WASD to move, 'q' to quit\n");
}

// Update snake position safely
void update_snake() {
    Point next = snake[0];
    if (dir == 'w') next.y--;
    if (dir == 's') next.y++;
    if (dir == 'a') next.x--;
    if (dir == 'd') next.x++;

    // Check collisions
    if (next.x < 0 || next.x >= WIDTH || next.y < 0 || next.y >= HEIGHT) { game_over = 1; return; }
    for (int i = 0; i < snake_len; i++) {
        if (snake[i].x == next.x && snake[i].y == next.y) { game_over = 1; return; }
    }

    // Move snake body
    for (int i = snake_len-1; i > 0; i--) snake[i] = snake[i-1];
    snake[0] = next;

    // Eat food
    if (next.x == food.x && next.y == food.y) {
        if (snake_len < MAX_SNAKE) snake_len++;
        score += 10;
        place_food();
    }
}

// Handle user input
void handle_input() {
    char ch;
    while (read(STDIN_FILENO, &ch, 1) > 0) {
        if (ch == 'w' && dir != 's') dir = 'w';
        else if (ch == 's' && dir != 'w') dir = 's';
        else if (ch == 'a' && dir != 'd') dir = 'a';
        else if (ch == 'd' && dir != 'a') dir = 'd';
        else if (ch == 'q') game_over = 1;
    }
}

int main() {
    srand(time(NULL));
    enable_nonblocking_input();
    init_snake();
    place_food();

    while (!game_over) {
        handle_input();
        update_snake();
        draw_board();
        usleep(200000); // 0.2s per frame
    }

    // Game over
    printf("Game Over! Final score: %d\n", score);
    printf("Press Enter to return to shell...\n");
    char buf[16];
    fgets(buf, sizeof(buf), stdin);

    return 0; // safely returns to init shell
}
