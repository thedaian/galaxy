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

void takeScreenshot(sf::RenderWindow  &window)
{
    static short number = 1;

    std::string filename;
    //filename<<"screenshot"<<number<<".png";
    filename = "screenshot.png";

    sf::Image Screen = window.capture();
    Screen.saveToFile(filename);
    number++;
}

void connectStars(std::vector<Star::star> &_list, const unsigned short &width)
{
    for(unsigned int i = 0; i < _list.size(); i++)
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
}

sf::Vector2f generateStarmap(std::vector<Star::star> &_list, const int &_y, const int &_x)
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

    connectStars(_list, width);

    return homeworld_pos;
}

bool addColony(Star::star &_starar, shipmapRender &_ships)
{
    Star::star* nearby = _starar.isNearbySupplied();
    if(nearby != nullptr)
    {
        _ships.add(nearby, &_starar);
        return true;
    }
    return false;
}

void updateYear(sf::Text &_text, const unsigned int &year)
{
    std::stringstream t;
    t<<year;
    _text.setString(t.str());
}

void save(const unsigned int &_year, const uint8_t &_year_clock, const std::vector<Star::star> &_starlist, const shipmapRender &_ships)
{
    std::ofstream out("autosave.sav");
    if(out.is_open())
    {
        out<<_year<<" "<<static_cast<unsigned int>(_year_clock)<<" "<<_starlist.size()<<"\n";
        for(const auto &_star : _starlist)
        {
            out<<_star.m_pos.x<<" "<<_star.m_pos.y<<" "<<static_cast<unsigned int>(_star.m_size)<<" "<<static_cast<unsigned int>(_star.m_type)<<" "
                <<_star.m_supplies<<" "<<_star.m_production<<" "<<_star.m_maxProduction<<" "<<_star.m_population<<" "<<_star.m_maxPopulation<<"\n";
        }
        for(auto _ship = _ships.shipsBegin(); _ship != _ships.shipsEnd(); _ship++)
        {
            out<<_ship->m_pos->m_index<<" "<<_ship->m_goal->m_index<<" "<<static_cast<unsigned int>(_ship->dt)<<" "<<_ship->m_hasSupplies<<" "<<_ship->m_hasPopulation<<"\n";
        }
        out.close();
    }
}

bool load(unsigned int &_year, uint8_t &_year_clock, std::vector<Star::star> &_starlist, shipmapRender &_ships, const int &_x)
{
    std::ifstream in("autosave.sav");
    if(in.is_open())
    {
        sf::Vector2f pos(0, 0);
        unsigned int start(0), goal(0), dt(0), type(0), size(0), year_clock(0);
        bool s_supplies, s_population;
        int supplies(0), production(0), maxProduction(0), population(0), maxPopulation(0), star_size(0);

        in>>_year>>year_clock>>star_size;
        _year_clock = year_clock;

        for(int i = 0; i < star_size; i++)
        {
            in>>pos.x>>pos.y>>size>>type>>supplies>>production>>maxProduction>>population>>maxPopulation;

            _starlist.push_back( Star::star(pos.x, pos.y, static_cast<Star::type>(type), _starlist.size()) );
            _starlist.back().m_size = size;
            _starlist.back().m_supplies = supplies;
            _starlist.back().m_production = production;
            _starlist.back().m_maxProduction = maxProduction;
            _starlist.back().m_population = population;
            _starlist.back().m_maxPopulation = maxPopulation;
        }
        connectStars(_starlist, _x/TILE_SIZE);

        while(in>>start)
        {
            in>>goal>>dt>>s_supplies>>s_population;

            _ships.load(&_starlist[start], &_starlist[goal], dt, s_supplies, s_population);
        }
        in.close();
        return true;
    }
    return false;
}

