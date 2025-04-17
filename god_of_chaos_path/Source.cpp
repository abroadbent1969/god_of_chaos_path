#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <fstream>
#include <ctime>

constexpr float PI = 3.141592653589793f;
constexpr float SCALE = 0.0000005f;
float DAYS_PER_SECOND = 500.0f;
bool paused = false;
float viewOffsetX = 0.0f, viewOffsetY = 0.0f;
float lastMouseX = 0.0f, lastMouseY = 0.0f;
bool draggingLeft = false;
float zoomLevel = 1.0f;

constexpr float AU_TO_KM = 149597870.7f;

// Orbital parameters
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
constexpr float APOPHIS_ECCENTRICITY = 0.1911f;
constexpr float APOPHIS_INCLINATION = 3.339f;
constexpr float APOPHIS_LAN = 204.183f;
constexpr float APOPHIS_AOP = 126.404f;
constexpr float APOPHIS_ORBITAL_PERIOD = 324.0f;
constexpr float APOPHIS_MEAN_ANOMALY_EPOCH = 297.8f;
constexpr float APOPHIS_YARKOVSKY_A2 = -2.899e-14f;
constexpr float YR4_SEMI_MAJOR_AXIS = 306675000.0f;
constexpr float YR4_ECCENTRICITY = 0.556f;
constexpr float YR4_INCLINATION = 3.41f;
constexpr float YR4_LAN = 73.5f;
constexpr float YR4_AOP = 297.4f;
constexpr float YR4_ORBITAL_PERIOD = 1458.0f;
constexpr float YR4_MEAN_ANOMALY_EPOCH = 0.0f;
constexpr float YR4_YARKOVSKY_A2 = -1.0e-14f;
constexpr float BENNU_SEMI_MAJOR_AXIS = 168580000.0f;
constexpr float BENNU_ECCENTRICITY = 0.2037f;
constexpr float BENNU_INCLINATION = 6.035f;
constexpr float BENNU_LAN = 2.061f;
constexpr float BENNU_AOP = 66.223f;
constexpr float BENNU_ORBITAL_PERIOD = 436.6f;
constexpr float BENNU_MEAN_ANOMALY_EPOCH = 340.0f;
constexpr float BENNU_YARKOVSKY_A2 = -1.9e-13f;
constexpr float WN5_SEMI_MAJOR_AXIS = 256016000.0f;
constexpr float WN5_ECCENTRICITY = 0.4672f;
constexpr float WN5_INCLINATION = 1.915f;
constexpr float WN5_LAN = 285.885f;
constexpr float WN5_AOP = 68.349f;
constexpr float WN5_ORBITAL_PERIOD = 817.7f;
constexpr float WN5_MEAN_ANOMALY_EPOCH = 10.0f;
constexpr float WN5_YARKOVSKY_A2 = -1.0e-14f;
constexpr float DA1950_SEMI_MAJOR_AXIS = 254121000.0f;
constexpr float DA1950_ECCENTRICITY = 0.5082f;
constexpr float DA1950_INCLINATION = 12.175f;
constexpr float DA1950_LAN = 356.747f;
constexpr float DA1950_AOP = 224.572f;
constexpr float DA1950_ORBITAL_PERIOD = 808.5f;
constexpr float DA1950_MEAN_ANOMALY_EPOCH = 200.0f;
constexpr float DA1950_YARKOVSKY_A2 = -2.5e-13f;

float godAlertTimer = 0.0f;
float yr4AlertTimer = 0.0f;
float bennuAlertTimer = 0.0f;
float wn5AlertTimer = 0.0f;
float da1950AlertTimer = 0.0f;
const float DISPLAY_DURATION = 10.0f;
const float PROXIMITY_THRESHOLD = 3800000.0f;

float degreesToRadians(float degrees) {
    return degrees * PI / 180.0f;
}

double getJulianDay(int year, int month, int day) {
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    double jd = day + (153.0 * m + 2) / 5 + 365.0 * y + (y / 4.0) - (y / 100.0) + (y / 400.0) - 32045;
    return jd;
}

