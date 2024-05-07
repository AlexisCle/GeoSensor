#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <iterator>
#include <string>
#include <cmath>
#include <RadioHead.h>

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

bool process_done_sendLoRa = true;

void send_LoRa()
{
  if (process_done_sendLoRa)
  {  
    byte tx_buf[sizeof(LoRaData)]={0};
    memcpy(tx_buf, &LoRaData, sizeof(LoRaData));
    

    if (manager->sendtoWait(tx_buf, sizeof(tx_buf), NODEADRESS1) == RH_ROUTER_ERROR_NONE)
    {
        uint8_t len = sizeof(buf); 
        uint8_t from;

    if (manager->recvfromAckTimeout(buf, &len,2000, &from))
        {
        Serial.print("got OK : 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.println((char*)buf);  
        }
            else
        {
        Serial.println("No reply, any Nodes running?");
        }
    }
        else
        Serial.println("sendtoWait failed");
    }
}

// Fonction pour générer une liste "pochon" de longueur n
std::vector<int> gen_pochon(int n) {
    std::vector<int> res;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 100);

    int i = 1;
    while (res.size() < n) {
        res.push_back(i);
        i = distrib(gen) + std::accumulate(res.begin(), res.end(), 0);
    }
    return res;
}

// Fonction pour générer une clé privée
struct ClePrivee {
    std::vector<int> pochon;
    int M;
    int W;
    std::vector<int> sigma;
};

ClePrivee gen_cle_privee(int n) {
    ClePrivee cle_privee;
    cle_privee.pochon = gen_pochon(n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib_m(1, 100);
    std::uniform_int_distribution<> distrib_w(1, std::accumulate(cle_privee.pochon.begin(), cle_privee.pochon.end(), 0) - 1);

    cle_privee.M = std::accumulate(cle_privee.pochon.begin(), cle_privee.pochon.end(), 0) + distrib_m(gen);

    do {
        cle_privee.W = distrib_w(gen);
    } while (std::gcd(cle_privee.W, cle_privee.M) != 1);

    // Génération d'une permutation aléatoire
    std::vector<int> sigma(n);
    std::iota(sigma.begin(), sigma.end(), 0);
    std::shuffle(sigma.begin(), sigma.end(), gen);
    cle_privee.sigma = sigma;

    return cle_privee;
}

// Fonction pour permuter un vecteur selon un ordre donné
std::vector<int> permutation_tableau(const std::vector<int>& tableau, const std::vector<int>& sigma) {
    std::vector<int> result(sigma.size());
    for (size_t i = 0; i < sigma.size(); ++i) {
        result[sigma[i]] = tableau[i];
    }
    return result;
}

// Fonction pour générer une clé publique à partir d'une clé privée
std::vector<int> gen_cle_publique(const ClePrivee& cle_privee) {
    std::vector<int> cle_publique;
    auto pochon_permute = permutation_tableau(cle_privee.pochon, cle_privee.sigma);

    for (const auto& elem : pochon_permute) {
        cle_publique.push_back((cle_privee.W * elem) % cle_privee.M);
    }

    return cle_publique;
}

// Fonction pour résoudre un "pochon"
std::vector<int> solve_pochon(const std::vector<int>& pochon, int c) {
    std::vector<int> res;
    int cnt = pochon.size();

    while (c >= 0 && cnt > 0) {
        if (c >= pochon[cnt - 1]) {
            res.push_back(1);
            c -= pochon[cnt - 1];
        } else {
            res.push_back(0);
        }
        cnt--;
    }
    std::reverse(res.begin(), res.end()); // Pour obtenir le même ordre que dans Python
    return res;
}

// Fonction pour chiffrer un message
int chiffrer(const std::vector<int>& message, const std::vector<int>& cle_publique) {
    int res = 0;
    for (size_t i = 0; i < message.size(); ++i) {
        res += message[i] * cle_publique[i];
    }
    return res;
}

// Fonction pour inverser une permutation
std::vector<int> inverse_permutation_tableau(const std::vector<int>& tableau, const std::vector<int>& sigma) {
    std::vector<int> result(sigma.size());
    for (size_t i = 0; i < sigma.size(); ++i) {
        result[sigma[i]] = tableau[i];
    }
    return result;
}

// Fonction pour déchiffrer un message chiffré
std::vector<int> dechiffrer(int message_chiffre, const ClePrivee& cle_privee) {
    int inverse_mod_w = std::pow(cle_privee.W, -1, cle_privee.M);
    int d = (inverse_mod_w * message_chiffre) % cle_privee.M;

    auto bits_message_original = solve_pochon(cle_privee.pochon, d);
    auto message_original = inverse_permutation_tableau(bits_message_original, cle_privee.sigma);

    return message_original;
}

// Fonction pour convertir des bits en texte
std::string binary_to_text(const std::vector<int>& binary) {
    std::string text;
    for (size_t i = 0; i < binary.size(); i += 8) {
        int char_value = 0;
        for (size_t j = 0; j < 8 && i + j < binary.size(); ++j) {
            char_value = (char_value << 1) | binary[i + j];
        }
        text += static_cast<char>(char_value);
    }
    return text;
}

// Fonction pour tester le chiffrement/déchiffrement
void test_chiffrement_dechiffrement(const std::vector<int>& message) {
    int n = message.size();
    ClePrivee cle_privee = gen_cle_privee(n);
    std::vector<int> cle_publique = gen_cle_publique(cle_privee);

    int mc = chiffrer(message, cle_publique);
    auto m = dechiffrer(mc, cle_privee);

    std::cout << "Message chiffré: " << mc << std::endl;
    std::cout << "Message déchiffré: " << binary_to_text(m) << std::endl;
}

int main() {
    std::vector<int> message = {1, 0, 1, 1, 0, 1, 0, 1};
    test_chiffrement_dechiffrement(message);
    return 0;
}