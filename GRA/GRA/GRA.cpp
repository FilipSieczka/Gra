#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <sstream>

enum class PoziomTrudnosci { Latwy, Sredni, Trudny };

struct Wynik {
    std::string nick;
    std::string poziom;
    sf::Time czas;
    std::string finalnyWynik;
};

struct GameState {
    int poziom;
    std::string nick1;
    std::string nick2;
    sf::Vector2f pilkaPos;
    sf::Vector2f pilkaPredkosc;
    sf::Vector2f dolnaPos;
    sf::Vector2f gornaPos;
    int punkty1;
    int sety1;
    int punkty2;
    int sety2;
    float czasRozgrywki;
};

class SaveManager {
public:
    static bool zapisz(const std::string& filename, const GameState& state) {
        std::ofstream file(filename);
        if (!file) return false;
        file << state.poziom << "\n"
            << state.nick1 << "\n"
            << state.nick2 << "\n"
            << state.pilkaPos.x << " " << state.pilkaPos.y << "\n"
            << state.pilkaPredkosc.x << " " << state.pilkaPredkosc.y << "\n"
            << state.dolnaPos.x << " " << state.dolnaPos.y << "\n"
            << state.gornaPos.x << " " << state.gornaPos.y << "\n"
            << state.punkty1 << " " << state.sety1 << "\n"
            << state.punkty2 << " " << state.sety2 << "\n"
            << state.czasRozgrywki << "\n";
        return true;
    }

    static bool wczytaj(const std::string& filename, GameState& state) {
        std::ifstream file(filename);
        if (!file) return false;
        file >> state.poziom;
        file >> state.nick1 >> state.nick2;
        file >> state.pilkaPos.x >> state.pilkaPos.y;
        file >> state.pilkaPredkosc.x >> state.pilkaPredkosc.y;
        file >> state.dolnaPos.x >> state.dolnaPos.y;
        file >> state.gornaPos.x >> state.gornaPos.y;
        file >> state.punkty1 >> state.sety1;
        file >> state.punkty2 >> state.sety2;
        file >> state.czasRozgrywki;
        return true;
    }
};

class Scoreboard {
private:
    std::vector<Wynik> wyniki;
    sf::Font czcionka;
public:
    Scoreboard() {
        if (!czcionka.loadFromFile("arial.ttf")) {
            std::cerr << "Nie mo¿na zaladowaæ czcionki dla Scoreboard." << std::endl;
        }
    }

    void dodajWynik(const Wynik& wynik) {
        wyniki.push_back(wynik);
        std::sort(wyniki.begin(), wyniki.end(), [](const Wynik& a, const Wynik& b) {
            return a.czas < b.czas;
            });
    }

    void wyswietl(sf::RenderWindow& okno) {
        okno.clear(sf::Color::Black);
        float y = 20.f;
        for (const auto& wynik : wyniki) {
            sf::Text tekst;
            tekst.setFont(czcionka);
            tekst.setCharacterSize(20);
            tekst.setFillColor(sf::Color::White);
            std::stringstream ss;
            ss << "Nick: " << wynik.nick
                << " | Poziom: " << wynik.poziom
                << " | Czas: " << static_cast<int>(wynik.czas.asSeconds()) << "s"
                << " | " << wynik.finalnyWynik;
            tekst.setString(ss.str());
            tekst.setPosition(20.f, y);
            y += 30.f;
            okno.draw(tekst);
        }
        okno.display();
    }
};

static Scoreboard scoreboardManager;

class NickInput {
public:
    static std::pair<std::string, std::string> pobierzNicki() {
        std::string nick1 = wprowadzNick("Podaj nick dla Gracza 1");
        std::string nick2 = wprowadzNick("Podaj nick dla Gracza 2");
        if (nick1.size() > 12) nick1 = nick1.substr(0, 12);
        if (nick2.size() > 12) nick2 = nick2.substr(0, 12);
        return { nick1, nick2 };
    }
private:
    static std::string wprowadzNick(const std::string& prompt) {
        sf::RenderWindow window(sf::VideoMode(500, 200), "Wprowadz Nick", sf::Style::Titlebar | sf::Style::Close);
        sf::Font czcionka;
        if (!czcionka.loadFromFile("arial.ttf")) return "";

        sf::Text promptText(prompt + ": ", czcionka, 24);
        promptText.setFillColor(sf::Color::White);
        promptText.setPosition(20, 50);

        sf::Text inputText("", czcionka, 24);
        inputText.setFillColor(sf::Color::Yellow);
        inputText.setPosition(20, 100);

        std::string nick;
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == '\b') {
                        if (!nick.empty()) nick.pop_back();
                    }
                    else if (event.text.unicode == '\r') {
                        window.close();
                        return nick;
                    }
                    else if (nick.size() < 12 && event.text.unicode < 128) {
                        nick += static_cast<char>(event.text.unicode);
                    }
                    inputText.setString(nick);
                }
                else if (event.type == sf::Event::Closed) {
                    window.close();
                    return nick;
                }
            }
            window.clear(sf::Color::Black);
            window.draw(promptText);
            window.draw(inputText);
            window.display();
        }
        return nick;
    }
};

