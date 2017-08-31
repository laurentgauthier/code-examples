#include <string>
#include <iostream>
#include <yaml-cpp/yaml.h>

int main(int argc, char **argv)
{
	YAML::Node node = YAML::Load("{name: Brewers, city: Milwaukee}");

	if (node["name"]) {
		std::cout << node["name"].as<std::string>() << "\n";
	}
	if (node["city"]) {
		std::cout << node["city"].as<std::string>() << "\n";
	}
	if (node["mascot"]) {
		std::cout << node["mascot"].as<std::string>() << "\n";
	}
}
