#pragma once

#include <SFML/System/Vector2.hpp>
#include <sstream>
#include <iomanip>

int getRandomInt(int min, int max);

namespace Star {

    //O and B stars are uncommon but very bright; M stars are common but dim..
    enum class type: uint8_t {
            //colour        temp                mass    radius  lums
        O,  //Blue	        over 25,000 K	    60	    15	    1,400,000
        B,  //Blue	        11,000 - 25,000 K	18	    7	    20,000
        A,  //Blue	        7,500 - 11,000 K	3.2	    2.5	    80
        F,  //Blue-White	6,000 - 7,500 K	    1.7	    1.3	    6
        G,  //White-Yellow  5,000 - 6,000 K	    1.1	    1.1	    1.2
        K,  //Orange-Red	3,500 - 5,000 K	    0.8	    0.9	    0.4
        M   //Red	        under 3,500 K	    0.3	    0.4	    0.04
    };

    const uint8_t STAR_SIZE = 14;
    const uint8_t MIN_SUPPLIES = 5;
    const uint8_t MIN_POP = 5;

    struct star
    {
        sf::Vector2f m_pos;
        sf::Color m_colour;
        uint8_t m_size;
        type m_type;
        int m_supplies, m_production, m_maxProduction;
        int m_population, m_maxPopulation;
        int m_index;
        std::vector<Star::star*> m_connected;

        star(const int& _x, const int& _y, const Star::type& _s, const int& _index) {
            m_pos.x = _x;
            m_pos.y = _y;
            m_size = Star::STAR_SIZE + getRandomInt(-1, 1);
            m_type = _s;
            m_index = _index;
            m_supplies = 0;
            m_production = 0;
            m_population = 0;
            m_maxProduction = 10;
            m_maxPopulation = 10;

            switch(_s)
            {
            case Star::type::O:
                m_colour = sf::Color(146,181,255);
                m_maxProduction *= 8;
                m_maxPopulation *= 8;
                m_size += 5;
                break;
            case Star::type::B:
                m_colour = sf::Color(162, 192, 255);
                m_maxProduction *= 6;
                m_maxPopulation *= 6;
                m_size += 2;
                break;
            case Star::type::A:
                m_colour = sf::Color(213, 224, 255);
                m_maxProduction *= 5;
                m_maxPopulation *= 5;
                break;
            case Star::type::F:
                m_colour = sf::Color(249, 245, 255);
                m_maxProduction *= 4;
                m_maxPopulation *= 4;
                break;
            case Star::type::G:
                m_colour = sf::Color(255, 237, 227);
                m_maxProduction *= 3;
                m_maxPopulation *= 3;
                break;
            case Star::type::K:
                m_colour = sf::Color(255, 218, 181);
                m_maxProduction *= 2;
                m_maxPopulation *= 2;
                break;
            default:
            case Star::type::M:
                m_colour = sf::Color(255, 181, 108);
                m_size -= 4;
                break;
            }
        }

        void add(Star::star* c)
        {
            m_connected.push_back(c);
        }

        Star::star* isNearbySupplied()
        {
            for(auto n = m_connected.begin(); n < m_connected.end(); n++)
            {
                if( ((*n)->m_supplies > Star::MIN_SUPPLIES) && ((*n)->m_population > Star::MIN_POP) )
                {
                    return (*n);
                }
            }
            return nullptr;
        }

        void update()
        {
            if(m_population > 0)
            {
                m_supplies += m_production;
                if(m_population < m_maxPopulation)
                {
                    m_population++;
                }
                if( (m_population < m_maxPopulation/2) && (m_production < m_maxProduction) )
                {
                    m_production++;
                }
            }

        }

        std::string toString() const
        {
            std::stringstream t;
            t<<"Type: ";
            switch(m_type)
            {
            case Star::type::O:
                t<<"O";
                break;
            case Star::type::B:
                t<<"B";
                break;
            case Star::type::A:
                t<<"A";
                break;
            case Star::type::F:
                t<<"F";
                break;
            case Star::type::G:
                t<<"G";
                break;
            case Star::type::K:
                t<<"K";
                break;
            default:
            case Star::type::M:
                t<<"M";
                break;
            }
            t<<" Supplies: "<<std::setw(3)<<m_supplies<<" Production: "<<std::setw(2)<<m_production<<" Population: "<<std::setw(2)<<m_population;
            return t.str();
        }
    };
};
