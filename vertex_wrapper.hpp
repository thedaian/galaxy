#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "star.hpp"

class starmapRender : public sf::Drawable, public sf::Transformable
{
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    void generateStarGraphic(sf::Vertex*, const sf::Vector2f&, const uint8_t&, sf::Color, const bool&);

    sf::VertexArray m_vertices;
public:
    bool load(const std::vector<Star::star>&);
    void update(const int&, const bool&);
};

bool starmapRender::load(const std::vector<Star::star>& starlist)
{
    m_vertices.clear();

    m_vertices.setPrimitiveType(sf::Triangles);
    m_vertices.resize(starlist.size() * 15);

    unsigned int i = 0;
    for (auto _star = starlist.begin(); _star != starlist.end(); _star++)
    {
        generateStarGraphic(&m_vertices[i], _star->m_pos, _star->m_size, _star->m_colour, (_star->m_population > 0));

        i += 15;
    }

    return true;
}

void starmapRender::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // apply the transform
    states.transform *= getTransform();

    // draw the vertex array
    target.draw(m_vertices, states);
}

void starmapRender::generateStarGraphic(sf::Vertex* quad, const sf::Vector2f& _pos, const uint8_t& _size, sf::Color _colour, const bool& _isPopulated)
{
    //top part
    quad[0].position = sf::Vector2f(_pos.x - _size/2, _pos.y);
    quad[1].position = sf::Vector2f(_pos.x,           _pos.y - _size);
    quad[2].position = sf::Vector2f(_pos.x + _size/2, _pos.y);
    //bottom part
    quad[3].position = sf::Vector2f(_pos.x - _size/2, _pos.y);
    quad[4].position = sf::Vector2f(_pos.x,           _pos.y + _size);
    quad[5].position = sf::Vector2f(_pos.x + _size/2, _pos.y);
    //left side
    quad[6].position = sf::Vector2f(_pos.x,           _pos.y - _size/2);
    quad[7].position = sf::Vector2f(_pos.x - _size,   _pos.y);
    quad[8].position = sf::Vector2f(_pos.x,           _pos.y + _size/2);
    //right side
    quad[9].position = sf::Vector2f(_pos.x,           _pos.y - _size/2);
    quad[10].position = sf::Vector2f(_pos.x + _size,  _pos.y);
    quad[11].position = sf::Vector2f(_pos.x,          _pos.y + _size/2);

    //status
    quad[12].position = sf::Vector2f(_pos.x - _size/2 - 6,  _pos.y - _size/2);
    quad[13].position = sf::Vector2f(_pos.x - _size/2 - 3,  _pos.y - _size/2 - 3);
    quad[14].position = sf::Vector2f(_pos.x - _size/2,  _pos.y - _size/2);

    sf::Color outer;
    _colour.a = 128;
    outer = _colour;
    outer.a = 32;

    quad[0].color = _colour;
    quad[1].color = outer;
    quad[2].color = _colour;
    quad[3].color = _colour;
    quad[4].color = outer;
    quad[5].color = _colour;
    quad[6].color = _colour;
    quad[7].color = outer;
    quad[8].color = _colour;
    quad[9].color = _colour;
    quad[10].color = outer;
    quad[11].color = _colour;

    quad[12].color = sf::Color::White;
    quad[13].color = sf::Color::White;
    quad[14].color = sf::Color::White;

    if(!_isPopulated)
    {
        quad[12].color.a = 0;
        quad[13].color.a = 0;
        quad[14].color.a = 0;
    }
}

void starmapRender::update(const int& index, const bool& _isPopulated)
{
    if(_isPopulated)
    {
        m_vertices[index + 12].color.a = 255;
        m_vertices[index + 13].color.a = 255;
        m_vertices[index + 14].color.a = 255;
    } else {
        m_vertices[index + 12].color.a = 0;
        m_vertices[index + 13].color.a = 0;
        m_vertices[index + 14].color.a = 0;
    }
}

/***********************************************************************************

*/

struct ship
{
    Star::star *m_pos, *m_goal;
    uint8_t dt;
    bool m_hasSupplies, m_hasPopulation;

    ship(Star::star* _pos, Star::star* _goal)
    {
        m_pos = _pos;
        m_goal = _goal;
        dt = 0;
        start();
    }

    sf::Vector2f update(starmapRender & _map)
    {
        dt++;
        if(dt >= 100)
        {
            if(end())
            {
                _map.update(m_goal->m_index * 15, true);
            }
            Star::star* t = m_pos;
            m_pos = m_goal;
            m_goal = t;
            return m_pos->m_pos;
        }
        sf::Vector2f t;
        t.x = m_pos->m_pos.x + (m_goal->m_pos.x - m_pos->m_pos.x) * dt/100.f;
        //do a movement
        t.y = m_pos->m_pos.y + (t.x - m_pos->m_pos.x) * (m_goal->m_pos.y - m_pos->m_pos.y)/(m_goal->m_pos.x - m_pos->m_pos.x);
        return t;
    }

    void start()
    {
        if(m_pos->m_supplies > Star::MIN_SUPPLIES)
        {
            m_pos->m_supplies--;
            m_hasSupplies = true;
        } else {
            m_hasSupplies = false;
        }
        if(m_pos->m_population > Star::MIN_POP)
        {
            m_pos->m_population--;
            m_hasPopulation = true;
        } else {
            m_hasPopulation = false;
        }
    }

    bool end()
    {
        dt = 0;
        if(m_hasSupplies)
        {
            m_goal->m_supplies++;
        }
        if(m_hasPopulation)
        {
            m_goal->m_population++;
        }
        if(m_goal->m_population == 1)
        {
            return true;
        }
        return false;
    }
};

class shipmapRender : public sf::Drawable, public sf::Transformable
{
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray m_vertices;
    std::vector<ship> m_ships;
public:
    bool load();
    void add(Star::star*, Star::star*);
    void update(starmapRender &);
};

bool shipmapRender::load()
{
    m_vertices.clear();
    m_vertices.setPrimitiveType(sf::Points);

    m_ships.clear();

    return true;
}

void shipmapRender::add(Star::star* _pos, Star::star* _goal)
{
    m_vertices.append(sf::Vertex(_pos->m_pos, sf::Color::White));
    m_ships.push_back(ship(_pos, _goal));
}

void shipmapRender::update(starmapRender & _map)
{
    //move the ships around a bit
    for(unsigned int i = 0; i < m_ships.size(); i++)
    {
        m_vertices[i].position = m_ships[i].update(_map);
    }
}

void shipmapRender::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // apply the transform
    states.transform *= getTransform();

    // draw the vertex array
    target.draw(m_vertices, states);
}
