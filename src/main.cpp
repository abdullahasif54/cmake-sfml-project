
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>

namespace
{
    constexpr unsigned WIN_W = 800;
    constexpr unsigned WIN_H = 600;
    constexpr unsigned PANEL_W = 180;
    constexpr unsigned BAR_H = 30;
    constexpr unsigned CANVAS_W = WIN_W - PANEL_W;
    constexpr unsigned CANVAS_H = WIN_H - BAR_H;

    constexpr double PI = 3.14159265358979323846;

    const std::vector<std::string> COLORS = {
        "red", "green", "blue", "magenta",
        "cyan", "yellow", "white", "black"
    };

    std::string trim(const std::string& s)
    {
        size_t a = s.find_first_not_of(" \t");
        if (a == std::string::npos)
            return "";

        size_t b = s.find_last_not_of(" \t");
        return s.substr(a, b - a + 1);
    }

    sf::Color getColor(const std::string& name)
    {
        if (name == "red") return sf::Color::Red;
        if (name == "green") return sf::Color::Green;
        if (name == "blue") return sf::Color::Blue;
        if (name == "magenta") return sf::Color::Magenta;
        if (name == "cyan") return sf::Color::Cyan;
        if (name == "yellow") return sf::Color::Yellow;
        if (name == "black") return sf::Color::Black;

        return sf::Color::White;
    }

    struct Command
    {
        std::string name;
        int number = 0;
        std::string argument;
        std::vector<Command> body;
    };

    bool validNumber(const std::string& s, int& number)
    {
        if (s.empty() || s.size() > 9)
            return false;

        for (char c : s)
            if (c < '0' || c > '9')
                return false;

        try
        {
            number = std::stoi(s);
        }
        catch (...)
        {
            return false;
        }

        return number > 0;
    }

    bool validFileName(const std::string& s)
    {
        if (s.empty() || !std::isalpha(
            static_cast<unsigned char>(s[0])))
            return false;

        for (char c : s)
        {
            if (!std::isalnum(static_cast<unsigned char>(c)))
                return false;
        }

        return true;
    }

    bool parseCommand(const std::string& input,
                      Command& cmd,
                      bool insideRepeat = false,
                      bool allowFile = true);

    bool parseRepeat(const std::string& rest, Command& cmd)
    {
        std::istringstream stream(rest);

        std::string countText;
        if (!(stream >> countText))
            return false;

        int count;
        if (!validNumber(countText, count) || count > 10000)
            return false;

        std::string remaining;
        std::getline(stream, remaining);
        remaining = trim(remaining);

        if (remaining.size() < 3 ||
            remaining.front() != '[' ||
            remaining.back() != ']')
            return false;

        remaining = remaining.substr(1, remaining.size() - 2);

        if (remaining.find('[') != std::string::npos ||
            remaining.find(']') != std::string::npos)
            return false;

        std::istringstream commands(remaining);
        std::string keyword, numberText;

        while (commands >> keyword)
        {
            if (!(commands >> numberText))
                return false;

            Command item;

            if (keyword != "fd" &&
                keyword != "forward" &&
                keyword != "bk" &&
                keyword != "backward" &&
                keyword != "rt" &&
                keyword != "rotate" &&
                keyword != "lt" &&
                keyword != "circle")
                return false;

            if (!validNumber(numberText, item.number))
                return false;

            item.name = keyword;
            cmd.body.push_back(item);
        }

        if (cmd.body.empty())
            return false;

        cmd.name = "repeat";
        cmd.number = count;
        return true;
    }

