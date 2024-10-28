#include "SpaceMono.h"
#include "background.h"
#include "player_bounce.h"
#include "side_bounce.h"

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

enum {
    WINDOW_W = 800,
    WINDOW_H = 800,
};

enum { PLAYER_SPEED = 100, PLAYER_MAX_SPEED = 300 };

static Rectangle player_1 = {
    .width  = 200,
    .height = 40,
    .x      = (WINDOW_W / 2) - 100,
    .y      = 680,
};
static Vector2 player_1_velocity = {0};

static Rectangle player_2 = {
    .width  = 200,
    .height = 40,
    .x      = (WINDOW_W / 2) - 100,
    .y      = 120,
};
static Vector2 player_2_velocity = {0};

static Vector2 ball_pos   = {400, 400};
static Vector2 ball_speed = {300, 300};

static Camera2D camera = {0};

static bool scored      = false;
static Vector2 scores   = {.x = 0, .y = 0};
static float count_down = 0;

static Sound player_bounce_sound = {0};
static Sound side_bounce_sound   = {0};

static Font font = {0};
static float i   = 0;
void update_players(Rectangle* p_1, Rectangle* p_2);
void update_ball(Vector2* ball_pos, Vector2* ball_speed, const Rectangle* p_1,
                 const Rectangle* p_2);
void draw_waiting_screen();
void draw_entities();

int main() {
    srand(time(NULL));
    InitWindow(WINDOW_W, WINDOW_H, "Ponglish");
    InitAudioDevice();
    DisableCursor();
    SetTargetFPS(120);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    side_bounce_sound = LoadSoundFromWave(
        LoadWaveFromMemory(".wav", side_bounce, sizeof(side_bounce)));
    player_bounce_sound = LoadSoundFromWave(
        LoadWaveFromMemory(".wav", player_bounce, sizeof(side_bounce)));

    font =
        LoadFontFromMemory(".ttf", SpaceMono, sizeof(SpaceMono), 96, NULL, 255);

    Texture2D background_image = LoadTextureFromImage(
        LoadImageFromMemory(".png", background, sizeof(background)));

    camera.zoom   = 1;
    camera.target = (Vector2) {400, 400};
    camera.offset = (Vector2) {400, 400};

    while (!WindowShouldClose()) {
        BeginDrawing();
        BeginMode2D(camera);
        if (scored) {
            if (IsKeyPressed(KEY_SPACE)) {
                scored = false;
            }
        }
        camera.target =
            Vector2Add(camera.offset, Vector2Scale(ball_pos, 0.025));
        camera.rotation = sinf(i / 50.0) / 2.0;

        ClearBackground(GetColor(0x181818ff));
        DrawTexture(background_image, 0, 0,
                    ColorFromHSV((int) i % 360, 0.5, 0.5));
        if (!scored) {
            draw_entities();
            if (count_down > 0) {
                draw_waiting_screen();
            } else {
                DrawTextEx(
                    font, TextFormat("%d:%d", (int) scores.x, (int) scores.y),
                    (Vector2) {380, sinf(i / 5.0) * 10 + 400}, 36, 5, WHITE);
                update_ball(&ball_pos, &ball_speed, &player_1, &player_2);
            }
            update_players(&player_1, &player_2);
        } else {
            DrawRectangle(-200, -200, 1200, 1200, GetColor(0x00000077));
            DrawTextEx(font, "Press space to continue!",
                       (Vector2) {125, sinf(i / 10.0) * 10 + 400}, 48, 5,
                       WHITE);
        }
        EndMode2D();
        EndDrawing();
        i += GetFrameTime() * 10;
    }

    CloseWindow();
}

void update_players(Rectangle* p_1, Rectangle* p_2) {
    player_1_velocity.x += PLAYER_SPEED * GetFrameTime() * IsKeyDown(KEY_A);
    player_1_velocity.x -= PLAYER_SPEED * GetFrameTime() * IsKeyDown(KEY_D);
    player_2_velocity.x += PLAYER_SPEED * GetFrameTime() * IsKeyDown(KEY_LEFT);
    player_2_velocity.x -= PLAYER_SPEED * GetFrameTime() * IsKeyDown(KEY_RIGHT);

    player_1_velocity.x =
        Clamp(player_1_velocity.x, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);
    player_2_velocity.x =
        Clamp(player_2_velocity.x, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);

    p_1->x               = Clamp(p_1->x - player_1_velocity.x, 0, 600);
    p_2->x               = Clamp(p_2->x - player_2_velocity.x, 0, 600);
    player_1_velocity.x /= 1.2;
    player_2_velocity.x /= 1.2;
}
void reset_ball(Vector2* ball_pos, Vector2* ball_speed) {
    static const int speeds[] = {-300, 300};
    ball_pos->x               = 400;
    ball_pos->y               = 400;
    if (ball_speed->y < 0) {
        scores.y++;
    } else {
        scores.x++;
    }

    ball_speed->x = speeds[rand() % 2];
    ball_speed->y = speeds[rand() % 2];
    scored        = true;
    count_down    = 3;
}

void update_ball(Vector2* ball_pos, Vector2* ball_speed, const Rectangle* p_1,
                 const Rectangle* p_2) {
    const Vector2 next_ball_pos =
        Vector2Add(*ball_pos, Vector2Scale(*ball_speed, GetFrameTime()));

    if (next_ball_pos.x - 15 <= 0 || next_ball_pos.x + 15 >= 800) {
        ball_speed->x *= -1;
        PlaySound(side_bounce_sound);
    }
    if (next_ball_pos.y - 15 <= 0 || next_ball_pos.y + 15 >= 800) {
        reset_ball(ball_pos, ball_speed);
        return;
    }

    if (CheckCollisionCircleRec(next_ball_pos, 20, *p_1)) {
        ball_speed->y       *= -1;
        *ball_speed          = Vector2Scale(*ball_speed, 1.025);
        player_1_velocity.y  = 10;
        PlaySound(player_bounce_sound);
    }
    if (CheckCollisionCircleRec(next_ball_pos, 20, *p_2)) {
        ball_speed->y       *= -1;
        *ball_speed          = Vector2Scale(*ball_speed, 1.025);
        player_2_velocity.y  = -10;
        PlaySound(player_bounce_sound);
    }
    player_1_velocity.y /= 1.3;
    player_2_velocity.y /= 1.3;

    *ball_pos =
        Vector2Add(*ball_pos, Vector2Scale(*ball_speed, GetFrameTime()));
}

void draw_waiting_screen() {

    DrawTextEx(font, TextFormat("%.1f", count_down),
               (Vector2) {350, sinf(i / 10.0) * 10 + 600}, 60, 5, WHITE);
    count_down -= GetFrameTime();
    DrawLineEx(camera.offset,
               Vector2Add(camera.offset, Vector2Scale(ball_speed, 0.25)), 1,
               WHITE);
}

void draw_entities() {
    DrawRectangleRoundedLines(
        (Rectangle) {.x      = player_1.x,
                     .y      = player_1.y + player_1_velocity.y,
                     .width  = player_1.width,
                     .height = player_1.height},
        0.5, 10, 1, ColorFromHSV(0, 0, 0.75));
    DrawRectangleRoundedLines(
        (Rectangle) {.x      = player_2.x,
                     .y      = player_2.y + player_2_velocity.y,
                     .width  = player_2.width,
                     .height = player_2.height},
        0.5, 10, 1, ColorFromHSV(0, 0, 0.75));
    DrawCircleLinesV(ball_pos, 20, WHITE);
}
