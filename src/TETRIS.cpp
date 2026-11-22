#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;

const int ROWS = 20;
const int COLS = 10;
const float CELL = 30.f;
const float BOARD_X = 20.f;
const float BOARD_Y = 20.f;

int board[ROWS][COLS] = {0};

int piece[4][4] = {
    {0, 1, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

int pieceX = 3;
int pieceY = 0;


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
            if (piece[r][c] == 1)
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


// Handle keyboard controls
void handleInput(sf::RenderWindow& window)
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (event->is<sf::Event::KeyPressed>())
        {
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
            if (piece[r][c] == 1)
            {
                board[pieceY + r][pieceX + c] = 3;
            }
        }
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

            pieceX = 3;
            pieceY = 0;
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
            if (piece[r][c] == 1)
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


// Main function
int main()
{
    sf::RenderWindow window(sf::VideoMode({520, 640}), "Tetris");
    window.setFramerateLimit(60);

    sf::Color colors[8];
    initializeColors(colors);

    sf::RectangleShape block;
    initializeBlock(block);

    sf::Clock clock;

    while (window.isOpen())
    {
        handleInput(window);

        updateFalling(clock);

        window.clear();

        drawBoard(window, block, colors);
        drawPiece(window, block, colors);

        window.display();
    }

    return 0;
}