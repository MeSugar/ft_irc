#pragma once

#include "Socket.hpp"

class TemplateRun {
    protected:
        Socket *s;
    public:
        TemplateRun();
        virtual int run() = 0;
        virtual int loop() = 0;
        virtual int chat(int sockfd) = 0;
};