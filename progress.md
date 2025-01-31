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

**Hauptrioritaet Training Engine:** Grundstruktur Genetic Algorithm

## 09.01.2025:

**Jan:** N/A (krank)

**Marvin:**
- Rudimentaere Darstellung Map inkl. api calls wird gehandlet 

**Malin:**
- Grundgeruest des Genetic Algorithm ist implementiert und mit einfachem Beispiel erfolgreich getestet
- Anpassungen der Logik in verschiedenen Actor-Actions.
- Code Review durchgefuehrt
- Crossbreed und Mutate Funktionen fuer Aktor (Mittwochs)

**Aktuelle Tasks:**
- Frontend: angefragte Anpassungen implementieren, handling von `manualAct`
- Training Engine: Mutator mit Random Chance implementieren, weitere Fixes in verschiedenen Actions

## 10.01.2025:

**Jan:** N/A (krank)

**Marvin:** N/A

**Malin:**
- ein Mutator mutiert Daten nur bei einer gewissen Wahrscheinlichkeit
- Actors haben Maegen: es passt nur eine bestimmte Kapazitaet in den Magen, sonst wird keine weitere Nahrung aufgenommen und jede Runde wird Nahrung verdaut, damit wird Energie freigesetzt.

**Aktuelle Tasks:**
- Frontend: angefragte Anpassungen implementieren, handling von `manualAct`
- Training Engine: `Mutator` mit Normalverteilung

## 13.01.2025:

**Jan:** N/A (krank)

**Marvin:**
- Actor manual actions implementiert
- setTile post request + tile info mit stats

**Malin:**
- mutator und crossbreeder verwenden Pointer, um ganze Datenbloecke auf einmal zu handlen
- Komplettes Rewrite des Neuronalen Netzes: unterstuetzt unterschiedliche grosse Layers in einem Netz (Wochenende)
- Tests fuer Evolution und NN (Wochenende)

**Aktuelle Tasks:**

Frontend: 
- level & actor daten korrekt anzeigen (bug fix)
- Testen ob manuelle Aktionen richtig funktionieren (Outcome und Stats kontrollieren)
- Darstellung: Collidable & Wasser farbig
- (Refactoring)

Training Engine:
- Speichersystem NN
- Level Generator

# 14.01.2025

**Marvin:**
- Refactoring, collidables werden korrekt angezeigt

**Jan:**
- Einarbeitung in Code

**Malin:**
- NN in Dateien schreiben vorbereitet

**Aktuelle Tasks:**

Frontend:
- Testen ob manuelle Aktionen richtig funktionieren (Outcome und Stats kontrollieren)
- Viewcone anzeigen
- ~~im Debugger durchsteppen was bei move passiert (im Backend)~~
- Set Tile mit besserer UI
- Info automatisch updaten
- Clean-Up und Refactoring

Training Engine:
- getLevel viewcone hinzufuegen
- NN in Datei schreiben
- Level Generator

## 16.01.2025

**Marvin:** N/A

**Jan:**
- Anfang view cone Implementierung

**Malin:**
- viewcone zu getlevel hinzugefuegt
- Level Generator
- Neuronales Netzwerk kann gespeichert und das neueste geladen werden (Mittwochs)
- Funktion zur Reevaluierung der Gene im Evolutionsprozess (Mittwochs)

Aktuelle Tasks:

Frontend:
- Branch fertig zum mergen machen!
- Viewcone fertig machen
- Testen!

Training Engine:
- Vorbereitung fuer Training

## 17.01.2025

**Marvin:** N/A

**Jan:**
- Bugfix viewcone
- Bugfix Ansicht

**Malin:**
- Code Review

Aktuelle Tasks:

Frontend:
- Testen
- Branch fertig machen!
- viewcone seperat anzeigen aus Frontend zusammengebaut (in neuer Branch1, wenn fertig -> neue MR)
- Button: single step fuer ai-actor `/ai_step` (neue Branch2)
- Button: generiertes Level vom Server anfragen `/level_generate` (neue Branch3 & MR)
- Buttons: `/start_training`, `/stop_training`, `is_training`  (neue Branch4 & MR)

Training Engine:
- Frontend Änderungen mergen, wenn bereit
- Training vorbereiten + training testen

## 20.01.2025

**Marvin:**
- Angefragte Änderungen aus Code Review implementiert
- Anpassung, fuer mehrere Actor

**Jan:**
- Testing actor Funktionalitaeten (Wochenende)

**Malin:**
- Code Review
- Weitere API calls hinzugefuegt
- Merge request gemerged (Wochenende)
- Fehler gefixt, dank Jans Testing (Wochenende)

Aktuelle Tasks:

Frontend:
- viewcone seperat anzeigen aus Frontend zusammengebaut (in neuer Branch1, wenn fertig -> neue MR)
- Button: single step fuer ai-actor `/ai_step` (neue Branch2)
- Button: generiertes Level vom Server anfragen `/level_generate` (neue Branch3 & MR)
- Buttons: `/start_training`, `/stop_training`, `is_training`  (neue Branch4 & MR)

