#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>

#include <SDL.h>

struct Point {
    int x;
    int y;

    // Define operator== for Point struct
    bool operator==(const Point& other) const {
        return (x == other.x && y == other.y);
    }
};

const int SPACE_WIDTH = 50;
const int SPACE_HEIGHT = 50;

const int WIDTH_SIZE = 15;
const int HEIGHT_SIZE = WIDTH_SIZE/1.15;

const int WINDOW_WIDTH = WIDTH_SIZE * SPACE_WIDTH;
const int WINDOW_HEIGHT = HEIGHT_SIZE * SPACE_HEIGHT;

const SDL_Color WHITE = { 255, 255, 255, 255 };
const SDL_Color BLACK = { 0, 0, 0, 255 };
const SDL_Color RED = { 255, 0, 0, 255 };
const SDL_Color GREEN = { 0, 255, 0, 255 };
const SDL_Color BLUE = { 0, 0, 255, 255 };
const SDL_Color YELLOW = { 255, 255, 0, 255 };
const SDL_Color CYAN = { 0, 255, 255, 255 };
const SDL_Color MAGENTA = { 255, 0, 255, 255 };
const SDL_Color ORANGE = { 255, 165, 0, 255 };
const SDL_Color GRAY = { 128, 128, 128, 255 };
const SDL_Color LIGHT_GRAY = { 192, 192, 192, 255 };
const SDL_Color DARK_GRAY = { 64, 64, 64, 255 };
const SDL_Color PURPLE = { 128, 0, 128, 255 };
const SDL_Color BROWN = { 165, 42, 42, 255 };
const SDL_Color PINK = { 255, 192, 203, 255 };

// exp() == e^x
// f(x) = 254 * exp(-ln(255) * 10 * x) + 1
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ Formula for light 

void drawLightTilesDataColor(SDL_Renderer* renderer, const Uint8(&lightTiles)[SPACE_HEIGHT][SPACE_WIDTH]);
void renderFilledRectangleColor(SDL_Renderer* renderer, int x, int y, const SDL_Color& color);