int main(int argc, char* argv[])
{
    sf::RenderWindow window(sf::VideoMode(1280, 960), "Space test: F8/F11 = Screenshot, Space = reset");
    window.setFramerateLimit(60);

    int width = (window.getSize().x/TILE_SIZE);
    unsigned int currentSelection(0), year(1234);
    uint8_t year_clock(0);
    sf::Clock _timer;
    int old_mouse_x(0), old_mouse_y(0);
    bool hasFocus(true), runIntro(false), shrinking(false), isPlaying(true), autoplay(false);

    if(argc > 1 && (std::string(argv[1]) == "-auto"))
    {
        autoplay = true;
    }

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

    sf::Vector2f start_pos(0, 0), gui_start_pos(0, 0);
    sf::Text _intro;
    sf::RectangleShape _bg;

    if(!load(year, year_clock, starlist, shipRender, window.getSize().x))
    {
        start_pos = generateStarmap(starlist, window.getSize().y, window.getSize().x);

        isPlaying = false;
        runIntro = true;

        _intro.setString("Welcome to the galaxy. Populate everything to win.\n<Space>: Pause. <g>: Toggle Grid. <Escape>: Close Game");
        _intro.setFont(font);
        _intro.setPosition(window.getSize().x/2 - _intro.getGlobalBounds().width/2, window.getSize().y/2 - _intro.getGlobalBounds().height/2);

        _bg.setSize(sf::Vector2f(_intro.getGlobalBounds().width + 10, _intro.getGlobalBounds().height + 10));
        _bg.setFillColor(sf::Color::Black);
        _bg.setOutlineColor(sf::Color::White);
        _bg.setOutlineThickness(1);
        _bg.setOrigin(_bg.getSize().x/2, 0);
        gui_start_pos = sf::Vector2f(_intro.getGlobalBounds().left - 5 + _bg.getSize().x/2, _intro.getGlobalBounds().top - 5);
        _bg.setPosition(gui_start_pos);
    }
    starRender.load(starlist, window.getSize().x, window.getSize().y, TILE_SIZE);

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
                if(runIntro)
                {
                    shrinking = true;
                    isPlaying = false;
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
                if(runIntro)
                {
                    shrinking = true;
                } else {
                    if(!addColony(starlist.at(currentSelection), shipRender))
                    {
                        header.setString("Cannot colonize so far from a populated star.");
                    }
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
                for(auto &st : starlist)
                {
                    if(autoplay)
                    {
                        addColony(st, shipRender);
                    }
                    st.update();
                }
                header.setString(starlist.at(currentSelection).toString());
            }
            shipRender.update(starRender);
            _timer.restart();
            year_display.setSize(sf::Vector2f( window.getSize().x * (year_clock/100.f), 40) );
        }

        if(shrinking && _timer.getElapsedTime().asMilliseconds() > 25)
        {
            year_clock++;

            if(_bg.getSize().y > 10)
            {
                _bg.setSize(sf::Vector2f((_intro.getGlobalBounds().width + 10)/(year_clock/2), _bg.getSize().y - 1));
            } else {
                _bg.setSize(sf::Vector2f((_intro.getGlobalBounds().width + 10)/(year_clock/2), _bg.getSize().y));
            }
            _bg.setOrigin(_bg.getSize().x/2, 0);
            _bg.setPosition(gui_start_pos.x + (start_pos.x - gui_start_pos.x) * year_clock/100.f, gui_start_pos.y + (start_pos.y - gui_start_pos.y) * year_clock/100.f);

            _timer.restart();
            if(year_clock >= 100)
            {
                year_clock = 0;
                shrinking = false;
                runIntro = false;
                isPlaying = true;
            }
        }

        window.clear();
        window.draw(year_display);
        window.draw(shipRender);
        window.draw(starRender);
        window.draw(header);
        window.draw(date);
        window.draw(cursor);
        if(runIntro)
        {
            window.draw(_bg);
            if(!shrinking)
            {
                window.draw(_intro);
            }
        }

        window.display();
    }

    save(year, year_clock, starlist, shipRender);

    return 0;
}
