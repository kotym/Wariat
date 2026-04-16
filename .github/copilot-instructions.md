<!--
This file contains instructions for GitHub Copilot.

It's a good practice to provide guidance on how to build and run the project,
the project's architecture, and any project-specific conventions.

For more information, see: https://github.com/github/copilot-docs/blob/main/docs/copilot/workspace/configuring-github-copilot-in-your-project.md
-->

## Witamy w projekcie Wariat!

To repozytorium zawiera kod dla hierarchicznego, rozproszonego systemu robotycznego. Celem jest sterowanie fizycznym lub symulowanym pojazdem.

System jest podzielony na trzy główne części:
- **WariatUE**: Projekt w Unreal Engine 5 służący do symulacji robota.
- **WariatCore2**: Aplikacja w C++ na kontroler Husarion Core2 (STM32F4). Odpowiada za niskopoziomową kontrolę silników i sensorów.
- **WariatESP**: Oprogramowanie w ESP-IDF na mikrokontroler ESP32. Odpowiada za autonomię robota, mapowanie terenu i wizualizację mapy na hostowanej stronie internetowej.

Biblioteka **WariatCommon** zawiera kod współdzielony między wszystkimi trzema platformami, aby zapewnić spójność logiki między robotem fizycznym a jego symulacją.

## Architektura

System ma jasno zdefiniowaną hierarchię:

```mermaid
graph TD
    subgraph "Warstwa Logiki i Wizualizacji"
        A[WariatUE - Symulacja]
        C[WariatESP - Autonomia i Web UI]
    end
    
    subgraph "Warstwa Sprzętowa"
        B(WariatCore2 - Kontroler STM32)
    end

    A -->|Logika z WariatCommon| B
    C -->|Polecenia (UART)| B
    B -->|Sterowanie| D{Silniki i Sensory}
    C -->|Wizualizacja Mapy| E((Przeglądarka WWW))

```

- **WariatUE**: Symulacja robota w Unreal Engine 5. Korzysta z `WariatCommon` do uruchamiania tej samej logiki, co na fizycznym robocie.
- **WariatESP**: Mózg operacji autonomicznych. Przetwarza dane, tworzy mapę otoczenia i udostępnia ją przez stronę internetową. Komunikuje się z `WariatCore2` przez UART, wysyłając polecenia.
- **WariatCore2**: Warstwa wykonawcza. Odbiera polecenia od `WariatESP` i bezpośrednio steruje silnikami oraz odczytuje dane z podłączonych sensorów.

### Współdzielony kod: WariatCommon

Katalog `WariatCommon` jest kluczowy dla spójności projektu. Zawiera kod używany przez `WariatCore2`, `WariatESP` oraz symulację `WariatUE`:
- `ComMath.hpp`: Szablonowa klasa `Vector2` i narzędzia matematyczne.
- `ComMap.hpp`/`.cpp`: System mapowania przestrzennego oparty na siatce, zoptymalizowany pod kątem wydajności pamięciowej na mikrokontrolerach.

## Jak budować i uruchamiać

### WariatCore2 (Kontroler STM32)

Projekt używa CMake i frameworka Husarion hFramework.

1.  Przejdź do katalogu build: `cd WariatCore2/build`
2.  Skonfiguruj projekt: `cmake ..`
3.  Zbuduj projekt: `make` (lub `ninja`, jeśli jest dostępny)

Główny punkt wejścia to `hMain` w pliku [WariatCore2/main.cpp](WariatCore2/main.cpp).

### WariatESP (Firmware ESP32)

Projekt używa systemu budowania ESP-IDF.

1.  Upewnij się, że masz skonfigurowane i aktywowane środowisko ESP-IDF.
2.  Przejdź do katalogu projektu: `cd WariatESP`
3.  Zbuduj oprogramowanie: `idf.py build`
4.  Wgraj oprogramowanie na urządzenie: `idf.py flash`
5.  Aby skonfigurować projekt (np. ustawienia WiFi, piny), uruchom `idf.py menuconfig`.

Główny punkt wejścia to `app_main` w pliku [WariatESP/main/main.cpp](WariatESP/main/main.cpp).

### WariatUE (Projekt Unreal Engine)

To standardowy projekt Unreal Engine 5.

1.  Upewnij się, że masz zainstalowane Visual Studio i Unreal Engine.
2.  Kliknij prawym przyciskiem myszy na `WariatUE.uproject` i wybierz "Generate Visual Studio project files".
3.  Otwórz wygenerowany plik `.sln` w Visual Studio.
4.  Zbuduj i uruchom projekt z poziomu Visual Studio lub edytora Unreal.


## Utrzymanie instrukcji Copilot

**Po każdej istotnej zmianie w projekcie (np. nowy moduł, zmiana architektury, zmiana sposobu budowania, nowe zależności) zaktualizuj ten plik (`copilot-instructions.md`), aby instrukcje były zawsze aktualne i pomocne dla innych programistów i AI.**

## Konwencje Kodowania

- **Język i komentarze**: Angielski. Kod, komentarze, nazwy zmiennych i dokumentacja powinny być w języku angielskim.
- **Styl**:
    - Nazwy klas: `PascalCase`.
    - Pliki nagłówkowe: `.hpp` dla C++, `.h` dla C.
    - Kod jest zorganizowany w klasy, z unikaniem surowych funkcji w stylu C, tam gdzie to możliwe.
- **Unikanie RTTI i v-table**: W kodzie na mikrokontrolery (WariatCore2, WariatESP, WariatCommon) unikaj używania funkcji wirtualnych, aby wyeliminować narzut związany z RTTI i v-table. Zamiast tego stosuj polimorfizm statyczny, np. za pomocą wzorca CRTP (Curiously Recurring Template Pattern), w stosownych miejscach możesz też stosować DI (Dependency Injection). Funkcje wirtualne są dozwolone w kodzie symulacji (WariatUE).
- **Zależności**:
    - `WariatCore2` zależy od `hFramework`.
    - `WariatESP` zależy od `ESP-IDF`.
    - `WariatUE` zależy od pluginu `ChaosVehiclesPlugin` do fizyki pojazdów.
- **Komunikacja**: Klasa `ESPInterface` w `WariatCore2` zarządza protokołem komunikacyjnym UART z ESP32. Używa protokołu binarnego opartego na ramkach ze zdefiniowanymi komendami i zdarzeniami.