    bool parseCommand(const std::string& input,
                      Command& cmd,
                      bool insideRepeat,
                      bool allowFile)
    {
        std::string s = trim(input);

        std::istringstream stream(s);
        std::string keyword;

        if (!(stream >> keyword))
            return false;

        std::string rest;
        std::getline(stream, rest);
        rest = trim(rest);

        cmd = Command();

        if (keyword == "repeat" && !insideRepeat)
            return parseRepeat(rest, cmd);

        if (keyword == "cs" ||
            keyword == "penup" ||
            keyword == "pendown")
        {
            if (!rest.empty() || insideRepeat)
                return false;

            cmd.name = keyword;
            return true;
        }

        if (keyword == "color" && !insideRepeat)
        {
            if (std::find(COLORS.begin(), COLORS.end(), rest)
                == COLORS.end())
                return false;

            cmd.name = "color";
            cmd.argument = rest;
            return true;
        }

        if ((keyword == "save" || keyword == "load") &&
            allowFile && !insideRepeat)
        {
            if (!validFileName(rest))
                return false;

            cmd.name = keyword;
            cmd.argument = rest;
            return true;
        }

        if (insideRepeat)
        {
            if (keyword != "fd" && keyword != "forward" &&
                keyword != "bk" && keyword != "backward" &&
                keyword != "rt" && keyword != "rotate" &&
                keyword != "lt" && keyword != "circle")
                return false;
        }
        else
        {
            if (keyword != "fd" && keyword != "forward" &&
                keyword != "bk" && keyword != "backward" &&
                keyword != "rt" && keyword != "rotate" &&
                keyword != "lt" && keyword != "leftrotate" &&
                keyword != "rightrotate" && keyword != "circle" &&
                keyword != "width")
                return false;
        }

        if (!validNumber(rest, cmd.number))
            return false;

        cmd.name = keyword;
        return true;
    }

    class Turtle
    {
    public:
        explicit Turtle(sf::RenderTexture& target)
            : canvas(target)
        {
            reset();
        }

        void reset()
        {
            x = CANVAS_W / 2.0;
            y = CANVAS_H / 2.0;
            angle = 0;
            penWidth = 2;
            penDown = true;
            color = sf::Color::White;

            canvas.clear(sf::Color::Black);
            canvas.display();
        }

        void run(const Command& cmd)
        {
            const std::string& n = cmd.name;

            if (n == "fd" || n == "forward")
                move(cmd.number);

            else if (n == "bk" || n == "backward")
                move(-cmd.number);

            else if (n == "rt" || n == "rotate" ||
                     n == "rightrotate")
                angle += cmd.number;

            else if (n == "lt" || n == "leftrotate")
                angle -= cmd.number;

            else if (n == "circle")
                drawCircle(cmd.number);

            else if (n == "width")
                penWidth = cmd.number;

            else if (n == "color")
                color = getColor(cmd.argument);

            else if (n == "penup")
                penDown = false;

            else if (n == "pendown")
                penDown = true;

            else if (n == "cs")
                reset();

            else if (n == "repeat")
            {
                for (int i = 0; i < cmd.number; ++i)
                {
                    for (const auto& item : cmd.body)
                        run(item);
                }
            }
        }

        void finish()
        {
            canvas.display();
        }

        void drawTurtle(sf::RenderTarget& target) const
        {
            double r = angle * PI / 180.0;

            double cx = std::cos(r);
            double cy = std::sin(r);

            sf::ConvexShape triangle(3);

            triangle.setPoint(0, {
                static_cast<float>(x + cx * 12),
                static_cast<float>(y + cy * 12)
            });

            triangle.setPoint(1, {
                static_cast<float>(x - cx * 6 - cy * 7),
                static_cast<float>(y - cy * 6 + cx * 7)
            });

            triangle.setPoint(2, {
                static_cast<float>(x - cx * 6 + cy * 7),
                static_cast<float>(y - cy * 6 - cx * 7)
            });

            triangle.setFillColor(sf::Color(60, 220, 90));
            target.draw(triangle);
        }

    private:
        sf::RenderTexture& canvas;

        double x = CANVAS_W / 2.0;
        double y = CANVAS_H / 2.0;
        double angle = 0;
        double penWidth = 2;

        bool penDown = true;
        sf::Color color = sf::Color::White;

        void move(double distance)
        {
            double r = angle * PI / 180.0;

            double nx = x + std::cos(r) * distance;
            double ny = y + std::sin(r) * distance;

            if (penDown)
                drawLine(x, y, nx, ny);

            x = nx;
            y = ny;
        }

