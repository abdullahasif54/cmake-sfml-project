#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <SFML/Graphics.hpp>
using namespace std;

const int ROWS = 20;
const int COLS = 10;
const float CELL = 30.f;
const float BOARD_X = 20.f;
const float BOARD_Y = 20.f;

int board[ROWS][COLS];

char pieces[7][4][4];
char activePiece[4][4];

int currentPiece = 0;
int score = 0;
int level = 1;
int linesCleared = 0;
int frameCount = 0;

bool gameOver = false;
bool paused = false;
bool mainMenu = true;
bool noSaveMessage = false;

int pieceX = 3;
int pieceY = 0;

bool loadBoard();
void hardDrop();
void lockPiece();
void clearLines();
void spawnPiece();
void resetGame(sf::Clock& clock);
void saveGame();
bool loadGame();
bool canMove(int dx, int dy);

// loads the game board
bool loadBoard()
{
    ifstream file("../../../board.txt");

    if (!file)
    {
        cout << "Could not open board.txt" << endl;
        return false;
    }

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            file >> board[r][c];
        }
    }

    file.close();
    return true;
}

//resets the game
void resetGame(sf::Clock& clock)
{
    // Load the empty board from file
    if (!loadBoard())
    return;

    // Reset game statistics
    score = 0;
    level = 1;
    linesCleared = 0;

    // Reset game states
    gameOver = false;
    paused = false;

    // Spawn a new piece
    spawnPiece();

    // Reset the falling timer
    clock.restart();

    //resets the frame to adjust speed
    frameCount = 0;
}

//saves game progress

void saveGame()
{
    ofstream file("savegame.txt");

    if (!file)
    {
        cout << "Could not save game!" << endl;
        return;
    }

    file << score << " " << level << endl;

    file << linesCleared << " " << currentPiece << endl;

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            file << board[r][c] << " ";
        }

        file << endl;
    }

    file.close();
}


//clears the saved game
void clearSave()
{
    ofstream file("savegame.txt");
    file.close();
}

//load saved game

bool loadGame()
{
    ifstream file("savegame.txt");

    if (!file)
    {
        cout << "No saved game found!" << endl;
        return false;
    }

    if (!(file >> score >> level))
    {
        return false;
    }

    if (!(file >> linesCleared >> currentPiece))
    {
        return false;
    }

    if (currentPiece < 0 || currentPiece >= 7)
    {
        return false;
    }

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (!(file >> board[r][c]))
            {
                return false;
            }
        }
    }

    file.close();

    // Load the saved piece
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            activePiece[r][c] = pieces[currentPiece][r][c];
        }
    }

    // Start the saved piece from the top
    pieceX = 3;
    pieceY = 0;

    gameOver = false;
    paused = false;
    frameCount = 0;

    if (!canMove(0, 0))
    {
        gameOver = true;
        clearSave();
        return false;
    }

    return true;
}


// Initialize colors
void initializeColors(sf::Color colors[])
{
    colors[0] = sf::Color(25, 25, 45);
    colors[1] = sf::Color::Cyan;
    colors[2] = sf::Color::Yellow;
    colors[3] = sf::Color(170, 90, 255);
    colors[4] = sf::Color::Green;
    colors[5] = sf::Color::Red;
    colors[6] = sf::Color::Blue;
    colors[7] = sf::Color(255, 150, 0);
}


// Initialize block
void initializeBlock(sf::RectangleShape& block)
{
    block.setSize({CELL - 2, CELL - 2});
}

// check whether the peace should fall or stop
bool canMove(int dx, int dy)
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (activePiece[r][c] == '1')
            {
                int newX = pieceX + c + dx;
                int newY = pieceY + r + dy;

                if (newX < 0 || newX >= COLS || newY >= ROWS)
                    return false;

                if (newY >= 0 && board[newY][newX] != 0)
                    return false;
            }
        }
    }

    return true;
}

// Rotates the current piece clockwise
void rotatePiece()
{
    char rotated[4][4];

    // Rotate the piece
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            rotated[r][c] = activePiece[3 - c][r];
        }
    }

    // Check whether the rotated piece fits
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (rotated[r][c] == '1')
            {
                int x = pieceX + c;
                int y = pieceY + r;

                if (x < 0 || x >= COLS || y >= ROWS)
                    return;

                if (y >= 0 && board[y][x] != 0)
                    return;
            }
        }
    }

    // Save the rotated piece
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            activePiece[r][c] = rotated[r][c];
        }
    }
}

