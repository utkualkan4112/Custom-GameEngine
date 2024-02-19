#include "octree.h"
#include "avl.h"
#include "../graphics/models/box.hpp"

// calculate bounds of specified quadrant in bounding region
void Octree::calculateBounds(BoundingRegion &out, Octant octant, BoundingRegion parentRegion) {
    // find min and max points of corresponding octant

    glm::vec3 center = parentRegion.calculateCenter();
    if (octant == Octant::O1) {
        out = BoundingRegion(center, parentRegion.max);
    }
    else if (octant == Octant::O2) {
        out = BoundingRegion(glm::vec3(parentRegion.min.x, center.y, center.z), glm::vec3(center.x, parentRegion.max.y, parentRegion.max.z));
    }
    else if (octant == Octant::O3) {
        out = BoundingRegion(glm::vec3(parentRegion.min.x, parentRegion.min.y, center.z), glm::vec3(center.x, center.y, parentRegion.max.z));
    }
    else if (octant == Octant::O4) {
        out = BoundingRegion(glm::vec3(center.x, parentRegion.min.y, center.z), glm::vec3(parentRegion.max.x, center.y, parentRegion.max.z));
    }
    else if (octant == Octant::O5) {
        out = BoundingRegion(glm::vec3(center.x, center.y, parentRegion.min.z), glm::vec3(parentRegion.max.x, parentRegion.max.y, center.z));
    }
    else if (octant == Octant::O6) {
        out = BoundingRegion(glm::vec3(parentRegion.min.x, center.y, parentRegion.min.z), glm::vec3(center.x, parentRegion.max.y, center.z));
    }
    else if (octant == Octant::O7) {
        out = BoundingRegion(parentRegion.min, center);
    }
    else if (octant == Octant::O8) {
        out = BoundingRegion(glm::vec3(center.x, parentRegion.min.y, parentRegion.min.z), glm::vec3(parentRegion.max.x, center.y, center.z));
    }
}

/*
    constructors
*/

// default
Octree::node::node()
    : region(BoundTypes::AABB) {}

// initialize with bounds (no objects yet)
Octree::node::node(BoundingRegion bound)
    : region(bound) {}

// initialize with bounds and list of objects
Octree::node::node(BoundingRegion bounds, std::vector<BoundingRegion> objectsList)
    : region(bounds) {
    // insert entire list of objects
    objects.insert(objects.end(), objectsList.begin(), objectsList.end());
}

/*
    functionality
*/

// add instance to pending queue
void Octree::node::addToPending(RigidBody* instance, Model* model) {
    // getting all the bounding regions of the model
    for (BoundingRegion br : model->boundingRegions) {
        br.instance = instance;
        br.transform();
        queue.push(br);
    }
}

// build tree (called during initialization)
void Octree::node::build() {
    // variable declerations
    BoundingRegion octants[NO_CHILDREN];
    glm::vec3 dimensions = region.calculateDimensions();
    std::vector<BoundingRegion> octList[NO_CHILDREN]; // array of list of object in each octant

    /*
        termination condition
        - 1 or less objects (ie an empty leaf node)
        - dimensions are too small
    */

    // <= 1 objects
    if (objects.size() <= 1) {
        // set state varaibles
        goto setVars;
    }

    // to small
    for (int i = 0; i < 3; i++) {
        if (dimensions[i] < MIN_BOUNDS) {
            // set state varaibles
            goto setVars;
        }
    }

    // create regions
    for (int i = 0; i < NO_CHILDREN; i++) {
        calculateBounds(octants[i], (Octant)(1 << i), region);
    }

    // determines wich octants to place objects in
    for (int i = 0, len = (int)objects.size(); i < len; i++) {
        BoundingRegion br = objects[i];
        for (int j = 0; j < NO_CHILDREN; j++) {
            if (octants[j].containsRegion(br)) {
                // octant contains region
                octList[j].push_back(br);
                objects.erase(objects.begin() + i);

                // offset because removed object from list
                i--;
                len--;
                break;
            }
        }
    }

    // populate octants
    for (int i = 0; i < NO_CHILDREN; i++) {
        if (octList[i].size() != 0) {
            // if children go into this octant, generate new child
            children[i] = new node(octants[i], octList[i]);
            States::activateIndex(&activeOctants, i); // activate octant
            children[i]->parent = this;
            children[i]->build();
        }
    }
    
setVars:
    // set state varaibles
    treeBuilt = true;
    treeReady = true;

    // set pointer to current cell of each object
    for (int i = 0; i < objects.size(); i++) {
        objects[i].cell = this;
    }
}