        void drawLine(double x0, double y0,
                      double x1, double y1)
        {
            double dx = x1 - x0;
            double dy = y1 - y0;

            float length = static_cast<float>(
                std::hypot(dx, dy));

            float w = static_cast<float>(penWidth);

            sf::RectangleShape line({length + w, w});

            line.setOrigin({w / 2.f, w / 2.f});
            line.setPosition({
                static_cast<float>(x0),
                static_cast<float>(y0)
            });

            line.setRotation(sf::degrees(
                static_cast<float>(
                    std::atan2(dy, dx) * 180.0 / PI)));

            line.setFillColor(color);
            canvas.draw(line);
        }

        void drawCircle(double radius)
        {
            unsigned points = static_cast<unsigned>(
                std::clamp(radius * 2.0, 30.0, 720.0));

            sf::CircleShape circle(
                static_cast<float>(radius), points);

            circle.setOrigin({
                static_cast<float>(radius),
                static_cast<float>(radius)
            });

            circle.setPosition({
                static_cast<float>(x),
                static_cast<float>(y)
            });

            circle.setFillColor(sf::Color::Transparent);
            circle.setOutlineColor(color);
            circle.setOutlineThickness(
                static_cast<float>(penWidth));

            canvas.draw(circle);
        }
    };

    class App
    {
    public:
        explicit App(sf::RenderTexture& canvas)
            : turtle(canvas)
        {
        }

        Turtle turtle;
        std::vector<std::string> history;
        std::string error;

        void submit(const std::string& input)
        {
            if (trim(input).empty())
                return;

            Command cmd;

            if (!parseCommand(input, cmd))
            {
                error = "Invalid or incomplete command";
                return;
            }

            if (cmd.name == "save")
            {
                save(cmd.argument);
                return;
            }

            if (cmd.name == "load")
            {
                load(cmd.argument);
                return;
            }

            turtle.run(cmd);
            turtle.finish();

            history.push_back(input);
            error.clear();
        }

    private:
        static std::string fileName(const std::string& name)
        {
            return name + ".txt";
        }

        void save(const std::string& name)
        {
            std::ofstream file(fileName(name));

            if (!file)
            {
                error = "Could not save file";
                return;
            }

            for (const auto& command : history)
                file << command << '\n';

            error.clear();
        }

        void load(const std::string& name)
        {
            std::ifstream file(fileName(name));

            if (!file)
            {
                error = "Could not open file";
                return;
            }

            std::vector<std::string> lines;
            std::vector<Command> commands;

            std::string line;

            while (std::getline(file, line))
            {
                Command cmd;

                if (lines.size() >= 100000 ||
                    line.size() > 1000 ||
                    !parseCommand(line, cmd, false, false))
                {
                    error = "Invalid drawing file";
                    return;
                }

                lines.push_back(line);
                commands.push_back(cmd);
            }

            if (file.bad())
            {
                error = "Could not read file";
                return;
            }

            turtle.reset();
            history.clear();

            for (size_t i = 0; i < commands.size(); ++i)
            {
                turtle.run(commands[i]);
                history.push_back(lines[i]);
            }

            turtle.finish();
            error.clear();
        }
    };

    std::string fitText(sf::Text& text,
                        std::string value,
                        float maxWidth)
    {
        text.setString(value);

        if (text.getLocalBounds().size.x <= maxWidth)
            return value;

        while (!value.empty())
        {
            value.pop_back();
            text.setString(value + "...");

            if (text.getLocalBounds().size.x <= maxWidth)
                return value + "...";
        }

        return "...";
    }
}

