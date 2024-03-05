/* Autor: Marcin Łobacz
 * Temat: std::jthread
 *
 * Klasa std::jthread w pełni wspiera API std::thread. Tym samym możliwe jest zastąpienie thread przez jthread bez zmian
 * w kodzie. Cechą wyróżniającą jthread jest brak potrzeby wywoływania join() lub detach() na wątku.
 * Jeżeli w destruktorze std::thread std::thread::joinable zwraca true, wywoływane jest std::terminate - program zostaje
 * zakończony. Jest to z oczywistych względów działanie nieporządane. Wątek t1 nie jest joinable, jeżeli zostanie
 * wykonane t1.join() lub t1.detach(). Dlatego musimy pamiętać o tej sytuacji programując. W programach o dużym stopniu
 * skomplikowania zarządzanie wątkami staje się trudne. Używanie jthread ułatwia te zarządzanie.
 *
 * Kompilacja:
 *   g++ --std=c++20 ZPR_zad_dodatkowe_MLobacz.cpp -o main
*/
#include <iostream>
#include <thread>
#include <stop_token>

using namespace std;

// Funkcja symulująca jakąś operację
void incr(uint32_t boundary){
    uint32_t val = 0;
    while(val < boundary){
        ++val;
    }

    cout << "Wynik incr():" << val << endl;
}


/*
 * W przykladThread() mamy następującą stytuację:
 *  void przykladThread(){
 *      std::thread t{incr, std::numeric_limits<uint32_t>::max()/4};
 *
 *      // Wywoływany jest jakiś kod, który może zawierać błedy lub wychodzić z funkcji. Tym samym program może nie
 *      // dojść do t.join() poniżej. Brak wywołania t.join(), kończy się wywołaniem w destruktorze std::terminate.
 *      // Dlatego musimy pamiętać o wszystkich przypadkach, gdzie musimy wywołać t.join(). O co pisząc skomplikowany
 *      // kod jest trudno.
 *
 *      t.join(); // Bieżący wątek czeka na zakończenie wątku t.
 *  }
 */
void przykladThread(){
    const uint32_t end = numeric_limits<uint32_t>::max()/32;
    thread t{incr, numeric_limits<uint32_t>::max()/4};

    for(uint32_t i = 0; i<=numeric_limits<uint32_t>::max()/24; ++i){
        if(i == end){
            // Zakomentowanie poniższej linijki i ustawienie odpowiedniej wartości end powoduje zakończenie programu
            t.join();
            cout << "Wejscie do if()" << endl;
            return;
        }
    }
    cout << "Poza forem" << endl;

    t.join();
}

/*
 * Jthread uprasza pisanie kodu. Nie musimy martwić się, że zapomnimy o t.join().
 *
 * przykladJThread() implementuje to samo co przykladThread() wykorzystując std::jthread. Nie musimy martwić sie o join()
 */
void przykladJThread(){
    const uint32_t end = numeric_limits<uint32_t>::max()/32;
    jthread t{incr, numeric_limits<uint32_t>::max()/4};

    for(int i = 0; i<=numeric_limits<uint32_t>::max()/24; ++i){
        if(i == end){
            cout << "Wejscie" << endl;
            return;
        }
    }
    cout << "Poza forem" << endl;
}

/*
 * Funkcja worker symuluje zadanie działające do momentu otrzymania żądania zatrzymania. Brany jest stop_token jako
 * argument, który używany jest do sprawdzenia, czy nastąpiło żadanie zatrzymiania.
 */
void worker(stop_token stoken) {
    while (!stoken.stop_requested()) {
        std::cout << "Działanie..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Zatrzymanie." << std::endl;
}


/*
 * Wraz z std::jthread zostało dodana funkcjonalność sygnalizowania wątku do zatrzymania wykonywania. Funkcjonalność tą
 * implementuje std::stop_token. Stop_token cechuje:
 *      - Lekkie i efektywne anulowanie asynchronicznych operacji.
 *      - Jest thead-safe, nie musimy tworzyć flag anulujących. Umożliwia pisanie ładniejszego kodu.
 *      - Skalowalność i elastyczność. Łatwość w pisaniu złożonych programów.
 *
 * Korzystając ze zwykłego thread z stop_token korzystamy w poniższy sposób.
 */
void przykladStopTokenThread() {
    const stop_source stopSource;

    // Pozyskanie stop_token z stop_source
    const stop_token stopToken = stopSource.get_token();

    /*
      * Funkcja lambda symuluje zadanie działające do momentu otrzymania żądania zatrzymania. Brany jest stop_token jako
      * argument, który używany jest do sprawdzenia, czy nastąpiło żadanie zatrzymiania.
    */
    thread t([stoken = stopToken]() {
        uint32_t value = 0;
       cout << "Working..." << endl;
        while (!stoken.stop_requested()) {
            ++value;
            cout << value;
            this_thread::sleep_for(chrono::seconds(1));
        }
        cout << "Stopped." << endl;
    });

    this_thread::sleep_for(chrono::seconds(4));

    // Żądanie zatrzymania wątku jest to miejsce krytyczne. Brak żądania powoduje działanie programu w nieskończoność.
    // Mamy tutaj dodatkowy czynnik, o którym musimy pamiętać. Co nie ułatwia programowania.
    stopSource.request_stop(); // Zakomentować, jeżeli chcemy przetestować

    t.join();
}


/*
 * Destruktor jthread woła request_stop() i następnie join(). Nie mamy, więc sytuacji jaka miała miejsce w
 * przykladStopTokenThread(). Wątek dostaje żądenie zatrzymania. Praca z stop_token zostaja znacznie ułatwiona.
 */
void przykladStopToken(){
    /*
     * Funkcja lambda symuluje zadanie działające do momentu otrzymania żądania zatrzymania. Brany jest stop_token jako
     * argument, który używany jest do sprawdzenia, czy nastąpiło żadanie zatrzymiania.
     */
    jthread t([](stop_token stoken){
        uint32_t value = 0;
        cout << "Working..." << endl;
        while (!stoken.stop_requested()) {
            ++value;
            this_thread::sleep_for(chrono::seconds(1));
        }
        cout << "Stopped. Value: " << value << endl;
    });

    // Wait for 5 seconds
    this_thread::sleep_for(chrono::seconds(4));
}



int main(){
    cout << "Przed uruchomieniem przykladThread:" << endl;
    przykladThread();
    cout << "Po uruchomieniu przykladThread \n" << endl;

    cout << "Przed uruchomieniem przykladJThread:" << endl;
    przykladJThread();
    cout << "Po uruchomieniu przykladJThread \n" << endl;

    cout << "Przyklad przykladStopTokenThread:" << endl;
    przykladStopTokenThread();
    cout << endl;

    cout << "Przyklad przykladStopToken:" << endl;
    przykladStopToken();
    return 0;
}

/*
 * Źródła:
 *     - https://www.youtube.com/watch?v=elFil2VhlH8
 *     - https://en.cppreference.com/w/cpp/thread/jthread
 *     - https://en.cppreference.com/w/cpp/thread/stop_token
 *     - https://www.geeksforgeeks.org/cpp-20-stop_token-header/
 */

