#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

// Base class: Person
class Person {
protected:
    std::string name;
    int age;

public:
    Person(const std::string& name, int age) : name(name), age(age) {}

    virtual void printDetails() const {
        std::cout << "Name: " << name << ", Age: " << age << std::endl;
    }

    virtual ~Person() = default; // Ensure proper cleanup in derived classes
};

// Derived class: Customer
class Customer : public Person {
private:
    std::string customerID;

public:
    Customer(const std::string& name, int age, const std::string& customerID)
        : Person(name, age), customerID(customerID) {}

    void printDetails() const override {
        std::cout << "Customer ID: " << customerID << ", Name: " << name << ", Age: " << age << std::endl;
    }
};

// CRM System Class with Thread Safety
class CRMSystem {
private:
    std::map<std::string, std::shared_ptr<Customer>> customerData;
    mutable std::mutex dataMutex; // Mutex for thread-safe access

public:
    void addCustomer(const std::string& id, const std::string& name, int age) {
        std::lock_guard<std::mutex> lock(dataMutex); // Lock the map
        customerData[id] = std::make_shared<Customer>(name, age, id);
        std::cout << "[Thread " << std::this_thread::get_id() << "] Customer " << id << " added.\n";
    }

    void removeCustomer(const std::string& id) {
        std::lock_guard<std::mutex> lock(dataMutex); // Lock the map
        customerData.erase(id);
        std::cout << "[Thread " << std::this_thread::get_id() << "] Customer " << id << " removed.\n";
    }

    void printCustomer(const std::string& id) const {
        std::lock_guard<std::mutex> lock(dataMutex); // Lock the map
        auto it = customerData.find(id);
        if (it != customerData.end()) {
            it->second->printDetails();
        } else {
            std::cout << "Customer with ID " << id << " not found.\n";
        }
    }

    void printAllCustomers() const {
        std::lock_guard<std::mutex> lock(dataMutex); // Lock the map
        for (const auto& pair : customerData) {
            pair.second->printDetails();
        }
    }
};

// Simulated workload for multithreading
void addCustomerTask(CRMSystem& crm, const std::string& id, const std::string& name, int age) {
    crm.addCustomer(id, name, age);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
}

void removeCustomerTask(CRMSystem& crm, const std::string& id) {
    crm.removeCustomer(id);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
}

int main() {
    CRMSystem crm;

    // Create threads for concurrent operations
    std::thread t1(addCustomerTask, std::ref(crm), "C001", "Alice", 30);
    std::thread t2(addCustomerTask, std::ref(crm), "C002", "Bob", 25);
    std::thread t3(removeCustomerTask, std::ref(crm), "C001");

    // Wait for threads to finish
    t1.join();
    t2.join();
    t3.join();

    // Print all remaining customers
    std::cout << "\nFinal list of customers:\n";
    crm.printAllCustomers();

    return 0;
}
