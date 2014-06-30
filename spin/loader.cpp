#include <iostream>
#include "loader.h"
#include "builtin_loader.h"
#include "trisrc_loader.h"

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
    GroupPtr manipulator;
    DefaultController(GroupPtr m_) :
        manipulator(m_)
    {}
    virtual void Update(float time);
    virtual ~DefaultController() {}
};
typedef shared_ptr<DefaultController> DefaultControllerPtr;

void DefaultController::Update(float time)
{
    // pass
}

tuple<bool, NodePtr, ControllerPtr> LoadScene(const string& filename)
{
    int index = filename.find_last_of(".");
    string extension = filename.substr(index + 1);

    if(extension == "lua") {

        // return LuaLoader::Load(filename);
        cerr << "barf" << endl;
        return make_tuple(false, NodePtr(), ControllerPtr());

    } else {

        bool success;
        NodePtr root;

        tie(success, root) = LoadModel(filename);

        if(!success) {
            cerr << "No loader available for extension " << extension << endl;

            return make_tuple(success, root, ControllerPtr());
        }

        GroupPtr manipulator(new Group(mat4f::identity, { root }));
        DefaultControllerPtr controller(new DefaultController(manipulator));

        return make_tuple(success, manipulator, controller);
    }

}