class ZarzadzanieZamknieciem {
private:
    bool widocznyKomunikatWyjscia;
    sf::Font czcionka;
    sf::Text komunikatWyjscia;
public:
    ZarzadzanieZamknieciem()
        : widocznyKomunikatWyjscia(false) {
        if (!czcionka.loadFromFile("arial.ttf")) exit(-1);
        komunikatWyjscia.setFont(czcionka);
        komunikatWyjscia.setString("Czy na pewno chcesz wyjsc? (T/N) \n Z: Zapisz gre");
        komunikatWyjscia.setCharacterSize(24);
        komunikatWyjscia.setFillColor(sf::Color::White);
        komunikatWyjscia.setPosition(200, 250);
    }
    bool czyWidocznyKomunikat() const { return widocznyKomunikatWyjscia; }
    void ustawWidocznoscKomunikatu(bool widocznosc) { widocznyKomunikatWyjscia = widocznosc; }
    void obslugaEventow(sf::RenderWindow& okno, const sf::Event& event, bool& graWstrzymana) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) { widocznyKomunikatWyjscia = true; graWstrzymana = true; }
            if (widocznyKomunikatWyjscia) {
                if (event.key.code == sf::Keyboard::T) { okno.close(); }
                else if (event.key.code == sf::Keyboard::N) { widocznyKomunikatWyjscia = false; graWstrzymana = false; }
            }
        }
    }
    void rysuj(sf::RenderWindow& okno) { if (widocznyKomunikatWyjscia) okno.draw(komunikatWyjscia); }
};

class Plansza {
private:
    sf::RectangleShape plansza;
    sf::RectangleShape liniaSrodkowa;
public:
    Plansza(const sf::Vector2u& rozmiarOkna) {
        plansza.setSize(sf::Vector2f(rozmiarOkna.x * 0.9f, rozmiarOkna.y * 0.8f));
        plansza.setFillColor(sf::Color(147, 197, 114));
        plansza.setPosition((rozmiarOkna.x - plansza.getSize().x) / 2, rozmiarOkna.y * 0.1f);
        liniaSrodkowa.setSize(sf::Vector2f(plansza.getSize().x, 5));
        liniaSrodkowa.setFillColor(sf::Color::White);
        liniaSrodkowa.setPosition(plansza.getPosition().x, plansza.getPosition().y + plansza.getSize().y / 2);
    }
    sf::FloatRect getBounds() const { return plansza.getGlobalBounds(); }
    float getGornaSciana() const { return plansza.getPosition().y; }
    float getDolnaSciana() const { return plansza.getPosition().y + plansza.getSize().y; }
    void rysuj(sf::RenderWindow& okno) { okno.draw(plansza); okno.draw(liniaSrodkowa); }
};

class Pilka {
private:
    sf::CircleShape pilka;
    sf::Vector2f predkosc;
public:
    Pilka() {
        pilka.setRadius(10);
        pilka.setFillColor(sf::Color::White);
    }
    void reset(float speed) {
        pilka.setPosition(400, 450);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distAngle(0.0f, 360.0f);
        float angle;
        // Losuj k¹t tak, aby nie by³ on blisko 0°, 90°, 180° lub 270°
        while (true) {
            angle = distAngle(gen);
            if ((angle > 25 && angle < 70) || (angle > 115 && angle < 160) ||
                (angle > 205 && angle < 250) || (angle > 295 && angle < 340)) {
                break;
            }
        }
        float rad = angle * 3.14159265f / 180.f;
        predkosc = sf::Vector2f(speed * cos(rad), speed * sin(rad));
    }
    sf::Vector2f getPosition() const { return pilka.getPosition(); }
    void setPosition(const sf::Vector2f& pos) { pilka.setPosition(pos); }
    sf::Vector2f getPredkosc() const { return predkosc; }
    int aktualizuj(const sf::FloatRect& granicePlanszy, const sf::RectangleShape& dolnaPlatforma,
        const sf::RectangleShape& gornaPlatforma, bool graWstrzymana) {
        if (graWstrzymana) return 0;
        sf::Vector2f pozycja = pilka.getPosition();
        if (pozycja.x <= granicePlanszy.left || pozycja.x + pilka.getRadius() * 2 >= granicePlanszy.left + granicePlanszy.width)
            predkosc.x = -predkosc.x;
        if (pozycja.y <= granicePlanszy.top) return 2;
        if (pozycja.y + pilka.getRadius() * 2 >= granicePlanszy.top + granicePlanszy.height) return 1;
        if (pilka.getGlobalBounds().intersects(dolnaPlatforma.getGlobalBounds()))
            predkosc.y = -std::abs(predkosc.y);
        else if (pilka.getGlobalBounds().intersects(gornaPlatforma.getGlobalBounds()))
            predkosc.y = std::abs(predkosc.y);
        pilka.move(predkosc);
        return 0;
    }
    void rysuj(sf::RenderWindow& okno) { okno.draw(pilka); }
};