//space = last row drop
void hardDrop()
{
    while (canMove(0, 1))
    {
        pieceY++;
    }

    lockPiece();
    clearLines();
    spawnPiece();
}

// Handle keyboard controls
void handleInput(sf::RenderWindow& window, sf::Clock& clock)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (mainMenu && event->is<sf::Event::MouseButtonPressed>())
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            // PLAY button
            if (mousePos.x >= 140 && mousePos.x <= 390 && mousePos.y >= 285 && mousePos.y <= 360)
            {
                mainMenu = false;
                resetGame(clock);
            }

            // EXIT button
            if (mousePos.x >= 140 && mousePos.x <= 390 && mousePos.y >= 455 && mousePos.y <= 530)
            {
                window.close();
            }

            // CONTINUE button
            if (mousePos.x >= 140 && mousePos.x <= 390 && mousePos.y >= 370 && mousePos.y <= 445)
            {
                if (loadGame())
                {
                    mainMenu = false;
                    noSaveMessage = false;
                    clock.restart();
                }
                else
                {
                    noSaveMessage = true;
                }
            }
        }    

        if (event->is<sf::Event::KeyPressed>())
        {
            if (!mainMenu && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            {
                if (!gameOver)
                    saveGame();
                mainMenu = true;
                paused = false;
                clock.restart();
            }
            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
                paused = !paused;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            {
                if (canMove(-1, 0))
                    pieceX--;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            {
                if (canMove(1, 0))
                    pieceX++;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            {
                rotatePiece();
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            {
                if (canMove(0, 1))
                    pieceY++;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
            {
                hardDrop();
            }

            if (gameOver && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
            {
                resetGame(clock);
            }

            if (gameOver && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M))
            {
                mainMenu = true;
            }
        }
    }
}

//Locks the piece in place
void lockPiece()
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (activePiece[r][c] == '1')
            {
                board[pieceY + r][pieceX + c] = currentPiece + 1;
            }
        }
    }
}

//removes the filled rows
void clearLines()
{
    for (int r = ROWS - 1; r >= 0; r--)
    {
        bool full = true;

        for (int c = 0; c < COLS; c++)
        {
            if (board[r][c] == 0)
            {
                full = false;
                break;
            }
        }

        if (full)
        {
            score += 100;
            linesCleared++;

            if (linesCleared % 10 == 0)
            {
                level++;
            }
            for (int i = r; i > 0; i--)
            {
                for (int c = 0; c < COLS; c++)
                {
                    board[i][c] = board[i - 1][c];
                }
            }

            // Clear the top row
            for (int c = 0; c < COLS; c++)
            {
                board[0][c] = 0;
            }

            r++;
        }
    }
}

//spawns a random piece
void spawnPiece()
{
    currentPiece = rand() % 7;

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            activePiece[r][c] = pieces[currentPiece][r][c];
        }
    }

    pieceX = 3;
    pieceY = 0;

    if (!canMove(0, 0))
    {
        gameOver = true;
        clearSave();
    }
}

// Handle automatic falling
void updateFalling()
{
    int speed = 30 - (level - 1) * 5;

    if (speed < 5)
    {
        speed = 5;
    }

    frameCount++;

    if (frameCount >= speed)
    {
        if (canMove(0, 1))
        {
            pieceY++;
        }
        else
        {
            lockPiece();
            clearLines();
            spawnPiece();
        }

        frameCount = 0;
    }
}

// Draw the board
void drawBoard(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[])
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            block.setFillColor(colors[board[r][c]]);

            block.setPosition({
                BOARD_X + c * CELL + 1,
                BOARD_Y + r * CELL + 1
            });

            window.draw(block);
        }
    }
}


// Draw the current piece
void drawPiece(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[])
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (activePiece[r][c] == '1')
            {
                block.setFillColor(colors[currentPiece + 1]);

                block.setPosition({
                    BOARD_X + (pieceX + c) * CELL + 1,
                    BOARD_Y + (pieceY + r) * CELL + 1
                });

                window.draw(block);
            }
        }
    }
}

