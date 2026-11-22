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

int board[ROWS][COLS] = {0};

char pieces[7][4][4];
char activePiece[4][4];

int currentPiece = 0;
int score = 0;

bool gameOver = false;
bool paused = false;

int pieceX = 3;
int pieceY = 0;

void hardDrop();
void lockPiece();
void clearLines();
void spawnPiece();

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
void handleInput(sf::RenderWindow& window)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (!gameOver && event->is<sf::Event::KeyPressed>())
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
                paused = !paused;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            {
                if (canMove(-1, 0))
                    pieceX--;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            {
                if (canMove(1, 0))
                    pieceX++;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            {
                rotatePiece();
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                if (canMove(0, 1))
                    pieceY++;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
            {
                hardDrop();
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
                board[pieceY + r][pieceX + c] = 3;
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
    }
}

// Handle automatic falling
void updateFalling(sf::Clock& clock)
{
    if (clock.getElapsedTime().asSeconds() >= 0.5f)
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

        clock.restart();
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
                block.setFillColor(colors[3]);

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

    sf::Text gameOverText(font);
    gameOverText.setString("GAME OVER!");
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition({130.f, 280.f});

    while (window.isOpen())
    {
        handleInput(window);

        if (paused)
        {
            clock.restart();
        }

        if (!gameOver && !paused)
        {
            updateFalling(clock);
        }

        window.clear();

        drawBoard(window, block, colors);
        drawPiece(window, block, colors);

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