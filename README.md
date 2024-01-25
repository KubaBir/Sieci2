# Sieci2

## System zdalnego wyłączania komputerów

Celem tego projektu jest zdalne wyłączanie komputerów podłączonych do serwera. System zakłada dwa typy urządzeń

1. Serwer: \
   Serwer posiada listę wszystkich klientów oraz ich uprawnień. Odpowiada on za komunikacje pomiędzy klientami oraz sprawdzanie uprawnień.
2. Klient: \
   Klienci Podłączają się do serwera i mogą wysyłać oraz odbierać komendy. Głównym zastosowaniem systemu jest zdalne wyłączanie systemu \
   (poprzez komendę `shutdown <nazwa_klienta>`) \
   System obsługuje tez kazdą inną komendę (maksymalnie jedno słowo)

## Kompilacja
`g++ server.cpp -lpthread -Wall -o server` \
`g++ client.cpp -lpthread -Wall -o client`

## Uruchomienie
`./server` \
`./client <nazwa>` 


## Implementacja

Komunikacja realizowana jest w architekturze Klient - Serwer, wykorzystując połączenie TCP. Do nasłuchiwania i akceptowania połączeń uzyte zostały funckje `listen` oraz `accept`. Komunikacja obsługiwana jest przez funkcje `recv` oraz `send`. Dane o kazdym kliencie przechowywane są w strukturze `std::tuple`. Obiekty klientów przechowywane są w strukturze `std::vector`. Program wymaga wersji c++11 lub wyzszej.

### Serwer

Serwer akceptuje połączenia, a następnie dla kazdego z nich tworzy nowy wątek, do obsługi klienta. Nowo utworzony wątek oczekuje wiadomości od klienta, a po jej odebraniu odczytuje typ polecenia i realizuje go. Przesłanie komendy do innego klienta weryfikowane jest poprzez sprawdzenie uprawnień.

### Klient

Klient po połączeniu z serwerem operuje na dwóch wątkach - Jeden do wysyłania poleceń oraz drugi do nasłuchiwania. Weryfikacja uprawnień odbywa się po stronie serwera, więc klient po otrzymaniu polecenia wywołuje je bezwzględnie.

### Uprawnienia

Po uruchomieniu serwera nalezy podłączyć klienta - admina.
Realizowane jest to poprzez uruchomienie klienta o nazwie `admin`.
Serwer wymusza unikalność nazw, więc uruchomienie admina wraz z uruchomieniem serwera zapewnie ochrone przed podszywaniem się. Admin ma mozliwość nadawania uprawnień administratora kazdemu uzytkownikowi. So one odbierane po rozłączeniu klienta.
W przypadku braku uprawnień do wykonania akcji wyświetlony zostaje stosowny komnikat.

### Komunikacja

Wybieranie celu komendy realizowane jest poprzez podanie nazwy klienta docelowego. Serwer wymusza unikalność nazw.

### Uzycie

#### Klient:

Uruchamiany poleceniem `./client <client_name>`. Klient o nazwie `admin` ma uprawnienia administratora i moze tworzyć nowych administratorów.

#### Serwer:

Uruchamiany poleceniem `./server`
