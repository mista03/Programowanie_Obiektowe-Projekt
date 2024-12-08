#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <memory>
#include <iomanip>
#include <limits>
#include <regex>
#include <windows.h>

using namespace std;

unsigned int nextGuestId = 0;
unsigned int nextReservationId = 0;
unsigned int nextPaymentId = 0;

class Hotel;
class Room;
class Guest;
class Reservation;
class Payment;

class Hotel {
private:
    string name;
    string address;
    vector<Guest*> guests;
    vector<Room*> rooms;
    // doba hotelowa
    string checkInTime;
    string checkOutTime;
    string password;

public:
    Hotel(string n, string a, string chkInTime, string chkOutTime, string p)
        : name(n), address(a), checkInTime(chkInTime), checkOutTime(chkOutTime),password(p) {}

    void addGuest(Guest* guest) { guests.push_back(guest); }
    void addRoom(Room& room) { rooms.push_back(&room); }


    vector<Room*> getAvailableRooms(time_t startDate, time_t endDate, unsigned int peopleCount);
    vector<Reservation*> getAllReservations();
    string getCheckInTime() { return checkInTime; }
    string getCheckOutTime() { return checkOutTime; }
    string getPassword() { return password; }
    vector<Guest*> getGuests() { return guests; }

    void displayRooms();
    void displayReservations();
    void displayGuests();
};

class Room {
private:
    string roomNumber;
    string standard;
    unsigned int pricePerNight;
    unsigned int maxAmountOfPeople;
    vector<pair<time_t, time_t>> bookings;

public:
    Room(string n, string s, unsigned int p, unsigned int m)
        : roomNumber(n), standard(s), pricePerNight(p), maxAmountOfPeople(m) {}

    void setPrice(unsigned int price) { pricePerNight = price; }
    void setMaxAmountOfPeople(unsigned int max) { maxAmountOfPeople = max; }

    string getRoomNumber() const { return roomNumber; }
    string getStandard() const { return standard; }
    unsigned int getPricePerNight() const { return pricePerNight; }
    unsigned int getMaxAmountOfPeople() const { return maxAmountOfPeople; }


    bool checkAvailability(time_t startDate, time_t endDate) {
        for (const auto& booking : bookings) {
            // FIXME? JESLI datetime albo konczy sie, miesci sie w calosci, lub zaczyna się w zajetym zakresie ZWROC FALSE
            // doba hotelowa nie przewiduje ze rezerwacja1 konczy sie np o 15 a rezerwacja2 zaczyna sie o 15, bo musi byc przerwa
            bool isContained = startDate <= booking.second && endDate >= booking.first;
            bool endsWithin = startDate < booking.first && (endDate >= booking.first && endDate <= booking.second);
            bool startsWithin = (startDate >= booking.first && startDate <= booking.second) && endDate > booking.second;
            if (isContained || endsWithin || startsWithin) {
                return false;
            }
        }
        return true;
    }

    void bookRoom(time_t startDate, time_t endDate) {
        bookings.push_back({startDate, endDate});
    }
    void cancelBooking(time_t startDate, time_t endDate) {
        bookings.erase(
            remove_if(
                bookings.begin(),
                bookings.end(),
                [startDate, endDate](const pair<time_t, time_t>& booking) {
                    return startDate == booking.first && endDate == booking.second;
                }
            ),
            bookings.end()
        );
    }

    void displayDetails() {
        cout << "Pokój " << roomNumber << " (" << standard << "), Cena: " << pricePerNight
            << " zł, Max osób: " << maxAmountOfPeople << "\n";
    }
};

class Guest {
private:
    string name;
    string email;
    string password;
    vector<Reservation*> reservations;

public:
    unsigned int guestId;

    Guest(string guestName, string guestEmail, string guestPassword)
        : guestId(nextGuestId++), name(guestName), email(guestEmail), password(guestPassword) {}

    void setName(string name) { this->name = name; }
    void setEmail(string email) { this->email = email; }
    void setPassword(string password) { this->password = password; }

    string getName() const { return name; }
    string getEmail() const { return email; }
    vector<Reservation*> getReservations() { return reservations; }


