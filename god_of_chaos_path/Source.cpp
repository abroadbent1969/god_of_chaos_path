#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>

const double PI = 3.141592653589793;
const float SCALE = 0.000001f;
float DAYS_PER_SECOND = 10.0f;
bool paused = false;
sf::Vector2f viewOffset(0, 0); // Offset for dragging the view
sf::Vector2f lastMousePos;     // Last mouse position for dragging
bool dragging = false;          // Whether the user is dragging the view

// Orbital parameters
const float EARTH_ORBIT_RADIUS = 149597870.7f;
const float MERCURY_ORBIT_RADIUS = 57909050.0f;
const float VENUS_ORBIT_RADIUS = 108208000.0f;
const float GOD_OF_CHAOS_ORBIT_RADIUS = 200000000.0f;
const float YR4_ORBIT_RADIUS = 180000000.0f;

// Orbital periods
const float EARTH_ORBITAL_PERIOD = 365.25f;
const float MERCURY_ORBITAL_PERIOD = 87.97f;
const float VENUS_ORBITAL_PERIOD = 224.7f;
const float GOD_OF_CHAOS_ORBITAL_PERIOD = 500.0f;
const float YR4_ORBITAL_PERIOD = 450.0f;

void calculatePosition(float days, float radius, float period, float& x, float& y) {
    float angle = 2 * PI * (days / period);
    x = radius * cos(angle) * SCALE + 400 + viewOffset.x; // Apply view offset
    y = radius * sin(angle) * SCALE + 400 + viewOffset.y; // Apply view offset
}

std::string getDateTimeString(int year, int month, int day) {
    while (day > 30) {
        day -= 30;
        month++;
    }
    while (month > 12) {
        month -= 12;
        year++;
    }
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << year << "-"
        << std::setw(2) << month << "-"
        << std::setw(2) << day;
    return oss.str();
}

int main() {
    int startYear, startMonth, startDay;
    std::cout << "Enter start date (YYYY MM DD): ";
    std::cin >> startYear >> startMonth >> startDay;

    sf::RenderWindow window(sf::VideoMode(800, 800), "Orbital Simulation");

    // Sun
    sf::CircleShape sun(20);
    sun.setFillColor(sf::Color::Yellow);
    sun.setOrigin(20, 20);
    sun.setPosition(400 + viewOffset.x, 400 + viewOffset.y); // Apply view offset

    // Planets and asteroids
    sf::CircleShape earth(8), mercury(5), venus(6), godOfChaos(4), yr4(4);
    earth.setFillColor(sf::Color::Blue);
    mercury.setFillColor(sf::Color(169, 169, 169)); // Gray
    venus.setFillColor(sf::Color(255, 215, 0));    // Gold
    godOfChaos.setFillColor(sf::Color::Red);
    yr4.setFillColor(sf::Color::Cyan);

    earth.setOrigin(8, 8);
    mercury.setOrigin(5, 5);
    venus.setOrigin(6, 6);
    godOfChaos.setOrigin(4, 4);
    yr4.setOrigin(4, 4);

    sf::Font font;
    if (!font.loadFromFile("C:/Users/abroadbent/source/repos/god_of_chaos_path/font/arial.ttf")) {
        std::cerr << "Error loading font!" << std::endl;
        return -1;
    }

    sf::Text dateText("", font, 20);
    dateText.setFillColor(sf::Color::White);
    dateText.setPosition(10, 10);

    sf::Text speedText("Speed: " + std::to_string(DAYS_PER_SECOND) + " days/sec", font, 20);
    speedText.setFillColor(sf::Color::White);
    speedText.setPosition(10, 40);

    std::vector<std::vector<sf::Vector2f>> trails(5);

    float days = 0;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Add) {
                    DAYS_PER_SECOND *= 1.1f;
                }
                else if (event.key.code == sf::Keyboard::Subtract) {
                    DAYS_PER_SECOND /= 1.1f;
                }
                else if (event.key.code == sf::Keyboard::Space) {
                    paused = !paused;
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    dragging = true;
                    lastMousePos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    dragging = false;
                }
            }
            if (event.type == sf::Event::MouseMoved && dragging) {
                sf::Vector2f newMousePos(event.mouseMove.x, event.mouseMove.y);
                viewOffset += newMousePos - lastMousePos; // Update view offset
                lastMousePos = newMousePos;
            }
        }

        if (!paused) {
            days += DAYS_PER_SECOND * 0.01f;
        }

        int currentYear = startYear, currentMonth = startMonth, currentDay = startDay + static_cast<int>(days);
        dateText.setString(getDateTimeString(currentYear, currentMonth, currentDay));
        speedText.setString("Speed: " + std::to_string(DAYS_PER_SECOND) + " days/sec");

        // Update positions of planets and asteroids
        float x, y;
        calculatePosition(days, EARTH_ORBIT_RADIUS, EARTH_ORBITAL_PERIOD, x, y);
        earth.setPosition(x, y);
        calculatePosition(days, MERCURY_ORBIT_RADIUS, MERCURY_ORBITAL_PERIOD, x, y);
        mercury.setPosition(x, y);
        calculatePosition(days, VENUS_ORBIT_RADIUS, VENUS_ORBITAL_PERIOD, x, y);
        venus.setPosition(x, y);
        calculatePosition(days, GOD_OF_CHAOS_ORBIT_RADIUS, GOD_OF_CHAOS_ORBITAL_PERIOD, x, y);
        godOfChaos.setPosition(x, y);
        calculatePosition(days, YR4_ORBIT_RADIUS, YR4_ORBITAL_PERIOD, x, y);
        yr4.setPosition(x, y);

        // Update sun position with view offset
        sun.setPosition(400 + viewOffset.x, 400 + viewOffset.y);

        window.clear(sf::Color::Black);
        window.draw(dateText);
        window.draw(speedText);
        window.draw(sun);
        window.draw(earth);
        window.draw(mercury);
        window.draw(venus);
        window.draw(godOfChaos);
        window.draw(yr4);

        window.display();
    }

    return 0;
}