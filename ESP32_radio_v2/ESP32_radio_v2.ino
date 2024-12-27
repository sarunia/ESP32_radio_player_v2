
#include "Arduino.h"              // Standardowy nagłówek Arduino, który dostarcza podstawowe funkcje i definicje
#include "Audio.h"                // Biblioteka do obsługi funkcji związanych z dźwiękiem i audio
#include "SPI.h"                  // Biblioteka do obsługi komunikacji SPI
#include "SD.h"                   // Biblioteka do obsługi kart SD
#include "FS.h"                   // Biblioteka do obsługi systemu plików
#include <U8g2lib.h>              // Biblioteka do obsługi wyświetlaczy
#include <ezButton.h>             // Biblioteka do obsługi enkodera z przyciskiem
#include <HTTPClient.h>           // Biblioteka do wykonywania żądań HTTP, umożliwia komunikację z serwerami przez protokół HTTP
#include <EEPROM.h>               // Biblioteka do obsługi pamięci EEPROM, przechowywanie danych w pamięci nieulotnej
#include <Ticker.h>               // Mechanizm tickera do odświeżania timera 1s, pomocny do cyklicznych akcji w pętli głównej
#include <WiFiManager.h>          // Biblioteka do zarządzania konfiguracją sieci WiFi, opis jak ustawić połączenie WiFi przy pierwszym uruchomieniu jest opisany tu: https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // Biblioteka do parsowania i tworzenia danych w formacie JSON, użyteczna do pracy z API
#include <Time.h>                 // Biblioteka do obsługi funkcji związanych z czasem, np. odczytu daty i godziny

#define SD_CS         47          // Pin CS (Chip Select) do komunikacji z kartą SD, wybierany jako interfejs SPI
#define SPI_MOSI      39          // Pin MOSI (Master Out Slave In) dla interfejsu SPI
#define SPI_MISO      0           // Pin MISO (Master In Slave Out) dla interfejsu SPI
#define SPI_SCK       38          // Pin SCK (Serial Clock) dla interfejsu SPI
#define I2S_DOUT      13          // Podłączenie do pinu DIN na DAC
#define I2S_BCLK      12          // Podłączenie po pinu BCK na DAC
#define I2S_LRC       14          // Podłączenie do pinu LCK na DAC
#define SCREEN_WIDTH 256          // Szerokość ekranu w pikselach
#define SCREEN_HEIGHT 64          // Wysokość ekranu w pikselach
#define CLK_PIN1 6                // Podłączenie z pinu 6 do CLK na enkoderze prawym
#define DT_PIN1  5                // Podłączenie z pinu 5 do DT na enkoderze prawym
#define SW_PIN1  4                // Podłączenie z pinu 4 do SW na enkoderze prawym (przycisk)
#define CLK_PIN2 11               // Podłączenie z pinu 10 do CLK na enkoderze
#define DT_PIN2  10               // Podłączenie z pinu 11 do DT na enkoderze lewym
#define SW_PIN2  1                // Podłączenie z pinu 1 do SW na enkoderze lewym (przycisk)
#define MAX_STATIONS 95           // Maksymalna liczba stacji radiowych, które mogą być przechowywane w jednym banku
#define MAX_LINK_LENGTH 110       // Maksymalna długość linku do stacji radiowej
#define STATIONS_URL    "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_01"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL1   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_02"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL2   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_03"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL3   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_04"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL4   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_05"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL5   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_06"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL6   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_07"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL7   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_08"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL8   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_09"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL9   "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_10"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL10  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_11"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL11  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_12"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL12  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_13"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL13  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_14"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL14  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_15"      // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL15  "https://raw.githubusercontent.com/sarunia/ESP32_stream/main/radio_v2_bank_16"      // Adres URL do pliku z listą stacji radiowych
#define MAX_FILES 100             // Maksymalna liczba plików lub katalogów w tablicy directories

int currentSelection = 0;         // Numer aktualnego wyboru na ekranie OLED
int firstVisibleLine = 0;         // Numer pierwszej widocznej linii na ekranie OLED
int station_nr;                   // Numer aktualnie wybranej stacji radiowej z listy
int stationFromBuffer = 0;        // Numer stacji radiowej przechowywanej w buforze do przywrocenia na ekran po bezczynności
int bank_nr;                      // Numer aktualnie wybranego banku stacji z listy
int bankFromBuffer = 0;           // Numer aktualnie wybranego banku stacji z listy do przywrocenia na ekran po bezczynności
int CLK_state1;                   // Aktualny stan CLK enkodera prawego
int prev_CLK_state1;              // Poprzedni stan CLK enkodera prawego    
int CLK_state2;                   // Aktualny stan CLK enkodera lewego
int prev_CLK_state2;              // Poprzedni stan CLK enkodera lewego          
int counter = 0;                  // Licznik dla przycisków
int stationsCount = 0;            // Aktualna liczba przechowywanych stacji w tablicy
int directoryCount = 0;           // Licznik katalogów
int fileIndex = 0;                // Numer aktualnie wybranego pliku audio ze wskazanego folderu
int folderIndex = 0;              // Numer aktualnie wybranego folderu podczas przełączenia do odtwarzania z karty SD
int totalFilesInFolder = 0;       // Zmienna przechowująca łączną liczbę plików w folderze
int volumeValue = 12;             // Wartość głośności, domyślnie ustawiona na 12
int cycle = 0;                    // Numer cyklu do danych pogodowych wyświetlanych w trzech rzutach co 10 sekund
const int maxVisibleLines = 5;    // Maksymalna liczba widocznych linii na ekranie OLED
bool button_1 = false;            // Flaga określająca stan przycisku 1
bool button_2 = false;            // Flaga określająca stan przycisku 2
bool button_3 = false;            // Flaga określająca stan przycisku 3
bool button_4 = false;            // Flaga określająca stan przycisku 4
bool encoderButton1 = false;      // Flaga określająca, czy przycisk enkodera 1 został wciśnięty
bool encoderButton2 = false;      // Flaga określająca, czy przycisk enkodera 2 został wciśnięty
bool fileEnd = false;             // Flaga sygnalizująca koniec odtwarzania pliku audio
bool displayActive = false;       // Flaga określająca, czy wyświetlacz jest aktywny
bool isPlaying = false;           // Flaga określająca, czy obecnie trwa odtwarzanie
bool mp3 = false;                 // Flaga określająca, czy aktualny plik audio jest w formacie MP3
bool flac = false;                // Flaga określająca, czy aktualny plik audio jest w formacie FLAC
bool aac = false;                 // Flaga określająca, czy aktualny plik audio jest w formacie AAC
bool noID3data = false;           // Flaga określająca, czy plik audio posiada dane ID3
bool timeDisplay = true;          // Flaga określająca kiedy pokazać czas na wyświetlaczu, domyślnie od razu po starcie
bool listedStations = false;      // Flaga określająca czy na ekranie jest pokazana lista stacji do wyboru
bool menuEnable = false;          // Flaga określająca czy na ekranie można wyświetlić menu
bool bankMenuEnable = false;      // Flaga określająca czy na ekranie jest wyświetlone menu wyboru banku
unsigned long debounceDelay = 300;        // Czas trwania debouncingu w milisekundach
unsigned long displayTimeout = 6000;      // Czas wyświetlania komunikatu na ekranie w milisekundach
unsigned long displayStartTime = 0;       // Czas rozpoczęcia wyświetlania komunikatu
unsigned long seconds = 0;                // Licznik sekund timera

String directories[MAX_FILES];            // Tablica z indeksami i ścieżkami katalogów
String currentDirectory = "/";            // Ścieżka bieżącego katalogu
String stationName;                       // Nazwa aktualnie wybranej stacji radiowej
String stationString;                     // Dodatkowe dane stacji radiowej (jeśli istnieją)
String bitrateString;                     // Zmienna przechowująca informację o bitrate
String sampleRateString;                  // Zmienna przechowująca informację o sample rate
String bitsPerSampleString;               // Zmienna przechowująca informację o liczbie bitów na próbkę
String artistString;                      // Zmienna przechowująca informację o wykonawcy
String titleString;                       // Zmienna przechowująca informację o tytule utworu
String fileNameString;                    // Zmienna przechowująca informację o nazwie pliku

// Przygotowanie danych pogody do wyświetlenia
String tempStr;           // Zmienna do przechowywania temperatury
String feels_likeStr;     // Zmienna do przechowywania temperatury odczuwalnej
String humidityStr;       // Zmienna do przechowywania wilgotności
String pressureStr;       // Zmienna do przechowywania ciśnienia atmosferycznego
String windStr;           // Zmienna do przechowywania prędkości wiatru
String windGustStr;       // Zmienna do przechowywania prędkości porywów wiatru

File myFile; // Uchwyt pliku

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/ 42, /* dc=*/ 40, /* reset=*/ 41); // Hardware SPI dla wyświetlacza

// Konfiguracja nowego SPI z wybranymi pinami dla czytnika kart SD
SPIClass customSPI = SPIClass(HSPI); // Używamy HSPI, ale z własnymi pinami
const int SD_CS_PIN = 47;  // Pin CS dla czytnika SD

ezButton button1(SW_PIN1);                // Utworzenie obiektu przycisku z enkodera 1 ezButton, podłączonego do pinu 4
ezButton button2(SW_PIN2);                // Utworzenie obiektu przycisku z enkodera 1 ezButton, podłączonego do pinu 1
Audio audio;                              // Obiekt do obsługi funkcji związanych z dźwiękiem i audio
Ticker timer1;                            // Timer do updateTimer co 1s
Ticker timer2;                            // Timer do getWeatherData co 60s
Ticker timer3;                            // Timer do przełączania wyświetlania danych pogodoych w ostatniej linii co 10s
WiFiClient client;                        // Obiekt do obsługi połączenia WiFi dla klienta HTTP

