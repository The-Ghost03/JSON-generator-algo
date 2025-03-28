# Hybrid JSON/XML Parser

Ce projet implémente un parseur hybride en langage C capable de traiter des fichiers JSON et XML.  
Le parseur lit le fichier d'entrée, détecte automatiquement le format (JSON ou XML), et construit une structure hiérarchique du document.  
Ensuite, il parcourt récursivement cette structure pour extraire **tous les éléments principaux** (à tous les niveaux) dans une liste chaînée générique.

## Table des matières

- [Description](#description)
- [Fonctionnalités](#fonctionnalités)
- [Structure du projet](#structure-du-projet)
- [Installation](#installation)
- [Utilisation](#utilisation)
- [Extensions possibles](#extensions-possibles)
- [Contribuer](#contribuer)
- [Licence](#licence)

## Description

Ce parseur hybride est conçu pour faciliter la lecture et l’analyse de fichiers JSON et XML dans le cadre d’un projet d’algorithmique.  
Il se compose de deux modules principaux :

- **Module JSON**

  - Tokenisation et parsing récursif pour construire une arborescence (structure `JsonValue`).
  - Extraction récursive de tous les éléments (champ et valeurs) dans une liste chaînée générique.

- **Module XML (minimal)**
  - Parsing d’un XML minimal pour extraire les balises, attributs, contenu textuel et enfants (structure `XmlNode`).
  - Extraction récursive de tous les nœuds dans la même liste chaînée.

La liste chaînée, définie via la structure `ElementRecord`, stocke pour chaque élément :

- La **clé** (pour JSON : le nom du champ, pour XML : le nom de la balise)
- Le **contenu** textuel (pour les valeurs primitives) ou le mot `"complex"` si l’élément est imbriqué (objet, tableau ou contient des enfants)

## Fonctionnalités

- **Détection automatique du format**  
  Le programme détecte si le fichier est au format JSON (commence par `{` ou `[`) ou XML (commence par `<`).  
  Il gère également la présence d’un BOM UTF-8 et peut ignorer un prologue XML (`<?xml ... ?>`) ou une déclaration DOCTYPE.

- **Parsing récursif**  
  Le parseur construit une structure hiérarchique pour le JSON ou le XML.  
  Une fonction d’extraction récursive parcourt ensuite l’arborescence pour récupérer tous les éléments et les stocker dans une liste chaînée.

- **Affichage**  
  Le contenu analysé (structure JSON ou XML) est affiché dans la console pour vérification, ainsi que la liste chaînée des éléments extraits.

## Structure du projet

- **Tokenisation et Parsing JSON**
  - Définition des tokens (structure `Token` et enum `TokenType`).
  - Fonctions de tokenisation (`tokenize`), parsing récursif (`parse_json`, `parse_array`, `parse_object`).
- **Parsing XML minimal**

  - Fonctions pour sauter les espaces et analyser les balises (`xml_parse_tag`, `xml_parse_attributes`, `xml_parse_text`).
  - Parsing récursif pour construire un arbre XML (`XmlNode`).

- **Extraction récursive des éléments**

  - Pour JSON : `extract_all_json_elements` qui parcourt toute l’arborescence.
  - Pour XML : `extract_all_xml_elements` qui parcourt récursivement l’arbre XML.

- **Liste chaînée générique**

  - Structure `ElementRecord` pour stocker la clé et le contenu.
  - Fonctions de création, d’ajout, d’affichage et de libération de la liste chaînée.

- **Fonction de lecture de fichier**
  - `read_file` permet de charger le contenu du fichier d'entrée.

## Installation

Ce projet est écrit en C et ne nécessite aucune bibliothèque externe.  
Pour compiler le projet, utilisez GCC (ou un autre compilateur C) en ligne de commande :

```bash
gcc xml-json-parseur.c -o xml-json-parseur.exe
```