class Platforma {
private:
    sf::RectangleShape platforma;
    float predkosc;
public:
    Platforma(const sf::Vector2f& rozmiar, const sf::Vector2f& pozycja, float predkoscPoczatkowa)
        : predkosc(predkoscPoczatkowa) {
        platforma.setSize(rozmiar);
        platforma.setFillColor(sf::Color::Blue);
        platforma.setPosition(pozycja);
    }
    void setPosition(const sf::Vector2f& pos) { platforma.setPosition(pos); }
    void aktualizuj(const sf::Keyboard::Key lewo, const sf::Keyboard::Key prawo,
        const sf::FloatRect& granicePlanszy, float deltaCzas) {
        sf::Vector2f pozycja = platforma.getPosition();
        if (sf::Keyboard::isKeyPressed(lewo) && pozycja.x > granicePlanszy.left)
            platforma.move(-predkosc * 2 * deltaCzas, 0);
        if (sf::Keyboard::isKeyPressed(prawo) && pozycja.x + platforma.getSize().x < granicePlanszy.left + granicePlanszy.width)
            platforma.move(predkosc * 2 * deltaCzas, 0);
    }
    sf::RectangleShape getShape() const { return platforma; }
    void rysuj(sf::RenderWindow& okno) { okno.draw(platforma); }
};

class SystemPunktacji {
private:
    int punktyGracz1;
    int punktyGracz2;
    int setyGracz1;
    int setyGracz2;
    sf::Font czcionka;
    sf::Text wynikTekst;
    sf::Text setyTekst;
    std::string nick1;
    std::string nick2;
public:
    SystemPunktacji()
        : punktyGracz1(0), punktyGracz2(0), setyGracz1(0), setyGracz2(0) {
        if (!czcionka.loadFromFile("arial.ttf")) exit(-1);
        wynikTekst.setFont(czcionka);
        wynikTekst.setCharacterSize(24);
        wynikTekst.setFillColor(sf::Color::White);
        wynikTekst.setPosition(5, 10);
        setyTekst.setFont(czcionka);
        setyTekst.setCharacterSize(24);
        setyTekst.setFillColor(sf::Color::White);
        setyTekst.setPosition(5, 40);
        zaktualizujWynik();
    }
    void ustawNicki(const std::string& n1, const std::string& n2) {
        nick1 = n1;
        nick2 = n2;
        zaktualizujWynik();
    }
    void ustawWyniki(int p1, int s1, int p2, int s2) {
        punktyGracz1 = p1; setyGracz1 = s1;
        punktyGracz2 = p2; setyGracz2 = s2;
        zaktualizujWynik();
    }
    void dodajPunktGracz1() { punktyGracz1++; if (punktyGracz1 >= 5) { setyGracz1++; punktyGracz1 = 0; punktyGracz2 = 0; } zaktualizujWynik(); }
    void dodajPunktGracz2() { punktyGracz2++; if (punktyGracz2 >= 5) { setyGracz2++; punktyGracz1 = 0; punktyGracz2 = 0; } zaktualizujWynik(); }
    bool czyWygrana() const { return (setyGracz1 >= 2) || (setyGracz2 >= 2); }
    std::string getWygrany() const {
        if (setyGracz1 >= 2) return nick1 + " wygrywa!";
        if (setyGracz2 >= 2) return nick2 + " wygrywa!";
        return "";
    }
    std::string getZwyciezca() const {
        if (setyGracz1 >= 2) return nick1;
        if (setyGracz2 >= 2) return nick2;
        return "";
    }
    void zaktualizujWynik() {
        wynikTekst.setString(nick1 + ": " + std::to_string(punktyGracz1) + "  Sety: " + std::to_string(setyGracz1));
        setyTekst.setString(nick2 + ": " + std::to_string(punktyGracz2) + "  Sety: " + std::to_string(setyGracz2));
    }
    void rysuj(sf::RenderWindow& okno) {
        okno.draw(wynikTekst);
        okno.draw(setyTekst);
    }
    int sumaSetow() const { return setyGracz1 + setyGracz2; }
    int getPunktyGracz1() const { return punktyGracz1; }
    int getSetyGracz1() const { return setyGracz1; }
    int getPunktyGracz2() const { return punktyGracz2; }
    int getSetyGracz2() const { return setyGracz2; }
};