char stations[MAX_STATIONS][MAX_LINK_LENGTH + 1];   // Tablica przechowująca linki do stacji radiowych (jedna na stację) +1 dla terminatora null

const char* ntpServer = "pool.ntp.org";      // Adres serwera NTP używany do synchronizacji czasu
const long  gmtOffset_sec = 3600;            // Przesunięcie czasu UTC w sekundach
const int   daylightOffset_sec = 3600;       // Przesunięcie czasu letniego w sekundach, dla Polski to 1 godzina

// Deklaracja obiektu JSON
StaticJsonDocument<1024> doc;     // Przyjęto rozmiar JSON na 1024 bajty

enum MenuOption
{
  PLAY_FILES,          // Odtwarzacz plików
  INTERNET_RADIO,      // Radio internetowe
  BANK_LIST,           // Lista banków stacji radiowych
};
MenuOption currentOption = INTERNET_RADIO;  // Aktualnie wybrana opcja menu (domyślnie radio internetowe)

bool isAudioFile(const char *filename)
{
  // Dodaj więcej rozszerzeń plików audio, jeśli to konieczne
  return (strstr(filename, ".mp3") || strstr(filename, ".MP3") || strstr(filename, ".wav") || strstr(filename, ".WAV") || strstr(filename, ".flac") || strstr(filename, ".FLAC"));
}


// Funkcja do obsługi przycisków enkoderów, odpowiedzialna za debouncing i wykrywanie długiego naciśnięcia
void handleButtons()  
{
  static unsigned long buttonPressTime = 0;  // Zmienna do przechowywania czasu naciśnięcia przycisku
  static bool isButtonPressed = false;       // Flaga do śledzenia, czy przycisk jest wciśnięty
  static bool actionTaken = false;           // Flaga do śledzenia, czy akcja została wykonana
  static unsigned long lastPressTime = 0;    // Zmienna do kontrolowania debouncingu (ostatni czas naciśnięcia)

  // Sprawdzamy stan przycisku enkodera 2
  int reading2 = digitalRead(SW_PIN2);
  
  // Debouncing dla przycisku enkodera 2
  if (reading2 == LOW)
  {  // Przycisk jest wciśnięty (stan niski)
    // Sprawdzamy, czy minął odpowiedni czas od ostatniego naciśnięcia przycisku (debounce)
    if (millis() - lastPressTime > debounceDelay)
    {
      encoderButton2 = true;  // Ustawiamy flagę, że przycisk został wciśnięty
      lastPressTime = millis();  // Aktualizujemy czas ostatniego naciśnięcia

      // Sprawdzamy, czy przycisk był wciśnięty przez 3 sekundy
      if (!isButtonPressed)
      {
        buttonPressTime = millis();
        isButtonPressed = true;
        actionTaken = false;  // Resetujemy flagę akcji
      }

      // Jeśli przycisk jest wciśnięty przez co najmniej 3 sekundy i akcja jeszcze nie była wykonana
      if (millis() - buttonPressTime >= 3000 && !actionTaken)
      {
        bankMenuEnable = true;
        timeDisplay = false;
        currentOption = BANK_LIST;  // Ustawienie listy banków do przewijania i wyboru

        Serial.println("Wyświetlenie listy banków");
        u8g2.clearBuffer();	
        u8g2.setFont(u8g2_font_ncenB18_tr);
        u8g2.drawStr(20, 40, "WYBIERZ  BANK");
        u8g2.sendBuffer();

        // Ustawiamy flagę akcji, aby wykonała się tylko raz
        actionTaken = true;
      }
    }
  }
  else
  {
    encoderButton2 = false;   // Przywracamy stan przycisku
    isButtonPressed = false;  // Resetujemy flagę naciśnięcia
    actionTaken = false;      // Resetujemy flagę akcji, aby można było ją wykonać ponownie
  }
}


// Funkcja do pobierania danych z API z serwera pogody openweathermap.org
void getWeatherData()
{
  HTTPClient http;  // Utworzenie obiektu HTTPClient
  
  String url = "http://api.openweathermap.org/data/2.5/weather?q=Piła,pl&appid=your_own_API_key";  // URL z danymi do API, na końcu musi być Twój unikalny klucz API otrzymany po resetracji w serwisie openweathermap.org

  http.begin(url);  // Inicjalizacja połączenia HTTP z podanym URL-em, otwieramy połączenie z serwerem.

  int httpCode = http.GET();  // Wysłanie żądanie GET do serwera, aby pobrać dane pogodowe

  if (httpCode == HTTP_CODE_OK)  // Sprawdzenie, czy odpowiedź z serwera była prawidłowa (kod 200 OK)
  {
    String payload = http.getString();  // Pobranie odpowiedzi z serwera w postaci ciągu znaków (JSON)
    Serial.println("Odpowiedź JSON z API:");
    Serial.println(payload); 

    DeserializationError error = deserializeJson(doc, payload);  // Deserializujemy dane JSON do obiektu dokumentu
    if (error)  // Sprawdzamy, czy deserializacja JSON zakończyła się niepowodzeniem
    {
      Serial.print(F("deserializeJson() failed: "));  // Jeśli jest błąd, drukujemy komunikat o błędzie
      Serial.println(error.f_str());  // Wydruk szczegółów błędu deserializacji
      return;  // Zakończenie funkcji w przypadku błędu
    }
    if (timeDisplay == true)
    {
     updateWeather();  // Jeśli deserializacja zakończyła się sukcesem, wywołujemy funkcję `updateWeather`, aby zaktualizować wyświetlacz i serial terminal
    }
  }
  else  // Jeśli połączenie z serwerem nie powiodło się
  {
    Serial.println("Błąd połączenia z serwerem.");
    u8g2.drawStr(0, 62, "                                           ");
    u8g2.drawStr(0, 62, "Brak polaczenia z serwerem pogody");
    u8g2.sendBuffer();  
  }

  http.end();  // Zakończenie połączenia HTTP, zamykamy zasoby
}

