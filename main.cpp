#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

// --- Game Settings & UI ---
const int TILE_SIZE = 25;
const int COLS = 32;
const int ROWS = 22;
const int UI_HEIGHT = 50;
const int WIDTH = COLS * TILE_SIZE;
const int HEIGHT = (ROWS * TILE_SIZE) + UI_HEIGHT;

enum Direction { UP, DOWN, LEFT, RIGHT };
enum GameState { MENU, PLAYING, GAMEOVER };

// Modern Color Palette
const sf::Color BG_COLOR(20, 20, 25);         
const sf::Color SNAKE_COLOR(57, 255, 20);     
const sf::Color FOOD_COLOR(255, 49, 49);      
const sf::Color OBSTACLE_COLOR(100, 100, 110); 
const sf::Color TEXT_COLOR(240, 240, 240);    
const sf::Color HIGHLIGHT_COLOR(255, 200, 50);

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Snake - Ultimate Edition");
    window.setFramerateLimit(60);
    srand(static_cast<unsigned>(time(0)));

    sf::Font font;
    if (!font.loadFromFile("font.ttf")) {
        // Font missing fallback
    }

    // UI Texts
    sf::Text scoreText("", font, 24);
    scoreText.setFillColor(TEXT_COLOR);
    scoreText.setPosition(10.f, 10.f);

    sf::Text centerText("", font, 36);
    centerText.setStyle(sf::Text::Bold);

    // Game Variables
    std::vector<sf::Vector2i> snake;
    std::vector<sf::Vector2i> obstacles;
    sf::Vector2i food;
    Direction dir;
    GameState state = MENU;
    int score = 0;
    sf::Clock clock;
    
    // Settings
    int selectedMap = 0;        // 0: Open, 1: Box, 2: Cross
    int selectedDiff = 1;       // 0: Chill, 1: Normal, 2: Vibe God
    float delay = 0.1f;

    // Helper to generate maps
    auto generateMap = [&](int mapId) {
        obstacles.clear();
        
        if (mapId == 1) { // Boxed In
            for (int x = 0; x < COLS; x++) {
                obstacles.push_back(sf::Vector2i(x, 0));
                obstacles.push_back(sf::Vector2i(x, ROWS - 1));
            }
            for (int y = 0; y < ROWS; y++) {
                obstacles.push_back(sf::Vector2i(0, y));
                obstacles.push_back(sf::Vector2i(COLS - 1, y));
            }
        } 
        else if (mapId == 2) { // The Perfect Cross
            // Horizontal bar (centered)
            for (int x = 8; x < COLS - 8; x++) {
                if (x == COLS/2 || x == (COLS/2)-1) continue; // Leave center gap
                obstacles.push_back(sf::Vector2i(x, ROWS / 2));
            }
            // Vertical bar (centered)
            for (int y = 4; y < ROWS - 4; y++) {
                if (y == ROWS/2 || y == (ROWS/2)-1) continue; // Leave center gap
                obstacles.push_back(sf::Vector2i(COLS / 2, y));
                obstacles.push_back(sf::Vector2i((COLS / 2) - 1, y)); // 2-thick vertical so it looks proportional
            }
        }
        else if (mapId == 3) { // Corridors
            // Draw horizontal walls across the screen with gaps
            for (int y = 5; y < ROWS; y += 6) {
                for (int x = 0; x < COLS; x++) {
                    // Create gaps on the left and right alternately
                    if (y % 12 == 5 && x > COLS - 6) continue;
                    if (y % 12 != 5 && x < 6) continue;
                    obstacles.push_back(sf::Vector2i(x, y));
                }
            }
        }
        else if (mapId == 4) { // Grid City
            // Small 2x2 pillars scattered symmetrically
            for (int x = 5; x < COLS - 5; x += 8) {
                for (int y = 5; y < ROWS - 5; y += 6) {
                    obstacles.push_back(sf::Vector2i(x, y));
                    obstacles.push_back(sf::Vector2i(x+1, y));
                    obstacles.push_back(sf::Vector2i(x, y+1));
                    obstacles.push_back(sf::Vector2i(x+1, y+1));
                }
            }
        }
    };

    // Helper to reset and start a round
    auto startGame = [&]() {
        snake.clear();
        snake.push_back(sf::Vector2i(COLS / 2, ROWS / 2 + 2)); // Start slightly off-center
        dir = RIGHT;
        score = 0;
        
        if (selectedDiff == 0) delay = 0.15f;
        else if (selectedDiff == 1) delay = 0.09f;
        else if (selectedDiff == 2) delay = 0.04f;

        generateMap(selectedMap);

        // Spawn first food safely
        bool validFood = false;
        while (!validFood) {
            food.x = rand() % COLS;
            food.y = rand() % ROWS;
            validFood = true;
            for (const auto& obs : obstacles) if (obs.x == food.x && obs.y == food.y) validFood = false;
            for (const auto& seg : snake) if (seg.x == food.x && seg.y == food.y) validFood = false;
        }
        
        state = PLAYING;
    };


    // --- MAIN GAME LOOP ---
    while (window.isOpen())
    {
        float time = clock.getElapsedTime().asSeconds();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // Menu Controls (Single press detection)
            if (state == MENU && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::M) selectedMap = (selectedMap + 1) % 5;
                if (event.key.code == sf::Keyboard::D) selectedDiff = (selectedDiff + 1) % 3;
                if (event.key.code == sf::Keyboard::Space) startGame();
            }
            
            // Game Over Controls
            if (state == GAMEOVER && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) startGame();
                if (event.key.code == sf::Keyboard::Escape) state = MENU;
            }
        }

        if (state == PLAYING)
        {
            // Input (Arrow Keys OR WASD)
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) && dir != DOWN) dir = UP;
            else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) && dir != UP) dir = DOWN;
            else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) && dir != RIGHT) dir = LEFT;
            else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) && dir != LEFT) dir = RIGHT;

            // Logic Update
            if (time > delay)
            {
                clock.restart();

                sf::Vector2i newHead = snake.front();
                if (dir == UP) newHead.y -= 1;
                else if (dir == DOWN) newHead.y += 1;
                else if (dir == LEFT) newHead.x -= 1;
                else if (dir == RIGHT) newHead.x += 1;

                // Screen wrapping (if no borders are hit)
                if (newHead.x < 0) newHead.x = COLS - 1;
                if (newHead.x >= COLS) newHead.x = 0;
                if (newHead.y < 0) newHead.y = ROWS - 1;
                if (newHead.y >= ROWS) newHead.y = 0;

                // Check Collisions
                for (const auto& segment : snake) if (segment.x == newHead.x && segment.y == newHead.y) state = GAMEOVER;
                for (const auto& obs : obstacles) if (obs.x == newHead.x && obs.y == newHead.y) state = GAMEOVER;

                if (state == PLAYING)
                {
                    snake.insert(snake.begin(), newHead);

                    if (newHead.x == food.x && newHead.y == food.y) {
                        score += (selectedDiff + 1) * 10; // Harder difficulty = more points
                        
                        bool validFood = false;
                        while (!validFood) {
                            food.x = rand() % COLS;
                            food.y = rand() % ROWS;
                            validFood = true;
                            for (const auto& obs : obstacles) if (obs.x == food.x && obs.y == food.y) validFood = false;
                            for (const auto& seg : snake) if (seg.x == food.x && seg.y == food.y) validFood = false;
                        }
                    } else {
                        snake.pop_back(); 
                    }
                }
            }
        }

        // --- RENDER ---
        window.clear(BG_COLOR);

        // Helper lambda to draw grid-aligned blocks
        auto drawBlock = [&](int x, int y, sf::Color color, float scale = 1.0f) {
            float size = (TILE_SIZE - 2.f) * scale; 
            float offset = (TILE_SIZE - size) / 2.f;
            sf::RectangleShape rect(sf::Vector2f(size, size));
            rect.setFillColor(color);
            rect.setPosition((x * TILE_SIZE) + offset, (y * TILE_SIZE) + UI_HEIGHT + offset);
            window.draw(rect);
        };

        if (state == MENU) 
        {
            std::string mapNames[] = {"Open World", "Boxed In", "The Cross", "Corridors", "Grid City"};
            std::string diffNames[] = {"Chill", "Normal", "Vibe God"};
            
            centerText.setFillColor(HIGHLIGHT_COLOR);
            centerText.setString("VIBE CODER SNAKE");
            centerText.setPosition(WIDTH / 2.f - 160.f, HEIGHT / 3.f - 50.f);
            window.draw(centerText);

            sf::Text options("Press 'M' to change Map: " + mapNames[selectedMap] + "\n" +
                             "Press 'D' to change Diff: " + diffNames[selectedDiff] + "\n\n" +
                             "    Press SPACE to Start", font, 24);
            options.setFillColor(TEXT_COLOR);
            options.setPosition(WIDTH / 2.f - 180.f, HEIGHT / 2.f);
            window.draw(options);
        }
        else 
        {
            // Draw UI Top Bar
            sf::RectangleShape uiBar(sf::Vector2f(WIDTH, UI_HEIGHT));
            uiBar.setFillColor(sf::Color(15, 15, 20));
            window.draw(uiBar);

            scoreText.setString("SCORE: " + std::to_string(score));
            window.draw(scoreText);

            // Draw game elements
            for (const auto& obs : obstacles) drawBlock(obs.x, obs.y, OBSTACLE_COLOR);
            drawBlock(food.x, food.y, FOOD_COLOR, 0.8f);
            for (size_t i = 0; i < snake.size(); i++) {
                if (i == 0) drawBlock(snake[i].x, snake[i].y, sf::Color(100, 255, 100)); 
                else drawBlock(snake[i].x, snake[i].y, SNAKE_COLOR);
            }

            if (state == GAMEOVER) {
                sf::RectangleShape overlay(sf::Vector2f(WIDTH, HEIGHT));
                overlay.setFillColor(sf::Color(0, 0, 0, 180));
                window.draw(overlay);

                centerText.setFillColor(FOOD_COLOR);
                centerText.setString("GAME OVER");
                centerText.setPosition(WIDTH / 2.f - 100.f, HEIGHT / 2.f - 60.f);
                window.draw(centerText);

                sf::Text restartText("SPACE to Restart | ESC for Menu", font, 20);
                restartText.setFillColor(TEXT_COLOR);
                restartText.setPosition(WIDTH / 2.f - 160.f, HEIGHT / 2.f + 10.f);
                window.draw(restartText);
            }
        }

        window.display();
    }

    return 0;
}