int main()
{
    sf::Font font;

    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        std::cerr << "Could not load Arial font.\n";
        return 1;
    }

    sf::RenderWindow window(
        sf::VideoMode({WIN_W, WIN_H}),
        "Turtle Graphics Drawing Pad",
        sf::Style::Titlebar | sf::Style::Close
    );

    window.setFramerateLimit(60);

    sf::RenderTexture canvas({CANVAS_W, CANVAS_H});
    canvas.clear(sf::Color::Black);
    canvas.display();

    sf::Sprite canvasSprite(canvas.getTexture());

    App app(canvas);
    std::string input;

    const sf::Color panelColor(16, 20, 28);
    const sf::Color accent(0, 200, 210);

    sf::Text text(font);
    text.setCharacterSize(14);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            else if (const auto* entered =
                event->getIf<sf::Event::TextEntered>())
            {
                char32_t unicode = entered->unicode;

                if (unicode == 8)
                {
                    if (!input.empty())
                        input.pop_back();
                }
                else if (unicode >= 32 &&
                         unicode < 127 &&
                         input.size() < 200)
                {
                    input.push_back(static_cast<char>(unicode));
                }
            }

            else if (const auto* key =
                event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Enter)
                {
                    try
                    {
                        app.submit(input);
                    }
                    catch (...)
                    {
                        app.error = "Invalid command";
                    }

                    input.clear();
                }

                if (key->code == sf::Keyboard::Key::Escape)
                    input.clear();
            }
        }

        window.clear(sf::Color::Black);

        // Drawing canvas and turtle
        window.draw(canvasSprite);
        app.turtle.drawTurtle(window);

        // Right history panel
        sf::RectangleShape panel({
            static_cast<float>(PANEL_W),
            static_cast<float>(WIN_H)
        });

        panel.setPosition({
            static_cast<float>(CANVAS_W), 0
        });

        panel.setFillColor(panelColor);
        window.draw(panel);

        // Bottom command bar
        sf::RectangleShape bar({
            static_cast<float>(CANVAS_W),
            static_cast<float>(BAR_H)
        });

        bar.setPosition({
            0, static_cast<float>(CANVAS_H)
        });

        bar.setFillColor(panelColor);
        window.draw(bar);

        // Separators
        sf::RectangleShape verticalLine({
            1.f, static_cast<float>(WIN_H)
        });

        verticalLine.setPosition({
            static_cast<float>(CANVAS_W), 0
        });

        verticalLine.setFillColor(accent);
        window.draw(verticalLine);

        sf::RectangleShape horizontalLine({
            static_cast<float>(CANVAS_W), 1.f
        });

        horizontalLine.setPosition({
            0, static_cast<float>(CANVAS_H)
        });

        horizontalLine.setFillColor(accent);
        window.draw(horizontalLine);

        // History heading
        text.setFillColor(sf::Color::White);
        text.setCharacterSize(14);
        text.setString("HISTORY");
        text.setPosition({
            static_cast<float>(CANVAS_W + 12), 10
        });

        window.draw(text);

        text.setFillColor(sf::Color(130, 140, 150));
        text.setCharacterSize(10);
        text.setString("Last 5 commands");
        text.setPosition({
            static_cast<float>(CANVAS_W + 12), 30
        });

        window.draw(text);

        // History entries
        text.setFillColor(sf::Color(200, 205, 215));
        text.setCharacterSize(12);

        size_t first = app.history.size() > 5
            ? app.history.size() - 5 : 0;

        float historyY = 56;

        for (size_t i = first;
             i < app.history.size();
             ++i, historyY += 20)
        {
            text.setString(fitText(
                text, app.history[i], PANEL_W - 24));

            text.setPosition({
                static_cast<float>(CANVAS_W + 12),
                historyY
            });

            window.draw(text);
        }

        // Error message
        if (!app.error.empty())
        {
            text.setFillColor(sf::Color(255, 60, 60));
            text.setCharacterSize(14);

            text.setString(fitText(
                text, app.error, CANVAS_W - 16));

            text.setPosition({8, static_cast<float>(CANVAS_H - 24)});
            window.draw(text);
        }

        // Command input
        text.setFillColor(sf::Color::White);
        text.setCharacterSize(14);

        std::string shown = input;

        while (true)
        {
            text.setString("Command: " + shown + "_");

            if (text.getLocalBounds().size.x <= CANVAS_W - 16 ||
                shown.empty())
                break;

            shown.erase(0, 1);
        }

        text.setPosition({8, static_cast<float>(CANVAS_H + 6)});
        window.draw(text);

        window.display();
    }

    return 0;
}