    bool verifyPassword(const string& inputPassword) const { return password == inputPassword; }
    void addReservation(Reservation* reservation) { reservations.push_back(reservation); }
    vector<int> getReservationIds();
    void displayReservations();
};

class Reservation {
private:
    time_t startDate;
    time_t endDate;
    Room* room;
    double totalPrice;
    string status;

public:
    unsigned int reservationId;
    Guest* guest;

    Reservation(Guest* g, time_t start, time_t end, Room* r)
        : reservationId(nextReservationId++), guest(g), startDate(start), endDate(end), room(r) {
        totalPrice = ((endDate - startDate) / 86400 + 1) * room->getPricePerNight();
        status = "potwierdzona";
    }

    void setStatus(string s) { status = s; }

    double getTotalPrice() { return totalPrice; }
    string getStatus() { return status; }

    void displayDetails() {
        char start[64], end[64];

        // v TODO: mozna by ujednolicic z get_time, czyli tutaj put_time
        struct tm* datetime1 = localtime(&startDate);
        strftime(start, 64, "%Y-%m-%d %H:%M", datetime1);

        struct tm* datetime2 = localtime(&endDate);
        strftime(end, 64, "%Y-%m-%d %H:%M", datetime2);
        // ^

        cout << "Rezerwacja #" << reservationId
            << " dla " << guest->getName() << ". Pokój: " << room->getRoomNumber()
            << ", Cena: " << totalPrice << " zł, \nod "
            << start << " do " << end
            << ",\nstatus: " << status << "\n\n";
    }

    void cancel() {
        status = "anulowana";
        room->cancelBooking(startDate, endDate);
        cout << "Rezerwacja anulowana.\n";
    }
};

class Payment {
protected:
    double amount;
    time_t paymentDate;

public:
    unsigned int paymentId;

    Payment(double a) : paymentId(nextPaymentId++), amount(a) {}

    void setDate(time_t date) { paymentDate = date; }
    virtual void processPayment() = 0;
};

class CardPayment : public Payment {
public:
    CardPayment(double amount) : Payment(amount) {}

    void processPayment() override {
        cout << "Płatność kartą. Kwota: " << amount << " zł\n";
    }
};

class CashPayment : public Payment {
public:
    CashPayment(double amount) : Payment(amount) {}

    void processPayment() override {
        cout << "Płatność gotówką. Kwota: " << amount << " zł\n";
    }
};


vector<Room*> Hotel::getAvailableRooms(time_t startDate, time_t endDate, unsigned int peopleCount) {
    vector<Room*> availableRooms;
    for (auto& room : rooms) {
        bool isBigEnough = room->getMaxAmountOfPeople() >= peopleCount;
        bool isAvailable = room->checkAvailability(startDate, endDate);
        if (isBigEnough && isAvailable) {
            availableRooms.push_back(room);
        }
    }
    return availableRooms;
}

vector<Reservation*> Hotel::getAllReservations() {
    vector<Reservation*> allReservations;
    vector<Reservation*> reservationIds;
    for (auto& guest : guests) {
        reservationIds = guest->getReservations();
        allReservations.insert(allReservations.end(), reservationIds.begin(), reservationIds.end());
    }
    return allReservations;
}

void Hotel::displayRooms() {
    cout << "Lista wszystkich pokoi:\n";
    for (auto& room : rooms) {
        room->displayDetails();
    }
}

void Hotel::displayReservations() {
    vector<Reservation*> allReservations = this->getAllReservations();
    cout << "Lista rezerwacji:\n";
    for (auto& reservation : allReservations) {
        reservation->displayDetails();
    }
}

void Hotel::displayGuests() {
    for (auto& guest : guests) {
        string name = guest->getName();
        cout << name << "\n";
    }
}


void Guest::displayReservations() {
    if (reservations.empty()) {
        cout << "Brak rezerwacji.\n";
    } else {
        cout << "Twoje rezerwacje:\n";
        for (auto& reservation : reservations) {
            reservation->displayDetails();
        }
    }
}

vector<int> Guest::getReservationIds() {
    vector<int> reservationIds;
    for (auto& reservation:reservations){
        reservationIds.push_back(reservation->reservationId);
    }
    return reservationIds;
}