// update objects in tree (called during each iteration of main loop)
void Octree::node::update(Box &box) {
    if(treeBuilt && treeReady){
        box.positions.push_back(region.calculateCenter());
        box.sizes.push_back(region.calculateDimensions());

        // countime timer 
        if (objects.size() == 0) {
            if (!activeOctants) {
                // ensure no child leaves
                if (currentLifeSpan == -1) {
                    // initial check
                    currentLifeSpan = maxLifeSpan;
                }
                else if (currentLifeSpan > 0) {
                    // decrement
                    currentLifeSpan--;
                }
            }
        }
        else {
            if (currentLifeSpan != -1) {
                if (maxLifeSpan <= 64) {
                    // extend lifespan because hot spot
                    maxLifeSpan <<= 2;
                }
            }
        }

        // removing moved objects that dont exist anymore
        for (int i = 0, listSize = (int)objects.size(); i < listSize; i++) {
            // remove if on list of dead objects
            if (States::isActive(&objects[i].instance->state, INSTANCE_DEAD)) {
                objects.erase(objects.begin() + i);
                // update counter/size
                i--;
                listSize--;
            }
        }

        // get moved objects that were in this leaf in previous frame
        std::stack<int> movedObjects;
        for (int i = 0, listSize = (int)objects.size(); i < listSize; i++) {
            if (States::isActive(&objects[i].instance->state, INSTANCE_MOVED)) {
                // if moved switch active, transform region and push to list
                objects[i].transform();
                movedObjects.push(i);
            }
            box.positions.push_back(objects[i].calculateCenter());
            box.sizes.push_back(objects[i].calculateDimensions());
        }

        // remove dead branches
        unsigned char flags = activeOctants;
        for (int i = 0;
            flags > 0;
            flags >>= 1,
            i++) {
            if (States::isIndexActive(&flags, 0) && children[i]->currentLifeSpan == 0) {
                // active and runout of time
                if (children[i]->objects.size() > 0) {
                    // branch is dead but has childeren so reset
                    children[i]->currentLifeSpan = -1;
                }
                else {
                    // branches dead and no objects
                    free(children[i]);
                    children[i] = nullptr;
                    States::deactivateIndex(&activeOctants, i);
                }
            }
        }

     
        // update child nodes
        if (children != nullptr) {
            // go through each octant using flags
            for (unsigned char flags = activeOctants, i = 0;
                flags > 0; flags >>= 1, i++) {
                if (States::isIndexActive(&flags, 0)) {
                    // active octant
                    if (children[i] != nullptr) {
                        // child not null
                        children[i]->update(box);
                    }
                }
            }
        }

        // move moved objects into new nodes
        BoundingRegion movedObj;
        while (movedObjects.size() != 0) {
            /*
                for each moved object 
                - traverse up to tree (start with current node) until find a node that completyly encloses the object
                - call insert (push object as far down as posible 
            */

            movedObj = objects[movedObjects.top()]; // set to top object
            node* current = this; // placeholder
            while (!current->region.containsRegion(movedObj)) {
                if (current->parent != nullptr) {
                    // set current to current's parent (recursion)
                    current = current->parent;
                }
                else {
                    break; // if root node, leave
                }
            }

            /*
                onces finished
                - remove from objects list
                - remove from objects stack
                - instert into found region
            */
            objects.erase(objects.begin() + movedObjects.top());
            movedObjects.pop();
            current->queue.push(movedObj);

            // collusion detection
            // itself
            current = movedObj.cell;
            current->checkCollisionsSelf(movedObj);

            // children
            current->checkCollisionsChildren(movedObj);

            // parents
            while (current->parent) {
                current = current->parent;
                current->checkCollisionsSelf(movedObj);
            }
        }
        
    }
    
    processPending();
}