// Funkcja do aktualizacji danych pogodowych
void updateWeather()
{
  u8g2.drawStr(0, 62, "                                           "); // Wypełnienie spacjami jako czyszczenie linii

  JsonObject root = doc.as<JsonObject>();  // Konwertuje dokument JSON do obiektu typu JsonObject

  JsonObject main = root["main"];  // Pobiera obiekt "main" zawierający dane główne, takie jak temperatura, wilgotność, ciśnienie
  JsonObject weather = root["weather"][0];  // Pobiera pierwszy element z tablicy "weather", który zawiera dane o pogodzie
  JsonObject wind = root["wind"];  // Pobiera obiekt "wind" zawierający dane o wietrze

  unsigned long timestamp = root["dt"];  // Pobiera timestamp (czas w sekundach) z JSON
  String formattedDate = convertTimestampToDate(timestamp);  // Konwertuje timestamp na sformatowaną datę i godzinę

  float temp = main["temp"].as<float>() - 273.15;  // Pobiera temperaturę w Kelvinach i konwertuje ją na °C
  float feels_like = main["feels_like"].as<float>() - 273.15;  // Pobiera odczuwalną temperaturę i konwertuje ją na °C

  int humidity = main["humidity"];  // Pobiera wilgotność powietrza
  String weatherDescription = weather["description"].as<String>();  // Pobiera opis pogody (np. "light rain")
  String icon = weather["icon"].as<String>();  // Pobiera kod ikony pogody (np. "10d" dla deszczu)
  float windSpeed = wind["speed"];  // Pobiera prędkość wiatru w m/s
  float windGust = wind["gust"];  // Pobiera prędkość podmuchów wiatru w m/s
  float pressure = main["pressure"].as<float>();  // Pobiera ciśnienie powietrza w hPa

  Serial.println("Dane z JSON:");
  Serial.print("Data: ");
  Serial.println(formattedDate);
  Serial.print("Temperatura: ");
  Serial.print(temp, 2);
  Serial.println(" °C");
  tempStr = "Temperatura: " + String(temp, 2) + " C";
  
  Serial.print("Odczuwalna temperatura: ");
  Serial.print(feels_like, 2);
  Serial.println(" °C");
  feels_likeStr = "Odczuwalna: " + String(feels_like, 2) + " C";
  
  Serial.print("Wilgotność: ");
  Serial.print(humidity);
  Serial.println(" %");
  humidityStr = "Wilgotnosc: " + String(humidity) + " %";
  
  Serial.print("Ciśnienie: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  pressureStr = "Cisnienie: " + String(pressure, 2) + " hPa";
  
  Serial.print("Opis pogody: ");
  Serial.println(weatherDescription);
  Serial.print("Ikona: ");
  Serial.println(icon);
  
  Serial.print("Prędkość wiatru: ");
  Serial.print(windSpeed, 2);
  Serial.println(" m/s");
  windStr = "Wiatr: " + String(windSpeed) + " m/s";
  
  Serial.print("Porywy wiatru: ");
  Serial.print(windGust, 2);
  Serial.println(" m/s");
  windGustStr = "W porywach: " + String(windGust) + " m/s";
}

// Funkcja do przełączania między różnymi danymi pogodowymi, które są wyświetlane na ekranie
void switchWeatherData()
{
  u8g2.setFont(u8g2_font_spleen6x12_mr);
  if (timeDisplay == true)
  {
    if (cycle == 0)
    {
      u8g2.drawStr(0, 62, "                                           ");
      u8g2.drawStr(0, 62, tempStr.c_str());
      u8g2.drawStr(130, 62, feels_likeStr.c_str()); 
    } 
    else if (cycle == 1)
    {
      u8g2.drawStr(0, 62, "                                           ");
      u8g2.drawStr(0, 62, windStr.c_str());
      u8g2.drawStr(110, 62, windGustStr.c_str());
    } 
    else if (cycle == 2)
    {
      u8g2.drawStr(0, 62, "                                           ");
      u8g2.drawStr(0, 62, humidityStr.c_str());
      u8g2.drawStr(115, 62, pressureStr.c_str());
    }

      u8g2.sendBuffer();
  }
  // Zmiana cyklu: przechodzimy do następnego zestawu danych
  cycle++;
  if (cycle > 2)
  {
    cycle = 0;  // Wracamy do cyklu 0 po trzecim cyklu
  }
}

// Funkcja konwertująca timestamp na datę i godzinę w formacie "YYYY-MM-DD HH:MM:SS"
String convertTimestampToDate(unsigned long timestamp)  
{
  int year, month, day, hour, minute, second;  // Deklaracja zmiennych dla roku, miesiąca, dnia, godziny, minuty i sekundy z pogodynki
  time_t rawTime = timestamp;                  // Konwersja timestamp na typ time_t, który jest wymagany przez funkcję localtime()
  struct tm* timeInfo;                         // Wskaźnik na strukturę tm, która zawiera informacje o czasie
  timeInfo = localtime(&rawTime);              // Konwertowanie rawTime na strukturę tm zawierającą szczegóły daty i godziny

  year = timeInfo->tm_year + 1900;             // Rok jest liczony od 1900 roku, więc musimy dodać 1900
  month = timeInfo->tm_mon + 1;                // Miesiąc jest indeksowany od 0, więc dodajemy 1
  day = timeInfo->tm_mday;                     // Dzień miesiąca
  hour = timeInfo->tm_hour;                    // Godzina (0-23)
  minute = timeInfo->tm_min;                   // Minuta (0-59)
  second = timeInfo->tm_sec;                   // Sekunda (0-59)

  // Formatowanie na dwie cyfry (dodawanie zer na początku, jeśli liczba jest mniejsza niż 10)
  String strMonth = (month < 10) ? "0" + String(month) : String(month);            // Dodaje zero przed miesiącem, jeśli miesiąc jest mniejszy niż 10
  String strDay = (day < 10) ? "0" + String(day) : String(day);                    // Dodaje zero przed dniem, jeśli dzień jest mniejszy niż 10
  String strHour = (hour < 10) ? "0" + String(hour) : String(hour);                // Dodaje zero przed godziną, jeśli godzina jest mniejsza niż 10
  String strMinute = (minute < 10) ? "0" + String(minute) : String(minute);        // Dodaje zero przed minutą, jeśli minuta jest mniejsza niż 10
  String strSecond = (second < 10) ? "0" + String(second) : String(second);        // Dodaje zero przed sekundą, jeśli sekunda jest mniejsza niż 10

  // Tworzenie sformatowanej daty w formacie "YYYY-MM-DD HH:MM:SS"
  String date = String(year) + "-" + strMonth + "-" + strDay + " " + strHour + ":" + strMinute + ":" + strSecond;
                
  return date;  // Zwraca sformatowaną datę jako String
}


//Funkcja odpowiedzialna za zapisywanie informacji o stacji do pamięci EEPROM.
void saveStationToEEPROM(const char* station)
{   
  // Sprawdź, czy istnieje jeszcze miejsce na kolejną stację w pamięci EEPROM.
  if (stationsCount < MAX_STATIONS)
  {
    int length = strlen(station);

    // Sprawdź, czy długość linku nie przekracza ustalonego maksimum.
    if (length <= MAX_LINK_LENGTH)
    {
      // Zapisz długość linku jako pierwszy bajt.
      EEPROM.write(stationsCount * (MAX_LINK_LENGTH + 1), length);

      // Zapisz link jako kolejne bajty w pamięci EEPROM.
      for (int i = 0; i < length; i++)
      {
        EEPROM.write(stationsCount * (MAX_LINK_LENGTH + 1) + 1 + i, station[i]);
      }

      // Potwierdź zapis do pamięci EEPROM.
      EEPROM.commit();

      // Wydrukuj informację o zapisanej stacji na Serialu.
      Serial.println(String(stationsCount + 1) + "   " + String(station)); // Drukowanie na serialu od nr 1 jak w banku na serwerze

      // Zwiększ licznik zapisanych stacji.
      stationsCount++;
    } 
    else
    {
      // Informacja o błędzie w przypadku zbyt długiego linku do stacji.
      Serial.println("Błąd: Link do stacji jest zbyt długi");
    }
  }
  else
  {
    // Informacja o błędzie w przypadku osiągnięcia maksymalnej liczby stacji.
    Serial.println("Błąd: Osiągnięto maksymalną liczbę zapisanych stacji");
  }
}

// Funkcja odpowiedzialna za zmianę aktualnie wybranej stacji radiowej.
void changeStation()
{  
  mp3 = flac = aac = false;
  stationString.remove(0);  // Usunięcie wszystkich znaków z obiektu stationString

  // Odczytaj link stacji o aktualnym numerze station_nr
  char station[MAX_LINK_LENGTH + 1];
  memset(station, 0, sizeof(station));

  // Odczytaj długość linku
  int length = EEPROM.read((station_nr - 1) * (MAX_LINK_LENGTH + 1));

  // Odczytaj link jako bajty
  for (int j = 0; j < length; j++)
  {
    station[j] = EEPROM.read((station_nr - 1) * (MAX_LINK_LENGTH + 1) + 1 + j);
  }

  // Skopiuj pierwsze 42 znaki do zmiennej stationName - będzie to wyświetlone w pierwszej linii na wyświetlaczu
  stationName = String(station).substring(0, 42);

  // Ręczne przycinanie znaków na końcu linku
  int lastValidCharIndex = length - 1;
  while (lastValidCharIndex >= 0 && (station[lastValidCharIndex] < 33 || station[lastValidCharIndex] > 126))
  {
    station[lastValidCharIndex] = '\0';
    lastValidCharIndex--;
  }

  // Weryfikacja, czy w linku znajduje się "http" lub "https"
  char* validLink = strstr(station, "http://");
  if (validLink == NULL)
  {
    validLink = strstr(station, "https://");
  }

  if (validLink != NULL) 
  {
    // Ustawienie station na początek właściwego linku
    strcpy(station, validLink);

    // Wydrukuj nazwę stacji i link na serialu
    Serial.print("Aktualnie wybrana stacja: ");
    Serial.println(station_nr);
    Serial.print("Link do stacji: ");
    Serial.println(station);

    // Połącz z daną stacją
    audio.connecttohost(station);
    seconds = 0;
    stationFromBuffer = station_nr;
    bankFromBuffer = bank_nr;
    saveStationOnSD();
  }
  else
  {
    Serial.println("Błąd: link stacji nie zawiera 'http' lub 'https'");
  }
}


// Funkcja do pobierania listy stacji radiowych z serwera
void fetchStationsFromServer()
{
  // Utwórz obiekt klienta HTTP
  HTTPClient http;

  // URL stacji dla danego banku
  String url;

  // Wybierz URL na podstawie bank_nr za pomocą switch
  switch (bank_nr)
  {
    case 1:
      url = STATIONS_URL;
      break;
    case 2:
      url = STATIONS_URL1;
      break;
    case 3:
      url = STATIONS_URL2;
      break;
    case 4:
      url = STATIONS_URL3;
      break;
    case 5:
      url = STATIONS_URL4;
      break;
    case 6:
      url = STATIONS_URL5;
      break;
    case 7:
      url = STATIONS_URL6;
      break;
    case 8:
      url = STATIONS_URL7;
      break;
    case 9:
      url = STATIONS_URL8;
      break;
    case 10:
      url = STATIONS_URL9;
      break;
    case 11:
      url = STATIONS_URL10;
      break;
    case 12:
      url = STATIONS_URL11;
      break;
    case 13:
      url = STATIONS_URL12;
      break;
    case 14:
      url = STATIONS_URL13;
      break;
    case 15:
      url = STATIONS_URL14;
      break;
    case 16:
      url = STATIONS_URL15;
      break;
    default:
      Serial.println("Nieprawidłowy numer banku");
      return;
  }

  // Inicjalizuj żądanie HTTP do podanego adresu URL
  http.begin(url);

  // Wykonaj żądanie GET i zapisz kod odpowiedzi HTTP
  int httpCode = http.GET();

  // Wydrukuj dodatkowe informacje diagnostyczne
  Serial.print("Kod odpowiedzi HTTP: ");
  Serial.println(httpCode);

  // Sprawdź, czy żądanie było udane (HTTP_CODE_OK)
  if (httpCode == HTTP_CODE_OK)
  {
    // Pobierz zawartość odpowiedzi HTTP w postaci tekstu
    String payload = http.getString();
    Serial.println("Stacje pobrane z serwera:");

    // Zapisz każdą niepustą stację do pamięci EEPROM z indeksem
    int startIndex = 0;
    int endIndex;
    stationsCount = 0;

    // Przeszukuj otrzymaną zawartość w poszukiwaniu nowych linii
    while ((endIndex = payload.indexOf('\n', startIndex)) != -1 && stationsCount < MAX_STATIONS)
    {
      // Wyodrębnij pojedynczą stację z otrzymanego tekstu
      String station = payload.substring(startIndex, endIndex);
      
      // Sprawdź, czy stacja nie jest pusta, a następnie przetwórz i zapisz
      if (!station.isEmpty())
      {
        //Serial.print("Nowa stacja: ");
        //Serial.println(station);
        sanitizeAndSaveStation(station.c_str());
      }
      
      // Przesuń indeks początkowy do kolejnej linii
      startIndex = endIndex + 1;
    }
  }
  else
  {
    // W przypadku nieudanego żądania wydrukuj informację o błędzie z kodem HTTP
    Serial.printf("Błąd podczas pobierania stacji. Kod HTTP: %d\n", httpCode);
  }

  // Zakończ połączenie HTTP
  http.end();
}

// Funkcja przetwarza i zapisuje stację do pamięci EEPROM
void sanitizeAndSaveStation(const char* station)
{
  // Bufor na przetworzoną stację - o jeden znak dłuższy niż maksymalna długość linku
  char sanitizedStation[MAX_LINK_LENGTH + 1];
  
  // Indeks pomocniczy dla przetwarzania
  int j = 0;

  // Przeglądaj każdy znak stacji i sprawdź czy jest to drukowalny znak ASCII
  for (int i = 0; i < MAX_LINK_LENGTH && station[i] != '\0'; i++)
  {
    // Sprawdź, czy znak jest drukowalnym znakiem ASCII
    if (isprint(station[i]))
    {
      // Jeśli tak, dodaj do przetworzonej stacji
      sanitizedStation[j++] = station[i];
    }
  }

  // Dodaj znak końca ciągu do przetworzonej stacji
  sanitizedStation[j] = '\0';

  // Zapisz przetworzoną stację do pamięci EEPROM
  saveStationToEEPROM(sanitizedStation);
}

void audio_info(const char *info)
{
  // Wyświetl informacje w konsoli szeregowej
  Serial.print("info        ");
  Serial.println(info);
  // Znajdź pozycję "BitRate:" w tekście
  int bitrateIndex = String(info).indexOf("BitRate:");
  bool bitrate = false;
  if (bitrateIndex != -1)
  {
    // Przytnij tekst od pozycji "BitRate:" do końca linii
    bitrateString = String(info).substring(bitrateIndex + 8, String(info).indexOf('\n', bitrateIndex));
    bitrate = true;
  }

  // Znajdź pozycję "SampleRate:" w tekście
  int sampleRateIndex = String(info).indexOf("SampleRate:");
  bool sample = false;
  if (sampleRateIndex != -1)
  {
    // Przytnij tekst od pozycji "SampleRate:" do końca linii
    sampleRateString = String(info).substring(sampleRateIndex + 11, String(info).indexOf('\n', sampleRateIndex));
    sample = true;
  }

  // Znajdź pozycję "BitsPerSample:" w tekście
  int bitsPerSampleIndex = String(info).indexOf("BitsPerSample:");
  bool bits = false;
  if (bitsPerSampleIndex != -1)
  {
    // Przytnij tekst od pozycji "BitsPerSample:" do końca linii
    bitsPerSampleString = String(info).substring(bitsPerSampleIndex + 15, String(info).indexOf('\n', bitsPerSampleIndex));
    bits = true;
  }

  // Znajdź pozycję "skip metadata" w tekście
  int metadata = String(info).indexOf("skip metadata");
  if (metadata != -1)
  {
    noID3data = true;
    Serial.println("Brak ID3 - nazwa pliku: " + fileNameString);
    if (fileNameString.length() > 63)
    {
      fileNameString = String(fileNameString).substring(0, 63);
    }
  }

  if (String(info).indexOf("MP3Decoder") != -1)
  {
    mp3 = true;
    flac = false;
    aac = false;
  }

  if (String(info).indexOf("FLACDecoder") != -1)
  {
    flac = true;
    mp3 = false;
    aac = false;
  }

  if (String(info).indexOf("AACDecoder") != -1)
  {
    aac = true;
    flac = false;
    mp3 = false;
  }

  if ((currentOption == INTERNET_RADIO) && (bitrate == true)) 
  {
    u8g2.clearBuffer();	
    u8g2.setFont(u8g2_font_spleen6x12_mr);
    u8g2.drawStr(0, 10, stationName.c_str());
    String displayString = sampleRateString.substring(1) + "Hz " + bitsPerSampleString + "bit " + bitrateString + "b/s";
    u8g2.drawStr(0, 52, displayString.c_str());
  }

  /*if (currentOption == PLAY_FILES)
  {
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    if (noID3data == true)
    {
      for (int y = 9; y <= 36; y++)
      {
        for (int x = 0; x < 127; x++)
        {
          display.drawPixel(x, y, SH110X_BLACK);
        }
      }
      display.setCursor(0, 10);
      display.println(fileNameString);
    }
    for (int y = 37; y <= 54; y++)
    {
      for (int x = 0; x < 127; x++)
      {
        display.drawPixel(x, y, SH110X_BLACK);
      }
    }
    display.setCursor(0, 37);
    display.println(sampleRateString.substring(1) + "Hz " + bitsPerSampleString + "bit");
    
    display.setCursor(0, 47);
    display.println(bitrateString.substring(1) + "b/s Plik " + String(fileIndex) + "/" + String(totalFilesInFolder));
    for (int y = 56; y <= 63; y++)
    {
      for (int x = 51; x < 127; x++)
      {
        display.drawPixel(x, y, SH110X_BLACK);
      }
    }
    display.setCursor(66, 56);
    display.println("Folder " + String(folderIndex));
    display.display();
    seconds = 0;
  }*/
}

// Funkcja do przetwarzania otrzymanego tekstu na polskie znaki - aktualnie nie działa z u8g2, do sprawdzenia poźniej
/*void processText(String &text)
{
  for (int i = 0; i < text.length(); i++)
  {
    switch (text[i])
    {
      case (char)0xC2:
        switch (text[i+1])
        {
          case (char)0xB3: text.setCharAt(i, 0x10); break; // Zamiana na pojedynczy znak "ł"
        }
        text.remove(i+1, 1); // Usunięcie kolejnego bajtu
        break;
      case (char)0xC3:
        switch (text[i+1])
        {
          case (char)0xB1: text.setCharAt(i, 0x0E); break; // Zamiana na pojedynczy znak "ń"
          case (char)0xB3: text.setCharAt(i, 0x0F); break; // Zamiana na pojedynczy znak "ó"
          case (char)0xBA: text.setCharAt(i, 0x16); break; // Zamiana na pojedynczy znak "ź"
          case (char)0xBB: text.setCharAt(i, 0x1D); break; // Zamiana na pojedynczy znak "Ż"
          
        }
        text.remove(i+1, 1); // Usunięcie kolejnego bajtu
        break;
      case (char)0xC4:
        switch (text[i+1])
        {
          case (char)0x85: text.setCharAt(i, 0x11); break; // Zamiana na pojedynczy znak "ą"
          case (char)0x99: text.setCharAt(i, 0x13); break; // Zamiana na pojedynczy znak "ę"
          case (char)0x87: text.setCharAt(i, 0x14); break; // Zamiana na pojedynczy znak "ć"
          case (char)0x84: text.setCharAt(i, 0x19); break; // Zamiana na pojedynczy znak "Ą"
          case (char)0x98: text.setCharAt(i, 0x1A); break; // Zamiana na pojedynczy znak "Ę"
          case (char)0x86: text.setCharAt(i, 0x1B); break; // Zamiana na pojedynczy znak "Ć"
        }
        text.remove(i+1, 1); // Usunięcie kolejnego bajtu
        break;
      case (char)0xC5:
        switch (text[i+1])
        {
          case (char)0x82: text.setCharAt(i, 0x10); break; // Zamiana na pojedynczy znak "ł"
          case (char)0x84: text.setCharAt(i, 0x0E); break; // Zamiana na pojedynczy znak "ń"
          case (char)0x9B: text.setCharAt(i, 0x12); break; // Zamiana na pojedynczy znak "ś"
          case (char)0xBB: text.setCharAt(i, 0x1D); break; // Zamiana na pojedynczy znak "Ż"
          case (char)0xBC: text.setCharAt(i, 0x15); break; // Zamiana na pojedynczy znak "ż"
          case (char)0x83: text.setCharAt(i, 0x17); break; // Zamiana na pojedynczy znak "Ń"
          case (char)0x9A: text.setCharAt(i, 0x18); break; // Zamiana na pojedynczy znak "Ś"
          case (char)0x81: text.setCharAt(i, 0x1C); break; // Zamiana na pojedynczy znak "Ł"
          case (char)0xB9: text.setCharAt(i, 0x1E); break; // Zamiana na pojedynczy znak "Ź"
        }
        text.remove(i+1, 1); // Usunięcie kolejnego bajtu
        break;
    }
  }
}*/

void audio_id3data(const char *info)
{
  Serial.print("id3data     ");
  Serial.println(info);
  
  // Znajdź pozycję "Artist: " lub "ARTIST: " w tekście
  int artistIndex = String(info).indexOf("Artist: ");
  if (artistIndex == -1)
  {
    artistIndex = String(info).indexOf("ARTIST: ");
  }

  if (artistIndex != -1)
  {
    // Przytnij tekst od pozycji "Artist:" do końca linii
    artistString = String(info).substring(artistIndex + 8, String(info).indexOf('\n', artistIndex));
    Serial.println("Znalazłem artystę: " + artistString);

    // Pomocnicza pętla w celu wyłapania bajtów artistString na serial terminalu 
    /*for (int i = 0; i < artistString.length(); i++) {
      Serial.print("0x");
      if (artistString[i] < 0x10) {
        Serial.print("0"); // Dodaj zero przed pojedynczymi cyframi w formacie hex
      }
      Serial.print(artistString[i], HEX); // Drukowanie znaku jako wartość hex
      Serial.print(" "); // Dodanie spacji po każdym bajcie
    }
    Serial.println(); // Nowa linia po zakończeniu drukowania bajtów*/

    //processText(artistString);
  }
  
  // Znajdź pozycję "Title: " lub "TITLE " w tekście
  int titleIndex = String(info).indexOf("Title: ");
  if (titleIndex == -1)
  {
    titleIndex = String(info).indexOf("TITLE: ");
  }
  
  if (titleIndex != -1)
  {
    // Przytnij tekst od pozycji "Title: " do końca linii
    titleString = String(info).substring(titleIndex + 7, String(info).indexOf('\n', titleIndex));
    Serial.println("Znalazłem tytuł: " + titleString);

    // Pomocnicza pętla w celu wyłapania bajtów titleString na serial terminalu 
    /*for (int i = 0; i < titleString.length(); i++) {
      Serial.print("0x");
      if (titleString[i] < 0x10) {
        Serial.print("0"); // Dodaj zero przed pojedynczymi cyframi w formacie hex
      }
      Serial.print(titleString[i], HEX); // Drukowanie znaku jako wartość hex
      Serial.print(" "); // Dodanie spacji po każdym bajcie
    }
    Serial.println(); // Nowa linia po zakończeniu drukowania bajtów*/

    //processText(titleString);
  }
  
  /*display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("   Odtwarzam plik:   ");

  if (artistString.length() > 21)
  {
    artistString = artistString.substring(0, 21); // Ogranicz długość tekstu do 21 znaków dla wyświetlacza OLED
  }
  display.setCursor(0, 10);
  display.println(artistString);

  if (titleString.length() > 42)
  {
    titleString = titleString.substring(0, 42); // Ogranicz długość tekstu do 42 znaków dla wyświetlacza OLED
  }
  display.setCursor(0, 19);
  display.println(titleString);
  display.display();*/
}

void audio_bitrate(const char *info)
{
  Serial.print("bitrate     ");
  Serial.println(info);
}

void audio_eof_mp3(const char *info)
{
  fileEnd = true;
  Serial.print("eof_mp3     ");
  Serial.println(info);
}

void audio_showstation(const char *info)
{
  Serial.print("station     ");
  Serial.println(info);
}

void audio_showstreamtitle(const char *info)
{
  u8g2.drawStr(0, 21, "                                           ");
  u8g2.drawStr(0, 31, "                                           ");
  u8g2.drawStr(0, 41, "                                           ");

  Serial.print("streamtitle ");
  Serial.println(info);
  stationString = String(info);

  if (stationString.length() > 123)
  {
    stationString = stationString.substring(0, 123); // Ogranicz długość tekstu do 120 znaków dla wyświetlacza OLED
  }

  // Pomocnicza pętla w celu wyłapania bajtów stationString na serial terminalu 
  /*for (int i = 0; i < stationString.length(); i++)  // Pętla iteruje przez każdy znak w `stationString`
  {
    Serial.print("0x"); // Wyświetla prefiks "0x" przed wartością heksadecymalną
    if (stationString[i] < 0x10)
    {
      Serial.print("0"); // Dodaj zero przed pojedynczymi cyframi w formacie hex
    }
    Serial.print(stationString[i], HEX); // // Wyświetla wartość znaku w formacie heksadecymalnym.
    Serial.print(" "); // Dodanie spacji po każdym bajcie
  }
  Serial.println(); // Nowa linia po zakończeniu drukowania bajtów*/

  //processText(stationString);   // Wywołuje funkcję `processText`, która przetwarza tekst zawarty w `stationString`

  // Parametry
  const int maxLineLength = 41;  // Maksymalna długość jednej linii w znakach
  String currentLine = "";  // Bieżąca linia
  int yPosition = 21;  // Początkowa pozycja Y

  // Podziel tekst na wyrazy
  String word;
  int wordStart = 0;

  for (int i = 0; i <= stationString.length(); i++)
  {
    // Sprawdź, czy dotarliśmy do końca słowa lub do końca tekstu
    if (i == stationString.length() || stationString.charAt(i) == ' ')
    {
      // Pobierz słowo
      String word = stationString.substring(wordStart, i);
      wordStart = i + 1;

      // Sprawdź, czy dodanie słowa do bieżącej linii nie przekroczy maxLineLength
      if (currentLine.length() + word.length() <= maxLineLength)
      {
        // Dodaj słowo do bieżącej linii
        if (currentLine.length() > 0)
        {
          currentLine += " ";  // Dodaj spację między słowami
        }
        currentLine += word;
      }
      else
      {
        // Jeśli słowo nie pasuje, wyświetl bieżącą linię i przejdź do nowej linii
        u8g2.drawStr(0, yPosition, currentLine.c_str());
        yPosition += 10;  // Przesunięcie w dół dla kolejnej linii

        // Zresetuj bieżącą linię i dodaj nowe słowo
        currentLine = word;
      }
    }
  }

  // Wyświetl ostatnią linię, jeśli coś zostało
  if (currentLine.length() > 0)
  {
    u8g2.drawStr(0, yPosition, currentLine.c_str());
  }

  // Wysyłanie bufora do wyświetlacza
  u8g2.sendBuffer();

}

void audio_commercial(const char *info)
{
  Serial.print("commercial  ");
  Serial.println(info);
}
void audio_icyurl(const char *info)
{
  Serial.print("icyurl      ");
  Serial.println(info);
}
void audio_lasthost(const char *info)
{
  Serial.print("lasthost    ");
  Serial.println(info);
}
void audio_eof_speech(const char *info)
{
  Serial.print("eof_speech  ");
  Serial.println(info);
}

void displayMenu()
{
  timeDisplay = false;
  menuEnable = true;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_spleen8x16_mr);
  u8g2.drawStr(65, 20, "MENU");

  switch (currentOption)
  {
    case PLAY_FILES:
      u8g2.drawStr(0, 40, ">> Odtwarzacz plikow");
      u8g2.drawStr(0, 60, "   Radio internetowe");
      break;
    case INTERNET_RADIO:
      u8g2.drawStr(0, 40, "   Odtwarzacz plikow");
      u8g2.drawStr(0, 60, ">> Radio internetowe");
      break;
  }
  u8g2.sendBuffer();
}

