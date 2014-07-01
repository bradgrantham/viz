#ifndef _LOADER_H_
#define _LOADER_H_

#include <string>
#include <tuple>
#include "drawable.h"

struct Controller
{
    virtual void Update(float time) = 0;
    virtual bool Key(int key, int scancode, int action, int mods) = 0;
    virtual void Resize(int w, int h) = 0;
    virtual bool Button(int b, int action, int mods, double x, double y) = 0;
    virtual bool Motion(double dx, double dy) = 0;
    virtual bool Scroll(double dx, double dy) = 0;
    virtual ~Controller() {}
};
typedef std::shared_ptr<Controller> ControllerPtr;

struct EmptyController : public Controller
{
    virtual void Update(float time) {}
    virtual bool Key(int key, int scancode, int action, int mods) { return false; }
    virtual void Resize(int w, int h) { }
    virtual bool Button(int b, int action, int mods, double x, double y) { return false; }
    virtual bool Motion(double dx, double dy) { return false; }
    virtual bool Scroll(double dx, double dy) { return false; }
    virtual ~EmptyController() { }
};
typedef std::shared_ptr<EmptyController> EmptyControllerPtr;

std::tuple<bool, NodePtr> LoadModel(const std::string& filename);
std::tuple<bool, NodePtr, ControllerPtr> LoadScene(const std::string& filename);

#endif /* _LOADER_H */