//Loads Pieces
bool loadPieces()
{
    ifstream file("../../../pieces.txt");

    if (!file)
    {
        cout << "Could not open pieces.txt" << endl;
        return false;
    }

    for (int p = 0; p < 7; p++)
    {
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                file >> pieces[p][r][c];
            }
        }
    }

    file.close();
    return true;
}


// Main function
int main()
{
    sf::RenderWindow window(sf::VideoMode({520, 640}), "Tetris");
    sf::Texture backgroundTexture;

    if (!backgroundTexture.loadFromFile("../../../forest_menu.png"))
    {
        cout << "Could not load forest background!" << endl;
        return 1;
    }

    sf::Sprite backgroundSprite(backgroundTexture);

    float scaleX = 520.f / backgroundTexture.getSize().x;
    float scaleY = 640.f / backgroundTexture.getSize().y;

    float scale = (scaleX > scaleY) ? scaleX : scaleY;

    backgroundSprite.setScale({scale, scale});

    float width = backgroundTexture.getSize().x * scale;
    float height = backgroundTexture.getSize().y * scale;

    backgroundSprite.setPosition({(520.f - width) / 2.f, (640.f - height) / 2.f});

    sf::Texture gameBackgroundTexture;

    if (!gameBackgroundTexture.loadFromFile("../../../game_background.png"))
    {
        cout << "Could not load game background!" << endl;
        return 1;
    }

    sf::Sprite gameBackgroundSprite(gameBackgroundTexture);

    float gameScaleX = 520.f / gameBackgroundTexture.getSize().x;
    float gameScaleY = 640.f / gameBackgroundTexture.getSize().y;

    float gameScale = (gameScaleX > gameScaleY) ? gameScaleX : gameScaleY;

    gameBackgroundSprite.setScale({gameScale, gameScale});

    float gameWidth = gameBackgroundTexture.getSize().x * gameScale;
    float gameHeight = gameBackgroundTexture.getSize().y * gameScale;

    gameBackgroundSprite.setPosition({(520.f - gameWidth) / 2.f, (640.f - gameHeight) / 2.f});
    window.setFramerateLimit(60);

    if (!loadPieces())
    return 1;

    srand(time(0));
    spawnPiece();

    sf::Color colors[8];
    initializeColors(colors);

    sf::RectangleShape block;
    initializeBlock(block);

    sf::Clock clock;
    sf::Font font;

    if (!font.openFromFile("../../../arial.ttf"))
    {
        cout << "Could not load font!" << endl;
        return 1;
    }

    sf::Text scoreText(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({350.f, 30.f});

    sf::Text levelText(font);
    levelText.setCharacterSize(24);
    levelText.setFillColor(sf::Color::White);
    levelText.setPosition({350.f, 80.f});

    sf::Text linesText(font);
    linesText.setCharacterSize(24);
    linesText.setFillColor(sf::Color::White);
    linesText.setPosition({350.f, 130.f});

    sf::Text gameOverText(font);
    gameOverText.setString("GAME OVER!");
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition({130.f, 280.f});

    sf::Text noSaveText(font);
    noSaveText.setString("No saved game!");
    noSaveText.setCharacterSize(30);
    noSaveText.setFillColor(sf::Color::Black);
    noSaveText.setPosition({160.f, 540.f});

    while (window.isOpen())
    {
        handleInput(window, clock);
        if (mainMenu)
        {
            window.clear();

            window.draw(backgroundSprite);

            if (noSaveMessage)
            {
                window.draw(noSaveText);
            }

            window.display();

            continue;
        }

        if (paused)
        {
            clock.restart();
        }

        if (!gameOver && !paused)
        {
            updateFalling();
        }

        window.clear();
        
        window.draw(gameBackgroundSprite);

        drawBoard(window, block, colors);
        drawPiece(window, block, colors);

        scoreText.setString("Score: " + to_string(score));
        levelText.setString("Level: " + to_string(level));
        linesText.setString("Lines: " + to_string(linesCleared));

        window.draw(scoreText);
        window.draw(levelText);
        window.draw(linesText);

        if (gameOver)
        {
            window.draw(gameOverText);
        }

        sf::Text pausedText(font);
        pausedText.setString("PAUSED");
        pausedText.setCharacterSize(40);
        pausedText.setFillColor(sf::Color::Yellow);
        pausedText.setPosition({170.f, 280.f});

        if (paused && !gameOver)
        {
            window.draw(pausedText);
        }

        window.display();
    }

    return 0;
}