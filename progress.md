# Progress Report DarWinWin

Vorab: Ziel Testbarkeit fuer Actor movement, level Generierung und Interaktion, Anpassung der Actorstats

## 30.12.2024: 

**Malin:** Projekt Setup, Webserver ist aufgesetzt

## 31.12.2024: 

**Malin:** 
- Es kann ein Level erstellt werden: Tileflags werden gesetzt mit Bits fuer `Underwater`, `Collidable`, `Protein` etc.
- Die Viewing Cone des Actors kann berechnet werden, je nach Blickrichtung wird eine `viewCone` ausgegeben: 
  ```
    14
   0257
    36
  ```
    mit 0 als aktueller Position des Actors

  Die ViewCone spiegelt wieder, was der Actor sehen kann und soll als Input fuer das NN dienen

## 01.01.2025: 

**Malin:** 
- Refactoring des ViewCone Codes mit LUTs

## 02.01.2025: 

**Malin:**
- Der Actor hat jetzt verschiedene Stats wie Energielevel, verbleibende Luft, Level fuer die verschiedenen Nahrungselemente.
- Der Actor kann sich jetzt um ein Feld in Blickrichtung bewegen und verliert dabei Energie.
- Der Actor kann sich drehen.
- Der Actor kann etwas essen, fuellt damit das Level der Nahrung auf und die Nahrung wird aus dem Level entfernt.

## 03.01.2025 

**Malin:**
- Weitere Arbeit an den ActorStats

## 04.01.2025: 

**Malin:** 
- Der Actor verliert Luft, wenn Unterwasser und Energie, auch ohne Aktion.

## 05.01.2025: 

**Malin:**
- Konsultierung eines Subject Matter Experts zum evolutionären Algorithmus, Evaluierung des Sketchouts fuer den evulutionären Algorithmus und Neuronales Netzwerk Implementierung in SIMD
- Ablauf eines Zuges: Aktuelle Stats anpassen, Viewcone erfassen, Neuronales Netzwerk evaluiert Werte, Ergebnis als Aktion des Actors auswerten und ausfuehren.
- Funktionen fuer Frontend-Anbindung: `getLevel` um das gesamte Level zu erhalten, `setTile` um eine einzelne Tile im Level auf einen Wert zu setzen, `manualAct` um den Actor manuell eine Aktion ausfuehren zu lassen.

## 06.01.2025: Besprechung der Grundlagen und Rahmenbedingungen.

Aktueller Stand:
- Level mit Bitflags für die Umgebung, kann generiert und benutzt werden.
- Actor hat verschieden Stats (z.B. Energie, Luft, Hunger etc.), befindet sich auf dem Spielfeld, kann Aktionen ausfuehren, Stats werden je nach Aktion angepasst.
- Neuronales Netz: Kann mit den Inputs Ergebnisse evaluieren.
- Genetic Algorithm: Prototyp der Infrastruktur besteht.
- Server: kann gestartet werden, handelt `getLevel`, `setTile` und `manualAct`.
- Frontend: Infrakstruktur zur Kommunikation mit dem Server besteht.

Projektorganisation: 
- Daily um 11 Uhr. 
- Taeglicher Progress Report.

Ziele fuer Ende der Woche:
- Level anzeigen koennen und testen.
- Generelle Implementierung evolutioniaerer Algorithmus, testen.

Aktuelle Tasks:
- Jan, Marvin: Level visualisieren, Level manuell veraendern, Player manuell steuern.
- Malin: Genetic Algorithm implementieren, weitere Bereitstellung fuer Frontend Funktionalitaeten im Server, Progress Report Setup

## 07.01.2025:

**Jan:** N/A (krank)

**Marvin:** N/A

**Malin:**
- Das Weblevel wird mit Testwerten initalisiert.
- Der Actor erhaelt Energie zurueck, wenn er Dinge isst.
- Erstellen und Updaten der Readne.md und des Progress Reports

**Hauptrioritaet Frontend:** Visualisierung Level.

**Hauptrioritaet Backend:** Grundstruktur Genetic Algorithm