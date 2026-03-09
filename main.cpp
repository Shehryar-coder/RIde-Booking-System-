#include <algorithm>
#include <cmath>
#include <fstream>//
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ============================================
//           GLOBAL CONSTANTS
// ============================================
const int MAX_LOCATIONS = 60;
const int MAX_USERS = 200;
const int MAX_DRIVERS = 200;
const int MAX_REQUESTS = 200;
const double BASE_FARE_BIKE = 1.50;
const double BASE_FARE_CAR = 2.50;
const double BASE_FARE_RICKSHAW = 2.00;
const double PER_KM_BIKE = 0.80;
const double PER_KM_CAR = 1.20;
const double PER_KM_RICKSHAW = 1.00;
const double SURGE_MED_DIST = 10.0;
const double SURGE_HIGH_DIST = 20.0;
const double SURGE_MED_MULT = 1.2;
const double SURGE_HIGH_MULT = 1.5;
const double BONUS_TIER_HIGH = 4.8;
const double BONUS_TIER_MID = 4.5;
const double BONUS_TIER_LOW = 4.0;
const double BONUS_HIGH_RATE = 0.15;
const double BONUS_MID_RATE = 0.10;
const double BONUS_LOW_RATE = 0.05;

// ============================================
//           HELPER FUNCTIONS
// ============================================
int safeStoi(const string& str, int defaultValue = 0) {
    try {
        if (str.empty()) return defaultValue;
        return stoi(str);
    } catch (...) {
        return defaultValue;
    }
}

double safeStod(const string& str, double defaultValue = 0.0) {
    try {
        if (str.empty()) return defaultValue;
        return stod(str);
    } catch (...) {
        return defaultValue;
    }
}

string toLowerCase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

string toTitleCase(const string& str) {
    if (str.empty()) return str;
    string result = str;
    result[0] = toupper(result[0]);
    for (size_t i = 1; i < result.length(); i++) {
        if (result[i - 1] == ' ')
            result[i] = toupper(result[i]);
        else
            result[i] = tolower(result[i]);
    }
    return result;
}

void printDivider() {
    cout << "====================================================\n";
}

void printSection(const string& title) {
    printDivider();
    cout << " " << title << "\n";
    printDivider();
}

