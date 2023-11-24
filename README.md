# Projet RFID Read/Write

## Description
Ce projet implémente un système de lecture et d'écriture RFID avec un microcontrôleur Arduino XIAO-ESP32-C3, un lecteur de carte RFID_RC522 et une communication avec un serveur PHP. Il permet de lire et d'enregistrer des identifiants RFID sur une base de données.

## Fichiers
- `Rfid_read_write.ino` : Script principal pour le microcontrôleur. Gère la lecture et l'écriture des tags RFID, le contrôle des LED et boutons, et la communication avec le serveur PHP.
- `index.php` : Script PHP pour la gestion des requêtes du microcontrôleur. Interagit avec une base de données pour enregistrer ou vérifier les identifiants RFID.

## Configuration Matérielle
- Pins configurables pour le reset, SS, LEDs et le bouton.
- Connexion WiFi avec SSID et mot de passe définissables.
- Lecture et écriture RFID avec délais et états modifiables.

## Configuration Logicielle
- Le mode (lecture ou écriture) est modifiable via un bouton.
- Les LED indiquent l'état actuel du système (lecture, écriture, succès, échec).

## Installation et Configuration du Serveur
- Serveur PHP hébergé sur Nginx.
- Configurer les paramètres de la base de données dans le script PHP.
- Activer les rapports d'erreurs pour le débogage (désactiver en production).

## Mise en Place du Serveur
- Installer et configurer Nginx pour héberger le script PHP.
- Assurer que Nginx est correctement configuré pour exécuter les scripts PHP.

## Utilisation
1. Modifier les paramètres de connexion WiFi et les pins selon votre matériel.
2. Téléverser `Rfid_read_write.ino` sur le microcontrôleur.
3. Configurer et démarrer le serveur Nginx avec le script PHP.
4. Interagir avec le système via RFID et observer les réponses du système.

## Auteur
Defint, Akajue, Doppio
