# 📷 Bildentrauschung – GRA Projekt A220

Dieses Projekt wurde im Rahmen des Grundlagenpraktikums Rechnerarchitektur (GRA) an der Technischen Universität München (TUM) umgesetzt. Ziel ist die Entwicklung eines C-Programms zur Verarbeitung von Farbbildern im **PPM-Format**, welches:

- das Bild in **Graustufen** konvertiert,
- eine **Kantenerkennung mit dem Laplace-Filter** durchführt,
- eine **Weichzeichnung (Blur)** anwendet,
- das Ergebnis zu einem **entrauschten Bild** kombiniert.

Das Programm wurde gemäß den Spezifikationen der Aufgabenstellung implementiert und ist vollständig über die Kommandozeile bedienbar.

---

## 🧠 Funktionsweise

1. **Graustufenumwandlung**  
   Ein gewichteter Mittelwert der RGB-Kanäle mit wählbaren Koeffizienten `a, b, c`.

2. **Laplace-Filter**  
   Für die Kantendetektion per Faltung mit einer 3×3-Matrix.

3. **Weichzeichnung (Gaussian Blur)**  
   Ebenfalls per Faltung mit einer 3×3-Matrix.

4. **Kombination der Filter**  
   Gewichtung zwischen Originalbild und Weichzeichnung basierend auf Kantenerkennung.

---

## 🧪 Beispielaufruf

```bash
./denoise -V 0 --coeffs 0.3,0.59,0.11 -B 5 -o output.ppm input.ppm
```

⚙️ Kommandozeilenoptionen
| Option           | Beschreibung                                                 |
| ---------------- | ------------------------------------------------------------ |
| `-V <Zahl>`      | Version der Implementierung (z. B. `-V 0` für Hauptversion)  |
| `-B <Zahl>`      | Führt die Berechnung `<Zahl>`-mal aus und misst die Laufzeit |
| `--coeffs a,b,c` | RGB-Koeffizienten für Graustufen (z. B. `0.3,0.59,0.11`)     |
| `-o <Dateiname>` | Name der Ausgabedatei                                        |
| `<Dateiname>`    | Eingabedatei im PPM P6-Format (24bpp)                        |
| `-h`, `--help`   | Zeigt Hilfe und Beispiele an                                 |

🖼️ Voraussetzungen
Linux (getestet unter Ubuntu)

C Compiler (GCC-kompatibel)

Eingabebild im PPM-Format (Typ P6, 24bpp)

Das Programm wurde ohne Inline-Assembler geschrieben und verwendet keine nicht unterstützten ISA-Erweiterungen.

📁 Beispielbilder
Zur Demonstration wurden Beispielbilder in Implementierung/Tests/ hinzugefügt (ggf. lokal wegen Dateigröße).

📜 Lizenz
Projekt im Rahmen des Praktikums an der TUM, keine kommerzielle Nutzung vorgesehen.