class Pomoc {
private:
    sf::Font czcionka;
    std::vector<sf::Text> tekstyPomocy;
    sf::RectangleShape tlo;
public:
    Pomoc() {
        if (!czcionka.loadFromFile("arial.ttf")) exit(-1);
        tlo.setSize(sf::Vector2f(600, 600));
        tlo.setFillColor(sf::Color(0, 0, 0, 200));
        tlo.setPosition(100, 100);
        std::vector<std::string> linie = {
            "Pomoc - sterowanie i informacje:", "",
            "Sterowanie platformami:", "Gracz 1 (dolna platforma):",
            "  Lewo: Strzalka w lewo", "  Prawo: Strzalka w prawo", "",
            "Gracz 2 (Gorna platforma):", "  A: Ruch w lewo", "  D: Ruch w prawo", "",
            "System Punktacji:", "  - Zyskujesz punkt, gdy pilka przelatuje przez przeciwnika.",
            "  - Pierwszy gracz, ktory zdobêdzie 5 punktow, wygrywa set.",
            "  - Gra do 2 wygranych setow.", "", "Inne Informacje:",
            "  - Nacisnij F1, aby otworzyc/pomin¹c ten ekran pomocy.",
            "  - Nacisnij ESC, aby wyjsc z gry lub ja zapisac (potwierdzenie wymagane)."
        };
        float poczatekY = 120.0f;
        for (const auto& linia : linie) {
            sf::Text tekst;
            tekst.setFont(czcionka);
            tekst.setString(linia);
            tekst.setCharacterSize(20);
            tekst.setFillColor(sf::Color::White);
            tekst.setPosition(120, poczatekY);
            tekstyPomocy.push_back(tekst);
            poczatekY += 30.0f;
        }
    }
    void rysuj(sf::RenderWindow& okno) {
        okno.draw(tlo);
        for (const auto& tekst : tekstyPomocy) okno.draw(tekst);
    }
};

class Menu {
private:
    sf::RenderWindow& okno;
    sf::Font czcionka;
    std::vector<sf::Text> opcje;
    int wybranaOpcja;
    Pomoc pomoc;
    bool pokazPomoc;
    sf::Text informacjaF1;
public:
    Menu(sf::RenderWindow& window)
        : okno(window), wybranaOpcja(0), pokazPomoc(false) {
        if (!czcionka.loadFromFile("arial.ttf")) exit(-1);
        sf::Text opcjaStart;
        opcjaStart.setFont(czcionka);
        opcjaStart.setString("Start gry");
        opcjaStart.setCharacterSize(30);
        opcjaStart.setFillColor(sf::Color::Yellow);
        opcjaStart.setPosition(300, 250);
        opcje.push_back(opcjaStart);

        sf::Text opcjaWyniki;
        opcjaWyniki.setFont(czcionka);
        opcjaWyniki.setString("Wyniki");
        opcjaWyniki.setCharacterSize(30);
        opcjaWyniki.setFillColor(sf::Color::White);
        opcjaWyniki.setPosition(300, 320);
        opcje.push_back(opcjaWyniki);

        sf::Text opcjaWczytaj;
        opcjaWczytaj.setFont(czcionka);
        opcjaWczytaj.setString("Wczytaj gre");
        opcjaWczytaj.setCharacterSize(30);
        opcjaWczytaj.setFillColor(sf::Color::White);
        opcjaWczytaj.setPosition(300, 390);
        opcje.push_back(opcjaWczytaj);

        sf::Text opcjaWyjscie;
        opcjaWyjscie.setFont(czcionka);
        opcjaWyjscie.setString("Wyjscie");
        opcjaWyjscie.setCharacterSize(30);
        opcjaWyjscie.setFillColor(sf::Color::White);
        opcjaWyjscie.setPosition(300, 460);
        opcje.push_back(opcjaWyjscie);

        informacjaF1.setFont(czcionka);
        informacjaF1.setString("Nacisnij F1, aby wyswietlic opcje");
        informacjaF1.setCharacterSize(18);
        informacjaF1.setFillColor(sf::Color::White);
        informacjaF1.setPosition(10, okno.getSize().y - 30);
    }
    int uruchomMenu() {
        while (okno.isOpen()) {
            sf::Event event;
            while (okno.pollEvent(event)) {
                if (event.type == sf::Event::Closed) { okno.close(); return -1; }
                if (event.type == sf::Event::KeyPressed) {
                    if (pokazPomoc) {
                        if (event.key.code == sf::Keyboard::F1) pokazPomoc = false;
                        continue;
                    }
                    switch (event.key.code) {
                    case sf::Keyboard::Up:
                        wybranaOpcja = (wybranaOpcja - 1 + opcje.size()) % opcje.size();
                        for (size_t i = 0; i < opcje.size(); ++i)
                            opcje[i].setFillColor(i == wybranaOpcja ? sf::Color::Yellow : sf::Color::White);
                        break;
                    case sf::Keyboard::Down:
                        wybranaOpcja = (wybranaOpcja + 1) % opcje.size();
                        for (size_t i = 0; i < opcje.size(); ++i)
                            opcje[i].setFillColor(i == wybranaOpcja ? sf::Color::Yellow : sf::Color::White);
                        break;
                    case sf::Keyboard::Enter:
                        return wybranaOpcja;
                    case sf::Keyboard::F1:
                        pokazPomoc = true; break;
                    default: break;
                    }
                }
            }
            okno.clear(sf::Color(50, 50, 50));
            for (auto& opcja : opcje) okno.draw(opcja);
            okno.draw(informacjaF1);
            if (pokazPomoc) pomoc.rysuj(okno);
            okno.display();
        }
        return -1;
    }
};

