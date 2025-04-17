#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <fstream> // Added for file output

constexpr float PI = 3.141592653589793f;
constexpr float SCALE = 0.0000005f;
float DAYS_PER_SECOND = 500.0f;
bool paused = false;
float viewOffsetX = 0.0f, viewOffsetY = 0.0f;
float lastMouseX = 0.0f, lastMouseY = 0.0f;
bool draggingLeft = false;
float zoomLevel = 1.0f;

// Orbital parameters (unchanged)
constexpr float AU_TO_KM = 149597870.7f;
constexpr float MERCURY_SEMI_MAJOR_AXIS = 57909050.0f;
constexpr float MERCURY_ECCENTRICITY = 0.205635f;
constexpr float MERCURY_INCLINATION = 7.005f;
constexpr float MERCURY_LAN = 48.331f;
constexpr float MERCURY_AOP = 29.124f;
constexpr float MERCURY_ORBITAL_PERIOD = 87.9691f;
constexpr float VENUS_SEMI_MAJOR_AXIS = 108208000.0f;
constexpr float VENUS_ECCENTRICITY = 0.006772f;
constexpr float VENUS_INCLINATION = 3.39458f;
constexpr float VENUS_LAN = 76.680f;
constexpr float VENUS_AOP = 54.884f;
constexpr float VENUS_ORBITAL_PERIOD = 224.701f;
constexpr float EARTH_SEMI_MAJOR_AXIS = 149598023.0f;
constexpr float EARTH_ECCENTRICITY = 0.0167086f;
constexpr float EARTH_INCLINATION = 0.0f;
constexpr float EARTH_LAN = 0.0f;
constexpr float EARTH_AOP = 114.20783f;
constexpr float EARTH_ORBITAL_PERIOD = 365.256363f;
constexpr float APOPHIS_SEMI_MAJOR_AXIS = 137996000.0f;
constexpr float APOPHIS_ECCENTRICITY = 0.1914f;
constexpr float APOPHIS_INCLINATION = 3.339f;
constexpr float APOPHIS_LAN = 204.43f;
constexpr float APOPHIS_AOP = 126.39f;
constexpr float APOPHIS_ORBITAL_PERIOD = 323.5f;
constexpr float YR4_SEMI_MAJOR_AXIS = 180000000.0f;
constexpr float YR4_ECCENTRICITY = 0.15f;
constexpr float YR4_INCLINATION = 10.0f;
constexpr float YR4_LAN = 150.0f;
constexpr float YR4_AOP = 90.0f;
constexpr float YR4_ORBITAL_PERIOD = 480.0f;

// Alert timing variables
float godAlertTimer = 0.0f;
float yr4AlertTimer = 0.0f;
const float DISPLAY_DURATION = 10.0f;

float degreesToRadians(float degrees) {
    return degrees * PI / 180.0f;
}

void calculateEllipticalPosition(float days, float semiMajorAxis, float eccentricity, float inclination,
    float lan, float aop, float orbitalPeriod, float& x, float& y) {
    float M = 2 * PI * (days / orbitalPeriod);
    float E = M;
    for (int i = 0; i < 10; ++i) {
        E -= (E - eccentricity * std::sin(E) - M) / (1 - eccentricity * std::cos(E));
    }
    float v = 2 * std::atan2(std::sqrt(1 + eccentricity) * std::sin(E / 2),
        std::sqrt(1 - eccentricity) * std::cos(E / 2));
    float r = semiMajorAxis * (1 - eccentricity * std::cos(E));
    float omega = degreesToRadians(aop);
    float Omega = degreesToRadians(lan);
    float i = degreesToRadians(inclination);
    float xEcliptic = r * (std::cos(Omega) * std::cos(v + omega) -
        std::sin(Omega) * std::sin(v + omega) * std::cos(i));
    float yEcliptic = r * (std::sin(Omega) * std::cos(v + omega) +
        std::cos(Omega) * std::sin(v + omega) * std::cos(i));
    x = xEcliptic * SCALE + 400 + viewOffsetX;
    y = yEcliptic * SCALE + 400 + viewOffsetY;
}

