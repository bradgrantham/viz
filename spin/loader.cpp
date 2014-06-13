#include <iostream>
#include "loader.h"
#include "builtin_loader.h"
#include "trisrc_loader.h"

using namespace std;

tuple<bool, NodePtr, ControllerPtr> LoadScene(const string& filename)
{
    int index = filename.find_last_of(".");
    string extension = filename.substr(index + 1);

    if(extension == "builtin") {

        return BuiltinLoader::Load(filename);

    } else if(extension == "trisrc") {

        return TriSrcLoader::Load(filename);

    } else {

        cerr << "No loader available for extension " << extension << endl;
        return make_tuple(false, NodePtr(), ControllerPtr());
    }

}

