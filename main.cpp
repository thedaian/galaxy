#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <SFML/Graphics.hpp>
#include "vertex_wrapper.hpp"

const uint8_t TILE_SIZE = 50;

bool isRandomPercent(int percent)
{
    return (getRandomInt(0, 100) >= percent);
}

int getRandomInt(int min, int max)
{
    static std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr))); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> dis(min, max + 1);

    return dis(gen);
}

void takeScreenshot(sf::RenderWindow & window)
{
    static short number = 1;

    std::string filename;
    //filename<<"screenshot"<<number<<".png";
    filename = "screenshot.png";

    sf::Image Screen = window.capture();
    Screen.saveToFile(filename);
    number++;
}

sf::Vector2f generateStarmap(std::vector<Star::star> & _list, const int& _y, const int& _x)
{
    sf::Vector2f homeworld_pos;
    _list.clear();
    const uint8_t SEPERATION = (Star::STAR_SIZE/2);
    unsigned short xOffset(0), yOffset(0), width(_x/TILE_SIZE);
    double totalCounts;

    totalCounts = width;
    totalCounts = floor(totalCounts * ((_y/TILE_SIZE) - 1));

    int homeworld;
    int currentClassCount = totalCounts * 0.70;
    Star::type currentClass = Star::type::M;
    std::vector<Star::type> typeDist;
    for(unsigned short i = 0; i <= totalCounts; i++)
    {
        typeDist.push_back(currentClass);
        if(i == currentClassCount)
        {
            switch(currentClass)
            {
            case Star::type::O://0
            case Star::type::B://1
                currentClass = Star::type::O;
                currentClassCount = totalCounts * 0.01;
                break;
            case Star::type::A://2
                currentClass = Star::type::B;
                currentClassCount = totalCounts * 0.01;
                break;
            case Star::type::F://3
                currentClass = Star::type::A;
                currentClassCount = totalCounts * 0.01;
                break;
            case Star::type::G://4
                currentClass = Star::type::F;
                currentClassCount = totalCounts * 0.03;
                break;
            case Star::type::K://5
                currentClass = Star::type::G;
                currentClassCount = totalCounts * 0.14;
                homeworld = getRandomInt(0, currentClassCount);
                break;
            default:
            case Star::type::M://6
                currentClass = Star::type::K;
                currentClassCount = totalCounts * 0.12;
                break;
            }
            currentClassCount += i;
        }
    }

    std::mt19937 g(static_cast<unsigned int>(std::time(nullptr)));

    std::shuffle(typeDist.begin(), typeDist.end(), g);

    while(yOffset + TILE_SIZE * 2 <= _y)
    {
        _list.push_back(Star::star(getRandomInt(SEPERATION, TILE_SIZE - SEPERATION) + xOffset + TILE_SIZE/4, getRandomInt(SEPERATION, TILE_SIZE - SEPERATION) + yOffset + TILE_SIZE, typeDist.front(), _list.size()));
        if(typeDist.front() == Star::type::G && homeworld > 0)
        {
            homeworld--;
            if(homeworld == 0)
            {
                _list.back().m_production = _list.back().m_maxProduction/2;
                _list.back().m_population = _list.back().m_maxPopulation/2;
                _list.back().m_supplies = _list.back().m_maxProduction/2;
                homeworld_pos = _list.back().m_pos;
            }
        }
        typeDist.erase(typeDist.begin());
        xOffset += TILE_SIZE;
        if(xOffset + TILE_SIZE >= _x)
        {
            xOffset = 0;
            yOffset += TILE_SIZE;
        }
    }

    for (unsigned int i = 0; i < _list.size(); i++)
    {
        if(i%width != 0)
        {
            _list[i].add(&_list[i - 1]);
        }
        if((i + 1)%width != 0)
        {
            _list[i].add(&_list[i + 1]);
        }
        if(i > width)
        {
            _list[i].add(&_list[i - width]);
        }
        if(i + width < _list.size())
        {
            _list[i].add(&_list[i + width]);
        }
    }
    return homeworld_pos;
}

bool addColony(Star::star& _starar, shipmapRender& _ships)
{
    Star::star* nearby = _starar.isNearbySupplied();
    if(nearby != nullptr)
    {
        _ships.add(nearby, &_starar);
        return true;
    }
    return false;
}

void updateYear(sf::Text& _text, const unsigned int& year)
{
    std::stringstream t;
    t<<year;
    _text.setString(t.str());
}

void save(const unsigned int& _year, const uint8_t& _year_clock, const std::vector<Star::star>& _starlist, const shipmapRender& _ships)
{
    std::ofstream out("autosave.sav");
    if(out.is_open())
    {
        out<<_year<<" "<<static_cast<unsigned int>(_year_clock)<<"\n";
        for(auto _star = _starlist.begin(); _star != _starlist.end(); _star++)
        {
            out<<_star->m_pos.x<<" "<<_star->m_pos.y<<" "<<static_cast<unsigned int>(_star->m_size)<<" "<<static_cast<unsigned int>(_star->m_type)<<" "
                <<_star->m_supplies<<" "<<_star->m_production<<" "<<_star->m_maxProduction<<" "<<_star->m_population<<" "<<_star->m_maxPopulation<<"\n";
        }
        out<<"-\n";
        for(auto _ship = _ships.shipsBegin(); _ship != _ships.shipsEnd(); _ship++)
        {
            out<<_ship->m_pos->m_index<<" "<<_ship->m_goal->m_index<<" "<<static_cast<unsigned int>(_ship->dt)<<" "<<_ship->m_hasSupplies<<" "<<_ship->m_hasPopulation<<"\n";
        }
        out.close();
    }
}

