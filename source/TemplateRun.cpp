#include "../include/TemplateRun.hpp"

TemplateRun::TemplateRun() {
    this->s = new Socket();
}

TemplateRun::TemplateRun(int port, std::string pass) {
    std::cout << "Template run\n";
    this->s = new Socket(port, pass);
}