class MenuTrudnosci {
private:
    sf::RenderWindow& okno;
    sf::Font czcionka;
    std::vector<sf::Text> opcje;
    int wybranaOpcja;
    sf::Text informacja;
public:
    MenuTrudnosci(sf::RenderWindow& window)
        : okno(window), wybranaOpcja(0) {
        if (!czcionka.loadFromFile("arial.ttf")) exit(-1);
        std::vector<std::string> poziomy = { "Latwy","Sredni","Trudny" };
        float y = 300;
        for (size_t i = 0; i < poziomy.size(); ++i) {
            sf::Text opcja;
            opcja.setFont(czcionka);
            opcja.setString(poziomy[i]);
            opcja.setCharacterSize(30);
            opcja.setFillColor(i == 0 ? sf::Color::Yellow : sf::Color::White);
            opcja.setPosition(300, y);
            opcje.push_back(opcja);
            y += 50;
        }
        informacja.setFont(czcionka);
        informacja.setString("Wybierz poziom trudnosci");
        informacja.setCharacterSize(24);
        informacja.setFillColor(sf::Color::White);
        informacja.setPosition(250, 200);
    }
    PoziomTrudnosci uruchomMenuTrudnosci() {
        while (okno.isOpen()) {
            sf::Event event;
            while (okno.pollEvent(event)) {
                if (event.type == sf::Event::Closed) { okno.close(); }
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                    case sf::Keyboard::Up:
                        wybranaOpcja = (wybranaOpcja - 1 + opcje.size()) % opcje.size();
                        for (size_t i = 0; i < opcje.size(); ++i)
                            opcje[i].setFillColor(i == wybranaOpcja ? sf::Color::Yellow : sf::Color::White);
                        break;
                    case sf::Keyboard::Down:
                        wybranaOpcja = (wybranaOpcja + 1) % opcje.size();
                        for (size_t i = 0; i < opcje.size(); ++i)
                            opcje[i].setFillColor(i == wybranaOpcja ? sf::Color::Yellow : sf::Color::White);
                        break;
                    case sf::Keyboard::Enter:
                        return static_cast<PoziomTrudnosci>(wybranaOpcja);
                    default: break;
                    }
                }
            }
            okno.clear(sf::Color(50, 50, 50));
            okno.draw(informacja);
            for (auto& opcja : opcje) okno.draw(opcja);
            okno.display();
        }
        return PoziomTrudnosci::Sredni;
    }
};

class Gra {
private:
    sf::RenderWindow okno;
    sf::Clock zegar;
    sf::Clock zegarGry;
    sf::Clock roundClock;
    sf::Color kolorTla;
    ZarzadzanieZamknieciem zarzadzanieZamknieciem;
    Plansza plansza;
    Pilka pilka;
    Platforma dolnaPlatforma;
    Platforma gornaPlatforma;
    SystemPunktacji systemPunktacji;
    Pomoc pomoc;
    bool graWstrzymana;
    bool pokazPomoc;
    sf::Font fontPowrot;
    sf::Text infoPowrot;
    sf::Text tekstCzasu;
    PoziomTrudnosci poziom;
    float baseSpeed;
    std::string nickGracz1;
    std::string nickGracz2;
    sf::Time czasWczytany;