// Clamp helper
template <typename T>
T clampValue(T v, T lo, T hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ============================================
//           ENUMS
// ============================================
enum class VehicleType { BIKE, CAR, RICKSHAW };

string vehicleTypeToString(VehicleType type) {
    switch (type) {
        case VehicleType::BIKE: return "Bike";
        case VehicleType::CAR: return "Car";
        case VehicleType::RICKSHAW: return "Rickshaw";
    }
    return "Unknown";
}

VehicleType stringToVehicleType(const string& str) {
    if (str == "Bike" || str == "bike" || str == "1") return VehicleType::BIKE;
    if (str == "Car" || str == "car" || str == "2") return VehicleType::CAR;
    if (str == "Rickshaw" || str == "rickshaw" || str == "3") return VehicleType::RICKSHAW;
    return VehicleType::CAR;
}

// ============================================
//           LOCATION GRAPH CLASS
// ============================================
class LocationGraph {
private:
    string locations[MAX_LOCATIONS];
    int locationCount = 0;
    double adjMatrix[MAX_LOCATIONS][MAX_LOCATIONS];

    int findIndex(const string& location) {
        string lower = toLowerCase(location);
        for (int i = 0; i < locationCount; i++) {
            if (toLowerCase(locations[i]) == lower) return i;
        }
        return -1;
    }

public:
    LocationGraph() {
        for (int i = 0; i < MAX_LOCATIONS; i++) {
            for (int j = 0; j < MAX_LOCATIONS; j++) {
                adjMatrix[i][j] = (i == j) ? 0.0 : -1.0;
            }
        }
    }

    void addLocation(const string& location) {
        if (locationCount >= MAX_LOCATIONS) return;
        if (findIndex(location) != -1) return;
        locations[locationCount++] = toTitleCase(location);
    }

    bool isValidLocation(const string& location) { return findIndex(location) != -1; }

    string getValidLocation(const string& location) {
        int idx = findIndex(location);
        if (idx == -1) return "";
        return locations[idx];
    }

    void addEdge(const string& from, const string& to, double distance) {
        addLocation(from);
        addLocation(to);
        int i = findIndex(from);
        int j = findIndex(to);
        if (i == -1 || j == -1) return;
        adjMatrix[i][j] = distance;
        adjMatrix[j][i] = distance;
    }

    double getDistance(const string& from, const string& to) {
        int s = findIndex(from);
        int t = findIndex(to);
        if (s == -1 || t == -1) return -1.0;
        if (s == t) return 0.0;

        double dist[MAX_LOCATIONS];
        bool visited[MAX_LOCATIONS];
        for (int i = 0; i < MAX_LOCATIONS; i++) {
            dist[i] = INT_MAX;
            visited[i] = false;
        }
        dist[s] = 0.0;

        for (int iter = 0; iter < locationCount; iter++) {
            int u = -1;
            double best = INT_MAX;
            for (int i = 0; i < locationCount; i++) {
                if (!visited[i] && dist[i] < best) {
                    best = dist[i];
                    u = i;
                }
            }
            if (u == -1) break;
            visited[u] = true;
            if (u == t) break;
            for (int v = 0; v < locationCount; v++) {
                if (adjMatrix[u][v] >= 0 && !visited[v]) {
                    double alt = dist[u] + adjMatrix[u][v];
                    if (alt < dist[v]) dist[v] = alt;
                }
            }
        }
        return (dist[t] == INT_MAX) ? -1.0 : dist[t];
    }

    void displayLocations() {
        cout << "\n=== Available Locations ===\n";
        for (int i = 0; i < locationCount; i++) {
            cout << "- " << locations[i] << "\n";
        }
    }

    void initializeDefaultLocations() {
        addEdge("Downtown", "Central Station", 3.5);
        addEdge("Downtown", "City Mall", 4.2);
        addEdge("Downtown", "Business District", 2.8);
        addEdge("Downtown", "Old Town", 5.0);
        addEdge("Central Station", "Airport", 18.5);
        addEdge("Central Station", "Bus Terminal", 2.3);
        addEdge("Central Station", "Train Station", 1.5);
        addEdge("Airport", "Highway Junction", 12.0);
        addEdge("City Mall", "Shopping Plaza", 6.8);
        addEdge("City Mall", "Entertainment Zone", 4.5);
        addEdge("Shopping Plaza", "Beach", 15.2);
        addEdge("Entertainment Zone", "Stadium", 3.2);
        addEdge("Old Town", "Residential Park", 7.5);
        addEdge("Residential Park", "Suburb North", 9.8);
        addEdge("Suburb North", "University", 8.3);
        addEdge("University", "Student Housing", 2.5);
        addEdge("Park", "Nature Reserve", 11.5);
        addEdge("Park", "Beach", 10.0);
        addEdge("Beach", "Marina", 4.8);
        addEdge("Marina", "Harbor", 6.2);
        addEdge("Business District", "Industrial Zone", 12.5);
        addEdge("Business District", "Tech Park", 8.7);
        addEdge("Tech Park", "Research Center", 5.3);
        addEdge("University", "Park", 9.2);
        addEdge("Stadium", "Park", 7.8);
        addEdge("Harbor", "Industrial Zone", 14.5);
        addEdge("Highway Junction", "Suburb North", 16.2);
        addEdge("Bus Terminal", "Student Housing", 11.3);
        addEdge("Train Station", "Tech Park", 13.7);
        addEdge("Nature Reserve", "Research Center", 9.5);
    }
};

// ============================================
//           FARE CALCULATOR CLASS
// ============================================
class FareCalculator {
private:
    double baseFares[3];
    double perKmRates[3];

    int idx(VehicleType t) {
        switch (t) {
            case VehicleType::BIKE: return 0;
            case VehicleType::CAR: return 1;
            case VehicleType::RICKSHAW: return 2;
        }
        return 1;
    }

public:
    FareCalculator() {
        baseFares[idx(VehicleType::BIKE)] = BASE_FARE_BIKE;
        baseFares[idx(VehicleType::CAR)] = BASE_FARE_CAR;
        baseFares[idx(VehicleType::RICKSHAW)] = BASE_FARE_RICKSHAW;
        perKmRates[idx(VehicleType::BIKE)] = PER_KM_BIKE;
        perKmRates[idx(VehicleType::CAR)] = PER_KM_CAR;
        perKmRates[idx(VehicleType::RICKSHAW)] = PER_KM_RICKSHAW;
    }

    double calculateFare(double distance, VehicleType vehicleType, double driverRating = 5.0) {
        if (distance <= 0) return baseFares[idx(vehicleType)];
        double baseFare = baseFares[idx(vehicleType)];
        double perKmRate = perKmRates[idx(vehicleType)];

        double fare = baseFare + (distance * perKmRate);
        double surgeMultiplier = 1.0;
        if (distance > SURGE_HIGH_DIST) surgeMultiplier = SURGE_HIGH_MULT;
        else if (distance > SURGE_MED_DIST) surgeMultiplier = SURGE_MED_MULT;

        fare *= surgeMultiplier;
        return round(fare * 100) / 100;
    }

    double calculateBonus(double baseFare, double driverRating) {
        double bonus = 0.0;
        if (driverRating >= BONUS_TIER_HIGH) bonus = baseFare * BONUS_HIGH_RATE;
        else if (driverRating >= BONUS_TIER_MID) bonus = baseFare * BONUS_MID_RATE;
        else if (driverRating >= BONUS_TIER_LOW) bonus = baseFare * BONUS_LOW_RATE;
        return round(bonus * 100) / 100;
    }
};

// ============================================
//           USER CLASS
// ============================================
class User {
private:
    string username;
    string password;
    string phone;
    double walletBalance;
    bool hasActiveRide;
    string activeDriverName;
    double activeRideFare;
    string activeRideOrigin;
    string activeRideDestination;
    double activeRideDistance;
    VehicleType activeRideVehicleType;

public:
    User()
        : walletBalance(0.0),
          hasActiveRide(false),
          activeRideFare(0.0),
          activeRideDistance(0.0),
          activeRideVehicleType(VehicleType::CAR) {}

    User(string uname, string pwd, string ph)
        : username(uname),
          password(pwd),
          phone(ph),
          walletBalance(0.0),
          hasActiveRide(false),
          activeRideFare(0.0),
          activeRideDistance(0.0),
          activeRideVehicleType(VehicleType::CAR) {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getPhone() const { return phone; }
    double getWalletBalance() const { return walletBalance; }
    bool getHasActiveRide() const { return hasActiveRide; }
    string getActiveDriverName() const { return activeDriverName; }
    double getActiveRideFare() const { return activeRideFare; }
    string getActiveRideOrigin() const { return activeRideOrigin; }
    string getActiveRideDestination() const { return activeRideDestination; }
    double getActiveRideDistance() const { return activeRideDistance; }
    VehicleType getActiveRideVehicleType() const { return activeRideVehicleType; }

    void addWalletBalance(double amount) { walletBalance += amount; }
    void deductWalletBalance(double amount) { walletBalance -= amount; }

    void setActiveRide(const string& driverName,
                       double fare,
                       const string& origin,
                       const string& destination,
                       double distance,
                       VehicleType vehicleType) {
        hasActiveRide = true;
        activeDriverName = driverName;
        activeRideFare = fare;
        activeRideOrigin = origin;
        activeRideDestination = destination;
        activeRideDistance = distance;
        activeRideVehicleType = vehicleType;
    }

    void clearActiveRide() {
        hasActiveRide = false;
        activeDriverName = "";
        activeRideFare = 0.0;
        activeRideOrigin = "";
        activeRideDestination = "";
        activeRideDistance = 0.0;
        activeRideVehicleType = VehicleType::CAR;
    }

    string toFileString() const {
        ostringstream oss;
        oss << username << "," << password << "," << phone << "," << walletBalance << ","
            << hasActiveRide << "," << activeDriverName << "," << activeRideFare << ","
            << activeRideOrigin << "," << activeRideDestination << "," << activeRideDistance << ","
            << static_cast<int>(activeRideVehicleType);
        return oss.str();
    }

    static User fromFileString(const string& line) {
        User u;
        stringstream ss(line);
        string item;
        vector<string> tokens;
        while (getline(ss, item, ',')) tokens.push_back(item);

        if (tokens.size() >= 4) {
            u.username = tokens[0];
            u.password = tokens[1];
            u.phone = tokens[2];
            u.walletBalance = safeStod(tokens[3], 0.0);
        }
        if (tokens.size() >= 11) {
            u.hasActiveRide = (tokens[4] == "1");
            u.activeDriverName = tokens[5];
            u.activeRideFare = safeStod(tokens[6], 0.0);
            u.activeRideOrigin = tokens[7];
            u.activeRideDestination = tokens[8];
            u.activeRideDistance = safeStod(tokens[9], 0.0);
            u.activeRideVehicleType = static_cast<VehicleType>(safeStoi(tokens[10], 1));
        }
        return u;
    }
};

// ============================================
//           DRIVER NODE CLASS (Linked List)
// ============================================
class DriverNode {
public:
    int id;
    string name;
    string password;
    string phone;
    VehicleType vehicleType;
    string vehicleNumber;
    string currentLocation;
    bool available;
    double rating;
    int totalRides;
    double earnings;
    double currentRideFare;
    DriverNode* next;

    DriverNode(int driverId,string n,string pass,string ph,VehicleType vt,string vn,string loc)
        : id(driverId),
          name(n),
          password(pass),
          phone(ph),
          vehicleType(vt),
          vehicleNumber(vn),
          currentLocation(loc),
          available(true),
          rating(5.0),
          totalRides(0),
          earnings(0.0),
          currentRideFare(0.0),
          next(nullptr) {}
};

// ============================================
//           DRIVER LIST CLASS (Linked List)
// ============================================
class DriverList {
private:
    DriverNode* head;
    int count;

public:
    DriverList() : head(nullptr), count(0) {}

    void addDriver(int id,
                   const string& name,
                   const string& password,
                   const string& phone,
                   VehicleType vehicleType,
                   const string& vehicleNumber,
                   const string& location) {
        DriverNode* newNode = new DriverNode(id, name, password, phone, vehicleType, vehicleNumber, location);
        newNode->next = head;
        head = newNode;
        count++;
    }

    DriverNode* authenticate(const string& name, const string& password) {
        DriverNode* current = head;
        while (current != nullptr) {
            if (current->name == name && current->password == password) return current;
            current = current->next;
        }
        return nullptr;
    }

    DriverNode* findNearestDriver(const string& location, LocationGraph& graph, VehicleType preferredType) {
        DriverNode* nearest = nullptr;
        double minDistance = INT_MAX;
        DriverNode* current = head;
        while (current != nullptr) {
            if (current->available && current->vehicleType == preferredType) {
                double dist = graph.getDistance(current->currentLocation, location);
                if (dist >= 0 && dist < minDistance) {
                    minDistance = dist;
                    nearest = current;
                }
            }
            current = current->next;
        }
        return nearest;
    }

    DriverNode* findDriverByName(const string& name) {
        DriverNode* current = head;
        while (current != nullptr) {
            if (current->name == name) return current;
            current = current->next;
        }
        return nullptr;
    }

    bool isVehicleNumberExists(const string& vehicleNumber) {
        DriverNode* current = head;
        while (current != nullptr) {
            if (toLowerCase(current->vehicleNumber) == toLowerCase(vehicleNumber)) return true;
            current = current->next;
        }
        return false;
    }

    bool isDriverExists(const string& name) {
        DriverNode* current = head;
        while (current != nullptr) {
            if (current->name == name) return true;
            current = current->next;
        }
        return false;
    }

    void displayAll() {
        cout << "\n=== ALL DRIVERS ===\n";
        DriverNode* current = head;
        int idx = 1;
        while (current != nullptr) {
            cout << idx++ << ". [" << current->id << "] " << current->name
                 << " | " << vehicleTypeToString(current->vehicleType)
                 << " (" << current->vehicleNumber << ")"
                 << " | Location: " << current->currentLocation
                 << " | Rating: " << fixed << setprecision(1) << current->rating
                 << " | Rides: " << current->totalRides
                 << " | Earnings: $" << fixed << setprecision(2) << current->earnings
                 << " | " << (current->available ? "Available" : "Busy") << "\n";
            current = current->next;
        }
        if (count == 0) cout << "No drivers registered.\n";
    }

    void displayAvailableByType(VehicleType type) {
        cout << "\n=== AVAILABLE " << vehicleTypeToString(type) << " DRIVERS ===\n";
        DriverNode* current = head;
        int idx = 1;
        bool found = false;
        while (current != nullptr) {
            if (current->available && current->vehicleType == type) {
                found = true;
                cout << idx++ << ". " << current->name
                     << " | " << vehicleTypeToString(current->vehicleType)
                     << " | Location: " << current->currentLocation
                     << " | Rating: " << fixed << setprecision(1) << current->rating << "\n";
            }
            current = current->next;
        }
        if (!found) cout << "No available " << vehicleTypeToString(type) << " drivers.\n";
    }

    int getCount() { return count; }
    DriverNode* getHead() { return head; }

    ~DriverList() {
        while (head != nullptr) {
            DriverNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

// ============================================
//           RIDE REQUEST CLASS
// ============================================
class RideRequest {
public:
    string username;
    string origin;
    string destination;
    double distance;
    double fare;
    string driverName;
    VehicleType vehicleType;
    string status;
    int requestId;

    RideRequest(string user, string orig, string dest, double dist, double f, VehicleType vt, int id = 0)
        : username(user),
          origin(orig),
          destination(dest),
          distance(dist),
          fare(f),
          vehicleType(vt),
          status("Pending"),
          requestId(id) {}
};

// ============================================
//           CUSTOM QUEUE
// ============================================
template <typename T>
class QueueNode {
public:
    T data;
    QueueNode<T>* next;
    QueueNode(T item) : data(item), next(nullptr) {}
};

template <typename T>
class CustomQueue {
private:
    QueueNode<T>* front;
    QueueNode<T>* rear;
    int queueSize;

public:
    CustomQueue() : front(nullptr), rear(nullptr), queueSize(0) {}
    ~CustomQueue() {
        while (!isEmpty()) dequeue();
    }

    void enqueue(T item) {
        QueueNode<T>* newNode = new QueueNode<T>(item);
        if (rear == nullptr) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        queueSize++;
    }

    T dequeue() {
        if (isEmpty()) return nullptr;
        QueueNode<T>* temp = front;
        T item = front->data;
        front = front->next;
        if (front == nullptr) rear = nullptr;
        delete temp;
        queueSize--;
        return item;
    }

    T peek() { return isEmpty() ? nullptr : front->data; }
    bool isEmpty() { return front == nullptr; }
    int size() { return queueSize; }

    CustomQueue<T>* copy() {
        CustomQueue<T>* newQueue = new CustomQueue<T>();
        QueueNode<T>* current = front;
        while (current != nullptr) {
            newQueue->enqueue(current->data);
            current = current->next;
        }
        return newQueue;
    }
};

// ============================================
//           RIDE REQUEST QUEUE
// ============================================
class RideRequestQueue {
private:
    CustomQueue<RideRequest*> requests;
    int nextId;

public:
    RideRequestQueue() : nextId(1) {}

    void enqueue(RideRequest* request) {
        request->requestId = nextId++;
        requests.enqueue(request);
    }

    RideRequest* dequeue() { return requests.dequeue(); }
    bool isEmpty() { return requests.isEmpty(); }
    int size() { return requests.size(); }

    void displayPending() {
        if (requests.isEmpty()) {
            cout << "No pending ride requests.\n";
            return;
        }
        cout << "\n=== PENDING RIDE REQUESTS ===\n";
        CustomQueue<RideRequest*>* temp = requests.copy();
        int idx = 1;
        while (!temp->isEmpty()) {
            RideRequest* req = temp->dequeue();
            cout << idx++ << ". ID: " << req->requestId
                 << " | User: " << req->username
                 << " | " << req->origin << " -> " << req->destination
                 << " | Vehicle: " << vehicleTypeToString(req->vehicleType)
                 << " | Distance: " << fixed << setprecision(2) << req->distance << " km"
                 << " | Fare: $" << fixed << setprecision(2) << req->fare << "\n";
        }
        delete temp;
    }
};

// ============================================
//           CUSTOM STACK
// ============================================
template <typename T>
class StackNode {
public:
    T data;
    StackNode<T>* next;
    StackNode(T item) : data(item), next(nullptr) {}
};

template <typename T>
class CustomStack {
private:
    StackNode<T>* top;
    int stackSize;

public:
    CustomStack() : top(nullptr), stackSize(0) {}
    ~CustomStack() {
        while (!isEmpty()) pop();
    }

    void push(T item) {
        StackNode<T>* newNode = new StackNode<T>(item);
        newNode->next = top;
        top = newNode;
        stackSize++;
    }

    T pop() {
        if (isEmpty()) return nullptr;
        StackNode<T>* temp = top;
        T item = top->data;
        top = top->next;
        delete temp;
        stackSize--;
        return item;
    }

    T peek() { return isEmpty() ? nullptr : top->data; }
    bool isEmpty() { return top == nullptr; }
    int size() { return stackSize; }
    StackNode<T>* getTop() { return top; }
};

// ============================================
//           RIDE HISTORY STACK
// ============================================
class RideHistoryStack {
private:
    CustomStack<RideRequest*> history;

public:
    void push(RideRequest* ride) { history.push(ride); }

    void displayHistory(const string& username) {
        if (history.isEmpty()) {
            cout << "No ride history.\n";
            return;
        }
        cout << "\n=== RIDE HISTORY (Most Recent First) ===\n";
        int idx = 1;
        bool found = false;
        StackNode<RideRequest*>* current = history.getTop();
        while (current != nullptr) {
            RideRequest* ride = current->data;
            if (ride->username == username || username == "") {
                found = true;
                cout << idx++ << ". ID: " << ride->requestId
                     << " | Driver: " << ride->driverName
                     << " | Vehicle: " << vehicleTypeToString(ride->vehicleType)
                     << " | " << ride->origin << " -> " << ride->destination
                     << " | Distance: " << fixed << setprecision(2) << ride->distance << " km"
                     << " | Fare: $" << fixed << setprecision(2) << ride->fare
                     << " | Status: " << ride->status << "\n";
            }
            current = current->next;
        }
        if (!found && username != "") cout << "No ride history for user: " << username << "\n";
    }

    int size() { return history.size(); }
};

// ============================================
//           RIDE BOOKING SYSTEM
// ============================================
class RideBookingSystem {
private:
    User users[MAX_USERS];
    int userCount = 0;
    DriverList driverList;
    int nextDriverId = 1;
    LocationGraph graph;
    RideRequestQueue requestQueue;
    RideHistoryStack historyStack;
    FareCalculator fareCalc;

    string currentUser;
    string currentDriver;

    User* getCurrentUser() {
        for (int i = 0; i < userCount; i++) {
            if (users[i].getUsername() == currentUser) return &users[i];
        }
        return nullptr;
    }

    DriverNode* getCurrentDriver() { return driverList.findDriverByName(currentDriver); }

    void saveUsers() {
        ofstream out("users.txt");
        for (int i = 0; i < userCount; i++) out << users[i].toFileString() << "\n";
    }

    void loadUsers() {
        ifstream in("users.txt");
        if (!in) return;
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            if (userCount < MAX_USERS) {
                users[userCount++] = User::fromFileString(line);
            }
        }
    }

    void saveDrivers() {
        ofstream out("drivers.txt");
        DriverNode* current = driverList.getHead();
        while (current != nullptr) {
            out << current->id << "," << current->name << "," << current->password << "," << current->phone << ","
                << static_cast<int>(current->vehicleType) << ","
                << current->vehicleNumber << "," << current->currentLocation << ","
                << current->available << "," << current->rating << ","
                << current->totalRides << "," << current->earnings << ","
                << current->currentRideFare << "\n";
            current = current->next;
        }
    }

    void loadDrivers() {
        ifstream in("drivers.txt");
        if (!in) return;
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string item;
            vector<string> tokens;
            while (getline(ss, item, ',')) tokens.push_back(item);
            if (tokens.size() < 9) continue;

            int tokenOffset = 0;
            int driverId = 0;
            // Backward compatibility: if first token is not an id, synthesize one
            if (tokens.size() >= 11 && all_of(tokens[0].begin(), tokens[0].end(), ::isdigit)) {
                driverId = safeStoi(tokens[0], nextDriverId);
            } else {
                driverId = nextDriverId;
                tokenOffset = -1;  // shift because legacy file has no id
            }

            int baseIdx = (tokenOffset == 0) ? 1 : 0;
            string name = tokens[baseIdx + 0];
            string pass = tokens[baseIdx + 1];
            string phone = tokens[baseIdx + 2];
            VehicleType vt = static_cast<VehicleType>(safeStoi(tokens[baseIdx + 3], 1));
            string vehicleNum = tokens[baseIdx + 4];
            string loc = tokens[baseIdx + 5];

            driverList.addDriver(driverId, name, pass, phone, vt, vehicleNum, loc);
            DriverNode* driver = driverList.findDriverByName(name);
            if (driver) {
                driver->available = (tokens[baseIdx + 6] == "1");
                driver->rating = clampValue(safeStod(tokens[baseIdx + 7], 5.0), 1.0, 5.0);
                driver->totalRides = max(0, safeStoi(tokens[baseIdx + 8], 0));
                if (tokens.size() >= baseIdx + 10) driver->earnings = max(0.0, safeStod(tokens[baseIdx + 9], 0.0));
                if (tokens.size() >= baseIdx + 11) driver->currentRideFare = max(0.0, safeStod(tokens[baseIdx + 10], 0.0));
            }
            nextDriverId = max(nextDriverId, driverId + 1);
        }
    }

public:
    RideBookingSystem() {
        graph.initializeDefaultLocations();
        loadUsers();
        loadDrivers();
    }

    ~RideBookingSystem() {
        saveUsers();
        saveDrivers();
    }

    // ---------------- USER FLOW ----------------
    void registerUser() {
        string username, password, phone;
        cout << "\n=== USER REGISTRATION ===\n";
        cout << "Enter username: ";
        cin >> username;
        for (int i = 0; i < userCount; i++) {
            if (users[i].getUsername() == username) {
                cout << "Error: Username already exists!\n";
                return;
            }
        }
        cout << "Enter password: ";
        cin >> password;
        cout << "Enter phone number: ";
        cin >> phone;
        if (userCount >= MAX_USERS) {
            cout << "User limit reached.\n";
            return;
        }
        users[userCount++] = User(username, password, phone);
        saveUsers();
        cout << "User registered successfully!\n";
    }

    bool loginUser() {
        string username, password;
        cout << "\n=== USER LOGIN ===\n";
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;
        for (int i = 0; i < userCount; i++) {
            if (users[i].getUsername() == username && users[i].getPassword() == password) {
                currentUser = username;
                cout << "Login successful! Welcome, " << username << "!\n";
                return true;
            }
        }
        cout << "Invalid username or password.\n";
        return false;
    }

    void logoutUser() {
        currentUser.clear();
        cout << "Logged out from user account.\n";
    }

    void addWalletBalance() {
        if (currentUser.empty()) {
            cout << "Please login as user first.\n";
            return;
        }
        User* user = getCurrentUser();
        if (!user) return;
        double amount;
        cout << "Enter amount to add: $";
        cin >> amount;
        if (amount > 0) {
            user->addWalletBalance(amount);
            saveUsers();
            cout << "Balance added! New balance: $" << fixed << setprecision(2) << user->getWalletBalance() << "\n";
        } else {
            cout << "Invalid amount.\n";
        }
    }

    double getWalletBalance() {
        User* user = getCurrentUser();
        return user ? user->getWalletBalance() : 0.0;
    }

    // ---------------- DRIVER FLOW ----------------
    bool loginDriver() {
        cout << "\n=== DRIVER LOGIN ===\n";
        string name, password;
        cout << "Enter driver username: ";
        cin >> name;
        cout << "Enter password: ";
        cin >> password;
        DriverNode* driver = driverList.authenticate(name, password);
        if (!driver) {
            cout << "Invalid driver credentials.\n";
            return false;
        }
        currentDriver = name;
        cout << "Driver login successful. Welcome, " << name << "!\n";
        return true;
    }

    void logoutDriver() {
        currentDriver.clear();
        cout << "Logged out from driver account.\n";
    }

    void registerDriver() {
        cout << "\n=== DRIVER REGISTRATION ===\n";
        string name, password, phone, vehicleNumber, location;
        int vehicleChoice;
        cout << "Create driver username: ";
        cin >> name;
        if (driverList.isDriverExists(name)) {
            cout << "Error: Driver with this username already exists!\n";
            return;
        }
        cout << "Create password: ";
        cin >> password;
        cout << "Enter phone number: ";
        cin >> phone;

        cout << "Select vehicle type:\n";
        cout << "1. Bike\n";
        cout << "2. Car\n";
        cout << "3. Rickshaw\n";
        cout << "Enter choice: ";
        cin >> vehicleChoice;

        VehicleType vehicleType = VehicleType::CAR;
        if (vehicleChoice == 1) vehicleType = VehicleType::BIKE;
        else if (vehicleChoice == 3) vehicleType = VehicleType::RICKSHAW;

        cout << "Enter vehicle number: ";
        cin.ignore();
        getline(cin, vehicleNumber);
        if (driverList.isVehicleNumberExists(vehicleNumber)) {
            cout << "Error: Vehicle number already registered!\n";
            return;
        }

        graph.displayLocations();
        cout << "Enter current location: ";
        getline(cin, location);
        string validLocation = graph.getValidLocation(location);
        if (validLocation.empty()) {
            cout << "Error: Invalid location! Please enter a valid location from the list.\n";
            return;
        }

        int driverId = nextDriverId++;
        driverList.addDriver(driverId, name, password, phone, vehicleType, vehicleNumber, validLocation);
        saveDrivers();
        cout << "Driver registered successfully! Your Driver ID: " << driverId << "\n";
    }

    void updateDriverLocation() {
        if (currentDriver.empty()) {
            cout << "Please login as driver first.\n";
            return;
        }
        DriverNode* driver = getCurrentDriver();
        if (!driver) {
            cout << "Driver record not found.\n";
            return;
        }
        graph.displayLocations();
        string newLocation;
        cout << "Enter new location: ";
        cin.ignore();
        getline(cin, newLocation);
        string validLocation = graph.getValidLocation(newLocation);
        if (validLocation.empty()) {
            cout << "Error: Invalid location! Please enter a valid location from the list.\n";
            return;
        }
        driver->currentLocation = validLocation;
        saveDrivers();
        cout << "Location updated successfully!\n";
    }

    void viewDriverEarnings() {
        if (currentDriver.empty()) {
            cout << "Please login as driver first.\n";
            return;
        }
        DriverNode* driver = getCurrentDriver();
        if (!driver) {
            cout << "Driver record not found.\n";
            return;
        }
        cout << "\n=== YOUR DRIVER STATISTICS ===\n";
        cout << "Name: " << driver->name << "\n";
        cout << "Vehicle: " << vehicleTypeToString(driver->vehicleType) << " (" << driver->vehicleNumber << ")\n";
        cout << "Location: " << driver->currentLocation << "\n";
        cout << "Rating: " << fixed << setprecision(1) << driver->rating << " / 5.0\n";
        cout << "Total Rides: " << driver->totalRides << "\n";
        cout << "Total Earnings: $" << fixed << setprecision(2) << driver->earnings << "\n";
        cout << "Status: " << (driver->available ? "Available" : "On a Ride") << "\n";
        if (driver->totalRides > 0) {
            double avgEarnings = driver->earnings / driver->totalRides;
            cout << "Average per Ride: $" << fixed << setprecision(2) << avgEarnings << "\n";
        }
    }

    // ---------------- RIDE FLOW ----------------
    void bookRide() {
        if (currentUser.empty()) {
            cout << "Please login as user first.\n";
            return;
        }
        User* user = getCurrentUser();
        if (!user) return;
        if (user->getHasActiveRide()) {
            cout << "You already have an active ride. Please complete it first.\n";
            return;
        }

        cout << "\n=== BOOK A RIDE ===\n";
        cout << "Select your preferred vehicle type:\n";
        cout << "1. Bike (Cheapest, Fast for short distances)\n";
        cout << "2. Car (Comfortable, Best for longer trips)\n";
        cout << "3. Rickshaw (Eco-friendly, Medium price)\n";
        cout << "Enter choice: ";
        int vehicleChoice;
        cin >> vehicleChoice;
        VehicleType preferredType = VehicleType::CAR;
        if (vehicleChoice == 1) preferredType = VehicleType::BIKE;
        else if (vehicleChoice == 3) preferredType = VehicleType::RICKSHAW;
        else {
            cout << "Invalid Choice!";
            return;
        }

        graph.displayLocations();
        string origin, destination;
        cout << "Enter pickup location: ";
        cin.ignore();
        getline(cin, origin);
        string validOrigin = graph.getValidLocation(origin);
        if (validOrigin.empty()) {
            cout << "Error: Invalid pickup location! Please enter a valid location from the list.\n";
            return;
        }

        cout << "Enter destination: ";
        getline(cin, destination);
        string validDestination = graph.getValidLocation(destination);
        if (validDestination.empty()) {
            cout << "Error: Invalid destination! Please enter a valid location from the list.\n";
            return;
        }
        if (validOrigin == validDestination) {
            cout << "Error: Origin and destination cannot be the same!\n";
            return;
        }

        double distance = graph.getDistance(validOrigin, validDestination);
        if (distance < 0) {
            cout << "Error: Cannot calculate distance between these locations.\n";
            return;
        }

        DriverNode* nearestDriver = driverList.findNearestDriver(validOrigin, graph, preferredType);
        if (!nearestDriver) {
            cout << "No available " << vehicleTypeToString(preferredType) << " drivers. Adding to queue...\n";
            double fare = fareCalc.calculateFare(distance, preferredType);
            RideRequest* request = new RideRequest(currentUser, validOrigin, validDestination, distance, fare, preferredType);
            requestQueue.enqueue(request);
            cout << "Ride request queued.\n";
            return;
        }

        double driverDistance = graph.getDistance(nearestDriver->currentLocation, validOrigin);
        double fare = fareCalc.calculateFare(distance, preferredType, nearestDriver->rating);

        cout << "\n=== RIDE DETAILS ===\n";
        cout << "Vehicle Type: " << vehicleTypeToString(preferredType) << "\n";
        cout << "Distance: " << fixed << setprecision(2) << distance << " km\n";
        cout << "Estimated Fare: $" << fixed << setprecision(2) << fare << "\n";
        cout << "Wallet Balance: $" << fixed << setprecision(2) << getWalletBalance() << "\n";
        if (getWalletBalance() < fare) {
            cout << "Insufficient balance! Please add money to wallet.\n";
            return;
        }

        cout << "\n=== DRIVER ASSIGNED ===\n";
        cout << "Driver: " << nearestDriver->name << "\n";
        cout << "Vehicle: " << vehicleTypeToString(nearestDriver->vehicleType) << " (" << nearestDriver->vehicleNumber << ")\n";
        cout << "Rating: " << fixed << setprecision(1) << nearestDriver->rating << "\n";
        cout << "Distance from you: " << fixed << setprecision(2) << driverDistance << " km\n";
        cout << "\nConfirm ride? (y/n): ";
        char confirm;
        cin >> confirm;
        if (confirm == 'y' || confirm == 'Y') {
            nearestDriver->available = false;
            nearestDriver->currentRideFare = fare;
            nearestDriver->currentLocation = validDestination;
            user->setActiveRide(nearestDriver->name, fare, validOrigin, validDestination, distance, preferredType);
            saveUsers();
            saveDrivers();
            cout << "\n=== RIDE CONFIRMED ===\n";
            cout << "Driver: " << nearestDriver->name << "\n";
            cout << "Route: " << validOrigin << " -> " << validDestination << "\n";
            cout << "Fare: $" << fixed << setprecision(2) << fare << " (will be charged after completion)\n";
            cout << "Your driver will arrive shortly!\n";
        } else {
            cout << "Ride booking cancelled.\n";
        }
    }

    void completeRide() {
        if (currentUser.empty()) {
            cout << "Please login as user first.\n";
            return;
        }
        User* user = getCurrentUser();
        if (!user) return;
        if (!user->getHasActiveRide()) {
            cout << "You don't have any active rides.\n";
            return;
        }

        cout << "\n=== COMPLETE YOUR RIDE ===\n";
        cout << "Driver: " << user->getActiveDriverName() << "\n";
        cout << "Vehicle: " << vehicleTypeToString(user->getActiveRideVehicleType()) << "\n";
        cout << "Route: " << user->getActiveRideOrigin() << " -> " << user->getActiveRideDestination() << "\n";
        cout << "Distance: " << fixed << setprecision(2) << user->getActiveRideDistance() << " km\n";
        cout << "Fare: $" << fixed << setprecision(2) << user->getActiveRideFare() << "\n";
        cout << "Current Balance: $" << fixed << setprecision(2) << getWalletBalance() << "\n";
        if (getWalletBalance() < user->getActiveRideFare()) {
            cout << "\nError: Insufficient balance!\n";
            return;
        }

        cout << "\nConfirm ride completion? (y/n): ";
        char confirm;
        cin >> confirm;
        if (confirm != 'y' && confirm != 'Y') {
            cout << "Ride completion cancelled.\n";
            return;
        }

        DriverNode* driver = driverList.findDriverByName(user->getActiveDriverName());
        if (!driver) {
            cout << "Error: Driver not found.\n";
            return;
        }

        cout << "\n=== RATE YOUR DRIVER ===\n";
        cout << "Current driver rating: " << fixed << setprecision(1) << driver->rating << "\n";
        cout << "Enter your rating (1.0 - 5.0): ";
        double userRating;
        cin >> userRating;
        userRating = clampValue(userRating, 1.0, 5.0);

        int totalRides = driver->totalRides;
        if (totalRides > 0)
            driver->rating = ((driver->rating * totalRides) + userRating) / (totalRides + 1);
        else
            driver->rating = userRating;
        driver->totalRides++;

        double baseFare = user->getActiveRideFare();
        double bonus = fareCalc.calculateBonus(baseFare, driver->rating);
        double totalEarnings = baseFare + bonus;

        user->deductWalletBalance(baseFare);
        driver->earnings += totalEarnings;
        driver->currentRideFare = 0.0;
        driver->available = true;

        RideRequest* ride = new RideRequest(user->getUsername(),user->getActiveRideOrigin(),user->getActiveRideDestination(),user->getActiveRideDistance(),baseFare,user->getActiveRideVehicleType());ride->driverName = user->getActiveDriverName();
        ride->status = "Completed";
        historyStack.push(ride);

        user->clearActiveRide();
        saveUsers();
        saveDrivers();

        cout << "\n=== RIDE COMPLETED SUCCESSFULLY ===\n";
        cout << "Your rating: " << fixed << setprecision(1) << userRating << " stars\n";
        cout << "\n=== PAYMENT SUMMARY ===\n";
        cout << "Fare Charged: $" << fixed << setprecision(2) << baseFare << "\n";
        cout << "Remaining Balance: $" << fixed << setprecision(2) << getWalletBalance() << "\n";
        cout << "\n=== DRIVER PAYMENT ===\n";
        cout << "Base Fare: $" << fixed << setprecision(2) << baseFare << "\n";
        if (bonus > 0) cout << "Rating Bonus: $" << fixed << setprecision(2) << bonus << "\n";
        cout << "Total Earnings: $" << fixed << setprecision(2) << totalEarnings << "\n";
        cout << "Driver's New Rating: " << fixed << setprecision(1) << driver->rating << " / 5.0\n";
    }

    // ---------------- MENUS ----------------
    void displayMainMenu() {
        cout << "\n";
        printSection("RIDE BOOKING SYSTEM");
        cout << "1) User Panel\n";
        cout << "2) Driver Panel\n";
        cout << "3) System\n";
        cout << "4) Exit\n";
        printDivider();
        cout << "Enter choice: ";
    }

    void displayUserMenu() {
        printSection("USER PANEL");
        cout << "User: " << (currentUser.empty() ? "(not logged in)" : currentUser)
             << " | Wallet: $" << fixed << setprecision(2) << getWalletBalance() << "\n";
        printDivider();
        cout << "1) Login\n";
        cout << "2) Register\n";
        cout << "3) Logout\n";
        cout << "4) Add Money to Wallet\n";
        cout << "5) Book a Ride\n";
        cout << "6) Complete Ride & Rate Driver\n";
        cout << "7) View Ride History\n";
        cout << "8) View All Drivers\n";
        cout << "9) View Available Drivers by Type\n";
        cout << "10) Back to Main Menu\n";
        printDivider();
        cout << "Enter choice: ";
    }

    void displayDriverMenu() {
        printSection("DRIVER PANEL");
        cout << "Driver: " << (currentDriver.empty() ? "(not logged in)" : currentDriver) << "\n";
        printDivider();
        cout << "1) Login\n";
        cout << "2) Register\n";
        cout << "3) Logout\n";
        cout << "4) Update Driver Location\n";
        cout << "5) View My Earnings\n";
        cout << "6) Back to Main Menu\n";
        printDivider();
        cout << "Enter choice: ";
    }

    void displaySystemMenu() {
        printSection("SYSTEM");
        printDivider();
        cout << "1) View Pending Requests\n";
        cout << "2) View All Locations\n";
        cout << "3) Back to Main Menu\n";
        printDivider();
        cout << "Enter choice: ";
    }

    void viewAvailableDriversByType() {
        cout << "\nSelect vehicle type to view:\n";
        cout << "1. Bike\n";
        cout << "2. Car\n";
        cout << "3. Rickshaw\n";
        cout << "Enter choice: ";
        int choice;
        cin >> choice;
        VehicleType type = VehicleType::CAR;
        if (choice == 1) type = VehicleType::BIKE;
        else if (choice == 3) type = VehicleType::RICKSHAW;
        driverList.displayAvailableByType(type);
    }

    // ---------------- PANELS ----------------
    void handleUserPanel() {
        bool back = false;
        while (!back) {
            displayUserMenu();
            int choice;
            cin >> choice;
            switch (choice) {
                case 1: loginUser(); break;
                case 2: registerUser(); break;
                case 3: logoutUser(); break;
                case 4: addWalletBalance(); break;
                case 5: bookRide(); break;
                case 6: completeRide(); break;
                case 7: historyStack.displayHistory(currentUser); break;
                case 8: driverList.displayAll(); break;
                case 9: viewAvailableDriversByType(); break;
                case 10: back = true; break;
                default: cout << "Invalid choice! Please try again.\n"; break;
            }
        }
    }

    void handleDriverPanel() {
        bool back = false;
        while (!back) {
            displayDriverMenu();
            int choice;
            cin >> choice;
            switch (choice) {
                case 1: loginDriver(); break;
                case 2: registerDriver(); break;
                case 3: logoutDriver(); break;
                case 4: updateDriverLocation(); break;
                case 5: viewDriverEarnings(); break;
                case 6: back = true; break;
                default: cout << "Invalid choice! Please try again.\n"; break;
            }
        }
    }

    void handleSystemPanel() {
        bool back = false;
        while (!back) {
            displaySystemMenu();
            int choice;
            cin >> choice;
            switch (choice) {
                case 1: requestQueue.displayPending(); break;
                case 2: graph.displayLocations(); break;
                case 3: back = true; break;
                default: cout << "Invalid choice! Please try again.\n"; break;
            }
        }
    }

    void run() {
        cout << "\n==============================================\n";
        cout << "   WELCOME TO UBER-LIKE RIDE BOOKING APP   \n";
        cout << "==============================================\n";
        bool exitApp = false;
        while (!exitApp) {
            displayMainMenu();
            int choice;
            cin >> choice;
            switch (choice) {
                case 1: handleUserPanel(); break;
                case 2: handleDriverPanel(); break;
                case 3: handleSystemPanel(); break;
                case 4:
                    cout << "\nThank you for using the Ride Booking System!\n";
                    exitApp = true;
                    break;
                default:
                    cout << "Invalid choice! Please try again.\n";
            }
        }
    }
};

// ============================================
//           MAIN
// ============================================
int main() {
    RideBookingSystem system;
    system.run();
    return 0;
}