// process pending queue
void Octree::node::processPending() {
    if (!treeBuilt) {
        // add objects to be sorted into branches when build
        while (queue.size() != 0) {
            objects.push_back(queue.front());
            queue.pop();
        }
        build();
    }
    else {
        for (int i = 0, len = (int)queue.size(); i < len; i++) {
            BoundingRegion br = queue.front();
            if (region.containsRegion(br)) {
                // interst objects imidiatly
                insert(br);
            }
            else {
                // return to queue
                br.transform();
                queue.push(br);
            }
            queue.pop();
        }
    }
}

// dynamically insert object into node
bool Octree::node::insert(BoundingRegion obj) {
    /*
        termination conditions
        - no objects (an empty leaf node)
        - dimensions are less than MIN_BOUNDS
    */

    glm::vec3 dimesions = region.calculateDimensions();
    if ((objects.size() == 0 ||
            dimesions.x < MIN_BOUNDS ||
            dimesions.y < MIN_BOUNDS ||
            dimesions.z < MIN_BOUNDS)
            ) {
        obj.cell = this;
        objects.push_back(obj);
        return true;
    }

    // safe guard if object doesnt fit
    if (!region.containsRegion(obj)) {
        return parent == nullptr ? false : parent->insert(obj);
    }

    // create region if not defined
    BoundingRegion octants[NO_CHILDREN];
    for (int i = 0; i < NO_CHILDREN; i++) {
        if (children[i] != nullptr) {
            // child exists, so take its region
            octants[i] = children[i]->region;
        }
        else {
            // get region for this octant
            calculateBounds(octants[i], (Octant)(1 << i), region);
        }
    }
    
    objects.push_back(obj);

    // determine which octants to put objects in
    std::vector<BoundingRegion> octLists[NO_CHILDREN]; // array of list of object in each octant
    for (int i = 0, len = (int)objects.size(); i < len; i++) {
        objects[i].cell = this;
        for (int j = 0; j < NO_CHILDREN; j++) {
            if (octants[j].containsRegion(objects[i])) {
                octLists[j].push_back(objects[i]);
                // remove from object list
                objects.erase(objects.begin() + i);
                i--;
                len--;
                break;
            }
        }
    }
    // populate the octants
    for (int i = 0; i < NO_CHILDREN; i++) {
        if (octLists[i].size() != 0) {
            // object exist in this octant
            if (children[i]) {
                for (BoundingRegion br : octLists[i]) {
                    children[i]->insert(br);
                }
            }
            else {
                // create new node
                children[i] = new node(octants[i], octLists[i]);
                children[i]->parent = this;
                States::activateIndex(&activeOctants, i);
                children[i]->build();
            }
        }
    }

    return true;
}

// check collision with all objects in node
void Octree::node::checkCollisionsSelf(BoundingRegion obj) {
    for (BoundingRegion br : objects) {
        if (br.instance->instanceId == obj.instance->instanceId) {
            // do not test collision with the same instance
            continue;
        }
        // coarse check for bounding region intersection
        if (br.intersectsWith(obj)) {
            // course check passed

            unsigned int noFacesBr = br.collisionMesh ? (unsigned int)br.collisionMesh->faces.size() : 0;
            unsigned int noFacesObj = obj.collisionMesh ? (unsigned int)obj.collisionMesh->faces.size() : 0;

            glm::vec3 norm;

            if (noFacesBr) {

                if (noFacesObj) {
                    // both have collision mesh
                    // check all faces in br againts all faces in obj
                    for (unsigned int i = 0; i < noFacesBr; i++) {
                        for (unsigned int j = 0; j < noFacesObj; j++) {
                            if (br.collisionMesh->faces[i].collidesWithFace(
                                br.instance,
                                obj.collisionMesh->faces[j],
                                obj.instance,
                                norm
                            )) {
                                std::cout << "Case 1: Instance " << br.instance->instanceId
                                    << "(" << br.instance->modelId << ") collides with instance "
                                    << obj.instance->instanceId << "(" << obj.instance->modelId << ")" << std::endl;
                                obj.instance->handleCollision(br.instance, norm);
                                break;
                            }
                        }
                    }
                }
                else {
                    // br has a collision mesh obj does not
                    // check all faces br againts onj's sphere
                    for (unsigned int i = 0; i < noFacesBr; i++) {
                        if (br.collisionMesh->faces[i].collidesWithSphere(
                            br.instance,
                            obj,
                            norm)) {
                            std::cout << "Case 2: Instance " << br.instance->instanceId
                                << "(" << br.instance->modelId << ") collides with instance "
                                << obj.instance->instanceId << "(" << obj.instance->modelId << ")" << std::endl;
                            obj.instance->handleCollision(br.instance, norm);
                            break;
                        }
                    }
                }
            }
            else {
                if (noFacesObj) {
                    // obj has collision mesh br does not
                    // check all faces in obj againts br's sphere
                    for (unsigned int i = 0; i < noFacesObj; i++) {
                        if (obj.collisionMesh->faces[i].collidesWithSphere(
                            obj.instance,
                            br,
                            norm)) {
                            std::cout << "Case 3: Instance " << br.instance->instanceId
                                << "(" << br.instance->modelId << ") collides with instance "
                                << obj.instance->instanceId << "(" << obj.instance->modelId << ")" << std::endl;

                            obj.instance->handleCollision(br.instance, norm);
                            break;
                        }
                    }
                }
                else {
                    // neither haver collision mesh
                    // coerse grain test passed (test collision between speheres)
                    std::cout << "Case 4: Instance " << br.instance->instanceId
                        << "(" << br.instance->modelId << ") collides with instance "
                        << obj.instance->instanceId << "(" << obj.instance->modelId << ")" << std::endl;
                    norm = obj.center - br.center;
                    obj.instance->handleCollision(br.instance, norm);
                }
            }
        }
    }
}

