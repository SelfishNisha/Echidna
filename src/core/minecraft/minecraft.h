#pragma once 
#include "../jvm.h"

class cEntity : public cJObject {
  public:
    cEntity(jobject objectIn) : cJObject(objectIn) {}
    void setSprinting(bool state);
    bool canSprint();
    bool isSprinting();
};

class cKeyBinding : public cJObject {
  public:
    cKeyBinding(jobject objectIn) : cJObject(objectIn) {}
    void setPressed(bool pressed);
};

class cGameOptions : public cJObject {
  public:
    cGameOptions(jobject objectIn) : cJObject(objectIn) {}
    cKeyBinding getSprintKey();
};



class cMinecraft : public cJObject {
  public:
    cMinecraft(jobject objectIn) : cJObject(objectIn) {}
    cGameOptions getOptions();

    static cMinecraft getMinecraft();
    cEntity getPlayer() const;
};

void tick();