std::string getDateTimeString(float daysSinceStart, int startYear, int startMonth, int startDay) {
    double jdStart = getJulianDay(startYear, startMonth, startDay);
    double jd = jdStart + daysSinceStart;
    int z = static_cast<int>(jd + 0.5);
    double f = jd + 0.5 - z;
    int alpha = static_cast<int>((z - 1867216.25) / 36524.25);
    int a = z + 1 + alpha - (alpha / 4);
    int b = a + 1524;
    int c = static_cast<int>((b - 122.1) / 365.25);
    int d = static_cast<int>(365.25 * c);
    int e = static_cast<int>((b - d) / 30.6001);
    int day = b - d - static_cast<int>(30.6001 * e);
    int month = e - (e < 13.5 ? 1 : 13);
    int year = c - (month > 2 ? 4716 : 4715);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << year << "-"
        << std::setw(2) << month << "-"
        << std::setw(2) << day;
    return oss.str();
}

void calculateEllipticalPosition(float daysSinceEpoch, float semiMajorAxis, float eccentricity, float inclination,
    float lan, float aop, float orbitalPeriod, float meanAnomalyEpoch, float yarkovskyA2, float& x, float& y) {
    float days = daysSinceEpoch;
    float deltaA = yarkovskyA2 * days * days * AU_TO_KM;
    float adjustedSemiMajorAxis = semiMajorAxis + deltaA;
    float M = degreesToRadians(meanAnomalyEpoch) + 2 * PI * (days / orbitalPeriod);
    float E = M;
    for (int i = 0; i < 10; ++i) {
        E -= ((E - eccentricity * std::sin(E) - M) / (1 - eccentricity * std::cos(E)));
    }
    float v = 2 * std::atan2(std::sqrt(1 + eccentricity) * std::sin(E / 2),
        std::sqrt(1 - eccentricity) * std::cos(E / 2));
    float r = adjustedSemiMajorAxis * (1 - eccentricity * std::cos(E));
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

int startYear, startMonth, startDay;
float days = 0;
std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
struct TrailPoint { float x, y; };
std::vector<std::vector<TrailPoint>> trails(8);
constexpr int TRAIL_LENGTH = 2000;
std::string godAlertText, yr4AlertText, bennuAlertText, wn5AlertText, da1950AlertText;
std::ofstream alertFile("proximity_alerts.txt", std::ios::app);

void setGodAlert(const std::string& text) {
    godAlertText = text;
    godAlertTimer = DISPLAY_DURATION;
    if (alertFile.is_open()) {
        alertFile << text << "\n";
        alertFile.flush();
    }
    else {
        std::cerr << "Error: Could not write to proximity_alerts.txt\n";
    }
}

void setYr4Alert(const std::string& text) {
    yr4AlertText = text;
    yr4AlertTimer = DISPLAY_DURATION;
    if (alertFile.is_open()) {
        alertFile << text << "\n";
        alertFile.flush();
    }
    else {
        std::cerr << "Error: Could not write to proximity_alerts.txt\n";
    }
}

void setBennuAlert(const std::string& text) {
    bennuAlertText = text;
    bennuAlertTimer = DISPLAY_DURATION;
    if (alertFile.is_open()) {
        alertFile << text << "\n";
        alertFile.flush();
    }
    else {
        std::cerr << "Error: Could not write to proximity_alerts.txt\n";
    }
}

void setWn5Alert(const std::string& text) {
    wn5AlertText = text;
    wn5AlertTimer = DISPLAY_DURATION;
    if (alertFile.is_open()) {
        alertFile << text << "\n";
        alertFile.flush();
    }
    else {
        std::cerr << "Error: Could not write to proximity_alerts.txt\n";
    }
}

void setDa1950Alert(const std::string& text) {
    da1950AlertText = text;
    da1950AlertTimer = DISPLAY_DURATION;
    if (alertFile.is_open()) {
        alertFile << text << "\n";
        alertFile.flush();
    }
    else {
        std::cerr << "Error: Could not write to proximity_alerts.txt\n";
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
    if (bennuAlertTimer > 0) {
        bennuAlertTimer -= deltaTime;
        if (bennuAlertTimer <= 0) bennuAlertText.clear();
    }
    if (wn5AlertTimer > 0) {
        wn5AlertTimer -= deltaTime;
        if (wn5AlertTimer <= 0) wn5AlertText.clear();
    }
    if (da1950AlertTimer > 0) {
        da1950AlertTimer -= deltaTime;
        if (da1950AlertTimer <= 0) da1950AlertText.clear();
    }
    lastTime = now;
    std::string currentDate = "Current Date: " + getDateTimeString(days, startYear, startMonth, startDay);
    float earthX, earthY, mercuryX, mercuryY, venusX, venusY, godX, godY, yr4X, yr4Y, bennuX, bennuY, wn5X, wn5Y, da1950X, da1950Y;
    calculateEllipticalPosition(days, EARTH_SEMI_MAJOR_AXIS, EARTH_ECCENTRICITY, EARTH_INCLINATION,
        EARTH_LAN, EARTH_AOP, EARTH_ORBITAL_PERIOD, 0.0f, 0.0f, earthX, earthY);
    calculateEllipticalPosition(days, MERCURY_SEMI_MAJOR_AXIS, MERCURY_ECCENTRICITY, MERCURY_INCLINATION,
        MERCURY_LAN, MERCURY_AOP, MERCURY_ORBITAL_PERIOD, 0.0f, 0.0f, mercuryX, mercuryY);
    calculateEllipticalPosition(days, VENUS_SEMI_MAJOR_AXIS, VENUS_ECCENTRICITY, VENUS_INCLINATION,
        VENUS_LAN, VENUS_AOP, VENUS_ORBITAL_PERIOD, 0.0f, 0.0f, venusX, venusY);
    calculateEllipticalPosition(days, APOPHIS_SEMI_MAJOR_AXIS, APOPHIS_ECCENTRICITY, APOPHIS_INCLINATION,
        APOPHIS_LAN, APOPHIS_AOP, APOPHIS_ORBITAL_PERIOD, APOPHIS_MEAN_ANOMALY_EPOCH, APOPHIS_YARKOVSKY_A2, godX, godY);
    calculateEllipticalPosition(days, YR4_SEMI_MAJOR_AXIS, YR4_ECCENTRICITY, YR4_INCLINATION,
        YR4_LAN, YR4_AOP, YR4_ORBITAL_PERIOD, YR4_MEAN_ANOMALY_EPOCH, YR4_YARKOVSKY_A2, yr4X, yr4Y);
    calculateEllipticalPosition(days, BENNU_SEMI_MAJOR_AXIS, BENNU_ECCENTRICITY, BENNU_INCLINATION,
        BENNU_LAN, BENNU_AOP, BENNU_ORBITAL_PERIOD, BENNU_MEAN_ANOMALY_EPOCH, BENNU_YARKOVSKY_A2, bennuX, bennuY);
    calculateEllipticalPosition(days, WN5_SEMI_MAJOR_AXIS, WN5_ECCENTRICITY, WN5_INCLINATION,
        WN5_LAN, WN5_AOP, WN5_ORBITAL_PERIOD, WN5_MEAN_ANOMALY_EPOCH, WN5_YARKOVSKY_A2, wn5X, wn5Y);
    calculateEllipticalPosition(days, DA1950_SEMI_MAJOR_AXIS, DA1950_ECCENTRICITY, DA1950_INCLINATION,
        DA1950_LAN, DA1950_AOP, DA1950_ORBITAL_PERIOD, DA1950_MEAN_ANOMALY_EPOCH, DA1950_YARKOVSKY_A2, da1950X, da1950Y);
    drawCircle(400 + viewOffsetX, 400 + viewOffsetY, 15 * zoomLevel, 1.0f, 1.0f, 0.0f); // Sun
    drawCircle(earthX, earthY, 8 * zoomLevel, 0.0f, 0.0f, 1.0f); // Earth
    drawCircle(mercuryX, mercuryY, 5 * zoomLevel, 0.66f, 0.66f, 0.66f); // Mercury
    drawCircle(venusX, venusY, 6 * zoomLevel, 1.0f, 0.84f, 0.0f); // Venus
    drawCircle(godX, godY, 4 * zoomLevel, 1.0f, 0.0f, 0.0f); // Apophis
    drawCircle(yr4X, yr4Y, 4 * zoomLevel, 0.0f, 1.0f, 1.0f); // YR4
    drawCircle(bennuX, bennuY, 4 * zoomLevel, 0.5f, 0.5f, 0.0f); // Bennu (olive)
    drawCircle(wn5X, wn5Y, 4 * zoomLevel, 0.7f, 0.0f, 0.7f); // WN5 (purple)
    drawCircle(da1950X, da1950Y, 4 * zoomLevel, 0.0f, 0.7f, 0.0f); // 1950 DA (green)
    trails[0].push_back({ earthX, earthY });
    trails[1].push_back({ mercuryX, mercuryY });
    trails[2].push_back({ venusX, venusY });
    trails[3].push_back({ godX, godY });
    trails[4].push_back({ yr4X, yr4Y });
    trails[5].push_back({ bennuX, bennuY });
    trails[6].push_back({ wn5X, wn5Y });
    trails[7].push_back({ da1950X, da1950Y });
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
    glBegin(GL_LINE_STRIP);
    glColor3f(0.5f, 0.5f, 0.0f);
    for (const auto& point : trails[5]) glVertex2f(point.x, point.y);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(0.7f, 0.0f, 0.7f);
    for (const auto& point : trails[6]) glVertex2f(point.x, point.y);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 0.7f, 0.0f);
    for (const auto& point : trails[7]) glVertex2f(point.x, point.y);
    glEnd();
    float distToGod = calculateDistance(earthX, earthY, godX, godY);
    float distToYr4 = calculateDistance(earthX, earthY, yr4X, yr4Y);
    float distToBennu = calculateDistance(earthX, earthY, bennuX, bennuY);
    float distToWn5 = calculateDistance(earthX, earthY, wn5X, wn5Y);
    float distToDa1950 = calculateDistance(earthX, earthY, da1950X, da1950Y);
    if (distToGod < PROXIMITY_THRESHOLD && godAlertTimer <= 0) {
        std::string alert = "Proximity Alert - APOPHIS: " + getDateTimeString(days, startYear, startMonth, startDay) + " (Distance: " + std::to_string(distToGod) + " km)";
        setGodAlert(alert);
    }
    if (distToYr4 < PROXIMITY_THRESHOLD && yr4AlertTimer <= 0) {
        std::string alert = "Proximity Alert - YR4: " + getDateTimeString(days, startYear, startMonth, startDay) + " (Distance: " + std::to_string(distToYr4) + " km)";
        setYr4Alert(alert);
    }
    if (distToBennu < PROXIMITY_THRESHOLD && bennuAlertTimer <= 0) {
        std::string alert = "Proximity Alert - BENNU: " + getDateTimeString(days, startYear, startMonth, startDay) + " (Distance: " + std::to_string(distToBennu) + " km)";
        setBennuAlert(alert);
    }
    if (distToWn5 < PROXIMITY_THRESHOLD && wn5AlertTimer <= 0) {
        std::string alert = "Proximity Alert - 2001 WN5: " + getDateTimeString(days, startYear, startMonth, startDay) + " (Distance: " + std::to_string(distToWn5) + " km)";
        setWn5Alert(alert);
    }
    if (distToDa1950 < PROXIMITY_THRESHOLD && da1950AlertTimer <= 0) {
        std::string alert = "Proximity Alert - 1950 DA: " + getDateTimeString(days, startYear, startMonth, startDay) + " (Distance: " + std::to_string(distToDa1950) + " km)";
        setDa1950Alert(alert);
    }
    // Draw current date
    drawText(10, 780, currentDate.c_str(), 1.0f, 1.0f, 1.0f);
    // Draw template with names and colors
    drawText(10, 760, "Sun", 1.0f, 1.0f, 0.0f);         // Yellow
    drawText(10, 740, "Mercury", 0.66f, 0.66f, 0.66f);  // Gray
    drawText(10, 720, "Venus", 1.0f, 0.84f, 0.0f);      // Orange
    drawText(10, 700, "Earth", 0.0f, 0.0f, 1.0f);       // Blue
    drawText(10, 680, "Apophis", 1.0f, 0.0f, 0.0f);     // Red
    drawText(10, 660, "YR4", 0.0f, 1.0f, 1.0f);         // Cyan
    drawText(10, 640, "Bennu", 0.5f, 0.5f, 0.0f);       // Olive
    drawText(10, 620, "2001 WN5", 0.7f, 0.0f, 0.7f);    // Purple
    drawText(10, 600, "1950 DA", 0.0f, 0.7f, 0.0f);     // Green
    // Draw alert texts
    if (!godAlertText.empty() && godAlertTimer > 0) {
        drawText(10, 120, godAlertText.c_str(), 1.0f, 0.0f, 0.0f);
    }
    if (!yr4AlertText.empty() && yr4AlertTimer > 0) {
        drawText(10, 100, yr4AlertText.c_str(), 0.0f, 1.0f, 1.0f);
    }
    if (!bennuAlertText.empty() && bennuAlertTimer > 0) {
        drawText(10, 80, bennuAlertText.c_str(), 0.5f, 0.5f, 0.0f);
    }
    if (!wn5AlertText.empty() && wn5AlertTimer > 0) {
        drawText(10, 60, wn5AlertText.c_str(), 0.7f, 0.0f, 0.7f);
    }
    if (!da1950AlertText.empty() && da1950AlertTimer > 0) {
        drawText(10, 40, da1950AlertText.c_str(), 0.0f, 0.7f, 0.0f);
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

    if (alertFile.is_open()) {
        alertFile.close();
    }

    return 0;
}