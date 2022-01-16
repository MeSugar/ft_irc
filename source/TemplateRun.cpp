#include "../include/TemplateRun.hpp"

TemplateRun::TemplateRun() {
    this->s = new Socket();
}

TemplateRun::TemplateRun(int port, std::string pass) {
    this->s = new Socket(port, pass);
}