Training Engine:
- Testen von Evolution und Neuronalem Netz

## 21.01.2025

**Marvin:**
- Controlpanel fuer AI_step, Training start etc.

**Jan:** -

**Malin:**
- Erste Testdurchlauefe zum Trainieren
- Frontend: neue Apis und Buttons hinzugefuegt

**Aktuelle Tasks:**

Frontend:
- viewcone im Frontend zusammenstellen (fertigstellen, Jan)
- neue Actions auswaehlen fuer manualAct: `moveDiagnonalLeft`, `moveDiagnonalRight`, `wait`
- set tile: checkboxes zum Auswaehlen der Tilefalgs, copy tileflags von anderer tile
- Feedback implementieren: MR & onclick (Marvin)

Training Engine:
- diagnonal move hinzufuegen
- Outputs gewichtet auswaehlen
- trainloop anpassen (asynchron unabhaengig voneinander trainieren lassen)

## 23.01.2025

**Marvin:** N/A

**Jan:**
- implementieren von grid-based viewcone

**Malin:**
- Diagonal Move, Logik und API calls hinzugefuegt
- Outputs des NNs gewichtet auswaehlen
- Outputs wieder in das neuronales Netzwerk als inputs zurueck fuettern
- neues Trainings-Loop, dass unabhaengige Instanzen parallel trainieren laesst -> massiver Performance Anhieb  (Mittwochs)

**Aktuelle Tasks:**

Frontend:
- neue Actions auswaehlen fuer manualAct: `moveDiagnonalLeft`, `moveDiagnonalRight`, `wait`
- set tile: checkboxes zum Auswaehlen der Tileflags, copy tileflags von anderer tile
- Bugfix: Start AI step macht nichts
- Bugfix: Ai Step ausfuehrt und der Actor ausgewaehlt ist, werden die Stats vom Actor nicht automatisch implementiert

Training Engine:
- mutator, der sich selbst mutated

## 24.01.2025

**Marvin:**
- Bugfix: Ai stop_training
- Delay fuer Start/Stop training button
- Fix: Start/Stop training toggle button 

**Jan:** N/A

**Malin:**
- Neuer smart Mutator, der sich selbst mutated
- Neues Level generiert, Ziel: beibringen zu essen
- Code Review mehrerer Merge Requests

**Aktuelle Tasks:**

Frontend:
- neue Actions auswaehlen fuer manualAct: `moveDiagnonalLeft`, `moveDiagnonalRight`, `wait`
- Reset Stats: automatisch Stats screen updaten
- set tile: checkboxes zum Auswaehlen der Tileflags, copy tileflags von anderer tile
- Jan: Changes for Merge request

Training Engine:
- Crossbreeder anpassen (Performance fix)
- Trainingsdaten anpassen -> essen lernen

## 27.01.2025

**Jan:** - 

**Marvin:**
- Ai_step auto updates Stats view (war Donnerstag, hatte aber MR noch nicht gemacht)
- Buttons für neue actions
- cooy paste tileflags

**Malin:**
- Verbesserte, viel effizientere Variante fuer den Crossbreeder
- Bugfixes im Mutator
- Refactoring
- Optimisierierung
- Hinzufuegen eines Inplace-Mergesort-Algorithmus, um die Reihenfolge der besten Ergebnisse im Training beizubehalten  

**Aktuelle Tasks:**

Frontend:
- view cone Merge Request Änderungen implementieren
- Check boxes für set tile
- In stats zusätzlich letzten Zug des Actors anzeigen (fehlt noch im backend, aber füge ich dann hinzu sobald ich kann -> actor bekommt noch zusätzlichen Datenpunkt “lastAction”)
- Blickrichtung des Actors auf Map anzeigen

Training Engine:
- Debugging im Training

## 28.01.2025

**Jan:**
- checkboxes fertig gemacht 
- merge request Änderungen (die die ich verstanden hab) 
- Design: 
- ⁠selected tile wird farblich markiert 
- ⁠diverse css Änderungen zur Verschönerung des Designs

**Marvin:** -

**Malin:**
- Ueberarbeitung der Energiewerte/Nahrungs/Luft-Werte in der Simulation damit das System ausgeglichen ist

**Aktuelle Tasks:**

Frontend:
- Marvin: bitte noch in deiner MR mit dem Start/Stop Training button, das so anpassen wie wir besprochen hatten
- ⁠lastAction anzeigen
- ⁠Jan: MR Änderungen anpassen

Training Engine:
- Debugging Evolution
- Training zum Essen Lernen

## 30.01.2025

**Jan:**
- Checkboxes funktionieren, ausgewaehlte Box anzeigen
- Design Aenderungen: Icons

**Marvin:**
- LastAction anzeigen

**Malin:**
- Debugging und Bugfixes in Evolution
- Bugfix in Berechnung des Scores fuer die Actor
- Training: Essen lernen
- Neues Level hinzugefuegt (Mittwochs)

**Aktuelle Tasks:**

Frontend:
- Alle Merge Requests fertigstellen

Training Engine:
- Actors beibringen um Collidables zu laufen
