#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm> 

class Property {
public:
	Property(const std::string& description) : description_(description) {}

	virtual ~Property() = default;

	std::string& GetDesc() { return description_; };
protected:
	std::string description_;
};

class BoolProperty : public Property {
public:
	BoolProperty(bool value, const std::string& description) : value_(value), default_(value), Property(description) {}
	bool Value() const { return value_; }
	bool DefaultValue() const { return default_; }
private:
	bool value_;
	bool default_;
	friend class Config;
};

class IntProperty : public Property {
public:
	IntProperty(int value, const std::string& description) : value_(value), default_(value), Property(description) {}
	int Value() const { return value_; }
	int DefaultValue() const { return default_; }
private:
	int value_;
	int default_;
	friend class Config;
};

class FloatProperty : public Property {
public:
	FloatProperty(float value, const std::string& description) : value_(value), default_(value), Property(description) {}
	float Value() const { return value_; }
	float DefaultValue() const { return default_; }
private:
	float value_;
	float default_;
	friend class Config;
};

class StringProperty : public Property {
public:
	StringProperty(const std::string& value, const std::string& description) : value_(value), default_(value), Property(description) {}
	std::string Value() const { return value_; }
	std::string DefaultValue() const { return default_; }
private:
	std::string value_;
	std::string default_;
	friend class Config;
};

class Config {
public:
	BoolProperty* RegisterBool(const std::string& name, const std::string& description, bool defaultValue) {
		BoolProperty* newProp = new BoolProperty(defaultValue, description);
		properties_[name] = newProp;
		return newProp;
	}

	IntProperty* RegisterInt(const std::string& name, const std::string& description, int defaultValue) {
		IntProperty* newProp = new IntProperty(defaultValue, description);
		properties_[name] = newProp;
		return newProp;
	}

	FloatProperty* RegisterFloat(const std::string& name, const std::string& description, float defaultValue) {
		FloatProperty* newProp = new FloatProperty(defaultValue, description);
		properties_[name] = newProp;
		return newProp;
	}

	StringProperty* RegisterString(const std::string& name, const std::string& description, const std::string& defaultValue) {
		StringProperty* newProp = new StringProperty(defaultValue, description);
		properties_[name] = newProp;
		return newProp;
	}

	void SaveToFile(const std::string& filename) const {
		std::ofstream file(filename);
		for (const auto& pair : properties_) {
			const std::string& name = pair.first;
			Property* prop = pair.second;
			if (BoolProperty* boolProp = dynamic_cast<BoolProperty*>(prop)) {
				file << "//[Bool] " << boolProp->GetDesc() << " (Default Value: " << (boolProp->DefaultValue() ? "true" : "false") << ")\n";
				file << name << " = " << (boolProp->Value() ? "true" : "false") << "\n\n";
			}
			else if (IntProperty* intProp = dynamic_cast<IntProperty*>(prop)) {
				file << "//[Int] " << intProp->GetDesc() << " (Default Value: " << intProp->DefaultValue() << ")\n";
				file << name << " = " << intProp->Value() << "\n\n";
			}
			else if (FloatProperty* floatProp = dynamic_cast<FloatProperty*>(prop)) {
				file << "//[Float] " << floatProp->GetDesc() << " (Default Value: " << floatProp->DefaultValue() << ")\n";
				file << name << " = " << floatProp->Value() << "\n\n";
			}
			else if (StringProperty* stringProp = dynamic_cast<StringProperty*>(prop)) {
				file << "//[String] " << stringProp->GetDesc() << " (Default Value: \"" << stringProp->DefaultValue() << "\")\n";
				file << name << " = " << stringProp->Value() << "\n\n";
			}
		}
	}

	void LoadFromFile(const std::string& filename) {
		std::ifstream file(filename);
		std::string line;
		while (std::getline(file, line)) {
			// Ignore comments
			if (line.empty() || line[0] == '/') continue;

			// Split the line into name and value
			std::string::size_type pos = line.find('=');
			if (pos == std::string::npos) continue;  // Invalid line

			std::string name = line.substr(0, pos);
			std::string value = line.substr(pos + 1);

			// Trim whitespace
			name.erase(name.begin(), std::find_if(name.begin(), name.end(), [](char ch){
				return !std::isspace(ch);
				}));
			name.erase(std::find_if(name.rbegin(), name.rend(), [](char ch){
				return !std::isspace(ch);
				}).base(), name.end());

			value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](char ch){
				return !std::isspace(ch);
				}));
			value.erase(std::find_if(value.rbegin(), value.rend(), [](char ch){
				return !std::isspace(ch);
				}).base(), value.end());

			// Update the corresponding property
			Property* prop = properties_[name];
			if (dynamic_cast<BoolProperty*>(prop)) {
				std::transform(value.begin(), value.end(), value.begin(),
					[](unsigned char c) { return std::tolower(c); });
				static_cast<BoolProperty*>(prop)->value_ = value == "true";
			}
			else if (dynamic_cast<IntProperty*>(prop)) {
				static_cast<IntProperty*>(prop)->value_ = std::stoi(value);
			}
			else if (dynamic_cast<FloatProperty*>(prop)) {
				static_cast<FloatProperty*>(prop)->value_ = std::stof(value);
			}
			else if (dynamic_cast<StringProperty*>(prop)) {
				static_cast<StringProperty*>(prop)->value_ = value;
			}
		}
	}

	BoolProperty* GetBool(const std::string& name) const {
		return dynamic_cast<BoolProperty*>(properties_.at(name));
	}

	IntProperty* GetInt(const std::string& name) const {
		return dynamic_cast<IntProperty*>(properties_.at(name));
	}

	FloatProperty* GetFloat(const std::string& name) const {
		return dynamic_cast<FloatProperty*>(properties_.at(name));
	}

	StringProperty* GetString(const std::string& name) const {
		return dynamic_cast<StringProperty*>(properties_.at(name));
	}

private:
	std::unordered_map<std::string, Property*> properties_;
};