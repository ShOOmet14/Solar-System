#include "planets_setup.h"
void initializePlanets(std::vector<Planet>& planets) {
    planets.clear();
    planets.reserve(9);

    const unsigned int moonTexture = loadTexture("resources/moon.jpg");

    Planet sun(glm::vec3(0.0f), 0.7f, glm::vec3(1.0f, 1.0f, 0.0f));
    planets.push_back(sun);

    // Mercury
    planets.emplace_back(glm::vec3(0.0f), 0.10f, glm::vec3(0.5f));

    // Venus
    planets.emplace_back(glm::vec3(0.0f), 0.18f, glm::vec3(0.9f, 0.7f, 0.2f));

    // Earth and Moon
    planets.emplace_back(glm::vec3(0.0f), 0.22f, glm::vec3(0.2f, 0.6f, 1.0f));
    Planet moon(glm::vec3(0.0f), 0.05f, glm::vec3(0.8f, 0.8f, 0.8f));
    moon.orbitRadius = 0.5f;
    moon.orbitSpeed = 100.0f;
    moon.textureId = moonTexture;
    planets[3].moons.push_back(moon);

    // Mars, Phobos, and Deimos
    planets.emplace_back(glm::vec3(0.0f), 0.15f, glm::vec3(0.8f, 0.3f, 0.2f));
    Planet phobos(glm::vec3(0.0f), 0.03f, glm::vec3(0.6f));
    phobos.orbitRadius = 0.3f;
    phobos.orbitSpeed = 120.0f;
    planets[4].moons.push_back(phobos);

    Planet deimos(glm::vec3(0.0f), 0.02f, glm::vec3(0.7f));
    deimos.orbitRadius = 0.5f;
    deimos.orbitSpeed = 90.0f;
    planets[4].moons.push_back(deimos);

    // Jupiter and four selected moons
    planets.emplace_back(glm::vec3(0.0f), 0.45f, glm::vec3(0.9f, 0.8f, 0.6f));
    float jMoonSize = 0.05f;

    Planet io(glm::vec3(0.0f), jMoonSize, glm::vec3(0.9f, 0.6f, 0.3f));
    io.orbitRadius = 0.7f;
    io.orbitSpeed = 55.0f;
    planets[5].moons.push_back(io);

    Planet europa(glm::vec3(0.0f), jMoonSize, glm::vec3(0.6f, 0.8f, 1.0f));
    europa.orbitRadius = 0.9f;
    europa.orbitSpeed = 50.0f;
    planets[5].moons.push_back(europa);

    Planet ganymede(glm::vec3(0.0f), jMoonSize, glm::vec3(0.4f, 0.7f, 0.9f));
    ganymede.orbitRadius = 1.2f;
    ganymede.orbitSpeed = 45.0f;
    planets[5].moons.push_back(ganymede);

    Planet callisto(glm::vec3(0.0f), jMoonSize, glm::vec3(0.6f, 0.5f, 0.4f));
    callisto.orbitRadius = 1.5f;
    callisto.orbitSpeed = 40.0f;
    planets[5].moons.push_back(callisto);

    // Saturn and Titan
    planets.emplace_back(glm::vec3(0.0f), 0.40f, glm::vec3(0.9f, 0.85f, 0.5f));
    Planet tytan(glm::vec3(0.0f), 0.06f, glm::vec3(0.8f, 0.7f, 0.4f));
    tytan.orbitRadius = 1.0f;
    tytan.orbitSpeed = 42.5f;
    planets[6].moons.push_back(tytan);

    // Uranus and Miranda
    planets.emplace_back(glm::vec3(0.0f), 0.30f, glm::vec3(0.6f, 0.9f, 0.9f));
    Planet miranda(glm::vec3(0.0f), 0.03f, glm::vec3(0.6f, 0.6f, 0.8f));
    miranda.orbitRadius = 0.8f;
    miranda.orbitSpeed = 45.0f;
    planets[7].moons.push_back(miranda);

    // Neptune and Triton
    planets.emplace_back(glm::vec3(0.0f), 0.28f, glm::vec3(0.4f, 0.5f, 0.9f));
    Planet tryton(glm::vec3(0.0f), 0.04f, glm::vec3(0.5f, 0.7f, 0.9f));
    tryton.orbitRadius = 0.7f;
    tryton.orbitSpeed = 47.5f;
    planets[8].moons.push_back(tryton);

    // Visual orbit radii and angular speeds; these are not astronomical values.
    planets[1].orbitRadius = 2.5f;
    planets[1].orbitSpeed = 30.0f;
    planets[2].orbitRadius = 3.5f;
    planets[2].orbitSpeed = 22.5f;
    planets[3].orbitRadius = 5.0f;
    planets[3].orbitSpeed = 17.5f;
    planets[4].orbitRadius = 6.5f;
    planets[4].orbitSpeed = 14.0f;
    planets[5].orbitRadius = 8.5f;
    planets[5].orbitSpeed = 9.0f;
    planets[6].orbitRadius = 10.5f;
    planets[6].orbitSpeed = 7.0f;
    planets[7].orbitRadius = 12.0f;
    planets[7].orbitSpeed = 5.0f;
    planets[8].orbitRadius = 13.5f;
    planets[8].orbitSpeed = 4.0f;

    planets[0].textureId = loadTexture("resources/sun.jpg");
    planets[1].textureId = loadTexture("resources/mercury.jpg");
    planets[2].textureId = loadTexture("resources/venus.jpg");
    planets[3].textureId = loadTexture("resources/earth.jpg");
    planets[4].textureId = loadTexture("resources/mars.jpg");
    planets[5].textureId = loadTexture("resources/jupiter.jpg");
    planets[6].textureId = loadTexture("resources/saturn.jpg");
    planets[7].textureId = loadTexture("resources/uranus.jpg");
    planets[8].textureId = loadTexture("resources/neptune.jpg");
}
