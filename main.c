#include <math.h>
#include <raylib.h>
#include <raymath.h>

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

static Camera2D camera = {0};

static bool scored      = false;
static Vector2 scores   = {.x = 0, .y = 0};
static float count_down = 0;

static Sound player_bounce = {0};
static Sound side_bounce   = {0};
void update_players(Rectangle* p_1, Rectangle* p_2);
void update_ball(Vector2* ball_pos, Vector2* ball_speed, const Rectangle* p_1,
                 const Rectangle* p_2);

int main() {
    InitWindow(WINDOW_W, WINDOW_H, "Ponglish");
    InitAudioDevice();
    DisableCursor();
    SetTargetFPS(60);
    int i = 0;

    player_bounce = LoadSound("player_bounce.wav");
    side_bounce   = LoadSound("side_bounce.wav");

    Texture2D background = LoadTexture("background.png");

    Vector2 ball_pos   = {200, 400};
    Vector2 ball_speed = {300, 300};
    camera.zoom        = 1;
    camera.target      = (Vector2) {400, 400};
    camera.offset      = (Vector2) {400, 400};

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
        DrawTexture(background, 0, 0, ColorFromHSV(i % 360, 0.5, 0.5));
        if (!scored) {
            if (count_down > 0) {
                DrawText(TextFormat("%.2f", count_down), 200,
                         sinf(i / 10.0) * 10 + 200, 34, WHITE);
                count_down -= GetFrameTime();
                DrawLineEx(
                    camera.offset,
                    Vector2Add(camera.offset, Vector2Scale(ball_speed, 0.25)),
                    10, WHITE);
            } else {
                DrawText(TextFormat("%d:%d", (int) scores.x, (int) scores.y),
                         380, 400, 36, WHITE);
                update_ball(&ball_pos, &ball_speed, &player_1, &player_2);
            }
            update_players(&player_1, &player_2);

            DrawRectangleRec((Rectangle) {.x = player_1.x,
                                          .y = player_1.y + player_1_velocity.y,
                                          .width  = player_1.width,
                                          .height = player_1.height},
                             ColorFromHSV(0, 0, 0.75));
            DrawRectangleRec((Rectangle) {.x = player_2.x,
                                          .y = player_2.y + player_2_velocity.y,
                                          .width  = player_2.width,
                                          .height = player_2.height},
                             ColorFromHSV(0, 0, 0.75));
            DrawCircleV(ball_pos, 20, WHITE);
        } else {
            DrawText("Press space to continue!", 200, sinf(i / 10.0) * 10 + 200,
                     34, WHITE);
        }
        EndMode2D();
        EndDrawing();
        i++;
    }

    CloseWindow();
}

void update_players(Rectangle* p_1, Rectangle* p_2) {
    if (IsKeyDown(KEY_A)) {
        player_1_velocity.x += PLAYER_SPEED * GetFrameTime();
    }
    if (IsKeyDown(KEY_D)) {
        player_1_velocity.x -= PLAYER_SPEED * GetFrameTime();
    }
    if (IsKeyDown(KEY_LEFT)) {
        player_2_velocity.x += PLAYER_SPEED * GetFrameTime();
    }
    if (IsKeyDown(KEY_RIGHT)) {
        player_2_velocity.x -= PLAYER_SPEED * GetFrameTime();
    }
    player_1_velocity.x =
        Clamp(player_1_velocity.x, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);
    player_2_velocity.x =
        Clamp(player_2_velocity.x, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);
    p_1->x               = Clamp(p_1->x - player_1_velocity.x, 0, 600);
    p_2->x               = Clamp(p_2->x - player_2_velocity.x, 0, 600);
    player_1_velocity.x /= 1.3;
    player_2_velocity.x /= 1.3;
}
void reset_ball(Vector2* ball_pos, Vector2* ball_speed) {
    ball_pos->x = 400;
    ball_pos->y = 400;
    if (ball_speed->y < 0) {
        scores.y++;
    } else {

        scores.x++;
    }
    ball_speed->x *= -1;
    ball_speed->y *= -1;
    scored         = true;
    count_down     = 5;
}

void update_ball(Vector2* ball_pos, Vector2* ball_speed, const Rectangle* p_1,
                 const Rectangle* p_2) {
    const Vector2 next_ball_pos =
        Vector2Add(*ball_pos, Vector2Scale(*ball_speed, GetFrameTime()));

    if (next_ball_pos.x - 15 <= 0 || next_ball_pos.x + 15 >= 800) {
        ball_speed->x *= -1;
        PlaySound(side_bounce);
    }
    if (next_ball_pos.y - 15 <= 0 || next_ball_pos.y + 15 >= 800) {
        reset_ball(ball_pos, ball_speed);
        return;
    }

    if (CheckCollisionCircleRec(next_ball_pos, 20, *p_1)) {
        ball_speed->y       *= -1;
        player_1_velocity.y  = 10;
        PlaySound(player_bounce);
    }
    if (CheckCollisionCircleRec(next_ball_pos, 20, *p_2)) {
        ball_speed->y       *= -1;
        player_2_velocity.y  = -10;
        PlaySound(player_bounce);
    }
    player_1_velocity.y /= 1.05;
    player_2_velocity.y /= 1.05;

    *ball_pos =
        Vector2Add(*ball_pos, Vector2Scale(*ball_speed, GetFrameTime()));
}
