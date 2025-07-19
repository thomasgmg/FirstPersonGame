#include "raylib.h"

int main(void)
{
    const int screenWidth = 1500;
    const int screenHeight = 800;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - first person maze");

    // Define the camera to look into our 3d world
    Camera camera = {0};
    camera.position = (Vector3){0.2f, 0.4f, 0.2f}; // Camera position
    camera.target = (Vector3){0.185f, 0.4f, 0.0f}; // Camera looking at point
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};       // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                           // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;        // Camera projection type

    Image imMap = LoadImage("resources/cubicmap.png"); // Load cubicmap image (RAM)
    Texture2D cubicmap = LoadTextureFromImage(imMap);  // Convert image to texture to display (VRAM)
    Mesh mesh = GenMeshCubicmap(imMap, (Vector3){1.0f, 1.0f, 1.0f});
    Model model = LoadModelFromMesh(mesh);

    // NOTE: By default each cube is mapped to one part of texture atlas
    Texture2D texture = LoadTexture("resources/cubicmap_atlas.png"); // Load map texture
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture; // Set map diffuse texture

    // Get map image data to be used for collision detection
    Color *mapPixels = LoadImageColors(imMap);
    UnloadImage(imMap); // Unload image from RAM

    Vector3 mapPosition = {-16.0f, 0.0f, -8.0f}; // Set model position

    DisableCursor();

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        Vector3 oldCamPos = camera.position; // Store old camera position

        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        // Check player collision (we simplify to 2D collision detection)
        Vector2 playerPos = {camera.position.x, camera.position.z};
        float playerRadius = 0.1f; // Collision radius (player is modelled as a
                                   // cilinder for collision)

        int playerCellX = (int)(playerPos.x - mapPosition.x + 0.5f);
        int playerCellY = (int)(playerPos.y - mapPosition.z + 0.5f);

        // Out-of-limits security check
        if (playerCellX < 0)
            playerCellX = 0;
        else if (playerCellX >= cubicmap.width)
            playerCellX = cubicmap.width - 1;

        if (playerCellY < 0)
            playerCellY = 0;
        else if (playerCellY >= cubicmap.height)
            playerCellY = cubicmap.height - 1;

        // Check map collisions using image data and player position
        // TODO: Improvement: Just check player surrounding cells for collision
        for (int y = 0; y < cubicmap.height; y++)
        {
            for (int x = 0; x < cubicmap.width; x++)
            {
                if ((mapPixels[y * cubicmap.width + x].r == 255) && // Collision: white pixel, only check R channel
                    (CheckCollisionCircleRec(
                        playerPos, playerRadius,
                        (Rectangle){mapPosition.x - 0.5f + x * 1.0f, mapPosition.z - 0.5f + y * 1.0f, 1.0f, 1.0f})))
                {
                    // Collision detected, reset camera position
                    camera.position = oldCamPos;
                }
            }
        }

        BeginDrawing();

        ClearBackground(BLANK);

        BeginMode3D(camera);
        DrawModel(model, mapPosition, 1.0f, WHITE); // Draw maze map
        EndMode3D();

        DrawTextureEx(cubicmap, (Vector2){GetScreenWidth() - cubicmap.width * 4.0f - 20, 20.0f}, 0.0f, 4.0f, WHITE);

        // Draw player position radar
        DrawRectangle(GetScreenWidth() - cubicmap.width * 4 - 20 + playerCellX * 4, 20 + playerCellY * 4, 4, 4, GREEN);

        DrawFPS(10, 10);

        EndDrawing();
    }

    UnloadImageColors(mapPixels); // Unload color array
    UnloadTexture(cubicmap);      // Unload cubicmap texture
    UnloadTexture(texture);       // Unload map texture
    UnloadModel(model);           // Unload map model

    CloseWindow();

    return 0;
}
