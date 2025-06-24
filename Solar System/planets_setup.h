#pragma once
#include <vector>
#include "planet.h"

// Funkcja ładowania tekstur, przyjmuje parametr: ścieżka do pliku tekstury
extern unsigned int loadTexture(const char* path);

// Funkcja inicjaluzująca planety, przyjmuje parametr: tablica wektorowa planet
void initializePlanets(std::vector<Planet>& planets);