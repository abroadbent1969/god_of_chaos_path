#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>

// Constants
const double PI = 3.141592653589793;
const float SCALE = 0.0000005f; // 1 pixel = 2,000,000 meters
float DAYS_PER_SECOND = 5.0f; // Speed of Simulation
bool paused = false;
sf::Vector2f viewOffset(0, 0);
sf::Vector2f lastMousePos;
bool dragging = false;

// Orbital parameters (in kilometers)
const float EARTH_SEMI_MAJOR_AXIS = 149597870.7f; // 1 AU
const float MERCURY_SEMI_MAJOR_AXIS = 57909050.0f;
const float VENUS_SEMI_MAJOR_AXIS = 108208000.0f;
const float GOD_OF_CHAOS_SEMI_MAJOR_AXIS = 200000000.0f; // Example value
const float YR4_SEMI_MAJOR_AXIS = 180000000.0f; // Example value

// Orbital eccentricities
const float EARTH_ECCENTRICITY = 0.0167f;
const float MERCURY_ECCENTRICITY = 0.2056f;
const float VENUS_ECCENTRICITY = 0.0068f;
const float GOD_OF_CHAOS_ECCENTRICITY = 0.1f; // Example value
const float YR4_ECCENTRICITY = 0.15f; // Example value

// Orbital inclinations (in degrees)
const float EARTH_INCLINATION = 0.0f;
const float MERCURY_INCLINATION = 7.0f;
const float VENUS_INCLINATION = 3.4f;
const float GOD_OF_CHAOS_INCLINATION = 5.0f; // Example value
const float YR4_INCLINATION = 10.0f; // Example value

// Orbital periods (in days)
const float EARTH_ORBITAL_PERIOD = 365.25f;
const float MERCURY_ORBITAL_PERIOD = 87.97f;
const float VENUS_ORBITAL_PERIOD = 224.7f;
const float GOD_OF_CHAOS_ORBITAL_PERIOD = 570.0f; // Adjusted for realism
const float YR4_ORBITAL_PERIOD = 480.0f; // Adjusted for realism

// Function to convert degrees to radians
double degreesToRadians(double degrees) {
    return degrees * PI / 180.0;
}

// Function to calculate position in elliptical orbit
void calculateEllipticalPosition(double days, double semiMajorAxis, double eccentricity, double inclination, double orbitalPeriod, double& x, double& y) {
    // Mean anomaly
    double M = 2 * PI * (days / orbitalPeriod);

    // Solve Kepler's Equation for Eccentric Anomaly (E) using Newton-Raphson method
    double E = M; // Initial guess: E ≈ M
    for (int i = 0; i < 5; ++i) {
        E = E - (E - eccentricity * sin(E) - M) / (1 - eccentricity * cos(E));
    }

    // True anomaly
    double v = 2 * atan2(sqrt(1 + eccentricity) * sin(E / 2), sqrt(1 - eccentricity) * cos(E / 2));

    // Distance from the focus (Sun) to the asteroid
    double r = semiMajorAxis * (1 - eccentricity * cos(E));

    // Convert inclination to radians
    double incl = degreesToRadians(inclination);

    // Position in the orbital plane
    double xOrbital = r * cos(v);
    double yOrbital = r * sin(v);

    // Rotate position by inclination to get position in the ecliptic plane
    x = xOrbital * SCALE + 400 + viewOffset.x;
    y = yOrbital * cos(incl) * SCALE + 400 + viewOffset.y;
}

// Function to calculate distance between two points in kilometers
double calculateDistance(double x1, double y1, double x2, double y2) {
    double dx = (x1 - x2) / SCALE; // Convert back to kilometers
    double dy = (y1 - y2) / SCALE;
    return sqrt(dx * dx + dy * dy); // Distance in kilometers
}