    void ustawPlatformy() {
        float minimalnaOdleglosc = 15.0f;
        sf::Vector2f pozycjaDolna(
            (plansza.getBounds().width - dolnaPlatforma.getShape().getSize().x) / 2 + plansza.getBounds().left,
            plansza.getDolnaSciana() - minimalnaOdleglosc - dolnaPlatforma.getShape().getSize().y
        );
        dolnaPlatforma.setPosition(pozycjaDolna);
        sf::Vector2f pozycjaGorna(
            (plansza.getBounds().width - gornaPlatforma.getShape().getSize().x) / 2 + plansza.getBounds().left,
            plansza.getGornaSciana() + minimalnaOdleglosc
        );
        gornaPlatforma.setPosition(pozycjaGorna);
    }

    void inicjujTekstCzasu() {
        if (!fontPowrot.loadFromFile("arial.ttf")) exit(-1);
        tekstCzasu.setFont(fontPowrot);
        tekstCzasu.setCharacterSize(24);
        tekstCzasu.setFillColor(sf::Color::White);
        tekstCzasu.setPosition(okno.getSize().x / 2 - 50, 10);
    }

    void odliczanie(int sekundy) {
        sf::Font font;
        if (!font.loadFromFile("arial.ttf")) exit(-1);
        sf::Text countdownText;
        countdownText.setFont(font);
        countdownText.setCharacterSize(100);
        countdownText.setFillColor(sf::Color::White);
        countdownText.setPosition(okno.getSize().x / 2 - 50, okno.getSize().y / 2 - 50);
        sf::Clock clock;
        int currentSecond = sekundy;
        while (currentSecond > 0) {
            float elapsed = clock.getElapsedTime().asSeconds();
            if (elapsed >= 1.0f) { currentSecond--; clock.restart(); }
            countdownText.setString(std::to_string(currentSecond));
            sf::Time czasGry = zegarGry.getElapsedTime() + czasWczytany;
            int minuty = static_cast<int>(czasGry.asSeconds()) / 60;
            int sekundy = static_cast<int>(czasGry.asSeconds()) % 60;
            tekstCzasu.setString(std::to_string(minuty) + ":" + (sekundy < 10 ? "0" : "") + std::to_string(sekundy));
            okno.clear(kolorTla);
            plansza.rysuj(okno);
            dolnaPlatforma.rysuj(okno);
            gornaPlatforma.rysuj(okno);
            systemPunktacji.rysuj(okno);
            okno.draw(tekstCzasu);
            okno.draw(countdownText);
            okno.display();
        }
        sf::Event event;
        while (okno.pollEvent(event)) {}
        roundClock.restart();
    }