void printDirectoriesAndSavePaths(File dir, int numTabs, String currentPath)
{
  directoryCount = 0;
  while (true)
  {
    // Otwórz kolejny plik w katalogu
    File entry = dir.openNextFile();
    
    // Jeżeli nie ma więcej plików, przerwij pętlę
    if (!entry)
    {
      break;
    }

    // Sprawdź, czy to katalog
    if (entry.isDirectory())
    {
      // Utwórz pełną ścieżkę do bieżącego katalogu
      String path = currentPath + "/" + entry.name();

      // Zapisz pełną ścieżkę do tablicy
      directories[directoryCount] = path;
      
      // Wydrukuj numer indeksu i pełną ścieżkę
      Serial.print(directoryCount);
      Serial.print(": ");
      Serial.println(path.substring(1));
      
      // Zwiększ licznik katalogów
      directoryCount++;
      
      // Rekurencyjnie wywołaj funkcję dla podkatalogu
      //printDirectoriesAndSavePaths(entry, numTabs + 1, path);
      
      // Jeżeli to nie katalog System Volume Information, wydrukuj na ekranie OLED
      if (path != "/System Volume Information")
      {
        for (int i = 1; i < 7; i++)
        {
          // Przygotuj pełną ścieżkę dla wyświetlenia
          String fullPath = directories[i];
          
          // Ogranicz długość do 21 znaków
          fullPath = fullPath.substring(1, 22);

          // Pomocnicza pętla w celu wyłapania bajtów fullPath na serial terminalu 
          /*for (int i = 0; i < fullPath.length(); i++)
          {
            Serial.print("0x");
            if (fullPath[i] < 0x10)
            {
              Serial.print("0"); // Dodaj zero przed pojedynczymi cyframi w formacie hex
            }
            Serial.print(fullPath[i], HEX); // Drukowanie znaku jako wartość hex
            Serial.print(" "); // Dodanie spacji po każdym bajcie
          }
          Serial.println(); // Nowa linia po zakończeniu drukowania bajtów*/

          //processText(fullPath);
          
          // Ustaw pozycję kursora na ekranie OLED
          //display.setCursor(0, i * 9);
          
          // Wydrukuj skróconą ścieżkę na ekranie OLED
          //display.print(fullPath);
        }
      }
      // Wyświetl aktualny stan ekranu OLED
      //display.display();
    }
    // Zamknij plik
    entry.close();
  }
}

