This C++ program is an orbital simulation that visualizes the movement of celestial bodies, including the Earth, Mercury, Venus, and two asteroids (God of Chaos and YR4), around the Sun. The simulation allows users to observe the orbits of these bodies, adjust the simulation speed, and view close approaches of asteroids to Earth.

The program uses the SFML (Simple and Fast Multimedia Library) for rendering and provides an interactive interface where users can:

Adjust the simulation speed.

Pause and resume the simulation.

Drag the view to explore different parts of the solar system.

View the current date and close approach events.

Features
Celestial Bodies: The simulation includes the Sun, Earth, Mercury, Venus, and two asteroids (God of Chaos and YR4).

Orbital Mechanics: Each body orbits the Sun based on its orbital radius and period.

Interactive Controls:

Speed Adjustment: Use the + and - keys to increase or decrease the simulation speed.

Pause/Resume: Press the Space key to pause or resume the simulation.

Drag to Pan: Click and drag the mouse to move the view around.

Close Approach Detection: The simulation detects when the God of Chaos or YR4 asteroids come within 3,393,500,000 meters of Earth and displays a warning message.

Date Display: The current date is displayed in the top-left corner of the window, updating as the simulation progresses.

Requirements
SFML: The program requires the SFML library to be installed. You can download it from SFML's official website.

Font: The program uses the Arial font. Ensure that the font file is located at the specified path (C:/Users/abroadbent/source/repos/god_of_chaos_path/font/arial.ttf). You can change the path in the code if necessary.

How to Run
Install SFML: Make sure SFML is installed on your system. Follow the installation instructions provided on the SFML website.

Compile the Code: Compile the C++ code using a compiler that supports SFML. For example, you can use the following command with g++:

bash
Copy
g++ -o orbital_simulation main.cpp -lsfml-graphics -lsfml-window -lsfml-system
Run the Executable: Execute the compiled program:

bash
Copy
./orbital_simulation
Enter Start Date: When prompted, enter the start date in the format YYYY MM DD.

Interact with the Simulation:

Use the + and - keys to adjust the simulation speed.

Press the Space key to pause or resume the simulation.

Click and drag to move the view around.

Observe the orbits and any close approach warnings.

Code Structure
Constants: The program defines constants for orbital radii, periods, and other parameters.

calculatePosition: A function that calculates the position of a celestial body based on the current day, orbital radius, and period.

getDateTimeString: A function that formats the current date as a string.

calculateDistance: A function that calculates the distance between two points in meters.

Main Loop: The main loop handles user input, updates the positions of the celestial bodies, and renders the scene.

Customization
Orbital Parameters: You can modify the orbital radii and periods in the constants section to simulate different celestial bodies or scenarios.

Font Path: If you want to use a different font, update the path in the font.loadFromFile function call.

Close Approach Threshold: Adjust the close approach threshold (currently set to 3,393,500,000 meters) in the calculateDistance function.

License
This project is open-source and available under the MIT License. Feel free to modify and distribute it as needed.

Acknowledgments
SFML: Thanks to the SFML development team for providing a powerful and easy-to-use multimedia library.

NASA: Orbital data and parameters are inspired by real-world astronomical data.
