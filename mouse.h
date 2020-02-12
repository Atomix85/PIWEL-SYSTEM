#ifndef __MOUSE_H
#define __MOUSE_H

#include "include/driver.h"
#include "types.h"
#include "interrupts.h"
#include "port.h"

class MouseEventHandler{
    public:
        MouseEventHandler();

        virtual void OnActivate();
        virtual void OnMouseDown(uint8_t button);
        virtual void OnMouseUp(uint8_t button);
        virtual void OnMouseMove(int x,int y);

};

class MouseDriver : public InterruptHandler, public Driver{

    Port8Bit dataport;
    Port8Bit commandport;
    uint8_t buffer[3];
    uint8_t offset;
    uint8_t buttons;

    MouseEventHandler* handler;

    public:
        MouseDriver(InterruptManager* manager, MouseEventHandler* handler);
        ~MouseDriver();
        virtual void Activate();
        virtual uint32_t HandlerInterrupt(uint32_t esp);

};


#endif