    void obslugaZdarzenOkna() {
        sf::Event event;
        while (okno.pollEvent(event)) {
            if (event.type == sf::Event::Closed) okno.close();
            zarzadzanieZamknieciem.obslugaEventow(okno, event, graWstrzymana);
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::F1) { pokazPomoc = !pokazPomoc; graWstrzymana = pokazPomoc; }
                if (event.key.code == sf::Keyboard::M) { okno.close(); }
                if (event.key.code == sf::Keyboard::Z && zarzadzanieZamknieciem.czyWidocznyKomunikat()) {
                    zapiszStan();
                    sf::Font font;
                    if (font.loadFromFile("arial.ttf")) {
                        sf::Text komunikat;
                        komunikat.setFont(font);
                        komunikat.setCharacterSize(24);
                        komunikat.setFillColor(sf::Color::Green);
                        komunikat.setString("Gra zostala zapisana");
                        komunikat.setPosition(okno.getSize().x / 2 - 100, okno.getSize().y / 2);
                        sf::Clock clock;
                        while (clock.getElapsedTime().asSeconds() < 2.0f) {
                            okno.clear(sf::Color::Black);
                            okno.draw(komunikat);
                            okno.display();
                        }
                    }
                }
            }
        }
    }

    void rysuj() {
        if (pokazPomoc) {
            okno.clear(kolorTla);
            plansza.rysuj(okno);
            pilka.rysuj(okno);
            dolnaPlatforma.rysuj(okno);
            gornaPlatforma.rysuj(okno);
            systemPunktacji.rysuj(okno);
            pomoc.rysuj(okno);
            okno.draw(infoPowrot);
        }
        else if (graWstrzymana) {
            okno.clear(sf::Color(128, 128, 128));
            zarzadzanieZamknieciem.rysuj(okno);
        }
        else {
            okno.clear(kolorTla);
            plansza.rysuj(okno);
            pilka.rysuj(okno);
            dolnaPlatforma.rysuj(okno);
            gornaPlatforma.rysuj(okno);
            systemPunktacji.rysuj(okno);
            okno.draw(infoPowrot);
            okno.draw(tekstCzasu);
        }
        okno.display();
    }

    void wyswietlWygrana(const std::string& message) {
        sf::Font font;
        if (!font.loadFromFile("arial.ttf")) return;
        sf::Text winText;
        winText.setFont(font);
        winText.setCharacterSize(50);
        winText.setFillColor(sf::Color::Red);
        winText.setString(message);
        sf::FloatRect textRect = winText.getLocalBounds();
        winText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        winText.setPosition(okno.getSize().x / 2.0f, okno.getSize().y / 2.0f);
        sf::Clock clock;
        while (clock.getElapsedTime().asSeconds() < 3.0f) {
            obslugaZdarzenOkna();
            okno.clear(kolorTla);
            plansza.rysuj(okno);
            okno.draw(winText);
            okno.display();
        }
    }

    void zapiszStan() {
        GameState stan;
        stan.poziom = static_cast<int>(poziom);
        stan.nick1 = nickGracz1;
        stan.nick2 = nickGracz2;
        stan.pilkaPos = pilka.getPosition();
        stan.pilkaPredkosc = pilka.getPredkosc();
        stan.dolnaPos = dolnaPlatforma.getShape().getPosition();
        stan.gornaPos = gornaPlatforma.getShape().getPosition();
        stan.punkty1 = systemPunktacji.getPunktyGracz1();
        stan.sety1 = systemPunktacji.getSetyGracz1();
        stan.punkty2 = systemPunktacji.getPunktyGracz2();
        stan.sety2 = systemPunktacji.getSetyGracz2();
        // Sumujemy czas bie¿¹cy z czasem wczytanym, aby uzyskaæ ³¹czny czas rozgrywki
        stan.czasRozgrywki = zegarGry.getElapsedTime().asSeconds() + czasWczytany.asSeconds();
        SaveManager::zapisz("savegame.txt", stan);
    }

    void aktualizuj() {
        float deltaCzas = zegar.restart().asSeconds();
        int punkt = pilka.aktualizuj(plansza.getBounds(), dolnaPlatforma.getShape(),
            gornaPlatforma.getShape(), graWstrzymana);
        if (punkt == 1) {
            systemPunktacji.dodajPunktGracz1();
            ustawPlatformy();
            pilka.reset(baseSpeed);
            odliczanie(3);
        }
        else if (punkt == 2) {
            systemPunktacji.dodajPunktGracz2();
            ustawPlatformy();
            pilka.reset(baseSpeed);
            odliczanie(3);
        }
        if (roundClock.getElapsedTime().asSeconds() > 0.1f) {
            dolnaPlatforma.aktualizuj(sf::Keyboard::Left, sf::Keyboard::Right, plansza.getBounds(), deltaCzas);
            gornaPlatforma.aktualizuj(sf::Keyboard::A, sf::Keyboard::D, plansza.getBounds(), deltaCzas);
        }
        sf::Time czasGry = zegarGry.getElapsedTime() + czasWczytany;
        int minuty = static_cast<int>(czasGry.asSeconds()) / 60;
        int sekundy = static_cast<int>(czasGry.asSeconds()) % 60;
        tekstCzasu.setString(std::to_string(minuty) + ":" + (sekundy < 10 ? "0" : "") + std::to_string(sekundy));
    }

