#ifndef __manager_hpp
#define __manager_hpp

class Manager
{
public:
    Manager();
    virtual void setup() {};
    virtual void loop() {};
    virtual const char *name();
};

void setupManagers();

#endif