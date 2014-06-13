#ifndef _LOADER_H_
#define _LOADER_H_

#include <string>
#include <tuple>
#include "drawable.h"

struct Controller
{
    virtual void Update(float time) = 0;
    virtual ~Controller() {}
};
typedef std::shared_ptr<Controller> ControllerPtr;

std::tuple<bool, NodePtr, ControllerPtr> LoadScene(const std::string& filename);

#endif /* _LOADER_H */
