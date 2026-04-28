# Blueberry

![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20C-blue)
![Repository](https://img.shields.io/badge/Repository-Hutarg%2FEngine-green)
![License](https://img.shields.io/badge/License-MIT-blue)


## Table des Matières

- [À propos](#à-propos)
- [Caractéristiques](#caractéristiques)
- [Prérequis](#prérequis)
- [Installation](#installation)
- [Utilisation](#utilisation)
- [Wiki](#wiki)

## À propos

Ce projet est une libraire développé en C++ conçu pour la création de jeux vidéos.

## Caractéristiques

- ✅ **Rendu graphique** - Mise en place du GUI en cours
- ❌ **Système de physique**
- ❌ **Gestion des ressources**
- ❌ **Support multi-plateforme**
- ❌ **Système d'événements**


## Prérequis

Pour utiliser ce moteur, il vous faudra :
- **C++ 17** ou supérieur
- **Cmake 3.16** ou supérieur
- Un compilateur compatible

### Dépendances externes :
 - [glfw](https://www.glfw.org/documentation.html) (gestion des fenêtres)
 - [vulkan](https://www.vulkan.org) (rendu graphique)
 - [glm](https://www.opengl.org/sdk/libs/GLM/) (maths)
 - [lodepng](https://github.com/lvandeve/lodepng) (lecture de fichiers .png)

## Installation

### 1. Cloner le repository

```bash
git clone https://github.com/Hutarg/Engine.git
cd Engine
```

### 2. Créer le répertoire de compilation

```bash
mkdir build
cd build
```

### 3. Configurer et compiler

```bash
cmake ..
```

## Utilisation

```c++
#include <blueberry.h>

using namespace blueberry;

int main() {
    Application::init();
    
    Window window = Window("Ma première fenêtre",800,600);
    
    Application::run();
    Application::terminate();
    return 0;
}
```

## Wiki

Un wiki sera bientôt créé pour une documentation plus complète.
