#pragma once
#include "widget.hpp"
#include "button_widget.hpp"
#include "label_widget.hpp"
#include "text_input_widget.hpp"
#include "panel_widget.hpp"

// T-186: WidgetFactory — singleton for runtime widget creation
class WidgetFactory {
public:
    using Creator = std::function<WidgetPtr(const std::string& id)>;
    
    static WidgetFactory& instance() {
        static WidgetFactory factory;
        return factory;
    }
    
    void register_type(const std::string& type_name, Creator creator) {
        registry_[type_name] = std::move(creator);
    }
    
    WidgetPtr create(const std::string& type_name, const std::string& id) const {
        auto it = registry_.find(type_name);
        if (it != registry_.end()) return it->second(id);
        return nullptr;
    }
    
    std::vector<std::string> registered_types() const {
        std::vector<std::string> types;
        for (auto& [name, _] : registry_) types.push_back(name);
        return types;
    }
    
    void register_builtin_widgets() {
        register_type("Button", [](const std::string& id) {
            return std::make_shared<ButtonWidget>(id);
        });
        register_type("Label", [](const std::string& id) {
            return std::make_shared<LabelWidget>(id);
        });
        register_type("TextInput", [](const std::string& id) {
            return std::make_shared<TextInputWidget>(id);
        });
        register_type("Panel", [](const std::string& id) {
            return std::make_shared<PanelWidget>(id);
        });
        register_type("Slider", [](const std::string& id) {
            return std::make_shared<SliderWidget>(id);
        });
        register_type("Checkbox", [](const std::string& id) {
            return std::make_shared<CheckboxWidget>(id);
        });
        register_type("Dropdown", [](const std::string& id) {
            return std::make_shared<DropdownWidget>(id);
        });
        register_type("ProgressBar", [](const std::string& id) {
            return std::make_shared<ProgressBarWidget>(id);
        });
        register_type("Console", [](const std::string& id) {
            return std::make_shared<ConsoleWidget>(id);
        });
    }

private:
    WidgetFactory() = default;
    std::map<std::string, Creator> registry_;
};
