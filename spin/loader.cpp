#include <iostream>
#include "loader.h"
#include "builtin_loader.h"
#include "trisrc_loader.h"
#include "manipulator.h"

using namespace std;

tuple<bool, NodePtr> LoadModel(const string& filename)
{
    int index = filename.find_last_of(".");
    string extension = filename.substr(index + 1);

    if(extension == "builtin") {

        return BuiltinLoader::Load(filename);

    } else if(extension == "trisrc") {

        return TriSrcLoader::Load(filename);

    } else {

        cerr << "No loader available for extension " << extension << endl;
        return make_tuple(false, NodePtr());
    }

}

struct DefaultController : public Controller
{
    const float gFOV = 45.0;
    manipulator manip;
    GroupPtr root;
    int width, height;
    int buttonPressed;
    DefaultController(GroupPtr r_) :
        manip(r_->bounds, gFOV / 180.0 * 3.14159),
        root(r_),
        width(512), // XXX hm
        height(512), // XXX hm
        buttonPressed(-1)
    {
        root->transform = manip.m_matrix;
    }
    virtual void Update(float time);
    virtual bool Key(int key, int scancode, int action, int mods);
    virtual void Resize(int w, int h);
    virtual bool Button(int b, int action, int mods, double x, double y);
    virtual bool Motion(double dx, double dy);
    virtual bool Scroll(double dx, double dy);
    virtual ~DefaultController() {}
};
typedef shared_ptr<DefaultController> DefaultControllerPtr;

void DefaultController::Update(float time)
{
    // pass
}

bool DefaultController::Scroll(double dx, double dy)
{
    manip.move(dx / width, dy / height);
    root->transform = manip.m_matrix;
    return false;
}

bool DefaultController::Motion(double dx, double dy)
{
    if(buttonPressed == 1) {
        manip.move(dx / width, dy / height);
        root->transform = manip.m_matrix;
    }
    return false;
}


// XXX action and mods are GLFW for now, but make independent soon
bool DefaultController::Button(int b, int action, int mods, double x, double y)
{
    if(b == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        buttonPressed = 1;
    } else {
        buttonPressed = -1;
    }
    return false;
}

void DefaultController::Resize(int w, int h)
{
    width = w;
    height = h;
}

// XXX action and mods are GLFW for now, but make independent soon
bool DefaultController::Key(int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS) {
        switch(key) {
            case 'Q': case '\033':
                return true;
                break;
                
            case 'R':
                manip.m_mode = manipulator::ROTATE;
                root->transform = manip.m_matrix;
                break;

            case 'O':
                manip.m_mode = manipulator::ROLL;
                root->transform = manip.m_matrix;
                break;

            case 'X':
                manip.m_mode = manipulator::SCROLL;
                root->transform = manip.m_matrix;
                break;

            case 'Z':
                manip.m_mode = manipulator::DOLLY;
                root->transform = manip.m_matrix;
                break;
        }
    }
    return false;
}


tuple<bool, NodePtr, ControllerPtr> LoadScene(const string& filename)
{
    int index = filename.find_last_of(".");
    string extension = filename.substr(index + 1);

    if(extension == "lua") {

        // return LuaLoader::Load(filename);
        cerr << "barf" << endl;
        return make_tuple(false, NodePtr(), ControllerPtr(new EmptyController()));

    } else {

        bool success;
        NodePtr root;

        tie(success, root) = LoadModel(filename);

        if(!success) {
            cerr << "No loader available for extension " << extension << endl;

            return make_tuple(success, root, ControllerPtr(new EmptyController()));
        }

        GroupPtr manipulator(new Group(mat4f::identity, { root }));
        DefaultControllerPtr controller(new DefaultController(manipulator));

        return make_tuple(success, manipulator, controller);
    }

}

