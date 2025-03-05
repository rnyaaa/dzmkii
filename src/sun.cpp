#include "sun.h"

Sun::Sun(glm::vec3 dir)
    : dir{glm::vec4(dir, 0.0)}
    , color{253, 251, 211, 0}
{ }
