# Blueberry

![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20C-blue)
![Repository](https://img.shields.io/badge/Repository-Hutarg%2FEngine-green)
![License](https://img.shields.io/badge/License-MIT-blue)


## Table des Matières

- [À propos](#à-propos)
- [Installation](#installation)
- [Utilisation](#utilisation)
- [Wiki](#wiki)

## À propos

Ce projet est une libraire développé en C++ conçu pour la création de jeux vidéos.

### Dépendances externes :
 - [glfw](https://www.glfw.org/documentation.hpptml) (gestion des fenêtres)
 - [vulkan](https://www.vulkan.org) (rendu graphique)
 - [glm](https://www.opengl.org/sdk/libs/GLM/) (maths)

## Installation

### 1. Clonage du répertoire

```bash
git clone https://github.com/Hutarg/Engine.git
```

### 2. Compilation

```bash
cmake -S Engine . -B build -G Ninja
```

## Utilisation

```c++
#include <blueberry.hpp>

using namespace blueberry;

class AppListener : public ApplicationListener
{
public:

	AppListener() = default;

	void create() override
	{
		Window window = Window("Ma première fenêtre", 800, 600);
	}
};

int main() {
    Application::init();
    Application::run(new AppListener());
    Application::terminate();
    return 0;
}
```

## Wiki

Un wiki sera bientôt créé pour une documentation plus complète.