void renderFilledRectangleRGBA(SDL_Renderer* renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

void removePoint(Point pointToRemove, std::vector<Point>& remove);
void find_linear_equation(Point p1, Point p2, float& slope, float& intercept);

int getDistance(Point p1, Point p2);

Uint8 distanceToLight(int distance);
Uint8 getLightLevel(int x, int y, std::vector<Point> &lights, std::vector<Point> &walls);

int calculateAverage(const Uint8 (&lightTiles)[SPACE_HEIGHT][SPACE_WIDTH], int row, int col);
void getNewBlurLightTiles(const Uint8 (&lightTiles)[SPACE_HEIGHT][SPACE_WIDTH], Uint8 (&newLightTiles)[SPACE_HEIGHT][SPACE_WIDTH]);

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Light Illumination System", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Uint8 lightTiles[SPACE_HEIGHT][SPACE_WIDTH] = {};

    std::vector<Point> walls;
    std::vector<Point> lights;

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderClear(renderer);

    drawLightTilesDataColor(renderer, lightTiles);

    SDL_RenderPresent(renderer);

    bool quit = false;
    bool drawing = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_SPACE) {
                            drawing = true;
                            quit = true;
                        }
                    break;
                default:
                    break;
            }
        }

        int mouseX, mouseY;
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        if (mouseX >= 0 && mouseX <= WINDOW_WIDTH && mouseY >= 0 && mouseY <= WINDOW_HEIGHT) {
            int lightTileX = (int)floor(mouseX / WIDTH_SIZE);
            int lightTileY = (int)floor(mouseY / HEIGHT_SIZE);

            Point point = { lightTileX, lightTileY };

            if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) && lightTiles[lightTileY][lightTileX] != -1) {
                renderFilledRectangleColor(renderer, lightTileX, lightTileY, RED);
                lightTiles[lightTileY][lightTileX] = -1;
                removePoint(point, walls);
                walls.push_back(point);
                removePoint(point, lights);
            } 
            else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) && lightTiles[lightTileY][lightTileX] != -2) {
                renderFilledRectangleColor(renderer, lightTileX, lightTileY, YELLOW);
                lightTiles[lightTileY][lightTileX] = -2;
                removePoint(point, lights);
                lights.push_back(point);
                removePoint(point, walls);
            }
            else if (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE) && lightTiles[lightTileY][lightTileX] > 0) {
                renderFilledRectangleColor(renderer, lightTileX, lightTileY, BLACK);
                lightTiles[lightTileY][lightTileX] = 0;
                removePoint(point, walls);
                removePoint(point, lights);
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    } 

    if (drawing) {
        SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
        SDL_RenderClear(renderer);
        for (int i = 0; i < SPACE_HEIGHT; i++) {
            for (int j = 0; j < SPACE_WIDTH; j++) {
                lightTiles[i][j] = getLightLevel(j, i, lights, walls);
            }
        }
        
        Uint8 lightTilesBlur1[SPACE_HEIGHT][SPACE_WIDTH] = {};
        getNewBlurLightTiles(lightTiles, lightTilesBlur1);
        Uint8 lightTilesBlur2[SPACE_HEIGHT][SPACE_WIDTH] = {};
        getNewBlurLightTiles(lightTilesBlur1, lightTilesBlur2);

        for (int i = 0; i < SPACE_HEIGHT; i++) {
            for (int j = 0; j < SPACE_WIDTH; j++) {
                renderFilledRectangleRGBA(renderer, j, i, lightTilesBlur2[i][j], lightTilesBlur2[i][j], lightTilesBlur2[i][j], 255);
            }
        }
        for (Point wall : walls) {
            renderFilledRectangleRGBA(renderer, wall.x, wall.y, RED.r, RED.g, RED.b, RED.a);
        }
        for (Point light : lights) {
            renderFilledRectangleRGBA(renderer, light.x, light.y, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        }
        SDL_RenderPresent(renderer);
    }

    quit = false;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void drawLightTilesDataColor(SDL_Renderer *renderer, const Uint8(&lightTiles)[SPACE_HEIGHT][SPACE_WIDTH]) {
    for (int x = 0; x < SPACE_WIDTH; x++) {
        for (int y = 0; y < SPACE_HEIGHT; y++) {
            renderFilledRectangleColor(renderer, x, y, BLACK);
        }
    }
}

void renderFilledRectangleColor(SDL_Renderer* renderer, int x, int y, const SDL_Color &color) {
    double offset = 0.2;

    SDL_Rect rect = { x * WIDTH_SIZE + offset, y * HEIGHT_SIZE + offset, WIDTH_SIZE - 2 * offset, HEIGHT_SIZE - 2 * offset };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_RenderFillRect(renderer, &rect);
}

void renderFilledRectangleRGBA(SDL_Renderer* renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    double offset = 0.2;

    SDL_Rect rect = { x * WIDTH_SIZE + offset, y * HEIGHT_SIZE + offset, WIDTH_SIZE - 2 * offset, HEIGHT_SIZE - 2 * offset };
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    SDL_RenderFillRect(renderer, &rect);
}

void removePoint(Point pointToRemove, std::vector<Point>& remove) {
    // Check if the point to remove exists in the "remove" vector
    auto it = std::find_if(remove.begin(), remove.end(), [&](const Point& p) {
        return p.x == pointToRemove.x && p.y == pointToRemove.y;
        });

    // If the point to remove exists in the "remove" vector, remove it
    if (it != remove.end()) {
        remove.erase(it);
    }
}

void find_linear_equation(Point p1, Point p2, float& slope, float& intercept) {
    slope = static_cast<float>(p2.y - p1.y) / (p2.x - p1.x);
    intercept = p1.y - slope * p1.x;
}

int getDistance(Point p1, Point p2) {
    return (int) sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

Uint8 distanceToLight(int distance) {
    int light = (int) round(254 * exp(-(1 * log(255) * distance) / 12) + 1);
    if (light > 255) return 255;
    return light;
}

Uint8 getLightLevel(int x, int y, std::vector<Point>& lights, std::vector<Point>& walls) {
    Uint8 currentLight = 0;

    for (Point light : lights) {
        float slope = 0;
        float intercept = 0;
        find_linear_equation(light, { x, y }, slope, intercept);

        bool collided = false;

        for (Point wall : walls) {
            if (wall.x >= std::min(x, light.x) && wall.x <= std::max(x, light.x)) {
                if( wall.y >= std::min(y, light.y) && wall.y <= std::max(y, light.y)) {
                collided = true;
                }
            }
        }

        if (!collided) {
            int distance = getDistance(light, { x, y });
            Uint8 newLight = distanceToLight(distance);
            if (newLight > currentLight) {
                currentLight = newLight;
            }
        }  
    }

    return currentLight;
}

int calculateAverage(const Uint8(&lightTiles)[SPACE_HEIGHT][SPACE_WIDTH], int row, int col) {
    int sum = 0;
    int count = 0;

    for (int r = std::max(row - 1, 0); r <= std::min(row + 1, SPACE_HEIGHT - 1); ++r) {
        for (int c = std::max(col - 1, 0); c <= std::min(col + 1, SPACE_WIDTH - 1); ++c) {
            if (r != row || c != col) {
                sum += lightTiles[r][c];
                count++;
            }
        }
    }

    return sum / count;
}

// Function to generate a new 2D array with averaged values of neighbors
void getNewBlurLightTiles(const Uint8(&lightTiles)[SPACE_HEIGHT][SPACE_WIDTH], Uint8(&newLightTiles)[SPACE_HEIGHT][SPACE_WIDTH]) {
    for (int row = 0; row < SPACE_HEIGHT; ++row) {
        for (int col = 0; col < SPACE_WIDTH; ++col) {
            int average = calculateAverage(lightTiles, row, col);
            newLightTiles[row][col] = average;
        }
    }
}