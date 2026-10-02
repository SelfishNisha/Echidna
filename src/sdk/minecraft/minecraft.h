#pragma once 
#include "../jvm.h"

class cEntity : public cJObject {
  public:
    cEntity(jobject objectIn) : cJObject(objectIn) {}
    void setSprinting(bool state);
};

class cMinecraft : public cJObject {
  public:
    cMinecraft(jobject objectIn) : cJObject(objectIn) {}

    static cMinecraft getMinecraft();
    cEntity getPlayer() const;
};

void tick();