bool load(unsigned int& _year, uint8_t& _year_clock, std::vector<Star::star>& _starlist, shipmapRender& _ships)
{
    std::ifstream in("autosave.sav");
    if(in.is_open())
    {
        in>>_year>>" ">>_year_clock>>"\n";
        for(auto _star = _starlist.begin(); _star != _starlist.end(); _star++)
        {
            in>>_star->m_pos.x>>" ">>_star->m_pos.y>>" ">>_star->m_size>>" ">>static_cast<int>(_star->m_type)>>" "
                >>_star->m_supplies>>" ">>_star->m_production>>" ">>_star->m_maxProduction>>" ">>_star->m_population>>" ">>_star->m_maxPopulation>>"\n";
        }
        for(auto _ship = _ships.shipsBegin(); _ship != _ships.shipsEnd(); _ship++)
        {
            in>>_ship->m_pos->m_index>>" ">>_ship->m_goal->m_index>>" ">>_ship->dt>>" ">>_ship->m_hasSupplies>>" ">>_ship->m_hasPopulation>>"\n";
        }
        in.close();
        return true;
    }
    return false;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 960), "Space test: F8/F11 = Screenshot, Space = reset");
    window.setFramerateLimit(60);

    int width = (window.getSize().x/TILE_SIZE);
    unsigned int currentSelection(0), year(1234);
    uint8_t year_clock(0);
    sf::Clock _timer;
    int old_mouse_x(0), old_mouse_y(0);
    bool hasFocus(true);

    sf::Font font;
    if(!font.loadFromFile("Dos_font.ttf"))
    {
        sf::err()<<"Error loading Dos_font.ttf";
        return 1;
    }

    sf::Text header("Select a Star", font, 32);
    header.setPosition(window.getSize().x/8, -4);

    sf::Text date("1234", font, 32);
    date.setPosition(10, -4);
    updateYear(date, year);

    sf::CircleShape cursor(Star::STAR_SIZE + 2, 6);
    cursor.setFillColor(sf::Color::Transparent);
    cursor.setOutlineColor(sf::Color::White);
    cursor.setOrigin(cursor.getRadius(), cursor.getRadius());
    cursor.setOutlineThickness(2);

    sf::RectangleShape year_display(sf::Vector2f(0, 40));
    year_display.setFillColor(sf::Color(0, 128, 0, 196));
    year_display.setPosition(0, 0);

    std::vector<Star::star> starlist;
    starmapRender starRender;
    shipmapRender shipRender;
    bool isPlaying(true), runIntro(false);
    sf::Vector2f start_pos(0, 0);

    if(!load(year, year_clock, starlist, shipRender))
    {
        start_pos = generateStarmap(starlist, window.getSize().y, window.getSize().x);
        starRender.load(starlist, window.getSize().x, window.getSize().y, TILE_SIZE);
        isPlaying = false;
        runIntro = true;
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
            {
                window.close();
            }
            if((event.type == sf::Event::LostFocus) || (event.type == sf::Event::MouseLeft))
            {
                hasFocus = false;
            }
            if((event.type == sf::Event::GainedFocus) || (event.type == sf::Event::MouseEntered))
            {
                hasFocus = true;
            }
            if((event.type == sf::Event::KeyReleased) && (hasFocus))
            {
                switch(event.key.code)
                {
                    case sf::Keyboard::F8:
                    case sf::Keyboard::F11:
                        takeScreenshot(window);
                        break;
                    case sf::Keyboard::F12:
                        generateStarmap(starlist, window.getSize().y, window.getSize().x);
                        starRender.load(starlist, window.getSize().x, window.getSize().y, TILE_SIZE);
                        break;
                    case sf::Keyboard::Space:
                        isPlaying = !isPlaying;
                        break;
                    case sf::Keyboard::G:
                        starRender.toggleGrid();
                        break;
                    case sf::Keyboard::Escape:
                        window.close();
                    default:
                        break;
                }
            }
            if((event.type == sf::Event::MouseMoved) && (hasFocus))
            {
                if(abs(event.mouseMove.x - old_mouse_x) > 10 || abs(event.mouseMove.y - old_mouse_y) > 10)
                {
                    int _x = (event.mouseMove.x - TILE_SIZE/4)/TILE_SIZE;
                    int _y = (event.mouseMove.y - TILE_SIZE)/TILE_SIZE;
                    currentSelection = (_y * width) + _x;
                    if((currentSelection < starlist.size()) && currentSelection >= 0)
                    {
                        header.setString(starlist.at(currentSelection).toString());
                        cursor.setPosition(starlist.at(currentSelection).m_pos);
                    } else {
                        currentSelection = 0;
                    }
                    old_mouse_x = event.mouseMove.x;
                    old_mouse_y = event.mouseMove.y;
                }
            }
            if((event.type == sf::Event::MouseButtonReleased) && (hasFocus))
            {
                if(!addColony(starlist.at(currentSelection), shipRender))
                {
                    header.setString("Cannot colonize so far from a populated star.");
                }
            }
        }

        if(isPlaying && _timer.getElapsedTime().asMilliseconds() > 50)
        {
            year_clock++;
            if(year_clock >= 100)
            {
                year_clock = 0;
                year++;
                updateYear(date, year);
                for(auto st = starlist.begin(); st != starlist.end(); st++)
                {
                    st->update();
                }
                header.setString(starlist.at(currentSelection).toString());
            }
            shipRender.update(starRender);
            _timer.restart();
            year_display.setSize(sf::Vector2f( window.getSize().x * (year_clock/100.f), 40) );
        }

        window.clear();
        window.draw(year_display);
        window.draw(shipRender);
        window.draw(starRender);
        window.draw(header);
        window.draw(date);
        window.draw(cursor);
        window.display();
    }

    save(year, year_clock, starlist, shipRender);

    return 0;
}