// Function to get date string
std::string getDateTimeString(int year, int month, int day) {
    while (day > 30) {  // Assuming each month has 30 days for simplicity
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

    // Create celestial bodies
    sf::CircleShape sun(15);
    sun.setFillColor(sf::Color::Yellow);
    sun.setOrigin(20, 20);

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

    // Load font
    sf::Font font;
    if (!font.loadFromFile("C:/Users/abroadbent/source/repos/god_of_chaos_path/font/arial.ttf")) {
        std::cerr << "Error loading font!" << std::endl;
        return -1;
    }

    // Create text objects
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

    sf::Text popupText("", font, 20);
    popupText.setFillColor(sf::Color::White);
    popupText.setPosition(10, 100);

    // Trails for each object
    std::vector<std::vector<sf::Vertex>> trails(5);
    const int TRAIL_LENGTH = 1000; // Number of points in the trail

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

        // Calculate positions
        double earthX, earthY, mercuryX, mercuryY, venusX, venusY, godX, godY, yr4X, yr4Y;
        calculateEllipticalPosition(days, EARTH_SEMI_MAJOR_AXIS, EARTH_ECCENTRICITY, EARTH_INCLINATION, EARTH_ORBITAL_PERIOD, earthX, earthY);
        earth.setPosition(earthX, earthY);
        calculateEllipticalPosition(days, MERCURY_SEMI_MAJOR_AXIS, MERCURY_ECCENTRICITY, MERCURY_INCLINATION, MERCURY_ORBITAL_PERIOD, mercuryX, mercuryY);
        mercury.setPosition(mercuryX, mercuryY);
        calculateEllipticalPosition(days, VENUS_SEMI_MAJOR_AXIS, VENUS_ECCENTRICITY, VENUS_INCLINATION, VENUS_ORBITAL_PERIOD, venusX, venusY);
        venus.setPosition(venusX, venusY);
        calculateEllipticalPosition(days, GOD_OF_CHAOS_SEMI_MAJOR_AXIS, GOD_OF_CHAOS_ECCENTRICITY, GOD_OF_CHAOS_INCLINATION, GOD_OF_CHAOS_ORBITAL_PERIOD, godX, godY);
        godOfChaos.setPosition(godX, godY);
        calculateEllipticalPosition(days, YR4_SEMI_MAJOR_AXIS, YR4_ECCENTRICITY, YR4_INCLINATION, YR4_ORBITAL_PERIOD, yr4X, yr4Y);
        yr4.setPosition(yr4X, yr4Y);

        // Update Sun's position with view offset
        sun.setPosition(400 + viewOffset.x, 400 + viewOffset.y);

        // Update trails
        trails[0].push_back(sf::Vertex(sf::Vector2f(earthX, earthY), sf::Color::Blue));
        trails[1].push_back(sf::Vertex(sf::Vector2f(mercuryX, mercuryY), sf::Color(169, 169, 169)));
        trails[2].push_back(sf::Vertex(sf::Vector2f(venusX, venusY), sf::Color(255, 215, 0)));
        trails[3].push_back(sf::Vertex(sf::Vector2f(godX, godY), sf::Color::Red));
        trails[4].push_back(sf::Vertex(sf::Vector2f(yr4X, yr4Y), sf::Color::Cyan));

        // Limit trail length
        for (auto& trail : trails) {
            if (trail.size() > TRAIL_LENGTH) {
                trail.erase(trail.begin());
            }
        }

        // Calculate distances to Earth
        double distToGod = calculateDistance(earthX, earthY, godX, godY);
        double distToYr4 = calculateDistance(earthX, earthY, yr4X, yr4Y);

        // Check for close approaches
        std::string popupMessage = "";
        //if (distToGod < 7479893.535) { // 0.05 AU in kilometers
        if (distToGod < 27479893.535) { // 0.05 AU in kilometers
            popupMessage += "GOD_OF_CHAOS near Earth: " + getDateTimeString(currentYear, currentMonth, currentDay) + "\n";
        }
        //if (distToYr4 < 7479893.535) { // 0.05 AU in kilometers
        if (distToYr4 < 27479893.535) { // 0.05 AU in kilometers
            popupMessage += "YR4 near Earth: " + getDateTimeString(currentYear, currentMonth, currentDay);
        }
        popupText.setString(popupMessage);

        // Draw everything
        window.clear(sf::Color::Black);
        window.draw(GOD_OF_CHAOSText);
        window.draw(YR4Text);
        window.draw(dateText);
        window.draw(speedText);
        window.draw(popupText);
        window.draw(sun);
        window.draw(earth);
        window.draw(mercury);
        window.draw(venus);
        window.draw(godOfChaos);
        window.draw(yr4);

        // Draw trails
        for (const auto& trail : trails) {
            window.draw(&trail[0], trail.size(), sf::LineStrip);
        }

        window.display();
    }

    return 0;
}