float calculateDistance(float x1, float y1, float x2, float y2) {
    float dx = (x1 - x2) / SCALE;
    float dy = (y1 - y2) / SCALE;
    return std::sqrt(dx * dx + dy * dy);
}

std::string getDateTimeString(float daysSinceStart, int startYear, int startMonth, int startDay) {
    int totalDays = static_cast<int>(daysSinceStart) + startDay;
    int year = startYear;
    int month = startMonth;
    int day = totalDays;
    while (day > 365) {
        day -= 365;
        year++;
    }
    while (day > 30) {
        day -= 30;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << year << "-"
        << std::setw(2) << month << "-"
        << std::setw(2) << day;
    return oss.str();
}

// Global state
int startYear, startMonth, startDay;
float days = 0;
std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
struct TrailPoint { float x, y; };
std::vector<std::vector<TrailPoint>> trails(5);
constexpr int TRAIL_LENGTH = 2000;
std::string godAlertText, yr4AlertText;

// File stream for proximity alerts
std::ofstream alertFile("proximity_alerts.txt", std::ios::app);

void setGodAlert(const std::string& text) {
    godAlertText = text;
    godAlertTimer = DISPLAY_DURATION;
    if (alertFile.is_open()) {
        alertFile << text << "\n";
        alertFile.flush(); // Ensure immediate write
    }
}

void setYr4Alert(const std::string& text) {
    yr4AlertText = text;
    yr4AlertTimer = DISPLAY_DURATION;
    if (alertFile.is_open()) {
        alertFile << text << "\n";
        alertFile.flush(); // Ensure immediate write
    }
}

void drawCircle(float x, float y, float radius, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i += 10) {
        float angle = i * PI / 180.0f;
        glVertex2f(x + radius * std::cos(angle), y + radius * std::sin(angle));
    }
    glEnd();
}