void listDirectories(const char *dirname)
{
  File root = SD.open(dirname);
  if (!root)
  {
    Serial.println("Błąd otwarcia katalogu!");
    return;
  }
  printDirectoriesAndSavePaths(root, 0, ""); // Początkowo pełna ścieżka jest pusta
  Serial.println("Wylistowano katalogi z karty SD");
  root.close();
  scrollDown();
  //displayFolders();
}

// Funkcja do przewijania w górę
void scrollUp()
{
  if (currentSelection > 0)
  {
    currentSelection--;
    if (currentSelection < firstVisibleLine)
    {
      firstVisibleLine = currentSelection;
    }
  }
  // Dodaj dodatkowy wydruk do diagnostyki
  Serial.print("Scroll Up: CurrentSelection = ");
  Serial.println(currentSelection);
}

void scrollDown()
{
  if (currentSelection < maxSelection())
  {
    currentSelection++;
    if (currentSelection >= firstVisibleLine + maxVisibleLines)
    {
      firstVisibleLine++;
    }
    // Dodaj dodatkowy wydruk do diagnostyki
    Serial.print("Scroll Down: CurrentSelection = ");
    Serial.println(currentSelection);
  }
}

int maxSelection()
{
  if (currentOption == INTERNET_RADIO)
  {
    return stationsCount - 1;
  }
  else if (currentOption == PLAY_FILES)
  {
    return directoryCount - 1;
  }
  return 0; // Zwraca 0, jeśli żaden warunek nie jest spełniony
}

