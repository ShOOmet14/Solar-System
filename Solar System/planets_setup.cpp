#include "planets_setup.h"
#include <cstdlib> // dla rand()
#include <iostream>

// Deklaracja funkcji ładowania tekstur, przyjmuje parametr: ścieżka do pliku tekstury
extern unsigned int loadTexture(const char* path);

// Funkcja inicjaluzująca planety, przyjmuje parametr: tablica wektorowa planet
void initializePlanets(std::vector<Planet>& planets) {
    unsigned int moonTexture = loadTexture("resources/moon.jpg");

    std::vector<unsigned int> moonTextures = {
        loadTexture("resources/eris_fictional.jpg"),
        loadTexture("resources/makemake_fictional.jpg"),
        loadTexture("resources/haumea_fictional.jpg"),
        loadTexture("resources/ceres_fictional.jpg")
    };

    Planet sun(glm::vec3(0.0f), 0.7f, glm::vec3(1.0f, 1.0f, 0.0f));
    planets.push_back(sun);

    // Merkury
	planets.emplace_back(glm::vec3(0.0f), 0.10f, glm::vec3(0.5f));

    // Wenus
    planets.emplace_back(glm::vec3(0.0f), 0.18f, glm::vec3(0.9f, 0.7f, 0.2f));

	// Ziemia + Księżyc
    planets.emplace_back(glm::vec3(0.0f), 0.22f, glm::vec3(0.2f, 0.6f, 1.0f));
    Planet moon(glm::vec3(0.0f), 0.05f, glm::vec3(0.8f, 0.8f, 0.8f));
    moon.orbitRadius = 0.5f;
    moon.orbitSpeed = 100.0f;
    moon.textureID = moonTexture;
    planets[3].moons.push_back(moon);

	// Mars + Deimos i Phobos
    planets.emplace_back(glm::vec3(0.0f), 0.15f, glm::vec3(0.8f, 0.3f, 0.2f));
    Planet phobos(glm::vec3(0.0f), 0.03f, glm::vec3(0.6f));
    phobos.orbitRadius = 0.3f;
    phobos.orbitSpeed = 120.0f;
    phobos.textureID = moonTextures[rand() % moonTextures.size()];
    planets[4].moons.push_back(phobos);

    Planet deimos(glm::vec3(0.0f), 0.02f, glm::vec3(0.7f));
    deimos.orbitRadius = 0.5f;
    deimos.orbitSpeed = 90.0f;
    deimos.textureID = moonTextures[rand() % moonTextures.size()];
    planets[4].moons.push_back(deimos);

	// Jowisz + Księżyce: Io, Europa, Ganymede, Callisto
    planets.emplace_back(glm::vec3(0.0f), 0.45f, glm::vec3(0.9f, 0.8f, 0.6f));
    float jMoonSize = 0.05f;

    Planet io(glm::vec3(0.0f), jMoonSize, glm::vec3(0.9f, 0.6f, 0.3f));
    io.orbitRadius = 0.7f;
    io.orbitSpeed = 55.0f;
    io.textureID = moonTextures[rand() % moonTextures.size()];
    planets[5].moons.push_back(io);

    Planet europa(glm::vec3(0.0f), jMoonSize, glm::vec3(0.6f, 0.8f, 1.0f));
    europa.orbitRadius = 0.9f;
    europa.orbitSpeed = 50.0f;
    europa.textureID = moonTextures[rand() % moonTextures.size()];
    planets[5].moons.push_back(europa);

    Planet ganymede(glm::vec3(0.0f), jMoonSize, glm::vec3(0.4f, 0.7f, 0.9f));
    ganymede.orbitRadius = 1.2f;
    ganymede.orbitSpeed = 45.0f;
    ganymede.textureID = moonTextures[rand() % moonTextures.size()];
    planets[5].moons.push_back(ganymede);

    Planet callisto(glm::vec3(0.0f), jMoonSize, glm::vec3(0.6f, 0.5f, 0.4f));
    callisto.orbitRadius = 1.5f;
    callisto.orbitSpeed = 40.0f;
    callisto.textureID = moonTextures[rand() % moonTextures.size()];
    planets[5].moons.push_back(callisto);

	// Saturn + Tytan
    planets.emplace_back(glm::vec3(0.0f), 0.40f, glm::vec3(0.9f, 0.85f, 0.5f));
    Planet tytan(glm::vec3(0.0f), 0.06f, glm::vec3(0.8f, 0.7f, 0.4f));
    tytan.orbitRadius = 1.0f;
    tytan.orbitSpeed = 42.5f;
    tytan.textureID = moonTextures[rand() % moonTextures.size()];
    planets[6].moons.push_back(tytan);

	// Uran + Miranda
    planets.emplace_back(glm::vec3(0.0f), 0.30f, glm::vec3(0.6f, 0.9f, 0.9f));
    Planet miranda(glm::vec3(0.0f), 0.03f, glm::vec3(0.6f, 0.6f, 0.8f));
    miranda.orbitRadius = 0.8f;
    miranda.orbitSpeed = 45.0f;
    miranda.textureID = moonTextures[rand() % moonTextures.size()];
    planets[7].moons.push_back(miranda);

	// Neptun + Tryton
    planets.emplace_back(glm::vec3(0.0f), 0.28f, glm::vec3(0.4f, 0.5f, 0.9f));
    Planet tryton(glm::vec3(0.0f), 0.04f, glm::vec3(0.5f, 0.7f, 0.9f));
    tryton.orbitRadius = 0.7f;
    tryton.orbitSpeed = 47.5f;
    tryton.textureID = moonTextures[rand() % moonTextures.size()];
    planets[8].moons.push_back(tryton);

    // Ustaw parametry orbity dla planet (nie słońca)
    planets[1].orbitRadius = 2.5f;  planets[1].orbitSpeed = 30.0f;
    planets[2].orbitRadius = 3.5f;  planets[2].orbitSpeed = 22.5f;
    planets[3].orbitRadius = 5.0f;  planets[3].orbitSpeed = 17.5f;
    planets[4].orbitRadius = 6.5f;  planets[4].orbitSpeed = 14.0f;
    planets[5].orbitRadius = 8.5f;  planets[5].orbitSpeed = 9.0f;
    planets[6].orbitRadius = 10.5f; planets[6].orbitSpeed = 7.0f;
    planets[7].orbitRadius = 12.0f; planets[7].orbitSpeed = 5.0f;
    planets[8].orbitRadius = 13.5f; planets[8].orbitSpeed = 4.0f;

    // Tekstury planet
    planets[0].textureID = loadTexture("resources/sun.jpg");
    planets[1].textureID = loadTexture("resources/mercury.jpg");
    planets[2].textureID = loadTexture("resources/venus.jpg");
    planets[3].textureID = loadTexture("resources/earth.jpg");
    planets[4].textureID = loadTexture("resources/mars.jpg");
    planets[5].textureID = loadTexture("resources/jupiter.jpg");
    planets[6].textureID = loadTexture("resources/saturn.jpg");
    planets[7].textureID = loadTexture("resources/uranus.jpg");
    planets[8].textureID = loadTexture("resources/neptune.jpg");
}