void waitForEnter() {

    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Aby przejść dalej naciśnij ENTER";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string inputMail() {
    string email;
    regex emailRegex(R"((^[^\s@]+@[^\s@]+\.[^\s@]+$))");

    while (true) {
        cout << "Podaj swój adres e-mail: ";
        cin >> email;

        if (regex_match(email, emailRegex)) {
            return email;
        } else {
            cout << "Niepoprawny format adresu e-mail. Sprobuj ponownie.\n";
        }
    }
}

string inputDate() {
    // uzytkownik podaje date jako string
    string date;
    // TODO: wykrywac nieprawidlowe daty, np. 2025-02-31, 2025-31-31
    regex datePattern(R"(\d{4}-\d{2}-\d{2})"); // YYYY-MM-DD

    // sprawdzanie regex daty, ewentualna prosba o ponowne podanie
    while (true) {
        cin >> date;
        if (regex_match(date, datePattern)) {
            return date;
        } else {
            cout << "Nieprawidłowy format daty. Spróbuj ponownie.\n";
        }
    }
}

time_t convertDate(string datetime) {
    tm dt = {};
    istringstream(datetime) >> get_time(&dt, "%Y-%m-%d %H:%M:%S");

    return mktime(&dt);
}

void manageHotelProfile(Hotel* hotel);
void manageGuestLogin(Hotel* hotel);
void manageGuestProfile(Guest* guest, Hotel* hotel);

void displayMainManu(Hotel* hotel) {
    int choice = 0;

    while (choice != 3) {
        system("cls");
        cout << "\n=== Witaj w aplikacji hotelowej ===\n";
        cout << "1. Profil hotelu\n";
        cout << "2. Profil gościa\n";
        cout << "3. Zakończ\n";
        cout << "Twój wybór: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Profil hotelu\n";
                manageHotelProfile(hotel);
                break;
            case 2:
                cout << "Profil gościa\n";
                manageGuestLogin(hotel);
                break;
            case 3:
                cout << "Do widzenia!\n";
                break;
            default:
                cout << "Nieprawidłowy wybór. Spróbuj ponownie.\n";
        }
    }
}

void manageHotelProfile(Hotel* hotel) {
    string password;
    string adminPassword = hotel->getPassword();
    int choice = 0;
    int attempts = 3;
    while (attempts > 0) {
        cout << "\nPodaj hasło administratora: ";
        cin >> password;

        if (password != adminPassword) {
            cout << "Nieprawidłowe hasło! Liczba pozostałych prób: " << --attempts << "\n";
            if (attempts == 0) {
                cout << "Powrót do menu!\n";
                return;
            }
        } else {
            break;
        }
    }

    while (choice != 5){
        system("cls");
        cout << "\n=== Profil hotelu ===\n";
        cout << "1. Wyświetl wszystkie pokoje\n";
        cout << "2. Sprawdź dostępność pokoi\n";
        cout << "3. Wyświetl rezerwacje\n";
        cout << "4. Anuluj rezerwacje\n";
        cout << "5. Powrót do głównego menu\n";
        cout << "Twój wybór: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                hotel->displayGuests();
                hotel->displayRooms();
                waitForEnter();
                break;
            }
            case 2: {
                cout << "Podaj datę początku rezerwacji\n";
                time_t startDate = convertDate(inputDate() + string(" ") + hotel->getCheckInTime());
                cout << "Podaj datę końca rezerwacji\n";
                time_t endDate = convertDate(inputDate() + string(" ") + hotel->getCheckOutTime());

                auto rooms = hotel->getAvailableRooms(startDate, endDate, 1);
                cout << "Lista wszystkich pokoi:\n";
                for (auto& room : rooms) {
                    room->displayDetails();
                }
                waitForEnter();
                break;
            }
            case 3:
                hotel->displayReservations();
                waitForEnter();
                break;
            case 4: {
                hotel->displayReservations();

                int reservationId;
                auto allReservations = hotel->getAllReservations();
                if (!allReservations.empty()) {
                    std::cout << "Podaj numer rezerwacji do anulowania: ";
                    cin >> reservationId;
                }

                for (auto& reservation : allReservations) {
                    if (reservationId == reservation->reservationId) {
                        reservation->cancel();
                        break;
                    }
                }
                waitForEnter();
                break;
            }
            case 5:
                cout << "Powrót do głównego menu\n";
                break;
            default: {
                cout << "Nieprawidłowy wybór. Spróbuj ponownie.\n";
                waitForEnter();
            }
        }
    }
}