void playFromSelectedFolder()
{
  
  String folder = directories[folderIndex];

  Serial.println("Odtwarzanie plików z wybranego folderu: " + folder);

  File root = SD.open(folder);

  if (!root) {
    Serial.println("Błąd otwarcia katalogu!");
    return;
  }
  
  totalFilesInFolder = 0;
  fileIndex = 1; // Domyślnie start odtwarzania od pierwszego pliku audio w folderze
  while (File entry = root.openNextFile())  // Zliczanie wszystkich plików audio w folderze
  {
    String fileName = entry.name();
    if (isAudioFile(fileName.c_str()))
    {
      totalFilesInFolder++;
      //Serial.println(fileName);
    }
    entry.close();
  }
  root.rewindDirectory(); // Przewiń katalog na początek

  File entry;

  while (entry = root.openNextFile())
  {
    String fileName = entry.name();

    // Pomijaj pliki, które nie są w zadeklarowanym formacie audio
    if (!isAudioFile(fileName.c_str()))
    {
      Serial.println("Pominięto plik: " + fileName);
      continue;
    }
    fileNameString = fileName;
    Serial.print("Odtwarzanie pliku: ");
    Serial.print(fileIndex); // Numeracja pliku
    Serial.print("/");
    Serial.print(totalFilesInFolder); // Łączna liczba plików w folderze
    Serial.print(" - ");
    Serial.println(fileName);
    
    // Pełna ścieżka do pliku
    String fullPath = folder + "/" + fileName;
    // Odtwarzaj plik
    audio.connecttoFS(SD, fullPath.c_str());

    isPlaying = true;
    noID3data = false;

    // Oczekuj, aż odtwarzanie się zakończy
    while (isPlaying)
    {
      audio.loop(); // Tutaj obsługujemy odtwarzacz w tle
      button1.loop();
      button2.loop();

      if (button_1) //Przejście do kolejnego pliku w folderze
      {
        counter = 0;
        button_1 = false;
        isPlaying = false;
        audio.stopSong();
        fileIndex++;
        if (fileIndex > totalFilesInFolder)
        {
          Serial.println("To był ostatni plik w folderze");
          folderIndex++;
          playFromSelectedFolder();
        }
        break;            // Wyjdź z pętli
      }

      if (button_2) //Przejście do poprzedniego pliku w folderze
      {
        counter = 0;
        button_2 = false;
        audio.stopSong();
        fileIndex--;
        if (fileIndex < 1)
        {
          fileIndex = 1;
        }
        // Odtwórz znaleziony plik
        root.rewindDirectory(); // Przewiń katalog na początek
        entry = root.openNextFile(); // Otwórz pierwszy plik w katalogu

        // Przesuń się do wybranego pliku
        for (int i = 1; i < fileIndex; i++)
        {
          entry = root.openNextFile();
          if (!entry)
          {
              break; // Wyjdź, jeśli nie znaleziono pliku
          }
        }
        
        // Sprawdź, czy udało się otworzyć plik
        if (entry)
        {
          // Zaktualizuj pełną ścieżkę do pliku
          String fullPath = folder + "/" + entry.name();

          // Odtwórz tylko w przypadku, gdy to jest szukany plik
          if (isAudioFile(entry.name()))
          {
            audio.connecttoFS(SD, fullPath.c_str());
            isPlaying = true;
            Serial.print("Odtwarzanie pliku: ");
            Serial.print(fileIndex); // Numeracja pliku
            Serial.print("/");
            Serial.print(totalFilesInFolder); // Łączna liczba plików w folderze
            Serial.print(" - ");
            Serial.println(fileName);
          }
        }
      } 

      if (fileEnd == true) // Przejście do odtwarzania następnego pliku
      {
        fileEnd = false;
        button_1 = true;
      }

      CLK_state1 = digitalRead(CLK_PIN1);
      if (CLK_state1 != prev_CLK_state1 && CLK_state1 == HIGH)
      {
        timeDisplay = false;
        displayActive = true;
        displayStartTime = millis();
        if (digitalRead(DT_PIN1) == HIGH)
        {
          volumeValue--;
          if (volumeValue < 5)
          {
            volumeValue = 5;
          }
        }
        else
        {
          volumeValue++;
          if (volumeValue > 15)
          {
            volumeValue = 15;
          }
        }
        audio.setVolume(volumeValue); // zakres 0...21
        Serial.print("Wartość głośności: ");
        Serial.println(volumeValue);
        /*display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(4, 0);
        display.println("Volume set");
        display.setTextSize(3);
        display.setCursor(48, 30);
        display.println(volumeValue);
        display.display();*/
      }
      prev_CLK_state1 = CLK_state1;

      CLK_state2 = digitalRead(CLK_PIN2);
      if (CLK_state2 != prev_CLK_state2 && CLK_state2 == HIGH)
      {
        folderIndex = currentSelection;
        timeDisplay = false;
        if (digitalRead(DT_PIN2) == HIGH)
        {
          folderIndex--;
          if (folderIndex < 1)
          {
            folderIndex = 1;
          }
          Serial.print("Numer folderu do tyłu: ");
          Serial.println(folderIndex);
          scrollUp();
          //displayFolders();
        }
        else
        {
          folderIndex++;
          if (folderIndex > (directoryCount - 1))
          {
            folderIndex = directoryCount - 1;
          }
          Serial.print("Numer folderu przodu: ");
          Serial.println(folderIndex);
          scrollDown();
          //displayFolders();
        }
        displayActive = true;
        displayStartTime = millis();
      }
      prev_CLK_state2 = CLK_state2;

      /*if (displayActive && (millis() - displayStartTime >= displayTimeout))   // Przywracanie poprzedniej zawartości ekranu po 6 sekundach
      {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(0, 0);
        display.println("   Odtwarzam plik:   ");
        
        if (noID3data == true)
        {
          display.setCursor(0, 10);
          display.println(fileNameString);
        }
        else
        {
          display.setCursor(0, 10);
          display.println(artistString);
          display.setCursor(0, 19);
          display.println(titleString);
        }

        display.setCursor(0, 37);
        display.println(sampleRateString.substring(1) + "Hz " + bitsPerSampleString + "bit");
        display.setCursor(0, 47);
        display.println(bitrateString.substring(1) + "b/s Plik " + String(fileIndex) + "/" + String(totalFilesInFolder));
        display.setCursor(66, 56);
        display.println("Folder " + String(folderIndex));
        display.display();
        displayActive = false;
        timeDisplay = true;
        currentOption == PLAY_FILES;
      }*/

      if (button2.isPressed())
      {
        audio.stopSong();
        playFromSelectedFolder();
      }

      if (button1.isPressed())
      {
        audio.stopSong();
        //display.clearDisplay();
        encoderButton1 = true;
        break;
      }
    }
    
    if (encoderButton1 == true)
    {
      encoderButton1 = false;
      displayMenu();
      break;
    }
  }
  root.close();
}

// Funkcja do wyświetlania folderów na ekranie OLED z uwzględnieniem zaznaczenia
/*void displayFolders()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("   LISTA KATALOG" + String((char)0x1F) + "W"); // Wyświetla napis "LISTA KATALOGÓW" na ekranie, 0x1F reprezentuje literę 'Ó'
  display.println(currentDirectory);

  int displayRow = 1;  // Zaczynamy od drugiego wiersza (pierwszy to nagłówek)

  for (int i = firstVisibleLine; i < min(firstVisibleLine + 7, directoryCount); i++)
  {
    String fullPath = directories[i];

    // Pomijaj "System Volume Information"
    if (fullPath != "/System Volume Information")
    {
      // Sprawdź, czy ścieżka zaczyna się od aktualnego katalogu
      if (fullPath.startsWith(currentDirectory))
      {
        // Ogranicz długość do 21 znaków
        String displayedPath = fullPath.substring(currentDirectory.length(), currentDirectory.length() + 21);

        processText(displayedPath);

        // Podświetlenie zaznaczenia
        if (i == currentSelection)
        {
          display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        }
        else
        {
          display.setTextColor(SH110X_WHITE);
        }

        // Wyświetl wiersz
        display.setCursor(0, displayRow * 9);
        display.print(displayedPath);
        for (int y = 61; y <= 63; y++) // Wygaszenie 2 ostatnich linii wyświetlacza
        {
          for (int x = 0; x < 127; x++)
          {
            display.drawPixel(x, y, SH110X_BLACK);
          }
        }
        // Przesuń się do kolejnego wiersza
        displayRow++;
      }
    }
  }
  // Przywróć domyślne kolory tekstu
  display.setTextColor(SH110X_WHITE);
  display.display();
}*/


