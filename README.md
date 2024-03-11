# Měření rychlosti proudění vzduchu ultrazvukovým sensorem

- Základem je modul ESP32-WROOM
- Pro programování modulu bylo využito Frameworku Platformio
- Jako programovací jazyk byl zvole C++ (C11) s podporou Exceptions
- Popis základních použitých tříd je v kódu
- Komunikace s operátorem probíhá pomocí REST-FULL API
- Při programování nebyla použita žádná část Arduino. Použil se výrobcem dodaný soubor knihoven ESP-IDF
- Implementace ModBus protokolu byla zvolena vlastní
- Jako úložiště pro konfiguraci a případné výsledky měření je použita SPIFFS partition
- Pro serializaci a deserializaci objektů JSON je použita výkonná knihovna nlohmann - JSON