void manageGuestLogin(Hotel* hotel) {
    int choice = 0;

    while (choice != 3) {
        system("cls");
        cout << "\n=== Wybierz działanie ===\n";
        cout << "1. Zaloguj się na swoje konto\n";
        cout << "2. Utwórz nowe konto\n";
        cout << "3. Powrót do menu\n";
        cout << "Twój wybór: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                string email;
                string password;
                Guest* loggingGuest;
                bool check = false;
                auto guests = hotel->getGuests();
                cout << "Logowanie do konta\n";
                email = inputMail();
                for (auto& guest : guests) {
                    if (email == guest->getEmail()) {
                        loggingGuest = guest;
                        check = true;
                        break;
                    }
                }
                if (!check) {
                    cout << "Konto o podanym adresie email nie istnieje\n";
                    waitForEnter();
                    break;
                }

                int attempts = 3;
                while (attempts > 0) {
                    cout << "Podaj hasło: ";
                    cin >> password;

                    if (!loggingGuest->verifyPassword(password)) {
                        cout << "Nieprawidłowe hasło! Liczba pozostałych prób: " << --attempts << "\n";
                        if (attempts == 0) {
                            cout << "Powrót do menu!\n";
                            waitForEnter();
                            return;
                        }
                    } else {
                        cout << "Udało się zalogować!\n";
                        waitForEnter();
                        manageGuestProfile(loggingGuest,hotel);
                        break;
                    }
                }
                break;
            }
            case 2: {
                string name;
                string email;
                string password;

                cout << "Podaj swoje imię i nazwisko: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, name);
                email = inputMail();
                cout << "Podaj hasło: ";
                cin >> password;
                Guest* newGuest = new Guest(name, email, password);
                hotel->addGuest(newGuest);
                cout << "Udało się założyć konto, teraz się zaloguj!\n";

                break;
            }
            case 3:
                cout << "Wracam do menu\n";
                break;
            default:
                cout << "Nieprawidłowy wybór. Spróbuj ponownie.\n";
        }
    }
}

