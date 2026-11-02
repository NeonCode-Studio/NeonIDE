neonIDE - IDE do HTML na Symbian (Qt 4.7 / C++)

BUDOWANIE
1. Zainstaluj Nokia Qt SDK 1.2.1 (Qt Creator + Symbian Anna/Belle).
2. Otworz neonIDE.pro w Qt Creatorze.
3. Wybierz cel "Symbian Device", konfiguracja Release.
4. Build -> w katalogu budowania powstanie neonIDE_installer.sis
   (albo neonIDE.sis). Podpisanie self-signed robi Qt Creator.
5. Wgraj .sis na Nokie E6 (kabel/Bluetooth/karta) i zainstaluj.

UWAGA: UID3 = 0xE6C0DE01 jest testowy (zakres self-sign).

IKONA: neonIDE.svg jest podpieta w neonIDE.pro (ICON = neonIDE.svg)
i zostanie wbudowana w pakiet .sis jako ikona aplikacji.
