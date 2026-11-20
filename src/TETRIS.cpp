#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;

const int ROWS = 20;
const int COLS = 10;
const float CELL = 30.f;
const float BOARD_X = 20.f;
const float BOARD_Y = 20.f;

int board[ROWS][COLS] = {0};

int main()
{
    sf::RenderWindow window(sf::VideoMode({520, 640}), "Tetris");
    window.setFramerateLimit(60);

    sf::Color colors[8] = {
        sf::Color(25, 25, 45),
        sf::Color::Cyan,
        sf::Color::Yellow,
        sf::Color(170, 90, 255),
        sf::Color::Green,
        sf::Color::Red,
        sf::Color::Blue,
        sf::Color(255, 150, 0)
    };

    sf::RectangleShape block({CELL - 2, CELL - 2});

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear();

        // Board drawing goes here

        window.display();
    }

    return 0;
}