void manageGuestProfile(Guest* guest, Hotel* hotel) {
    cout << "Zarządzenie swoim profilem\n";
    int choice = 0;
    while (choice != 6){
        system("cls");
        cout << "\n=== Profil Użytkownika ===\n";
        cout << "1. Dokonaj rezerwacji\n";
        cout << "2. Wyświetl swoje rezerwacje\n";
        cout << "3. Odwołaj rezerwacje\n";
        cout << "4. Oplac rezerwacje\n";
        cout << "5. Zmień dane\n";
        cout << "6. Powrót do głównego menu\n";
        cout << "Twój wybór: ";
        cin >> choice;

        switch (choice) {
            // dokonanie rezerwacji
            case 1: {
                int peopleCount;
                cout << "Podaj datę początku rezerwacji: ";
                time_t startDate = convertDate(inputDate() + string(" ") + hotel->getCheckInTime());
                cout << "Podaj datę końca rezerwacji: ";
                time_t endDate = convertDate(inputDate() + string(" ") + hotel->getCheckOutTime());
                cout << "Podaj liczbę osób: ";
                cin >> peopleCount;

                cout << "Wybierz dostępny pokój: \n";
                auto rooms = hotel->getAvailableRooms(startDate, endDate, peopleCount);
                for (auto& room : rooms) room->displayDetails(); //
                string roomNumber;
                cout << "Podaj numer pokoju: \n";
                cin >> roomNumber;
                bool check = false;
                for (auto& room : rooms) {
                    if (room->getRoomNumber() == roomNumber) {
                        auto chosenRoom = room;
                        auto* new_reservation = new Reservation(guest,startDate,endDate,chosenRoom);
                        guest->addReservation(new_reservation);

                        check = true;
                        cout << "Pomyślnie dokonano rezerwacji! Calkowita cena: " << new_reservation->getTotalPrice() << "zl\n";
                        waitForEnter();
                        break;
                    }
                }
                if (!check) {
                    cout << "Błędny numer pokoju, spróbuj jeszcze raz.\n";
                    waitForEnter();
                }

                break;
            }
            // wyswietlenie rezerwacji uzytkownika
            case 2: {
                guest->displayReservations();
                waitForEnter();
                break;
            }
            // odwolanie rezerwacji
            case 3: {
                guest->displayReservations();
                auto allReservations = hotel->getAllReservations();
                vector<int> reservationIds = guest->getReservationIds();

                int reservationIdToDelete;
                if(!reservationIds.empty()) {
                    cout << "Podaj numer rezerwacji do anulowania: ";
                    cin >> reservationIdToDelete;
                }

                for (auto& reservation : allReservations) {
                    if (find(reservationIds.begin(), reservationIds.end(), reservationIdToDelete) != reservationIds.end()) {
                        if (reservationIdToDelete == reservation->reservationId) {
                            reservation->cancel();
                            break;
                        }
                    } else {
                        cout << "Nie posiadzasz rezerwacji o podanym ID: " << reservationIdToDelete << "\n";
                        waitForEnter();
                        break;
                    }
                }

                break;
            }
            // oplacenie rezerwacji
            case 4: {
                vector<Reservation*> reservations = guest->getReservations();
                Reservation* chosenReservation;
                int reservation_num = reservations.size();
                if (reservation_num < 1) {
                    cout << "Uzytkownik nie ma rezerwacji.\n";
                    break;
                }

                int payFor;
                guest->displayReservations();
                cout << "\nPodaj id rezerwacji do oplacenia\n";
                cout << "Twój wybór: ";
                cin >> payFor;

                bool alreadyPaid = false;
                bool isCancelled = false;
                bool foundId = false;
                for (auto& reservation : reservations) {
                    if (reservation->reservationId == payFor) {
                        foundId = true;
                        chosenReservation = reservation;
                        if (chosenReservation->getStatus() == "oplacona") alreadyPaid = true;
                        if (chosenReservation->getStatus() == "anulowana") isCancelled = true;
                        break;
                    }
                }
                if (!foundId) {
                    cout << "Nie znaleziono rezerwacji o takim ID.\n";
                    waitForEnter();
                    break;
                }
                if (isCancelled) {
                    cout << "Rezerwacja została już anulowana. Złóż nową rezerwację.\n";
                    waitForEnter();
                    break;
                }
                if (alreadyPaid) {
                    cout << "Rezerwacja jest juz oplacona.\n";
                    waitForEnter();
                    break;
                }

                int choice2 = 0;
                cout << "\nWybierz metode platnosci\n";
                cout << "1. Platnosc karta\n";
                cout << "2. Platnosc gotowka\n";
                cout << "3. Powrot\n";
                cout << "Twój wybór: ";
                cin >> choice2;

                switch (choice2) {
                    case 1: {
                        Payment* cardPayment = new CardPayment(chosenReservation->getTotalPrice());
                        cardPayment->processPayment();
                        cardPayment->setDate(time(NULL));
                        chosenReservation->setStatus("oplacona");
                        waitForEnter();
                        break;
                    }
                    case 2: {
                        Payment* cashPayment = new CashPayment(chosenReservation->getTotalPrice());
                        cashPayment->processPayment();
                        cashPayment->setDate(time(NULL));
                        chosenReservation->setStatus("oplacona");
                        waitForEnter();
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            // zmiana danych uzytkownika
            case 5: {

                cout << "Zmiana danych użytkownika\n";
                int choice2 = 0;
                while (choice2 != 5) {
                    system("cls");
                    cout << "\n=== Profil Użytkownika ===\n";
                    cout << "1. Wyświetl dane\n";
                    cout << "2. Zmień imię i nazwisko\n";
                    cout << "3. Zmień adres e-mail\n";
                    cout << "4. Zmień hasło\n";
                    cout << "5. Powrót\n";
                    cout << "Twój wybór: ";
                    cin >> choice2;

                    switch (choice2) {
                        case 1: {
                            cout << "Imię i Nazwiko: " << guest->getName() << "\n"
                            << "Adres e-mail: "<< guest->getEmail() << "\n";
                            waitForEnter();
                            break;
                        }
                        case 2: {
                            string name;
                            cout << "Podaj imię i nazwisko na jakie chcesz zmienić: ";
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            getline(cin, name);
                            guest->setName(name);
                            break;
                        }
                        case 3: {
                            cout << "Podaj adres e-mail na jaki chcesz zmienić: ";
                            string email = inputMail();
                            guest->setEmail(email);
                            break;
                        }
                        case 4: {
                            string password;
                            string newPassword;
                            cout << "Podaj swoje hasło: ";
                            cin >> password;
                            if (guest->verifyPassword(password)) {
                                cout << "Podaj nowe hasło: ";
                                cin >> newPassword;
                                guest->setPassword(newPassword);
                            } else {
                                cout << "Błędne hasło. Spróbuj jeszcze raz\n";
                                waitForEnter();
                                break;
                            }
                            break;
                        }
                        case 5: {
                            cout << "Powrót do głównego menu\n";
                            break;
                        }
                        default:
                            cout << "Nieprawidłowy wybór. Spróbuj ponownie.\n";
                    }
                }
                break;
            }
        }
    }
}


int main() {
    Hotel hotel("Słoneczny młyn", "Portowa 5", "15:00:00", "10:00:00", "admin123");
    Room room1("101", "Standard", 200, 2);
    Room room2("102", "Deluxe", 300, 3);
    Room room3("103", "Standard", 300, 4);
    Room room4("104", "Deluxe", 600, 5);
    Guest guest1("Jan Kowalski", "jankowalski@gmail.com", "haslo123");
    Guest guest2("Paweł Nowak", "asd@gmail.com", "asd");

    hotel.addRoom(room1);
    hotel.addRoom(room2);
    hotel.addRoom(room3);
    hotel.addRoom(room4);
    hotel.addGuest(&guest1);
    hotel.addGuest(&guest2);

    displayMainManu(&hotel);

    return 0;
}

// // usunac
// string checkInTime = hotel.getCheckInTime();
//     string checkOutTime = hotel.getCheckOutTime();
//     string from = "2025-01-02";
//     string to = "2025-01-06";
//     time_t startDate = convertDate(from + string(" ") + checkInTime);
//     time_t endDate = convertDate(to + string(" ") + checkOutTime);

//     auto availableRooms = hotel.getAvailableRooms(startDate, endDate, 2);
//     cout << "dostepne: ";
//     for (auto room : availableRooms) {
//         cout << room->getRoomNumber() << " ";
//     }
//     cout << "\n";

//     if (!availableRooms.empty()) {
//         auto chosenRoom = availableRooms[0];
//         auto reservation1 = new Reservation(&guest1, startDate, endDate, chosenRoom);
//         guest1.addReservation(reservation1);
//         chosenRoom->bookRoom(startDate, endDate);

//         // cout << "Rezerwacja pomyślna!\n";
//     }
//     hotel.displayReservations();

//     availableRooms = hotel.getAvailableRooms(startDate, endDate, 2);
//     cout << "dostepne: ";
//     for (auto room : availableRooms) {
//         cout << room->getRoomNumber() << " ";
//     }
//     cout << "\n";

//     if (!availableRooms.empty()) {
//         auto chosenRoom = availableRooms[0];
//         auto reservation2 = new Reservation(&guest1, startDate, endDate, chosenRoom);
//         guest1.addReservation(reservation2);
//         chosenRoom->bookRoom(startDate, endDate);
//     }

//     availableRooms = hotel.getAvailableRooms(startDate, endDate, 2);
//     cout << "dostepne: ";
//     for (auto room : availableRooms) {
//         cout << room->getRoomNumber() << " ";
//     }
//     cout << "\n";
//     if (!availableRooms.empty()) {
//         auto chosenRoom = availableRooms[0];
//         auto reservation3 = new Reservation(&guest2, startDate, endDate, chosenRoom);
//         guest2.addReservation(reservation3);
//         chosenRoom->bookRoom(startDate, endDate);
//     }

//     hotel.displayReservations();