void drawText(float x, float y, const char* text, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastTime).count();
    if (!paused) {
        days += DAYS_PER_SECOND * deltaTime;
    }
    if (godAlertTimer > 0) {
        godAlertTimer -= deltaTime;
        if (godAlertTimer <= 0) godAlertText.clear();
    }
    if (yr4AlertTimer > 0) {
        yr4AlertTimer -= deltaTime;
        if (yr4AlertTimer <= 0) yr4AlertText.clear();
    }
    lastTime = now;
    std::string currentDate = "Current Date: " + getDateTimeString(days, startYear, startMonth, startDay);
    float earthX, earthY, mercuryX, mercuryY, venusX, venusY, godX, godY, yr4X, yr4Y;
    calculateEllipticalPosition(days, EARTH_SEMI_MAJOR_AXIS, EARTH_ECCENTRICITY, EARTH_INCLINATION,
        EARTH_LAN, EARTH_AOP, EARTH_ORBITAL_PERIOD, earthX, earthY);
    calculateEllipticalPosition(days, MERCURY_SEMI_MAJOR_AXIS, MERCURY_ECCENTRICITY, MERCURY_INCLINATION,
        MERCURY_LAN, MERCURY_AOP, MERCURY_ORBITAL_PERIOD, mercuryX, mercuryY);
    calculateEllipticalPosition(days, VENUS_SEMI_MAJOR_AXIS, VENUS_ECCENTRICITY, VENUS_INCLINATION,
        VENUS_LAN, VENUS_AOP, VENUS_ORBITAL_PERIOD, venusX, venusY);
    calculateEllipticalPosition(days, APOPHIS_SEMI_MAJOR_AXIS, APOPHIS_ECCENTRICITY, APOPHIS_INCLINATION,
        APOPHIS_LAN, APOPHIS_AOP, APOPHIS_ORBITAL_PERIOD, godX, godY);
    calculateEllipticalPosition(days, YR4_SEMI_MAJOR_AXIS, YR4_ECCENTRICITY, YR4_INCLINATION,
        YR4_LAN, YR4_AOP, YR4_ORBITAL_PERIOD, yr4X, yr4Y);
    drawCircle(400 + viewOffsetX, 400 + viewOffsetY, 15 * zoomLevel, 1.0f, 1.0f, 0.0f);
    drawCircle(earthX, earthY, 8 * zoomLevel, 0.0f, 0.0f, 1.0f);
    drawCircle(mercuryX, mercuryY, 5 * zoomLevel, 0.66f, 0.66f, 0.66f);
    drawCircle(venusX, venusY, 6 * zoomLevel, 1.0f, 0.84f, 0.0f);
    drawCircle(godX, godY, 4 * zoomLevel, 1.0f, 0.0f, 0.0f);
    drawCircle(yr4X, yr4Y, 4 * zoomLevel, 0.0f, 1.0f, 1.0f);
    trails[0].push_back({ earthX, earthY });
    trails[1].push_back({ mercuryX, mercuryY });
    trails[2].push_back({ venusX, venusY });
    trails[3].push_back({ godX, godY });
    trails[4].push_back({ yr4X, yr4Y });
    for (auto& trail : trails) {
        if (trail.size() > TRAIL_LENGTH) trail.erase(trail.begin());
    }
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 0.0f, 1.0f);
    for (const auto& point : trails[0]) glVertex2f(point.x, point.y);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(0.66f, 0.66f, 0.66f);
    for (const auto& point : trails[1]) glVertex2f(point.x, point.y);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    for (const auto& point : trails[2]) glVertex2f(point.x, point.y);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.0f, 0.0f);
    for (const auto& point : trails[3]) glVertex2f(point.x, point.y);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 1.0f, 1.0f);
    for (const auto& point : trails[4]) glVertex2f(point.x, point.y);
    glEnd();
    float distToGod = calculateDistance(earthX, earthY, godX, godY);
    float distToYr4 = calculateDistance(earthX, earthY, yr4X, yr4Y);
    if (distToGod < 27479893.535f && godAlertTimer <= 0) {
        setGodAlert("Proximity Alert - APOPHIS: " + getDateTimeString(days, startYear, startMonth, startDay));
    }
    if (distToYr4 < 27479893.535f && yr4AlertTimer <= 0) {
        setYr4Alert("Proximity Alert - YR4: " + getDateTimeString(days, startYear, startMonth, startDay));
    }
    drawText(10, 780, currentDate.c_str(), 1.0f, 1.0f, 1.0f);
    if (!godAlertText.empty() && godAlertTimer > 0) {
        drawText(10, 120, godAlertText.c_str(), 1.0f, 0.0f, 0.0f);
    }
    if (!yr4AlertText.empty() && yr4AlertTimer > 0) {
        drawText(10, 100, yr4AlertText.c_str(), 0.0f, 1.0f, 1.0f);
    }
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float baseWidth = 800.0f;
    float baseHeight = 800.0f;
    float zoomWidth = baseWidth / zoomLevel;
    float zoomHeight = baseHeight / zoomLevel;
    float centerX = 400 + viewOffsetX;
    float centerY = 400 + viewOffsetY;
    gluOrtho2D(centerX - zoomWidth / 2, centerX + zoomWidth / 2, centerY - zoomHeight / 2, centerY + zoomHeight / 2);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '+': DAYS_PER_SECOND *= 1.1f; break;
    case '-': DAYS_PER_SECOND /= 1.1f; break;
    case ' ': paused = !paused; break;
    case 27: exit(0);
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            draggingLeft = true;
            lastMouseX = static_cast<float>(x);
            lastMouseY = static_cast<float>(y);
        }
        else draggingLeft = false;
    }
    else if (button == 3 && state == GLUT_UP) {
        zoomLevel = std::min(10.0f, zoomLevel * 1.1f);
        reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    }
    else if (button == 4 && state == GLUT_UP) {
        zoomLevel = std::max(0.1f, zoomLevel / 1.1f);
        reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    }
}

void motion(int x, int y) {
    if (draggingLeft) {
        viewOffsetX += (static_cast<float>(x) - lastMouseX) / zoomLevel;
        viewOffsetY -= (static_cast<float>(y) - lastMouseY) / zoomLevel;
        lastMouseX = static_cast<float>(x);
        lastMouseY = static_cast<float>(y);
    }
}

void idle() {
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    std::cout << "Enter start date (YYYY MM DD): ";
    std::cin >> startYear >> startMonth >> startDay;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Orbital Simulation");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);

    glutMainLoop();

    // Close the file when the program exits
    if (alertFile.is_open()) {
        alertFile.close();
    }

    return 0;
}