// check collisions with all objects in child nodes
void Octree::node::checkCollisionsChildren(BoundingRegion obj)
{
    if (children) {
        for (int flags = activeOctants, i = 0;
            flags > 0;
            flags >>= 1, i++) {
            if(States::isIndexActive(&flags, 0) && children[i]) {
                children[i]->checkCollisionsSelf(obj);
                children[i]->checkCollisionsChildren(obj);
            }
        }
    }
}

// check collisions with a ray
BoundingRegion* Octree::node::checkCollisionsRay(Ray r, float& tmin) {
    float tmin_tmp = std::numeric_limits<float>::max();
    float tmax_tmp = std::numeric_limits<float>::lowest();
    float t_tmp = std::numeric_limits<float>::max();

    // check current region
    if (r.intersectsBoundingRegion(region, tmin_tmp, tmax_tmp)) {
        // know ray collides with the current region
        if (tmin_tmp >= tmin) {
            // found nearer collision
            return nullptr;
        }

        BoundingRegion* ret = nullptr, * ret_tmp = nullptr;
        
        // check objects in the node
        for (BoundingRegion& br : this->objects) {
            tmin_tmp = std::numeric_limits<float>::max();
            tmax_tmp = std::numeric_limits<float>::lowest();

            // coarse check - check againts BR
            if (r.intersectsBoundingRegion(br, tmin_tmp, tmax_tmp)) {
                std::cout << "Passed coarse check, ";
                if (tmin_tmp > tmin) {
                    continue;
                }
                else if (br.collisionMesh) {
                    // fine grain check with collision mesh
                    t_tmp = std::numeric_limits<float>::max();
                    if (r.intersectsMesh(br.collisionMesh, br.instance, t_tmp)) {
                        if (t_tmp < tmin) {
                            // found closer collision
                            tmin = t_tmp;
                            ret = &br;
                        }
                    }
                }
                else {
                    // relly on coarse check
                    if (tmin_tmp < tmin) {
                        tmin = tmin_tmp;
                        ret = &br;
                    }
                }
            }
            else {
                std::cout << "Failed to coarse check, ";
            }
        }
        // check childeren
        if (children) {
            for (unsigned char flags = activeOctants, i = 0;
                flags;
                flags >>= 1, i++) {
                ret_tmp = children[i]->checkCollisionsRay(r, tmin);
                if (ret_tmp) {
                    ret = ret_tmp;
                }
            }
        }
        return ret;
    }
    return nullptr;
}

// destroy object (free memory)
void Octree::node::destroy() {
    // clearing out children
    if (children != nullptr) {
        for (int flags = activeOctants, i = 0;
            flags > 0;
            flags >>= 1, i++) {
            if (States::isIndexActive(&flags, 0)) {
                // active
                if (children[i] != nullptr) {
                    children[i]->destroy();
                    children[i] = nullptr;
                }
            }
        }
    }

    // clear this node
    objects.clear();
    while (queue.size() != 0) {
        queue.pop();
    }

}
