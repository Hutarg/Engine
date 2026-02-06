# Engine

### Présentation :
Ce projet est une base de moteur de jeu, il pourra servir pour la création d'un moteur de jeu ainsi que d'un jeu dans le futur.

### Utilisation :
Il peut être importé directement depuis un fichier c++ en important le fichier .h et en utilisant le fichier .dll

### Architecture :
Le moteur est structuré sous la forme d'un [ECS](https://en.wikipedia.org/wiki/Entity_component_system).

### Librairies utilisés :
 - [glfw](https://www.glfw.org/documentation.html)
 - [vulkan](https://www.vulkan.org)
 - [glm](https://www.opengl.org/sdk/libs/GLM/)

### TODO :
 - [x] Dessiner les sprites dans la boucle principale
 - [ ] Ajout des textures
 - [ ] Mettre en place la physique pour les entités
 - [ ] Ajout de la sérialisation