// Funkcja do wyświetlania listy stacji radiowych z opcją wyboru poprzez zaznaczanie w negatywie
void displayStations()
{
  u8g2.clearBuffer();  // Wyczyść bufor przed rysowaniem, aby przygotować ekran do nowej zawartości
  u8g2.setCursor(60, 10);  // Ustaw pozycję kursora (x=60, y=10) dla nagłówka
  u8g2.print("STACJE RADIOWE    ");  // Wyświetl nagłówek "STACJE RADIOWE"
  u8g2.print(String(station_nr) + " / " + String(stationsCount));  // Dodaj numer aktualnej stacji i licznik wszystkich stacji
  
  int displayRow = 1;  // Zmienna dla numeru wiersza, zaczynając od drugiego (pierwszy to nagłówek)

  // Wyświetlanie stacji, zaczynając od drugiej linii (y=21)
  for (int i = firstVisibleLine; i < min(firstVisibleLine + maxVisibleLines, stationsCount); i++)
  {
    char station[MAX_LINK_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez MAX_LINK_LENGTH
    memset(station, 0, sizeof(station));  // Wyczyszczenie tablicy zerami przed zapisaniem danych
    
    // Odczytaj długość nazwy stacji z EEPROM dla bieżącego indeksu stacji
    int length = EEPROM.read(i * (MAX_LINK_LENGTH + 1));

    // Odczytaj nazwę stacji z EEPROM jako ciąg bajtów, maksymalnie do MAX_LINK_LENGTH
    for (int j = 0; j < min(length, MAX_LINK_LENGTH); j++)
    {
      station[j] = EEPROM.read(i * (MAX_LINK_LENGTH + 1) + 1 + j);  // Odczytaj znak po znaku nazwę stacji
    }

    // Sprawdź, czy bieżąca stacja to ta, która jest aktualnie zaznaczona
    if (i == currentSelection)
    {
      u8g2.setDrawColor(1);  // Ustaw biały kolor rysowania
      u8g2.drawBox(0, displayRow * 11, 256, 10);  // Narysuj prostokąt jako tło dla zaznaczonej stacji (x=0, szerokość 256, wysokość 10)
      u8g2.setDrawColor(0);  // Zmień kolor rysowania na czarny dla tekstu zaznaczonej stacji
    }
    else
    {
      u8g2.setDrawColor(1);  // Dla niezaznaczonych stacji ustaw zwykły biały kolor tekstu
    }

    // Wyświetl nazwę stacji, ustawiając kursor na odpowiedniej pozycji
    u8g2.setCursor(0, displayRow * 10 + 11);  // Kursor dla danej linii (x=0, y=21 + 10 * numer_wiersza)
    u8g2.print(station);  // Wyświetl nazwę stacji

    // Przejdź do następnej linii (następny wiersz na ekranie)
    displayRow++;
  }

  // Przywróć domyślne ustawienia koloru rysowania (biały tekst na czarnym tle)
  u8g2.setDrawColor(1);  // Biały kolor rysowania
  u8g2.sendBuffer();  // Wyślij zawartość bufora do ekranu OLED, aby wyświetlić zmiany
}

// Funkcja wywoływana co sekundę przez timer do aktualizacji czasu na wyświetlaczu
void updateTimer()  
{
  // Wypełnij spacjami, aby wyczyścić pole
  u8g2.drawStr(170, 51, "              ");

  // Zwiększ licznik sekund
  seconds++;

  // Wyświetl aktualny czas w sekundach
  // Konwertuj sekundy na minutę i sekundy
  unsigned int minutes = seconds / 60;
  unsigned int remainingSeconds = seconds % 60;

  u8g2.setDrawColor(1); // Ustaw kolor na biały

  if (timeDisplay == true)
  {
    if (audio.isRunning() == true)
    {
      if (mp3 == true)
      {
        u8g2.drawStr(170, 51, "MP3");
        //Serial.println("Gram MP3");
      }
      if (flac == true)
      {
        u8g2.drawStr(170, 51, "FLAC");
        //Serial.println("Gram FLAC");
      }
      if (aac == true)
      {
        u8g2.drawStr(170, 51, "AAC");
        //Serial.println("Gram AAC");
      }
    }

    /*if (currentOption == PLAY_FILES)
    {
      // Formatuj czas jako "mm:ss"
      char timeString[10];
      snprintf(timeString, sizeof(timeString), "%02um:%02us", minutes, remainingSeconds);
      display.print(timeString);
      display.display();
    }*/
    if ((currentOption == INTERNET_RADIO) && ((mp3 == true) || (flac == true) || (aac == true)))
    {
      // Struktura przechowująca informacje o czasie
      struct tm timeinfo;

      // Sprawdź, czy udało się pobrać czas z lokalnego zegara czasu rzeczywistego
      if (!getLocalTime(&timeinfo))
      {
        // Wyświetl komunikat o niepowodzeniu w pobieraniu czasu
        Serial.println("Nie udało się uzyskać czasu");
        return; // Zakończ funkcję, gdy nie udało się uzyskać czasu
      }

      // Konwertuj godzinę, minutę i sekundę na stringi w formacie "HH:MM:SS"
      char timeString[9]; // Bufor przechowujący czas w formie tekstowej
      snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

      //u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(205, 51, timeString);
      u8g2.sendBuffer();
    }
  }
}

// Funkcja do zapisywania danych stacji radiowej na karcie SD
void saveStationOnSD()
{
  // Sprawdź, czy plik station_nr.txt istnieje
  if (SD.exists("/station_nr.txt"))
  {
    Serial.println("Plik station_nr.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość station_nr
    myFile = SD.open("/station_nr.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(station_nr);
      myFile.close();
      Serial.println("Aktualizacja station_nr.txt na karcie SD.");
    }
    else
    {
      Serial.println("Błąd podczas otwierania pliku station_nr.txt.");
    }
  }
  else
  {
    Serial.println("Plik station_nr.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość station_nr
    myFile = SD.open("/station_nr.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(station_nr);
      myFile.close();
      Serial.println("Utworzono i zapisano station_nr.txt na karcie SD.");
    }
    else
    {
      Serial.println("Błąd podczas tworzenia pliku station_nr.txt.");
    }
  }

  // Sprawdź, czy plik bank_nr.txt istnieje
  if (SD.exists("/bank_nr.txt"))
  {
    Serial.println("Plik bank_nr.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość bank_nr
    myFile = SD.open("/bank_nr.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(bank_nr);
      myFile.close();
      Serial.println("Aktualizacja bank_nr.txt na karcie SD.");
    }
    else
    {
      Serial.println("Błąd podczas otwierania pliku bank_nr.txt.");
    }
  }
  else
  {
    Serial.println("Plik bank_nr.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość bank_nr
    myFile = SD.open("/bank_nr.txt", FILE_WRITE);
    if (myFile)
    {
      myFile.println(bank_nr);
      myFile.close();
      Serial.println("Utworzono i zapisano bank_nr.txt na karcie SD.");
    }
    else
    {
      Serial.println("Błąd podczas tworzenia pliku bank_nr.txt.");
    }
  }
}

// Funkcja do odczytu danych stacji radiowej z karty SD
void readStationFromSD()
{
  // Sprawdź, czy karta SD jest dostępna
  if (!SD.begin(47))
  {
    Serial.println("Nie można znaleźć karty SD. Ustawiam domyślne wartości.");
    station_nr = 9; // Domyślny numer stacji gdy brak karty SD
    bank_nr = 1;  // Domyślny numer banku gdy brak karty SD
    return;
  }

  // Sprawdź, czy plik station_nr.txt istnieje
  if (SD.exists("/station_nr.txt"))
  {
    myFile = SD.open("/station_nr.txt");
    if (myFile)
    {
      station_nr = myFile.parseInt();
      myFile.close();
      Serial.print("Wczytano station_nr z karty SD: ");
      Serial.println(station_nr);
    }
    else
    {
      Serial.println("Błąd podczas otwierania pliku station_nr.txt.");
    }
  }
  else
  {
    Serial.println("Plik station_nr.txt nie istnieje.");
  }

  // Sprawdź, czy plik bank_nr.txt istnieje
  if (SD.exists("/bank_nr.txt"))
  {
    myFile = SD.open("/bank_nr.txt");
    if (myFile)
    {
      bank_nr = myFile.parseInt();
      myFile.close();
      Serial.print("Wczytano bank_nr z karty SD: ");
      Serial.println(bank_nr);
    }
    else
    {
      Serial.println("Błąd podczas otwierania pliku bank_nr.txt.");
    }
  }
  else
  {
    Serial.println("Plik bank_nr.txt nie istnieje.");
  }
}


void setup()
{
  // Ustaw pin CS dla karty SD jako wyjście i ustaw go na wysoki stan
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Konfiguruj piny enkodera jako wejścia
  pinMode(CLK_PIN1, INPUT);
  pinMode(DT_PIN1, INPUT);
  pinMode(CLK_PIN2, INPUT);
  pinMode(DT_PIN2, INPUT);
    // Inicjalizacja przycisków enkoderów jako wejścia
  pinMode(SW_PIN1, INPUT_PULLUP);
  pinMode(SW_PIN2, INPUT_PULLUP);

  // Odczytaj początkowy stan pinu CLK enkodera
  prev_CLK_state1 = digitalRead(CLK_PIN1);
  prev_CLK_state2 = digitalRead(CLK_PIN2);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT); // Konfiguruj pinout dla interfejsu I2S audio
  audio.setVolume(volumeValue); // Ustaw głośność na podstawie wartości zmiennej volumeValue w zakresie 0...21

  // Inicjalizuj interfejs SPI wyświetlacza
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);

  // Inicjalizuj komunikację szeregową
  Serial.begin(115200);

  // Inicjalizacja SPI z nowymi pinami dla czytnika kart SD
  customSPI.begin(45, 21, 48, SD_CS_PIN); // SCLK = 45, MISO = 21, MOSI = 48, CS = 47

  // Inicjalizacja karty SD
  if (!SD.begin(SD_CS_PIN, customSPI))
  {
    Serial.println("Błąd inicjalizacji karty SD!");
    return;
  }
  Serial.println("Karta SD zainicjalizowana pomyślnie.");
  
  // Inicjalizuj pamięć EEPROM z odpowiednim rozmiarem
  EEPROM.begin((MAX_STATIONS * (MAX_LINK_LENGTH + 1)));

  // Inicjalizuj wyświetlacz i odczekaj 250 milisekund na włączenie
  u8g2.begin();
  delay(250);
  
  u8g2.clearBuffer();	
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.drawStr(5, 40, "INTERNET RADIO");
  u8g2.sendBuffer();	

  button2.setDebounceTime(50);  // Ustawienie czasu debouncingu dla przycisku enkodera 2

  delay(1000); // Rozgrzewka wyświetlacza, popatrz jak ładnie świeci napis

  // Inicjalizacja WiFiManagera
  WiFiManager wifiManager;
  readStationFromSD();

  // Rozpoczęcie konfiguracji Wi-Fi i połączenie z siecią, jeśli konieczne
  if (wifiManager.autoConnect("ESP Internet Radio"))
  {
    Serial.println("Połączono z siecią WiFi");
    u8g2.clearBuffer();	
    u8g2.setFont(u8g2_font_ncenB18_tr);
    u8g2.drawStr(5, 40, "WIFI CONNECTED");
    u8g2.sendBuffer();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    timer1.attach(1, updateTimer);   // Ustaw timer, aby wywoływał funkcję updateTimer co sekundę
    timer2.attach(300, getWeatherData);   // Ustaw timer, aby wywoływał funkcję getWeatherData co 5 minut
    timer3.attach(10, switchWeatherData);   // Ustaw timer, aby wywoływał funkcję switchWeatherData co 10 sekund
    fetchStationsFromServer();
    changeStation();
    getWeatherData();
  }
  else
  {
    Serial.println("Brak połączenia z siecią WiFi");
    /*display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(35, 5);
    display.println("Wi-Fi");
    display.setCursor(40, 25);
    display.println("not");
    display.setCursor(10, 45);
    display.println("connected");
    display.display();*/
  }
}

void loop()
{
  audio.loop();            // Wykonuje główną pętlę dla obiektu audio (np. odtwarzanie dźwięku, obsługa audio)
  button1.loop();          // Wykonuje pętlę dla obiektu button1 (sprawdza stan przycisku z enkodera 1)
  button2.loop();          // Wykonuje pętlę dla obiektu button2 (sprawdza stan przycisku z enkodera 2)
  handleButtons();         // Wywołuje funkcję obsługującą przyciski i wykonuje odpowiednie akcje (np. zmiana opcji, wejście do menu)

  CLK_state1 = digitalRead(CLK_PIN1);
  if (CLK_state1 != prev_CLK_state1 && CLK_state1 == HIGH)
  {
    timeDisplay = false;
    displayActive = true;
    displayStartTime = millis();
    if (menuEnable == true)  // Przewijanie menu prawym enkoderem
    {
      int DT_state1 = digitalRead(DT_PIN1);
      switch(currentOption)
      {
        case PLAY_FILES:
          if (DT_state1 == HIGH)
          {
            currentOption = BANK_LIST;
          }
          else
          {
            currentOption = INTERNET_RADIO;
          }
          break;
          
        case INTERNET_RADIO:
          if (DT_state1 == HIGH)
          {
            currentOption = PLAY_FILES;
          }
          else
          {
            currentOption = BANK_LIST;
          }
          break;
          
        case BANK_LIST:
          if (DT_state1 == HIGH)
          {
            currentOption = INTERNET_RADIO;
          }
          else
          {
            currentOption = PLAY_FILES;
          }
          break;
      }
      displayMenu();
    }

    else  // Regulacja głośności
    {
      if (digitalRead(DT_PIN1) == HIGH)
      {
        volumeValue--;
        if (volumeValue < 5)
        {
          volumeValue = 5;
        }
      } 
      else
      {
        volumeValue++;
        if (volumeValue > 15)
        {
          volumeValue = 15;
        }
      }
      Serial.print("Wartość głośności: ");
      Serial.println(volumeValue);
      audio.setVolume(volumeValue); // zakres 0...21

      // Używamy funkcji String() do konwersji liczby na tekst
      String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.drawStr(65, 25, "VOLUME SET");
      u8g2.drawStr(115, 50, volumeValueStr.c_str());
      u8g2.sendBuffer();
    }
  }
  prev_CLK_state1 = CLK_state1;

  CLK_state2 = digitalRead(CLK_PIN2);  // Odczytanie aktualnego stanu pinu CLK enkodera 2
  if (CLK_state2 != prev_CLK_state2 && CLK_state2 == HIGH)  // Sprawdzenie, czy stan CLK zmienił się na wysoki
  {
    timeDisplay = false;  // Wyłączanie wyświetlania czasu
    displayActive = true;  // Ustawienie flagi aktywności wyświetlacza
    displayStartTime = millis();  // Zapisanie czasu rozpoczęcia wyświetlania

    if (currentOption == INTERNET_RADIO)  // Przewijanie listy stacji radiowych
    {
      station_nr = currentSelection + 1;
      if (digitalRead(DT_PIN2) == HIGH)  // Obracanie w lewo
      {
        station_nr--;
        if (station_nr < 1)
        {
          station_nr = 1;
        }
        Serial.print("Numer stacji do tyłu: ");
        Serial.println(station_nr);
        scrollUp();  // Wywołanie funkcji przewijania w górę
      }
      else  // Obracanie w prawo
      {
        station_nr++;
        if (station_nr > stationsCount)
        {
          station_nr = stationsCount;
        }
        Serial.print("Numer stacji do przodu: ");
        Serial.println(station_nr);
        scrollDown();  // Wywołanie funkcji przewijania w dół
      }
      displayStations();  // Aktualizacja wyświetlacza z listą stacji
    }

    if ((currentOption == BANK_LIST) && (bankMenuEnable == true))  // Przewijanie listy banków stacji radiowych
    {
      if (digitalRead(DT_PIN2) == HIGH)  // Obracanie w lewo
      {
        bank_nr--;
        if (bank_nr < 1)
        {
          bank_nr = 16;
        }
      } 
      else  // Obracanie w prawo
      {
        bank_nr++;
        if (bank_nr > 16)
        {
          bank_nr = 1;
        }
      }
      // Używamy funkcji String() do konwersji liczby na tekst
      String bankNrStr = String(bank_nr);  // Zamiana liczby banku na ciąg znaków

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB18_tr);
      u8g2.drawStr(40, 40, "NR  BANKU ");

      // Przekazujemy bankNrStr jako ciąg znaków do wyświetlacza
      u8g2.drawStr(210, 40, bankNrStr.c_str());  // Wyświetlenie numeru banku
      u8g2.sendBuffer();
    }
  }
  prev_CLK_state2 = CLK_state2;  // Zapisanie aktualnego stanu CLK jako poprzedni stan

  // Przywracanie poprzedniej zawartości ekranu po 6 sekundach
  if (displayActive && (millis() - displayStartTime >= displayTimeout))   
  {
    // Parametry
    const int maxLineLength = 41;  // Maksymalna długość jednej linii w znakach
    String currentLine = "";  // Bieżąca linia
    int yPosition = 21;  // Początkowa pozycja Y

    // Podziel tekst na wyrazy
    String word;
    int wordStart = 0;

    u8g2.clearBuffer();	
    u8g2.setFont(u8g2_font_spleen6x12_mr);
    u8g2.drawStr(0, 10, stationName.c_str());
    String displayString = sampleRateString.substring(1) + "Hz " + bitsPerSampleString + "bit " + bitrateString + "b/s";
    u8g2.drawStr(0, 52, displayString.c_str());

    for (int i = 0; i <= stationString.length(); i++)
    {
      // Sprawdź, czy dotarliśmy do końca słowa lub do końca tekstu
      if (i == stationString.length() || stationString.charAt(i) == ' ')
      {
        // Pobierz słowo
        String word = stationString.substring(wordStart, i);
        wordStart = i + 1;

        // Sprawdź, czy dodanie słowa do bieżącej linii nie przekroczy maxLineLength
        if (currentLine.length() + word.length() <= maxLineLength)
        {
          // Dodaj słowo do bieżącej linii
          if (currentLine.length() > 0)
          {
            currentLine += " ";  // Dodaj spację między słowami
          }
          currentLine += word;
        }
        else
        {
          // Jeśli słowo nie pasuje, wyświetl bieżącą linię i przejdź do nowej linii
          u8g2.drawStr(0, yPosition, currentLine.c_str());
          yPosition += 10;  // Przesunięcie w dół dla kolejnej linii

          // Zresetuj bieżącą linię i dodaj nowe słowo
          currentLine = word;
        }
      }
    }

    // Wyświetl ostatnią linię, jeśli coś zostało
    if (currentLine.length() > 0)
    {
      u8g2.drawStr(0, yPosition, currentLine.c_str());
    }
    u8g2.sendBuffer();

    /*display.setCursor(102, 37);
    if (mp3 == true)
    {
      display.print("MP3");
    }
    if (flac == true)
    {
      display.print("FLAC");
    }
    if (aac == true)
    {
      display.print("AAC");
    }*/
    
    displayActive = false;
    timeDisplay = true;
    listedStations = false;
    menuEnable = false;
    currentOption = INTERNET_RADIO;
  }
  
  if ((currentOption == PLAY_FILES) && (button1.isPressed()) && (menuEnable == true))
  {
    if (!SD.begin(SD_CS))
    {
      Serial.println("Błąd inicjalizacji karty SD!");
      return;
    }
    /*display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("   LISTA KATALOG" + String((char)0x1F) + "W"); // Wyświetla komunikat "LISTA KATALOGÓW" na ekranie, 0x1F reprezentuje literę 'Ó'
    display.display();*/
    folderIndex = 1;
    currentSelection = 0;
    firstVisibleLine = 0;
    listDirectories("/");
    playFromSelectedFolder();
  }

  if ((currentOption == INTERNET_RADIO) && (button1.isPressed()) && (menuEnable == true))
  {
    changeStation();
  }

  if (button1.isPressed())
  {
    timeDisplay = false;
    displayMenu();
    menuEnable = true;
    displayActive = true;
    displayStartTime = millis();
  }

  if ((currentOption == INTERNET_RADIO) && (button2.isReleased()))
  {
    changeStation();
  }

  if ((currentOption == BANK_LIST) && (button2.isPressed()) && (bankMenuEnable == true))
  {
    bankMenuEnable = false;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(20, 25, "POBIERANIE  STACJI");
    u8g2.drawStr(20, 50, "Z  SERWERA  GITHUB");
    u8g2.sendBuffer();
    currentSelection = 0;
    firstVisibleLine = 0;
    station_nr = 1;
    currentOption = INTERNET_RADIO;
    fetchStationsFromServer();
    changeStation();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_spleen6x12_mr);
    u8g2.sendBuffer();
  }
}