public:
    Gra(PoziomTrudnosci p, const std::string& nick1, const std::string& nick2)
        : okno(sf::VideoMode(800, 900), "Tenis 2D"),
        kolorTla(128, 128, 128),
        plansza(okno.getSize()),
        pilka(),
        dolnaPlatforma(sf::Vector2f(100, 10), sf::Vector2f(0, 0), 300.0f),
        gornaPlatforma(sf::Vector2f(100, 10), sf::Vector2f(0, 0), 300.0f),
        graWstrzymana(false),
        pokazPomoc(false),
        poziom(p),
        nickGracz1(nick1),
        nickGracz2(nick2),
        czasWczytany(sf::Time::Zero)
    {
        okno.setFramerateLimit(60);
        if (!fontPowrot.loadFromFile("arial.ttf")) exit(-1);
        infoPowrot.setFont(fontPowrot);
        infoPowrot.setString("Nacisnij M, aby wrocic do menu");
        infoPowrot.setCharacterSize(18);
        infoPowrot.setFillColor(sf::Color::White);
        infoPowrot.setPosition(10, okno.getSize().y - 50);
        ustawPlatformy();
        switch (poziom) {
        case PoziomTrudnosci::Latwy: baseSpeed = 6.0f; break;
        case PoziomTrudnosci::Sredni: baseSpeed = 8.0f; break;
        case PoziomTrudnosci::Trudny: baseSpeed = 10.0f; break;
        }
        pilka.reset(baseSpeed);
        zegarGry.restart();
        inicjujTekstCzasu();
        systemPunktacji.ustawNicki(nickGracz1, nickGracz2);
        odliczanie(3);
        roundClock.restart();
    }

    Gra(const GameState& stan)
        : okno(sf::VideoMode(800, 900), "Tenis 2D"),
        kolorTla(128, 128, 128),
        plansza(sf::Vector2u(800, 900)),
        pilka(),
        dolnaPlatforma(sf::Vector2f(100, 10), sf::Vector2f(0, 0), 300.0f),
        gornaPlatforma(sf::Vector2f(100, 10), sf::Vector2f(0, 0), 300.0f),
        graWstrzymana(false),
        pokazPomoc(false),
        poziom(static_cast<PoziomTrudnosci>(stan.poziom)),
        nickGracz1(stan.nick1),
        nickGracz2(stan.nick2),
        czasWczytany(sf::seconds(stan.czasRozgrywki))
    {
        okno.setFramerateLimit(60);
        if (!fontPowrot.loadFromFile("arial.ttf")) exit(-1);
        infoPowrot.setFont(fontPowrot);
        infoPowrot.setString("Nacisnij M, aby wrocic do menu");
        infoPowrot.setCharacterSize(18);
        infoPowrot.setFillColor(sf::Color::White);
        infoPowrot.setPosition(10, okno.getSize().y - 50);
        ustawPlatformy();
        switch (poziom) {
        case PoziomTrudnosci::Latwy: baseSpeed = 6.0f; break;
        case PoziomTrudnosci::Sredni: baseSpeed = 8.0f; break;
        case PoziomTrudnosci::Trudny: baseSpeed = 10.0f; break;
        }
        pilka.reset(baseSpeed);
        pilka.setPosition(stan.pilkaPos);
        dolnaPlatforma.setPosition(stan.dolnaPos);
        gornaPlatforma.setPosition(stan.gornaPos);
        zegarGry.restart();
        inicjujTekstCzasu();
        systemPunktacji.ustawNicki(nickGracz1, nickGracz2);
        systemPunktacji.ustawWyniki(stan.punkty1, stan.sety1, stan.punkty2, stan.sety2);
        odliczanie(3);
        roundClock.restart();
    }

    void uruchom() {
        while (okno.isOpen()) {
            obslugaZdarzenOkna();
            if (!graWstrzymana && !pokazPomoc) { aktualizuj(); }
            if (systemPunktacji.czyWygrana()) {
                std::string winMessage = systemPunktacji.getWygrany();
                winMessage += " na poziomie ";
                switch (poziom) {
                case PoziomTrudnosci::Latwy: winMessage += "Latwy!"; break;
                case PoziomTrudnosci::Sredni: winMessage += "Sredni!"; break;
                case PoziomTrudnosci::Trudny: winMessage += "Trudny!"; break;
                }
                wyswietlWygrana(winMessage);

                Wynik wynik;
                wynik.nick = systemPunktacji.getZwyciezca();
                wynik.poziom = (poziom == PoziomTrudnosci::Latwy) ? "Latwy" :
                    (poziom == PoziomTrudnosci::Sredni) ? "Sredni" : "Trudny";
                wynik.czas = zegarGry.getElapsedTime() + czasWczytany;
                std::stringstream ss;
                ss << "Sety - " << nickGracz1 << ": " << systemPunktacji.sumaSetow();
                wynik.finalnyWynik = ss.str();
                scoreboardManager.dodajWynik(wynik);

                okno.close();
            }
            rysuj();
        }
    }
};

class Aplikacja {
public:
    void run() {
        while (true) {
            sf::RenderWindow okno(sf::VideoMode(800, 900), "Tenis 2D");
            okno.setFramerateLimit(60);
            Menu menu(okno);
            int wybor = menu.uruchomMenu();
            if (wybor == -1) break;
            if (wybor == 0) { // Start gry
                MenuTrudnosci menuTrudnosci(okno);
                PoziomTrudnosci wybranyPoziom = menuTrudnosci.uruchomMenuTrudnosci();
                auto nicki = NickInput::pobierzNicki();
                Gra gra(wybranyPoziom, nicki.first, nicki.second);
                gra.uruchom();
            }
            else if (wybor == 1) { // Wyniki
                sf::RenderWindow winokno(sf::VideoMode(600, 400), "Tablica wynikow");
                while (winokno.isOpen()) {
                    sf::Event event;
                    while (winokno.pollEvent(event)) {
                        if (event.type == sf::Event::Closed) winokno.close();
                    }
                    scoreboardManager.wyswietl(winokno);
                }
            }
            else if (wybor == 2) { // Wczytaj grê
                GameState stan;
                if (SaveManager::wczytaj("savegame.txt", stan)) {
                    Gra gra(stan);
                    gra.uruchom();
                }
                else {
                    std::cout << "Nie mozna wczytac stanu gry." << std::endl;
                }
            }
            else { 
                break;
            }
        }
    }
};

int main() {
    Aplikacja app;
    app.run();
    return 0;
}
