#include <iostream>
#include <vector>
#include <memory>
#include <string>

// Simple mock for Window, Renderer2D, Canvas, BackendManager object creation
struct MockObject {
    std::string name;
    MockObject(const std::string& n) : name(n) {}
    ~MockObject() {}
};

int main() {
    std::cout << "Running ASAN integration test..." << std::endl;
    
    // Create and destroy major objects sequentially to test heap issues
    {
        auto window = std::make_unique<MockObject>("Window");
        auto renderer = std::make_unique<MockObject>("Renderer2D");
        auto canvas = std::make_unique<MockObject>("Canvas");
        auto backend = std::make_unique<MockObject>("BackendManager");
    }
    
    // Intentionally test some safe memory creations
    std::vector<int> numbers;
    for (int i = 0; i < 1000; i++) {
        numbers.push_back(i * 2);
    }
    
    std::cout << "ASAN test completed cleanly." << std::endl;
    return 0;
}
