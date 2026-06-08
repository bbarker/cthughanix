// -*- C++ -*-
#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "keymap.h"
#include "Option.h"
#include "CoreOption.h"

class InterfaceElement {
public:
    const char * str;
    InterfaceElement(const char * s) : str(s) {}
    virtual ~InterfaceElement() {}
    virtual const char * text(int selected) = 0;
    virtual int doKey(int key) { return 1; }
};


class InterfaceElementOption : public InterfaceElement {
public:
    static Keymap keymap;
    Option * opt;
    int inc1, inc2, inc3;

    InterfaceElementOption(const char * t, Option * o,
                           int i1 = 1, int i2 = 10, int i3 = 100);
    virtual const char * text(int selected);
    virtual int doKey(int key);
};


class InterfaceElementCoreOption : public InterfaceElementOption {
public:
    static Keymap coreKeymap;
    CoreOption * coreOpt;

    InterfaceElementCoreOption(const char * t, CoreOption * o,
                               int i1 = 1, int i2 = 10, int i3 = 100);
    virtual int doKey(int key);
};


class Interface {
public:
    const char * name;
    const char * title;
    const char * text;

    InterfaceElement ** elements;
    int nElements;
    int sel;

    Interface * next;

    static Interface * head;
    static Interface * current;
    static int saveToHot;
    static int showStatus;

    char * silenceMsg;
    int silenceLine;

    static char osdText[256];
    static int osdTimer;	// frames remaining to show OSD

    Interface(const char * n, const char * ti, const char * te);
    Interface(const char * n, const char * ti, const char * te,
              InterfaceElement * el[], int nEl);
    virtual ~Interface() {}

    void setElements(InterfaceElement ** el, int nEl);

    static void set(const char * n);

    virtual void display();
    virtual void run();
    virtual void preRun() {}
    virtual void doKey(int key);

    void msg(char * msg);
    static void osd(const char * text);

    static Interface * getCurrent() { return current; }
};


class ErrorMessages {
public:
    char msgs[128][128];
    int on_screen[128];
    int nMsgs;

    ErrorMessages() : nMsgs(0) {}
    void addMessage(const char * text);
    void display();
};

extern ErrorMessages errors;
extern Option * currentOption;
extern CoreOption * currentCoreOption;
extern InterfaceElementOption * currentOptionInterfaceElement;

#endif
