#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>

const double PI = 3.141592653589793;
const float SCALE = 0.000001f; // 1 pixel = 1,000,000 meters (adjust as needed)
float DAYS_PER_SECOND = 10.0f;
bool paused = false;
sf::Vector2f viewOffset(0, 0);
sf::Vector2f lastMousePos;
bool dragging = false;

// Orbital parameters (in kilometers, converted to meters in calculations)
const float EARTH_ORBIT_RADIUS = 149597870.7f; // in km
const float MERCURY_ORBIT_RADIUS = 57909050.0f;
const float VENUS_ORBIT_RADIUS = 108208000.0f;
const float GOD_OF_CHAOS_ORBIT_RADIUS = 200000000.0f;
const float YR4_ORBIT_RADIUS = 180000000.0f;

// Orbital periods (in days)
const float EARTH_ORBITAL_PERIOD = 365.25f;
const float MERCURY_ORBITAL_PERIOD = 87.97f;
const float VENUS_ORBITAL_PERIOD = 224.7f;
const float GOD_OF_CHAOS_ORBITAL_PERIOD = 500.0f;
const float YR4_ORBITAL_PERIOD = 450.0f;

void calculatePosition(float days, float radius, float period, float& x, float& y) {
    float angle = 2 * PI * (days / period);
    x = radius * cos(angle) * SCALE + 400 + viewOffset.x;
    y = radius * sin(angle) * SCALE + 400 + viewOffset.y;
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

// Calculate distance in meters between two objects
float calculateDistance(float x1, float y1, float x2, float y2, float scale) {
    float dx = (x1 - x2) / scale; // Convert back to kilometers
    float dy = (y1 - y2) / scale;
    return sqrt(dx * dx + dy * dy) * 1000; // Convert to meters
}

int main() {
    int startYear, startMonth, startDay;
    std::cout << "Enter start date (YYYY MM DD): ";
    std::cin >> startYear >> startMonth >> startDay;

    sf::RenderWindow window(sf::VideoMode(800, 800), "Orbital Simulation");

    sf::CircleShape sun(20);
    sun.setFillColor(sf::Color::Yellow);
    sun.setOrigin(20, 20);
    sun.setPosition(400 + viewOffset.x, 400 + viewOffset.y);

    sf::CircleShape earth(8), mercury(5), venus(6), godOfChaos(4), yr4(4);
    earth.setFillColor(sf::Color::Blue);
    mercury.setFillColor(sf::Color(169, 169, 169));
    venus.setFillColor(sf::Color(255, 215, 0));
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

    sf::Text GOD_OF_CHAOSText("God Of Chaos Asteroid", font, 20);
    GOD_OF_CHAOSText.setFillColor(sf::Color::Red);
    GOD_OF_CHAOSText.setPosition(10, 60);

    sf::Text YR4Text("YR4 Asteroid", font, 20);
    YR4Text.setFillColor(sf::Color::Cyan);
    YR4Text.setPosition(10, 80);

    sf::Text dateText("", font, 20);
    dateText.setFillColor(sf::Color::White);
    dateText.setPosition(10, 10);

    sf::Text speedText("Speed: " + std::to_string(DAYS_PER_SECOND) + " days/sec", font, 20);
    speedText.setFillColor(sf::Color::White);
    speedText.setPosition(10, 40);

    // Popup text for close approaches
    sf::Text popupText("", font, 20);
    popupText.setFillColor(sf::Color::Red);
    popupText.setPosition(10, 70);

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
                viewOffset += newMousePos - lastMousePos;
                lastMousePos = newMousePos;
            }
        }

        if (!paused) {
            days += DAYS_PER_SECOND * 0.01f;
        }

        int currentYear = startYear, currentMonth = startMonth, currentDay = startDay + static_cast<int>(days);
        dateText.setString(getDateTimeString(currentYear, currentMonth, currentDay));
        speedText.setString("Speed: " + std::to_string(DAYS_PER_SECOND) + " days/sec");

        float earthX, earthY, godX, godY, yr4X, yr4Y;
        calculatePosition(days, EARTH_ORBIT_RADIUS, EARTH_ORBITAL_PERIOD, earthX, earthY);
        earth.setPosition(earthX, earthY);
        calculatePosition(days, MERCURY_ORBIT_RADIUS, MERCURY_ORBITAL_PERIOD, earthX, earthY);
        mercury.setPosition(earthX, earthY);
        calculatePosition(days, VENUS_ORBIT_RADIUS, VENUS_ORBITAL_PERIOD, earthX, earthY);
        venus.setPosition(earthX, earthY);
        calculatePosition(days, GOD_OF_CHAOS_ORBIT_RADIUS, GOD_OF_CHAOS_ORBITAL_PERIOD, godX, godY);
        godOfChaos.setPosition(godX, godY);
        calculatePosition(days, YR4_ORBIT_RADIUS, YR4_ORBITAL_PERIOD, yr4X, yr4Y);
        yr4.setPosition(yr4X, yr4Y);

        sun.setPosition(400 + viewOffset.x, 400 + viewOffset.y);

        // Calculate distances to Earth
        float distToGod = calculateDistance(earthX, earthY, godX, godY, SCALE);
        float distToYr4 = calculateDistance(earthX, earthY, yr4X, yr4Y, SCALE);

        // Check if within 4,393,500,000 meters and set popup text
        std::string popupMessage = "";
        if (distToGod < 44393500000) {
            popupMessage += "GOD_OF_CHAOS near Earth: " + getDateTimeString(currentYear, currentMonth, currentDay) + "\n";
        }
        if (distToYr4 < 4393500000) {
            popupMessage += "YR4 near Earth: " + getDateTimeString(currentYear, currentMonth, currentDay);
        }
        popupText.setString(popupMessage);

        window.clear(sf::Color::Black);
        window.draw(GOD_OF_CHAOSText);
        window.draw(YR4Text);
        window.draw(dateText);
        window.draw(speedText);
        window.draw(popupText); // Draw the popup text
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