#include "loader.h"
#include "builtin_loader.h"
#include "trisrc_loader.h"

using namespace std;

NodePtr LoadScene(const string& filename)
{
    int index = filename.find_last_of(".");
    string extension = filename.substr(index + 1);

    NodePtr root;

    if(extension == "builtin") {

        bool success;
        tie(success, root) = BuiltinLoader::Load(filename);
        if(!success)
            root = NodePtr();

    } else if(extension == "trisrc") {


        bool success;
        tie(success, root) = TriSrcLoader::Load(filename);
        if(!success)
            return NodePtr();
    

    } else {

        root = NodePtr();
    }

    return root;
}

