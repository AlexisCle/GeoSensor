#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <iterator>
#include <string>
#include <cmath>
#include <RadioHead.h>
#include <tuple>

struct dataStruct{
    uint8_t nodeId=0; //Node-Id of this Node
    double gpsLatitude=0; //GPS-Location Latitude
    double gpsLongitude=0; //GPS-Location Longitude
    uint8_t gpsHour=0; //GPS-Time Hour
    uint8_t gpsMinute=0; //GPS-Time Minute
    uint8_t gpsSecond=0; //GPS-Time Second
    float temp1 = 0;
    float temp2 = 0;
    float pressure = 0;
    float distance = 0;
}LoRaData;



void dummy_sensor_function()
{
    // Do Sensor stuff, fill the Datastruct
    LoRaData.nodeId = 1; //example
    LoRaData.gpsLatitude = 53.503215;
    LoRaData.gpsLongitude = 8.074883;
    LoRaData.gpsHour = 12;
    LoRaData.gpsMinute = 56;
    LoRaData.gpsSecond = 8;
    LoRaData.temp1 = 20.25;
    LoRaData.temp2 = 18.15;
    LoRaData.pressure = 100001;
    LoRaData.distance = 210.20;
}

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Fonction pour trouver un nombre e tel que 1 < e < phi et gcd(e, phi) = 1
int findE(int phi) {
    int e = 2;
    while (e < phi) {
        if (gcd(e, phi) == 1) {
            return e;
        }
        e++;
    }
    return -1; // Si aucun e trouvé (ce qui ne devrait pas arriver pour phi correct)
}

// Fonction pour calculer l'inverse modulaire de e modulo phi (algorithme d'Euclide étendu)
int modInverse(int e, int phi) {
    int t = 0, newT = 1;
    int r = phi, newR = e;

    while (newR != 0) {
        int quotient = r / newR;
        std::tie(t, newT) = std::make_tuple(newT, t - quotient * newT);
        std::tie(r, newR) = std::make_tuple(newR, r - quotient * newR);
    }

    if (r > 1) {
        throw std::invalid_argument("e n'a pas d'inverse modulaire.");
    }

    if (t < 0) {
        t += phi;
    }

    return t;
}

// Fonction pour calculer (base^exponent) % mod de manière efficace (exponentiation rapide modulaire)
long long modExp(long long base, long long exponent, long long mod) {
    long long result = 1;
    base = base % mod;
    while (exponent > 0) {
        if (exponent % 2 == 1) {
            result = (result * base) % mod;
        }
        exponent = exponent >> 1;
        base = (base * base) % mod;
    }
    return result;
}

// Fonction to test the encryption/decryption
void test_chiffrement_dechiffrementInt(int message) {
    // Choix de deux nombres premiers p et q supérieurs à 1000
    long long p = 1009;
    long long q = 1013;

    long long n = p * q;
    long long phi = (p - 1) * (q - 1);

    long long e = findE(phi);
    long long d = modInverse(e, phi);

    std::cout << "Clé publique: (e = " << e << ", n = " << n << ")\n";
    std::cout << "Clé privée: (d = " << d << ", n = " << n << ")\n";

    // Chiffrement
    long long encrypted = modExp(message, e, n);
    std::cout << "Message chiffré: " << encrypted << "\n";

    // Déchiffrement
    long long decrypted = modExp(encrypted, d, n);
    std::cout << "Message déchiffré: " << decrypted << "\n";
}

void test_chiffrement_dechiffrementDouble(double message) {
    // Choix de deux nombres premiers p et q supérieurs à 1000
    long long p = 19937; // Plus grand nombre premier
    long long q = 21319; // Plus grand nombre premier

    long long n = p * q;
    long long phi = (p - 1) * (q - 1);

    long long e = findE(phi);
    long long d = modInverse(e, phi);

    std::cout << "Clé publique: (e = " << e << ", n = " << n << ")\n";
    std::cout << "Clé privée: (d = " << d << ", n = " << n << ")\n";

    // Chiffrement
    long long scale = 100000; // Facteur d'échelle pour conserver la précision
    long long scaledMessage = static_cast<long long>(message * scale);
    if (scaledMessage >= n) {
        std::cerr << "Erreur: le message doit être inférieur à n.\n";
    }
    long long encrypted = modExp(scaledMessage, e, n);
    std::cout << "Message chiffré: " << encrypted << "\n";

    // Déchiffrement
    long long decrypted = modExp(encrypted, d, n);
    double decryptedMessage = static_cast<double>(decrypted) / scale;
    std::cout << "Message déchiffré: " << decryptedMessage << "\n";
}

int main() {
    std::cout << "Begin test!" << std::endl;
    dummy_sensor_function();
    std::cout << "Data collected!" << std::endl;
    std::vector<int> messageInt = {LoRaData.nodeId, LoRaData.gpsHour, LoRaData.gpsMinute, LoRaData.gpsSecond, static_cast<int>(LoRaData.pressure)};
    std::vector<double> messageDouble= {LoRaData.gpsLatitude, LoRaData.gpsLongitude, LoRaData.temp1, LoRaData.temp2, LoRaData.distance};
    std::cout << "Message: loaded" << std::endl;
    for(int elem : messageInt) {
        std::cout << "Element: " << elem << std::endl;
        test_chiffrement_dechiffrementInt(elem);
    }
    std::cout << "" << "\n";
    std::cout << "Double" << "\n";
    for(double elem : messageDouble) {
        std::cout << "Element: " << elem << std::endl;
        test_chiffrement_dechiffrementDouble(elem);
    }
    std::cout << "Test Done!" << std::endl;
    return 0;
}